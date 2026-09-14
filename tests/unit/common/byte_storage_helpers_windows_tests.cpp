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

#ifdef _WIN32

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>

#include "platform.h"
#include "common/byte_storage_helpers_windows.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::byte_storage_file;

class spill_directory {
 public:
  spill_directory() {
    namespace fs = std::filesystem;
    static std::atomic<std::size_t> sequence{0};
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    path_ = fs::temp_directory_path() /
            ("doba_byte_storage_file_" + std::to_string(stamp) + "_" +
             std::to_string(sequence.fetch_add(1)));
    fs::create_directory(path_);
  }
  ~spill_directory() {
    std::error_code error;
    std::filesystem::remove_all(path_, error);
  }

  const std::filesystem::path& path() const { return path_; }

 private:
  std::filesystem::path path_;
};

}  // namespace

// +===========================================================================+
// | [>] closed files reject nonempty reads and writes           ( test-case ) |
// +===========================================================================+
DOBA_TEST("closed files reject nonempty reads and writes") {
  byte_storage_file value;
  char output = '!';
  std::size_t read = 99;
  DOBA_EXPECT(!value.write("x", 1));
  DOBA_EXPECT(!value.read(0, &output, 1, read));
  DOBA_EXPECT_EQUAL(read, 0);
  DOBA_EXPECT_EQUAL(output, '!');
  value.close();
  value.close();
  DOBA_EXPECT(!value.write("x", 1));
}
// +===========================================================================+
// | [>] file writes preserve every byte and positional reads    ( test-case ) |
// +===========================================================================+
DOBA_TEST("file writes preserve every byte and positional reads") {
  spill_directory directory;
  byte_storage_file value;
  std::filesystem::path path;
  DOBA_EXPECT(value.open(directory.path(), path));
  DOBA_EXPECT_EQUAL(path.parent_path(), directory.path());
  DOBA_EXPECT(std::filesystem::exists(path));
  std::string input(256, '\0');
  for (std::size_t i = 0; i < input.size(); i++) {
    input[i] = static_cast<char>(i);
  }
  DOBA_EXPECT(value.write(input.data(), 63));
  DOBA_EXPECT(value.write(input.data() + 63, input.size() - 63));
  DOBA_EXPECT(value.flush());
  for (std::size_t position : {0, 1, 63, 64, 255, 256, 257}) {
    std::array<char, 258> output;
    output.fill('!');
    std::size_t read = 99;
    DOBA_EXPECT(value.read(position, output.data(), 256, read));
    const std::size_t expected = position < 256 ? 256 - position : 0;
    DOBA_EXPECT_EQUAL(read, expected);
    DOBA_EXPECT_EQUAL(std::string_view(output.data(), read),
                      std::string_view(input).substr(
                          position < 256 ? position : 256));
    for (std::size_t i = read; i < output.size(); i++) {
      DOBA_EXPECT_EQUAL(output[i], '!');
    }
  }
  value.close();
  DOBA_EXPECT(std::filesystem::exists(path));
}
// +===========================================================================+
// | [>] empty file operations preserve the supplied buffer      ( test-case ) |
// +===========================================================================+
DOBA_TEST("empty file operations preserve the supplied buffer") {
  spill_directory directory;
  byte_storage_file value;
  std::filesystem::path path;
  DOBA_EXPECT(value.open(directory.path(), path));
  DOBA_EXPECT(value.write(nullptr, 0));
  DOBA_EXPECT(value.flush());
  char output = '!';
  std::size_t read = 99;
  DOBA_EXPECT(value.read(0, &output, 1, read));
  DOBA_EXPECT_EQUAL(read, 0);
  DOBA_EXPECT_EQUAL(output, '!');
  read = 99;
  DOBA_EXPECT(value.read(0, nullptr, 0, read));
  DOBA_EXPECT_EQUAL(read, 0);
  DOBA_EXPECT_EQUAL(std::filesystem::file_size(path), 0);
}
// +===========================================================================+
// | [>] file open failure permits a later valid open            ( test-case ) |
// +===========================================================================+
DOBA_TEST("file open failure permits a later valid open") {
  spill_directory directory;
  byte_storage_file value;
  std::filesystem::path path;
  DOBA_EXPECT(!value.open(directory.path() / "missing", path));
  DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
  DOBA_EXPECT(!value.write("x", 1));
  DOBA_EXPECT(value.open(directory.path(), path));
  DOBA_EXPECT(value.write("ok", 2));
  value.close();
  DOBA_EXPECT_EQUAL(std::filesystem::file_size(path), 2);
}
// +===========================================================================+
// | [>] independent files keep distinct paths and contents      ( test-case ) |
// +===========================================================================+
DOBA_TEST("independent files keep distinct paths and contents") {
  spill_directory directory;
  byte_storage_file first;
  byte_storage_file second;
  std::filesystem::path first_path;
  std::filesystem::path second_path;
  DOBA_EXPECT(first.open(directory.path(), first_path));
  DOBA_EXPECT(first.write("first", 5));
  DOBA_EXPECT(second.open(directory.path(), second_path));
  DOBA_EXPECT(second.write("other", 5));
  DOBA_EXPECT(first_path != second_path);
  std::array<char, 5> output{};
  std::size_t read = 0;
  DOBA_EXPECT(first.read(0, output.data(), output.size(), read));
  DOBA_EXPECT_EQUAL(read, 5);
  DOBA_EXPECT_EQUAL(std::string_view(output.data(), read), "first");
  DOBA_EXPECT(second.read(0, output.data(), output.size(), read));
  DOBA_EXPECT_EQUAL(read, 5);
  DOBA_EXPECT_EQUAL(std::string_view(output.data(), read), "other");
}
// +===========================================================================+
// | [>] file moves transfer ownership and preserve contents     ( test-case ) |
// +===========================================================================+
DOBA_TEST("file moves transfer ownership and preserve contents") {
  spill_directory directory;
  std::optional<byte_storage_file> moved;
  std::filesystem::path source_path;
  {
    byte_storage_file source;
    DOBA_EXPECT(source.open(directory.path(), source_path));
    DOBA_EXPECT(source.write("abcdef", 6));
    moved.emplace(std::move(source));
    DOBA_EXPECT(!source.write("x", 1));
  }
  byte_storage_file destination;
  std::filesystem::path previous_path;
  DOBA_EXPECT(destination.open(directory.path(), previous_path));
  DOBA_EXPECT(destination.write("old", 3));
  destination = std::move(*moved);
  DOBA_EXPECT(!moved->write("x", 1));
  moved.reset();
  byte_storage_file& same = destination;
  destination = std::move(same);
  std::array<char, 4> output{};
  std::size_t read = 0;
  DOBA_EXPECT(destination.read(2, output.data(), output.size(), read));
  DOBA_EXPECT_EQUAL(read, 4);
  DOBA_EXPECT_EQUAL(std::string_view(output.data(), read), "cdef");
  destination.close();
  DOBA_EXPECT(!destination.write("x", 1));
  DOBA_EXPECT(std::filesystem::exists(source_path));
  DOBA_EXPECT_EQUAL(std::filesystem::file_size(previous_path), 3);
}

#endif
