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

#ifndef martianlabs_doba_tests_integration_file_directory_h
#define martianlabs_doba_tests_integration_file_directory_h

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace martianlabs::doba::tests {
namespace fs = std::filesystem;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] file_directory                                              ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class file_directory {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  file_directory() {
    static std::atomic<unsigned int> counter{0};
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    do {
      path_ = fs::temp_directory_path() /
              ("doba_files_" + std::to_string(stamp) + "_" +
               std::to_string(counter.fetch_add(1)));
    } while (!fs::create_directory(path_));
  }
  ~file_directory() {
    std::error_code error;
    fs::remove_all(path_, error);
  }
  // +=========================================================================+
  // | [>] path                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  const fs::path& path() const { return path_; }
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void write(std::string_view name, std::string_view contents) {
    const fs::path relative(std::u8string(name.begin(), name.end()));
    std::ofstream output(path_ / relative, std::ios::binary);
    output.write(contents.data(),
                 static_cast<std::streamsize>(contents.size()));
    if (!output) throw std::runtime_error("Unable to write fixture");
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  fs::path path_;
};
}  // namespace martianlabs::doba::tests

#endif
