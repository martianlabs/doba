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
#include <span>
#include <string>
#include <utility>

#include "common/reader.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::byte_storage;
using martianlabs::doba::common::reader;
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
