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
#include "campaign.h"

using namespace martianlabs::doba::tests::tools;

template<class Function>
static void rejected(Function function, const std::string& diagnostic) {
  try { function(); }
  catch (const std::exception& error) {
    require(std::string(error.what()).find(diagnostic) != std::string::npos,
      "Wrong failure diagnostic: " + std::string(error.what()));
    return;
  }
  throw std::runtime_error("Negative control accepted: " + diagnostic);
}

static void coverage_controls(const fs::path& root) {
  auto header = root / "include/common/measured.h";
  write(header, "<script>untrusted source</script>\n");
  write(root / "include/common/uninstantiated.h", "");
  json item = json::object{{"filename", header.string()},
    {"summary", json::object{{"lines", json::object{{"percent", 100}}}}},
    {"branches", json::array{
      json(json::array{2, 1, 2, 4, 0, 1, 0, 0, 0})}}};
  json document = json::object{{"data", json::array{json::object{
    {"files", json::array{item, json::object{{"filename",
      (root / "tests/test.cpp").string()}}}}}}}};
  auto lcov = "SF:" + header.string() +
    "\nDA:1,2\nDA:2,0\nDA:3,1\nend_of_record\n";
  coverage_files files;
  merge(files, document, lcov, root);
  auto summary = summarize(files, root);
  auto& stats = summary["modules"]["common"];
  require(stats["lines"]["percent"].number() == 100. * 2 / 3,
    "Aggregate lines trusted supplied percentages");
  require(stats["branches"]["percent"].number() == 50,
    "Branch outcomes were not measured separately");
  require(summary["unmapped_headers"] ==
    json(json::array{"include/common/uninstantiated.h"}),
    "Uninstantiated header disappeared from inventory");
  require(!summary["uncovered_branch_outcomes"][size_t{0}]["true"].boolean(),
    "Uncovered branch frontier was not reported");
  json baseline = json::object{{"modules", json::object{{"common",
    json::object{{"lines", 66}, {"branches", 50}}}}},
    {"measured_headers", json::array{"include/common/measured.h"}},
    {"unmapped_headers", json::array{"include/common/uninstantiated.h"}}};
  enforce(summary, baseline);
  for (const auto& floor : {json(-1), json(101), json(true), json("50"),
    json(std::numeric_limits<double>::quiet_NaN()), json()}) {
    auto invalid = baseline;
    invalid["modules"]["common"]["branches"] = floor;
    rejected([&] { enforce(summary, invalid); }, floor.null() ?
      "Missing coverage floor" : "Invalid coverage floor");
  }
  auto bad = summary;
  bad["modules"]["common"]["branches"]["percent"] = 49;
  rejected([&] { enforce(bad, baseline); }, "common/branches");
  bad = summary; bad["measured_headers"] = json::array{};
  rejected([&] { enforce(bad, baseline); }, "Lost measured header");
  bad = summary;
  bad["unmapped_headers"].items().push_back("include/common/new_untested.h");
  rejected([&] { enforce(bad, baseline); }, "New unmapped header");
  bad = summary; bad["modules"] = json::object{};
  rejected([&] { enforce(bad, baseline); }, "Missing measured module");
  rejected([&] { summarize({}, root); }, "No production coverage");
  auto second = document;
  auto& branch = second["data"][size_t{0}]["files"][size_t{0}]
    ["branches"][size_t{0}];
  branch[size_t{4}] = 1; branch[size_t{5}] = 0;
  auto more = "SF:" + header.string() + "\nDA:2,1\nend_of_record\n";
  coverage_files forward, reverse;
  merge(forward, document, lcov, root); merge(forward, second, more, root);
  merge(reverse, second, more, root); merge(reverse, document, lcov, root);
  require(forward == reverse, "Coverage union depends on executable order");
  auto total = summarize(forward, root);
  for (auto metric : {"lines", "branches"}) {
    require(total["modules"]["common"][metric]["percent"].number() == 100,
      "Coverage was averaged, double counted or not combined");
  }
  html_report(files, summary, root, root / "html");
  auto html = read(root / "html/file-0.html");
  require(html.find("<script>") == std::string::npos &&
    html.find("&lt;script&gt;") != std::string::npos,
    "HTML report did not escape source text");
}

