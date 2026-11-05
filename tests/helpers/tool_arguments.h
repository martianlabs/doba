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

#ifndef martianlabs_doba_tests_helpers_tool_arguments_h
#define martianlabs_doba_tests_helpers_tool_arguments_h

#include "tool_support.h"

#include <set>

namespace martianlabs::doba::tests::tools {

// +---------------------------------------------------------------------------+
// | [>] arguments                                                   ( class ) |
// +---------------------------------------------------------------------------+
class arguments {
  std::map<std::string, std::string> values_;
  std::set<std::string> switches_;

 public:
  // +-------------------------------------------------------------------------+
  // | [>] PUBLIC INTERFACE                                         ( public ) |
  // +-------------------------------------------------------------------------+
  arguments(int count, char** values,
            const std::set<std::string>& switches = {}) {
    for (int index = 1; index < count; ++index) {
      std::string name(values[index]);
      require(name.starts_with("--"), "Expected an option: " + name);
      require(!values_.contains(name) && !switches_.contains(name),
        "Duplicate option: " + name);
      if (switches.contains(name)) switches_.insert(name);
      else {
        require(index + 1 < count, "Missing option value: " + name);
        values_[name] = values[++index];
      }
    }
  }
  // +-------------------------------------------------------------------------+
  // | [>] get                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  std::string get(const std::string& name,
                  const std::string& fallback = "") {
    auto found = values_.find(name);
    if (found == values_.end()) return fallback;
    auto result = found->second;
    values_.erase(found);
    return result;
  }
  // +-------------------------------------------------------------------------+
  // | [>] required                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  std::string required(const std::string& name) {
    auto value = get(name);
    require(!value.empty(), "Required option: " + name);
    return value;
  }
  // +-------------------------------------------------------------------------+
  // | [>] integer                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  int integer(const std::string& name, int fallback, int low, int high) {
    auto text = get(name, std::to_string(fallback));
    require(std::regex_match(text, std::regex("[0-9]+")),
      "Invalid integer option: " + name);
    auto number = std::stoll(text);
    require(number >= low && number <= high, "Option out of range: " + name);
    return static_cast<int>(number);
  }
  // +-------------------------------------------------------------------------+
  // | [>] enabled                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  bool enabled(const std::string& name) {
    return switches_.erase(name) != 0;
  }
  // +-------------------------------------------------------------------------+
  // | [>] finish                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  void finish() const {
    require(values_.empty() && switches_.empty(), "Unknown command option");
  }
};

} // namespace martianlabs::doba::tests::tools

#endif
