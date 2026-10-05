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

#ifndef martianlabs_doba_tests_unit_special_file_paths_linux_h
#define martianlabs_doba_tests_unit_special_file_paths_linux_h

#include <filesystem>
#include <stdexcept>
#include <string_view>
#include <vector>

#include <sys/stat.h>

namespace martianlabs::doba::tests::unit {
inline std::vector<std::string_view> special_file_paths(
    const std::filesystem::path& directory,
    const std::filesystem::path& outside) {
  std::filesystem::create_symlink(outside / "secret", directory / "link");
  std::filesystem::create_directory_symlink(outside, directory / "linked");
  std::filesystem::create_symlink("missing", directory / "broken");
  if (::mkfifo((directory / "pipe").c_str(), 0600) != 0) {
    throw std::runtime_error("Unable to create FIFO");
  }
  return {"link", "linked/secret", "broken", "pipe"};
}
}  // namespace martianlabs::doba::tests::unit

#endif
