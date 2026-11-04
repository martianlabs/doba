//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/LICENSE-2.0
//
// Copyright 2026 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#include "coverage_union.h"
#include "../helpers/tool_arguments.h"
#include "../helpers/tool_process.h"
#include "../helpers/tool_sha256.h"

using namespace martianlabs::doba::tests::tools;

// +---------------------------------------------------------------------------+
// | [>] logged_run                                               ( function ) |
// +---------------------------------------------------------------------------+
static int logged_run(const std::vector<std::string>& command,
  const fs::path& output, const std::string& name,
  const process::environment& environment = {}, bool check = true,
  clock::duration timeout = 1800s) {
  json arguments(json::array{});
  for (const auto& item : command) arguments.items().push_back(item);
  write_json(output / (name + "-command.json"), arguments);
  process child(command, environment,
    environment.contains("DOBA_COVERAGE_IDENTIFY") ?
      std::vector<std::string>{} :
      std::vector<std::string>{"DOBA_COVERAGE_IDENTIFY"});
  int code;
  try { code = child.wait(timeout); }
  catch (...) {
    child.kill();
    child.wait(5s);
    write(output / (name + ".log"), child.output() + child.errors() +
      "\nSubprocess deadline exceeded\n");
    throw;
  }
  write(output / (name + ".log"), child.output() + child.errors());
  require(!check || code == 0, name + " failed: see " + output.string());
  return code;
}

static void llvm(const std::string& tool,
  const std::vector<std::string>& arguments, const fs::path& output,
  const std::string& name, const fs::path& destination = {}) {
  std::string response;
  for (size_t index = 1; index < arguments.size(); ++index) {
    auto item = arguments[index];
    std::replace(item.begin(), item.end(), '\\', '/');
    response += json::quote(item) + '\n';
  }
  auto path = output / (name + ".rsp");
  write(path, response);
  std::vector<std::string> command{
    tool, arguments.front(), '@' + path.string()};
  if (destination.empty()) {
    logged_run(command, output, name);
  } else {
    auto result = run(command, 300s);
    write(destination, result.output);
    write(output / (name + ".log"), result.errors);
    require(result.code == 0, name + " failed: " + result.errors);
  }
}

