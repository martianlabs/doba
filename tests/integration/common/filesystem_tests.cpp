//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
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

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "common/filesystem.h"
#include "common/reader.h"
#include "test_helper.h"

namespace {
namespace fs = std::filesystem;
using martianlabs::doba::common::filesystem_file;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] file_directory                                              ( class ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class file_directory {
 public:
  // +=========================================================================+
  // | [>] METHODs                                                  ( public ) |
  // +=========================================================================+
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
  const fs::path& path() const { return path_; }
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
  // +=========================================================================+
  fs::path path_;
};
}  // namespace

// +===========================================================================+
// | [>] file reader ownership and cursors                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem readers retain separate cursors across ownership moves") {
  file_directory directory;
  fs::create_directory(directory.path() / "sub");
  const std::string input(131071, 'x');
  directory.write("sub/data", input);
  std::error_code error;
  filesystem_file first;
  filesystem_file second;
  DOBA_EXPECT(first.open(directory.path(), "sub/data", error));
  DOBA_EXPECT(second.open(directory.path(), "sub/data", error));
  martianlabs::doba::common::reader reader(std::move(first));
  std::array<std::byte, 13> buffer{};
  DOBA_EXPECT_EQUAL(reader.read(buffer), buffer.size());
  auto moved = std::move(reader);
  std::string output;
  moved.read_all(output);
  DOBA_EXPECT_EQUAL(output, input.substr(buffer.size()));
  DOBA_EXPECT_EQUAL(second.read(buffer), buffer.size());
  DOBA_EXPECT(!moved.failed());
  DOBA_EXPECT_EQUAL(moved.size().value(), input.size());
}

// +===========================================================================+
// | [>] file reader direct access                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem readers observe changes before the first read") {
  file_directory directory;
  directory.write("file", "old");
  std::error_code error;
  filesystem_file file;
  DOBA_EXPECT(file.open(directory.path(), "file", error));
  martianlabs::doba::common::reader reader(std::move(file));
  directory.write("file", "new");
  std::string output;
  reader.read_all(output);
  DOBA_EXPECT_EQUAL(output, "new");
}
