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
// | [>] filesystem reads binary blocks with a fixed size        ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem reads binary blocks with a fixed size") {
  file_directory directory;
  std::string input(256, '\0');
  for (std::size_t i = 0; i < input.size(); i++) input[i] = char(i);
  directory.write("bytes", input);
  filesystem_file file;
  std::error_code error;
  DOBA_EXPECT(file.open(directory.path(), "bytes", error));
  DOBA_EXPECT(!error);
  DOBA_EXPECT_EQUAL(file.size(), input.size());
  std::array<std::byte, 31> buffer{};
  DOBA_EXPECT_EQUAL(file.read({}), 0);
  std::string output;
  while (!file.eof()) {
    const auto count = file.read(buffer);
    DOBA_EXPECT(count > 0);
    if (!count) break;
    output.append(reinterpret_cast<char*>(buffer.data()), count);
  }
  DOBA_EXPECT_EQUAL(output, input);
  DOBA_EXPECT_EQUAL(file.read(buffer), 0);
  DOBA_EXPECT(!file.failed());
  DOBA_EXPECT(fs::exists(directory.path() / "bytes"));
}

// +===========================================================================+
// | [>] filesystem paths and errors                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem distinguishes empty missing directories and escapes") {
  file_directory directory;
  directory.write("empty", "");
  fs::create_directory(directory.path() / "sub");
  filesystem_file file;
  std::error_code error;
  DOBA_EXPECT(file.open(directory.path(), "empty", error));
  DOBA_EXPECT(file.eof());
  DOBA_EXPECT_EQUAL(file.size(), 0);
  DOBA_EXPECT(!file.open(directory.path(), "missing", error));
  DOBA_EXPECT(error == std::errc::no_such_file_or_directory);
  DOBA_EXPECT(!file.open(directory.path(), "sub", error));
  DOBA_EXPECT(error == std::errc::is_a_directory);
  for (std::string_view path : {"../x", "/x", "sub/../x", "sub/./x",
                                "sub//x", "C:x", "sub\\x"}) {
    DOBA_EXPECT(!file.open(directory.path(), path, error));
    DOBA_EXPECT(error == std::errc::permission_denied);
  }
  DOBA_EXPECT(!file.open(directory.path(), std::string_view("a\0b", 3),
                         error));
  DOBA_EXPECT(error == std::errc::permission_denied);
  DOBA_EXPECT(!file.open(directory.path() / "empty", "x", error));
  DOBA_EXPECT(error == std::errc::not_a_directory);
}

// +===========================================================================+
// | [>] filesystem ownership moves                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem moves preserve position and release prior resources") {
  static_assert(!std::is_copy_constructible_v<filesystem_file>);
  static_assert(!std::is_copy_assignable_v<filesystem_file>);
  static_assert(std::is_nothrow_move_constructible_v<filesystem_file>);
  file_directory directory;
  directory.write("one", "abc");
  directory.write("two", "xyz");
  filesystem_file first;
  std::error_code error;
  DOBA_EXPECT(first.open(directory.path(), "one", error));
  std::array<std::byte, 1> byte{};
  DOBA_EXPECT_EQUAL(first.read(byte), 1);
  filesystem_file moved(std::move(first));
  DOBA_EXPECT(first.eof());
  filesystem_file assigned;
  DOBA_EXPECT(assigned.open(directory.path(), "two", error));
  assigned = std::move(moved);
  DOBA_EXPECT(moved.eof());
  DOBA_EXPECT_EQUAL(assigned.read(byte), 1);
  DOBA_EXPECT_EQUAL(byte[0], std::byte{'b'});
  assigned.close();
  assigned.close();
  DOBA_EXPECT(fs::exists(directory.path() / "one"));
  DOBA_EXPECT_EQUAL(assigned.read(byte), 0);
  DOBA_EXPECT(assigned.failed());
}

// +===========================================================================+
// | [>] filesystem size changes                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem bounds growth and reports premature truncation") {
  file_directory directory;
  directory.write("file", "abc");
  filesystem_file file;
  std::error_code error;
  DOBA_EXPECT(file.open(directory.path(), "file", error));
  directory.write("file", "abcdef");
  std::array<std::byte, 16> output{};
  DOBA_EXPECT_EQUAL(file.read(output), 3);
  DOBA_EXPECT(file.eof());
  DOBA_EXPECT(!file.failed());
  DOBA_EXPECT(file.open(directory.path(), "file", error));
  fs::resize_file(directory.path() / "file", 1);
  DOBA_EXPECT_EQUAL(file.read(output), 1);
  DOBA_EXPECT_EQUAL(file.read(output), 0);
  DOBA_EXPECT(file.failed());
  DOBA_EXPECT(!file.eof());
}

