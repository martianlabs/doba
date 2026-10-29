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
// Copyright 2025 martianLabs
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

#ifndef martianlabs_doba_tests_unit_temporary_directory_linux_h
#define martianlabs_doba_tests_unit_temporary_directory_linux_h

#include <cstdlib>
#include <string>
#include <optional>

namespace martianlabs::doba::tests::unit {
class temporary_directory {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit temporary_directory(const std::string& path) {
    const char* value = std::getenv("TMPDIR");
    had_previous_ = value != nullptr;
    if (value) previous_ = value;
    changed_ = setenv("TMPDIR", path.c_str(), 1);
  }
  temporary_directory(const temporary_directory&) = delete;
  temporary_directory& operator=(const temporary_directory&) = delete;
  ~temporary_directory() { if (changed_ == 0) (void)restore(); }
  // +=========================================================================+
  // | [>] current                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  static std::optional<std::string> current() {
    const char* value = std::getenv("TMPDIR");
    if (!value) return std::nullopt;
    return std::string(value);
  }
  // +=========================================================================+
  // | [>] changed                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  int changed() const { return changed_; }
  // +=========================================================================+
  // | [>] restore                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  int restore() {
    if (restored_) return 0;
    const int result = had_previous_ ? setenv("TMPDIR", previous_.c_str(), 1)
                                     : unsetenv("TMPDIR");
    if (result == 0) restored_ = true;
    return result;
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Whether the original environment has already been restored.
  bool restored_ = false;
  std::string previous_;
  bool had_previous_ = false;
  int changed_ = -1;
};
}  // namespace martianlabs::doba::tests::unit
#endif
