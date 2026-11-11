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

#ifndef martianlabs_doba_tests_qualification_campaign_h
#define martianlabs_doba_tests_qualification_campaign_h

#include <set>

#include "../helpers/tool_json.h"
#include "../helpers/tool_process.h"

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] campaign                                                   ( header ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
namespace martianlabs::doba::tests::tools {

inline std::vector<std::string> required_groups(bool windows, bool tls) {
  std::vector<std::string> groups{
      "doba_security_model_tests", "doba_engine_concurrency_tests",
      "doba_stream_integrity_tests", "doba_test_diagnostics_tests",
      "doba_qualification_tests"};
  std::vector<std::string> native =
      windows
          ? std::vector<std::string>{"iocp_completion", "windows_lifecycle",
                                     "windows_processing", "network_fault"}
          : std::vector<std::string>{"linux_transport", "linux_accept_error",
                                     "linux_processing"};
  for (const auto& name : native) groups.push_back("doba_" + name + "_tests");
  if (tls) {
    for (auto name : {"core", "processing", "processing_wire"}) {
      groups.push_back(std::string("doba_tls_") + name + "_tests");
    }
    groups.push_back("doba_tls_tests");
  }
  return groups;
}

inline void validate_inventory(const json& inventory,
                               const std::vector<std::string>& required) {
  std::map<std::string, json> tests;
  for (const auto& item : inventory["tests"].items()) {
    require(tests.emplace(item["name"].string(), item).second,
            "Duplicate registered test name");
  }
  for (const auto& name : required) {
    require(tests.contains(name) && tests.at(name).contains("command") &&
                !tests.at(name)["command"].items().empty(),
            "Missing executable campaign group: " + name);
  }
}

inline std::vector<std::string> case_names(const std::string& listing) {
  std::vector<std::string> result;
  std::regex pattern("^.+:[0-9]+ - (.+)$");
  for (const auto& line : lines(listing)) {
    std::smatch match;
    if (std::regex_match(line, match, pattern)) result.push_back(match[1]);
  }
  return result;
}

inline std::string execute(const std::vector<std::string>& command,
                           const fs::path& log, clock::duration timeout,
                           const std::string& selection = "") {
  process child(command);
  int code;
  try {
    code = child.wait(timeout);
  } catch (...) {
    child.kill();
    child.wait(5s);
    write(log, child.output() + child.errors() +
                   "\nCampaign subprocess deadline exceeded\n");
    throw;
  }
  auto text = child.output() + child.errors();
  write(log, text);
  clean_diagnostics(text);
  require(code == 0 && text.find("assertion failed:") == std::string::npos,
          "Campaign failure: " + log.string());
  require(selection.empty() || std::regex_search(text, std::regex(selection)),
          "Campaign selection not proven: " + log.string());
  return text;
}

}  // namespace martianlabs::doba::tests::tools

#endif