// +===========================================================================+
// | [>] filesystem opens UTF8 names and clears prior errors     ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem opens UTF8 names and clears prior errors") {
  file_directory directory;
  directory.write("caf\xc3\xa9.txt", "utf8");
  filesystem_file file;
  std::error_code error;
  DOBA_EXPECT(!file.open(directory.path(), "missing", error));
  DOBA_EXPECT(file.open(directory.path(), "caf\xc3\xa9.txt", error));
  DOBA_EXPECT(!error);
  std::array<std::byte, 4> bytes{};
  DOBA_EXPECT_EQUAL(file.read(bytes), 4);
  DOBA_EXPECT_EQUAL(std::string_view(
      reinterpret_cast<char*>(bytes.data()), bytes.size()), "utf8");
  DOBA_EXPECT(!file.open(fs::path("relative"), "file", error));
  DOBA_EXPECT(error == std::errc::invalid_argument);
}

// +===========================================================================+
// | [>] filesystem resolves roots and clears errors             ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem resolves roots and clears errors") {
  file_directory directory;
  directory.write("file", "data");
  std::error_code error = std::make_error_code(std::errc::io_error);
  const auto root = martianlabs::doba::common::filesystem_root(".", error);
  DOBA_EXPECT(!error);
  DOBA_EXPECT(root.is_absolute());
  DOBA_EXPECT_EQUAL(root, fs::canonical(fs::current_path()));
  DOBA_EXPECT(martianlabs::doba::common::filesystem_root(
      directory.path() / "missing", error).empty());
  DOBA_EXPECT(error == std::errc::no_such_file_or_directory);
  DOBA_EXPECT(martianlabs::doba::common::filesystem_root(
      directory.path() / "file", error).empty());
  DOBA_EXPECT(error == std::errc::not_a_directory);
  DOBA_EXPECT_EQUAL(martianlabs::doba::common::filesystem_root(
      directory.path() / ".", error), fs::canonical(directory.path()));
  DOBA_EXPECT(!error);
}

// +===========================================================================+
// | [>] filesystem validates complete bounded paths             ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem validates complete bounded paths") {
  file_directory directory;
  directory.write("file", "data");
  directory.write("..file", "dots");
  filesystem_file file;
  std::error_code error;
  DOBA_EXPECT(!file.open(directory.path(), "", error));
  DOBA_EXPECT(error == std::errc::is_a_directory);
  for (std::string_view path : {".", "..", "/", "//", "file/", "file//",
                                "./file", "file/."}) {
    DOBA_EXPECT(!file.open(directory.path(), path, error));
    DOBA_EXPECT(error == std::errc::permission_denied);
  }
  std::string path = "fi?le";
  for (unsigned int value = 0; value <= 0x7f; value++) {
    if (value >= 0x20 && value != 0x7f) continue;
    path[2] = static_cast<char>(value);
    DOBA_EXPECT(!file.open(directory.path(), path, error));
    DOBA_EXPECT(error == std::errc::permission_denied);
  }
  DOBA_EXPECT(file.open(directory.path(), "..file", error));
  DOBA_EXPECT(file.open(directory.path(),
                         std::string_view("file/../outside", 4), error));
  std::array<std::byte, 4> bytes{};
  DOBA_EXPECT_EQUAL(file.read(bytes), bytes.size());
  DOBA_EXPECT_EQUAL(std::string_view(
      reinterpret_cast<char*>(bytes.data()), bytes.size()), "data");
}

