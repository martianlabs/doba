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

#include "campaign.h"
#include "../helpers/tool_arguments.h"
#include "../helpers/tool_sha256.h"

#include <algorithm>

using namespace martianlabs::doba::tests::tools;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
  return guarded([&] {
    arguments options(argc, argv, {"--tls"});
    auto build = fs::canonical(options.required("--build"));
    auto output = fs::absolute(options.required("--output"));
    int rounds = options.integer("--rounds", 2, 1, 32);
    int seed = options.integer("--seed", 104729, 0, INT32_MAX);
    bool tls = options.enabled("--tls");
    options.finish();
    require(!fs::exists(output), "Fresh output directory required");
    fs::create_directories(output);
    auto cache = lines(read(build / "CMakeCache.txt"));
    require(std::find(cache.begin(), cache.end(),
      "CMAKE_BUILD_TYPE:STRING=Release") != cache.end(),
      "Campaigns require Release builds");
    auto root = fs::path(DOBA_TOOL_SOURCE_ROOT);
    write_json(output / "source-sha256.json",
      source_hashes(root, {"include", "tests"}));
    json state = json::object{{"status", "failed"},
#ifdef _WIN32
      {"platform", "windows"},
#else
      {"platform", "linux"},
#endif
      {"seed", seed}, {"rounds", rounds}, {"cases", json::array{}}};
    for (auto key : {"ASAN_OPTIONS", "UBSAN_OPTIONS", "TSAN_OPTIONS"}) {
      auto value = environment_value(key);
      state["sanitizer_environment"][key] = value ? json(*value) : json();
    }
    auto revision = environment_value("GITHUB_SHA");
    state["ci_revision"] = revision ? json(*revision) : json();
    try {
      auto inventory_result = run({"ctest", "--test-dir", build.string(),
        "--show-only=json-v1"}, 30s);
      require(inventory_result.code == 0, "Cannot inventory CTest");
      auto inventory = json::parse(inventory_result.output);
#ifdef _WIN32
      constexpr bool windows = true;
      const std::string suffix = ".exe";
#else
      constexpr bool windows = false;
      const std::string suffix;
#endif
      auto groups = required_groups(windows, tls);
      validate_inventory(inventory, groups);
      write_json(output / "ctest-inventory.json", inventory);
      std::vector<json> actions;
      for (const auto& name : groups) {
        actions.push_back(json::object{{"id", name}, {"command", json::array{
          "ctest", "--test-dir", build.string(), "-R", '^' + name + '$',
          "--output-on-failure", "--no-tests=error"}}, {"selection",
          "1/1 Test\\s+#[0-9]+: " + name + "\\b[^\\r\\n]*\\bPassed\\b"}});
      }
      std::vector<std::pair<std::string, size_t>> files{
        {"common/cache_tests.cpp", 14}, {"common/byte_storage_tests.cpp", 19},
        {"common/reader_tests.cpp", 9},
        {"protocol/http/v11/operating_limits_tests.cpp", 1}};
      if (!windows) files.emplace_back(
        "transport/tcp_context_linux_tests.cpp", 17);
      auto unit = (build / ("tests/unit/doba_unit_tests" + suffix)).string();
      size_t index = 0;
      for (const auto& [file, minimum] : files) {
        auto listing = execute({unit, "--file", file, "--list"},
          output / ("unit-" + std::to_string(index++) + "-inventory.log"), 30s);
        auto names = case_names(listing);
        std::set<std::string> distinct(names.begin(), names.end());
        require(names.size() >= minimum && distinct.size() == names.size(),
          "Missing or duplicated fault cases: " + file);
        for (const auto& name : names) {
          actions.push_back(json::object{{"id", file + ':' + name},
            {"command", json::array{unit, "--file", file, "--name", name}},
            {"selection", "running 1 unit tests"}});
        }
      }
      state["actions_per_round"] = actions.size();
      for (int round = 0; round < rounds; ++round) {
        auto order = actions;
        std::mt19937 random(static_cast<unsigned>(seed + round));
        std::shuffle(order.begin(), order.end(), random);
        index = 0;
        for (auto action : order) {
          action["round"] = round;
          action["status"] = "failed";
          state["cases"].items().push_back(action);
          std::vector<std::string> command;
          for (const auto& item : action["command"].items()) {
            command.push_back(item.string());
          }
          execute(command, output / ("round-" + std::to_string(round) +
            "-case-" + std::to_string(index++) + ".log"), 180s,
            action["selection"].string());
          state["cases"].items().back()["status"] = "passed";
        }
      }
      state["status"] = "passed";
    } catch (...) {
      write_json(output / "results.json", state);
      throw;
    }
    write_json(output / "results.json", state);
    std::cout << "Campaign passed: " << state["cases"].items().size() <<
      " actions\n";
  });
}