static std::vector<fs::path> profiles_in(const fs::path& directory) {
  std::vector<fs::path> result;
  if (fs::exists(directory)) {
    for (const auto& entry : fs::directory_iterator(directory)) {
      if (entry.is_regular_file() && entry.path().extension() == ".profraw") {
        result.push_back(entry.path());
      }
    }
  }
  std::sort(result.begin(), result.end());
  return result;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
  return guarded([&] {
    arguments options(argc, argv);
    auto build = fs::absolute(options.required("--build"));
    auto output = fs::absolute(options.required("--output"));
    auto compiler = options.get("--compiler", "clang++");
    auto cov = options.get("--llvm-cov", "llvm-cov");
    auto profdata = options.get("--llvm-profdata", "llvm-profdata");
    auto openssl = options.get("--openssl-root");
    auto baseline = options.get("--baseline");
    int parallel = options.integer("--parallel", 2, 1, 1024);
    options.finish();
    require(!fs::exists(output), "Fresh output directory required");
    fs::create_directories(output);
    auto root = fs::canonical(DOBA_TOOL_SOURCE_ROOT);
#ifdef _WIN32
    const std::string platform = "windows";
#else
    const std::string platform = "linux";
#endif
    json status = json::object{{"status", "failed"}, {"platform", platform},
      {"release_only", true}, {"root", root.string()},
      {"build", build.string()}};
    try {
      write(build / ".cmake/api/v1/query/codemodel-v2", "");
      auto flags = "-fprofile-instr-generate -fcoverage-mapping "
        "-fprofile-update=atomic -include \"" +
        (root / "tests/qualification/profile_identity.h").generic_string() +
        "\"";
      std::vector<std::string> command{"cmake", "-S", root.string(), "-B",
        build.string(), "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_CXX_COMPILER=" + compiler, "-DCMAKE_CXX_FLAGS=" + flags,
        "-DCMAKE_EXE_LINKER_FLAGS=-fprofile-instr-generate",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DCMAKE_PROJECT_INCLUDE=" +
        (root / "tests/qualification/instrument.cmake").string(),
        "-DDOBA_ENABLE_STRICT_WARNINGS=ON", "-DDOBA_ENABLE_TLS=ON",
        "-DDOBA_BUILD_TESTS=ON", "-DDOBA_BUILD_EXAMPLES=ON"};
      if (!openssl.empty()) command.push_back("-DOPENSSL_ROOT_DIR=" + openssl);
      logged_run(command, output, "configure");
      logged_run({"cmake", "--build", build.string(), "--parallel",
        std::to_string(parallel)}, output, "build");
      for (const auto& [tool, name] :
        std::vector<std::pair<std::string, std::string>>{
          {compiler, "compiler"}, {cov, "llvm-cov"},
          {profdata, "llvm-profdata"}}) {
        logged_run({tool, "--version"}, output, name);
      }
      write_json(output / "source-sha256.json",
        source_hashes(root, {"include", "tests", "cmake"}));
      fs::create_directories(output / "profiles");
      process::environment environment{{"LLVM_PROFILE_FILE",
        (output / "profiles/%p-%m.profraw").string()}};
      int ctest = logged_run({"ctest", "--test-dir", build.string(),
        "--output-on-failure", "--no-tests=error"}, output, "ctest",
        environment, false);
      status["ctest_exit"] = ctest;
      auto profiles = profiles_in(output / "profiles");
      require(!profiles.empty(), "No raw profiles produced");
      status["profiles"] = profiles.size();
      auto reply = build / ".cmake/api/v1/reply";
      std::vector<fs::path> indices;
      for (const auto& entry : fs::directory_iterator(reply)) {
        if (entry.path().filename().string().starts_with("index-") &&
          entry.path().extension() == ".json") indices.push_back(entry.path());
      }
      std::sort(indices.begin(), indices.end());
      require(!indices.empty(), "No CMake codemodel index");
      auto index = json::load(indices.back());
      auto model = json::load(reply /
        index["reply"]["codemodel-v2"]["jsonFile"].string());
      json objects(json::array{});
      for (const auto& target :
        model["configurations"][size_t{0}]["targets"].items()) {
        auto data = json::load(reply / target["jsonFile"].string());
        if (data["type"].string() != "EXECUTABLE") continue;
        for (const auto& artifact : data["artifacts"].items()) {
          auto path = build / artifact["path"].string();
          if (platform == "windows" && path.extension() != ".exe") continue;
          objects.items().push_back(json::object{{"binary", path.string()},
            {"target", target["name"]}});
        }
      }
      require(!objects.items().empty(), "No instrumented executable inventory");
      write_json(output / "objects.json", objects);
      coverage_files files;
      json identities(json::array{});
      std::set<fs::path> matched;
      std::set<std::string> namespaces;
      size_t number = 0;
      for (const auto& object : objects.items()) {
        auto binary = object["binary"].string();
        auto target = object["target"].string();
        auto directory = output / ("object-" + std::to_string(number++));
        fs::create_directories(directory / "identity");
        process::environment probe{{"DOBA_COVERAGE_IDENTIFY", "1"},
          {"LLVM_PROFILE_FILE",
            (directory / "identity/%p-%m.profraw").string()}};
        logged_run({binary}, directory, "identify", probe, true, 10s);
        auto identity = profiles_in(directory / "identity");
        require(identity.size() == 1,
          "Expected one executable profile identity");
        std::string prefix = target + '-';
        auto filename = identity.front().filename().string();
        require(filename.starts_with(prefix),
          "Executable identity is not namespaced by target");
        auto separator = filename.find('-', prefix.size());
        require(separator != std::string::npos, "Malformed profile identity");
        auto signature = filename.substr(separator + 1);
        require(namespaces.insert(target).second,
          "Executable profile namespace collision");
        std::vector<fs::path> own;
        for (const auto& profile : profiles) {
          auto name = profile.filename().string();
          if (!name.starts_with(prefix)) continue;
          auto split = name.find('-', prefix.size());
          if (split != std::string::npos &&
              name.substr(split + 1) == signature) {
            own.push_back(profile);
            matched.insert(profile);
          }
        }
        identities.items().push_back(json::object{{"binary", binary},
          {"signature", signature}, {"target", target},
          {"observed_profiles", own.size()}});
        auto merged = directory / "coverage.profdata";
        std::vector<std::string> arguments{"merge", "-sparse"};
        for (const auto& path : own.empty() ? identity : own) {
          arguments.push_back(path.string());
        }
        arguments.push_back("-o"); arguments.push_back(merged.string());
        llvm(profdata, arguments, directory, "merge");
        std::string profile = "-instr-profile=" + merged.string();
        llvm(cov, {"export", binary, profile, "-skip-functions"}, directory,
          "export", directory / "coverage.json");
        llvm(cov, {"export", binary, profile, "-format=lcov"}, directory,
          "lcov", directory / "coverage.lcov");
        require(trim(read(directory / "export.log")).empty() &&
          trim(read(directory / "lcov.log")).empty(),
          "Unqualified LLVM coverage diagnostic: " + binary);
        merge(files, json::load(directory / "coverage.json"),
          read(directory / "coverage.lcov"), root);
      }
      require(matched.size() == profiles.size(),
        "Raw profiles have no matching executable identity");
      write_json(output / "profile-identities.json", identities);
      auto summary = summarize(files, root);
      write_json(output / "summary.json", summary);
      auto report = markdown_report(summary);
      write(output / "summary.md", report);
      if (auto step = environment_value("GITHUB_STEP_SUMMARY")) {
        std::ofstream stream(*step, std::ios::app | std::ios::binary);
        stream << report;
        require(static_cast<bool>(stream), "Cannot append CI summary");
      }
      json coverage(json::object{});
      for (const auto& [name, file] : files) {
        json line_counts(json::object{}), branches(json::array{});
        for (const auto& [line, count] : file.lines) {
          line_counts[std::to_string(line)] = count;
        }
        for (const auto& [position, outcomes] : file.branches) {
          json coordinates(json::array{});
          for (auto value : position) {
            coordinates.items().push_back(static_cast<double>(value));
          }
          branches.items().push_back(json::object{{"position", coordinates},
            {"outcomes", json::array{outcomes[0], outcomes[1]}}});
        }
        coverage[name] = json::object{{"lines", line_counts},
          {"branches", branches}};
      }
      write_json(output / "coverage.json", coverage);
      html_report(files, summary, root, output / "html");
      if (!baseline.empty()) {
        auto floor = json::load(baseline);
        require(floor["platform"].string() == platform,
          "Coverage baseline platform does not match");
        enforce(summary, floor);
      }
      require(ctest == 0, "CTest failed; coverage is diagnostic only");
      status["status"] = "passed";
      std::cout << json(json::object{{"status", "passed"},
        {"modules", summary["modules"]}}).dump() << '\n';
    } catch (...) {
      write_json(output / "status.json", status);
      throw;
    }
    write_json(output / "status.json", status);
  });
}
