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

#ifndef martianlabs_doba_tests_unit_temporary_directory_windows_h
#define martianlabs_doba_tests_unit_temporary_directory_windows_h

#include <cstdlib>
#include <string>

namespace martianlabs::doba::tests::unit {
class temporary_directory {
 public:
  explicit temporary_directory(const std::string& path) {
    char* value = nullptr;
    std::size_t size = 0;
    saved_ = _dupenv_s(&value, &size, "TMP");
    if (value) previous_ = value;
    std::free(value);
    if (saved_ == 0) changed_ = _putenv_s("TMP", path.c_str());
  }
  int changed() const { return changed_; }
  int restore() const { return _putenv_s("TMP", previous_.c_str()); }

 private:
  std::string previous_;
  int saved_ = -1;
  int changed_ = -1;
};
}  // namespace martianlabs::doba::tests::unit
#endif