// +===========================================================================+
// | [>] filesystem reopens after open and read failures         ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem reopens after open and read failures") {
  file_directory directory;
  directory.write("file", "abc");
  filesystem_file file;
  std::error_code error;
  DOBA_EXPECT(file.open(directory.path(), "file", error));
  std::array<std::byte, 1> byte{};
  DOBA_EXPECT_EQUAL(file.read(byte), 1);
  DOBA_EXPECT(!file.open(directory.path(), "missing", error));
  DOBA_EXPECT_EQUAL(file.size(), 0);
  DOBA_EXPECT(file.eof());
  DOBA_EXPECT(!file.failed());
  DOBA_EXPECT_EQUAL(file.read({}), 0);
  DOBA_EXPECT(!file.failed());
  DOBA_EXPECT_EQUAL(file.read(byte), 0);
  DOBA_EXPECT(file.failed());
  DOBA_EXPECT(file.open(directory.path(), "file", error));
  DOBA_EXPECT(!error);
  DOBA_EXPECT(!file.failed());
  DOBA_EXPECT_EQUAL(file.read(byte), 1);
  DOBA_EXPECT_EQUAL(byte[0], std::byte{'a'});
  fs::resize_file(directory.path() / "file", 1);
  DOBA_EXPECT_EQUAL(file.read(byte), 0);
  DOBA_EXPECT(file.failed());
  directory.write("file", "xyz");
  DOBA_EXPECT_EQUAL(file.read(byte), 0);
  DOBA_EXPECT_EQUAL(byte[0], std::byte{'a'});
  DOBA_EXPECT(file.open(directory.path(), "file", error));
  DOBA_EXPECT(!error);
  DOBA_EXPECT(!file.failed());
  DOBA_EXPECT_EQUAL(file.read(byte), 1);
  DOBA_EXPECT_EQUAL(byte[0], std::byte{'x'});
}

// +===========================================================================+
// | [>] filesystem moves preserve failed and closed states      ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem moves preserve failed and closed states") {
  file_directory directory;
  directory.write("file", "abc");
  filesystem_file file;
  std::error_code error;
  DOBA_EXPECT(file.open(directory.path(), "file", error));
  auto& same = file;
  file = std::move(same);
  std::array<std::byte, 1> byte{};
  DOBA_EXPECT_EQUAL(file.read(byte), 1);
  DOBA_EXPECT_EQUAL(byte[0], std::byte{'a'});
  fs::resize_file(directory.path() / "file", 1);
  DOBA_EXPECT_EQUAL(file.read(byte), 0);
  DOBA_EXPECT(file.failed());
  filesystem_file moved(std::move(file));
  DOBA_EXPECT(moved.failed());
  DOBA_EXPECT(!moved.eof());
  DOBA_EXPECT_EQUAL(moved.size(), 3);
  DOBA_EXPECT(!file.failed());
  DOBA_EXPECT(file.eof());
  DOBA_EXPECT_EQUAL(file.size(), 0);
  file = std::move(moved);
  DOBA_EXPECT(file.failed());
  DOBA_EXPECT(!file.eof());
  DOBA_EXPECT_EQUAL(file.size(), 3);
  DOBA_EXPECT(!moved.failed());
  DOBA_EXPECT(moved.eof());
  DOBA_EXPECT_EQUAL(moved.size(), 0);
  file = std::move(moved);
  DOBA_EXPECT(!file.failed());
  DOBA_EXPECT(file.eof());
  DOBA_EXPECT_EQUAL(file.size(), 0);
  DOBA_EXPECT_EQUAL(file.read(byte), 0);
  DOBA_EXPECT(file.failed());
}

// +===========================================================================+
// | [>] filesystem reads preserve buffer boundaries             ( test-case ) |
// +===========================================================================+
DOBA_TEST("filesystem reads preserve buffer boundaries") {
  file_directory directory;
  directory.write("file", "abc");
  filesystem_file file;
  std::error_code error;
  DOBA_EXPECT(file.open(directory.path(), "file", error));
  std::array<std::byte, 4> bytes;
  bytes.fill(std::byte{'!'});
  const auto output = std::span<std::byte>(bytes).subspan(1, 2);
  DOBA_EXPECT_EQUAL(file.read(output), 2);
  DOBA_EXPECT_EQUAL(bytes[0], std::byte{'!'});
  DOBA_EXPECT_EQUAL(bytes[1], std::byte{'a'});
  DOBA_EXPECT_EQUAL(bytes[2], std::byte{'b'});
  DOBA_EXPECT_EQUAL(bytes[3], std::byte{'!'});
  bytes.fill(std::byte{'!'});
  DOBA_EXPECT_EQUAL(file.read(output), 1);
  DOBA_EXPECT_EQUAL(bytes[0], std::byte{'!'});
  DOBA_EXPECT_EQUAL(bytes[1], std::byte{'c'});
  DOBA_EXPECT_EQUAL(bytes[2], std::byte{'!'});
  DOBA_EXPECT_EQUAL(bytes[3], std::byte{'!'});
  DOBA_EXPECT(file.eof());
  DOBA_EXPECT(!file.failed());
  bytes.fill(std::byte{'!'});
  DOBA_EXPECT_EQUAL(file.read(output), 0);
  for (const auto byte : bytes) DOBA_EXPECT_EQUAL(byte, std::byte{'!'});
}