static void path_controls(const fs::path& root) {
  auto source = root / "source tree";
  auto header = source / "include/common/probe.h";
  write(header, "");
  auto outside = root / "neighbor/include/common/outside.h";
  write(outside, "");
  std::vector<fs::path> aliases{source, source / "include/.."};
#ifdef _WIN32
  std::wstring buffer(32768, L'\0');
  DWORD size = GetShortPathNameW(source.c_str(), buffer.data(),
    static_cast<DWORD>(buffer.size()));
  require(size && size < buffer.size(), "Cannot obtain Windows short alias");
  buffer.resize(size);
  aliases.emplace_back(buffer);
#endif
  size_t index = 0;
  for (const auto& alias : aliases) {
    for (auto filename : {header.string(), header.generic_string()}) {
      json sources(json::array{});
      std::string lcov;
      for (const auto& name : {filename, outside.string(),
        (source / "tests/probe.h").string()}) {
        sources.items().push_back(json::object{{"filename", name}});
        lcov += "SF:" + name + "\nDA:1,1\nend_of_record\n";
      }
      json document = json::object{{"data", json::array{
        json::object{{"files", sources}}}}};
      coverage_files files;
      merge(files, document, lcov, alias);
      require(files.size() == 1 && files.contains("include/common/probe.h"),
        "Canonical containment lost or admitted files");
      auto summary = summarize(files, alias);
      require(summary["modules"]["common"]["lines"] == json(json::object{
        {"count", 1}, {"covered", 1}, {"percent", 100}}),
        "JSON/LCOV paths lost their production mapping");
    }
    auto fixture = alias / ("full fixture " + std::to_string(index++));
    fs::create_directory(fixture);
    coverage_controls(fixture);
  }
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
  if (argc == 3 && std::string(argv[1]) == "--child") {
    std::string mode(argv[2]);
    if (mode == "timeout") std::this_thread::sleep_for(10s);
    else if (mode == "sanitizer") {
      std::cout << "WARNING: ThreadSanitizer: data race\n";
    } else if (mode == "assertion") {
      std::cout << "assertion failed: corruption\n";
    }
    else if (mode == "skip") std::cout << "1/1 Test #1: probe ... ***Skipped\n";
    else if (mode != "none") std::cout << "running 1 unit tests\n";
    return mode == "failure" ? 1 : 0;
  }
  return guarded([&] {
    temporary_directory directory;
    auto root = directory.path();
    coverage_controls(root);
    path_controls(root);
    require(case_names("a.cpp:12 - first\r\nb.cpp:7 - second\r\n") ==
      std::vector<std::string>{"first", "second"},
      "Windows line endings changed exact test names");
    auto required = required_groups(false, true);
    json inventory = json::object{{"tests", json::array{}}};
    for (const auto& name : required) {
      inventory["tests"].items().push_back(json::object{{"name", name},
        {"command", json::array{"test"}}});
    }
    validate_inventory(inventory, required);
    auto bad = inventory; bad["tests"].items().pop_back();
    rejected([&] { validate_inventory(bad, required); }, "Missing");
    bad = inventory;
    bad["tests"].items().push_back(bad["tests"][size_t{0}]);
    rejected([&] { validate_inventory(bad, required); }, "Duplicate");
    for (const auto& [mode, diagnostic] :
      std::vector<std::pair<std::string, std::string>>{
        {"none", "selection not proven"}, {"failure", "failure"},
        {"sanitizer", "Sanitizer"}, {"assertion", "failure"}}) {
      rejected([&] { execute({argv[0], "--child", mode},
        root / (mode + ".log"), 10s, "running 1 unit tests"); }, diagnostic);
    }
    execute({argv[0], "--child", "positive"}, root / "positive.log", 10s,
      "running 1 unit tests");
    rejected([&] { execute({argv[0], "--child", "skip"}, root / "skip.log",
      10s, "1/1 Test\\s+#[0-9]+: probe\\b[^\\r\\n]*\\bPassed\\b"); },
      "selection not proven");
    rejected([&] { execute({argv[0], "--child", "timeout"},
      root / "timeout.log", 100ms); }, "deadline exceeded");
    require(read(root / "timeout.log").find("deadline exceeded") !=
      std::string::npos, "Timeout lost its diagnostic");
    std::cout << "Coverage and campaign negative controls: PASS\n";
  });
}
