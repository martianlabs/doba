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
#include <stdexcept>
#include <string>

#include "common/filesystem.h"
#include "common/reader.h"
#include "file_directory.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::tests::file_directory;
namespace fs = std::filesystem;
using martianlabs::doba::common::filesystem_file;
}  // namespace

// +===========================================================================+
// | [>] file reader ownership and cursors                       ( test-case ) |
// +---------------------------------------------------------------------------+
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
// +---------------------------------------------------------------------------+
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
