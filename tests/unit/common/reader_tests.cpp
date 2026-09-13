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

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <utility>

#include "common/reader.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::byte_storage;
using martianlabs::doba::common::byte_storage_options;
using martianlabs::doba::common::reader;

class spill_directory {
 public:
  spill_directory() {
    namespace fs = std::filesystem;
    static std::atomic<std::size_t> sequence{0};
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    path_ = fs::temp_directory_path() /
            ("doba_reader_" + std::to_string(stamp) + "_" +
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

std::filesystem::path only_spill_file(const std::filesystem::path& directory) {
  std::filesystem::path result;
  for (const auto& entry : std::filesystem::directory_iterator(directory)) {
    if (entry.path().filename() != "existing.tmp") result = entry.path();
  }
  return result;
}
}  // namespace

// +===========================================================================+
// | [>] empty readers report their size contract                ( test-case ) |
// +===========================================================================+
DOBA_TEST("empty readers report their size contract") {
  reader value;
  DOBA_EXPECT(value.ok());
  DOBA_EXPECT(value.eof());
  DOBA_EXPECT(!value.failed());
  DOBA_EXPECT(!value.size().has_value());
  auto borrowed = reader::borrowed({});
  DOBA_EXPECT_EQUAL(borrowed.size().value(), 0);
  DOBA_EXPECT(borrowed.exhausted());
  std::byte byte{0x5a};
  DOBA_EXPECT(!borrowed.fetch(byte));
  DOBA_EXPECT_EQUAL(byte, std::byte{0x5a});
  DOBA_EXPECT_EQUAL(value.read({}), 0);
  DOBA_EXPECT_EQUAL(borrowed.read({}), 0);
}
// +===========================================================================+
// | [>] read and fetch share the borrowed cursor                ( test-case ) |
// +===========================================================================+
DOBA_TEST("read and fetch share the borrowed cursor") {
  const std::string input("a\0b\xff", 4);
  auto value = reader::borrowed(std::as_bytes(std::span(input)));
  std::array<std::byte, 6> output;
  output.fill(std::byte{0x5a});
  DOBA_EXPECT_EQUAL(value.read(std::span(output).first(1)), 1);
  DOBA_EXPECT_EQUAL(output[0], std::byte{'a'});
  std::byte byte{0x5a};
  DOBA_EXPECT(value.fetch(byte));
  DOBA_EXPECT_EQUAL(byte, std::byte{0});
  DOBA_EXPECT_EQUAL(value.read(output), 2);
  DOBA_EXPECT_EQUAL(output[0], std::byte{'b'});
  DOBA_EXPECT_EQUAL(output[1], std::byte{0xff});
  DOBA_EXPECT_EQUAL(output[2], std::byte{0x5a});
  DOBA_EXPECT(value.eof());
  DOBA_EXPECT(value.ok());
  DOBA_EXPECT(!value.failed());
  DOBA_EXPECT_EQUAL(value.size().value(), 4);
  DOBA_EXPECT_EQUAL(value.read(output), 0);
  DOBA_EXPECT(!value.fetch(byte));
}
// +===========================================================================+
// | [>] read all appends across its buffer boundary             ( test-case ) |
// +===========================================================================+
DOBA_TEST("read all appends across its buffer boundary") {
  for (const std::size_t size : {0, 1, 8191, 8192, 8193}) {
    const std::string input(size, '\x80');
    for (const bool borrowed : {false, true}) {
      byte_storage storage;
      DOBA_EXPECT(storage.write(input.data(), input.size()));
      storage.finish(input.size());
      auto value = borrowed
                       ? reader::borrowed(std::as_bytes(std::span(input)))
                       : reader(std::move(storage));
      std::string output = "prefix";
      DOBA_EXPECT_EQUAL(value.read_all(output), size);
      DOBA_EXPECT_EQUAL(output, "prefix" + input);
      DOBA_EXPECT_EQUAL(value.read_all(output), 0);
      DOBA_EXPECT(value.exhausted());
      DOBA_EXPECT(!value.failed());
    }
  }
}
// +===========================================================================+
// | [>] moves retain the unread suffix across ownership modes   ( test-case ) |
// +===========================================================================+
DOBA_TEST("moves retain the unread suffix across ownership modes") {
  const std::string input = "abcdef";
  for (const bool borrowed_source : {false, true}) {
    for (const bool borrowed_target : {false, true}) {
      byte_storage source_storage;
      DOBA_EXPECT(source_storage.write(input.data(), input.size()));
      source_storage.finish(input.size());
      auto source = borrowed_source
                        ? reader::borrowed(std::as_bytes(std::span(input)))
                        : reader(std::move(source_storage));
      std::byte byte{};
      DOBA_EXPECT(source.fetch(byte));
      reader moved(std::move(source));
      DOBA_EXPECT(moved.fetch(byte));
      DOBA_EXPECT_EQUAL(byte, std::byte{'b'});
      byte_storage target_storage;
      DOBA_EXPECT(target_storage.write("old", 3));
      target_storage.finish(3);
      auto target = borrowed_target
                        ? reader::borrowed(std::as_bytes(std::span(input)))
                        : reader(std::move(target_storage));
      target = std::move(moved);
      reader& same = target;
      target = std::move(same);
      std::string output;
      DOBA_EXPECT_EQUAL(target.read_all(output), 4);
      DOBA_EXPECT_EQUAL(output, "cdef");
      DOBA_EXPECT_EQUAL(target.size().value(), input.size());
    }
  }
}
// +===========================================================================+
// | [>] borrowed reads allow overlapping storage                ( test-case ) |
// +===========================================================================+
DOBA_TEST("borrowed reads allow overlapping storage") {
  std::array<char, 8> input{'a', 'b', 'c', 'd', 'e', 'f', '!', '!'};
  auto value = reader::borrowed(std::as_bytes(std::span(input).first(6)));
  DOBA_EXPECT_EQUAL(
      value.read(std::as_writable_bytes(std::span(input).subspan(1, 6))), 6);
  DOBA_EXPECT_EQUAL(std::string_view(input.data(), input.size()), "aabcdef!");
  DOBA_EXPECT(value.eof());
}
// +===========================================================================+
// | [>] truncated spill files fail the reader                  ( test-case )  |
// +===========================================================================+
DOBA_TEST("truncated spill files fail the reader") {
  spill_directory directory;
  {
    byte_storage storage(
        byte_storage_options{.spill_threshold = 1,
                             .spill_dir = directory.path().string()});
    DOBA_EXPECT(storage.write("abcdef", 6));
    storage.finish(6);
    DOBA_EXPECT_EQUAL(
        std::distance(std::filesystem::directory_iterator(directory.path()),
                      std::filesystem::directory_iterator()), 1);
    const auto spill_file = only_spill_file(directory.path());
    DOBA_EXPECT(!spill_file.empty());
    std::filesystem::resize_file(spill_file, 2);
    reader source(std::move(storage));
    std::array<std::byte, 6> output{};
    DOBA_EXPECT_EQUAL(source.read(output), 2);
    DOBA_EXPECT(source.failed());
    DOBA_EXPECT(!source.eof());
    DOBA_EXPECT_EQUAL(source.read(output), 0);
    std::byte byte{};
    DOBA_EXPECT(!source.fetch(byte));
    DOBA_EXPECT(source.failed());
  }
  DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
}
// +===========================================================================+
// | [>] reader owns moved storage                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("reader owns storage after the moved source is destroyed") {
  spill_directory directory;
  for (std::size_t threshold : {0, 1}) {
    {
      std::optional<reader> destination;
      {
        byte_storage storage(
            byte_storage_options{.spill_threshold = threshold,
                                 .spill_dir = directory.path().string()});
        DOBA_EXPECT(storage.write("abcdef", 6));
        storage.finish(6);
        destination.emplace(std::move(storage));
      }
      DOBA_EXPECT_EQUAL(std::filesystem::is_empty(directory.path()),
                        threshold == 0);
      std::string output;
      DOBA_EXPECT_EQUAL(destination->read_all(output), 6);
      DOBA_EXPECT_EQUAL(output, "abcdef");
      DOBA_EXPECT(destination->eof());
      DOBA_EXPECT(!destination->failed());
    }
    DOBA_EXPECT(std::filesystem::is_empty(directory.path()));
  }
}

// +===========================================================================+
// | [>] borrowed reader move semantics                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("moving a borrowed reader preserves the view and cursor") {
  std::string backing = "abcdef";
  auto source = reader::borrowed(std::as_bytes(std::span(backing)));
  std::array<std::byte, 2> prefix{};
  DOBA_EXPECT_EQUAL(source.read(prefix), 2);
  DOBA_EXPECT_EQUAL(
      std::string_view(reinterpret_cast<const char*>(prefix.data()), 2), "ab");
  reader destination(std::move(source));
  backing[2] = 'C';
  std::string output;
  DOBA_EXPECT_EQUAL(destination.read_all(output), 4);
  DOBA_EXPECT_EQUAL(output, "Cdef");
  DOBA_EXPECT(destination.eof());
  DOBA_EXPECT(!destination.failed());
}
