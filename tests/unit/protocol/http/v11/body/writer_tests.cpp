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

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

#include "common/reader.h"
#include "protocol/http/v11/body/writer.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::reader;
using martianlabs::doba::protocol::http::v11::body::body_writer;

std::string release(body_writer& value) {
  reader source(value.release());
  std::string output;
  source.read_all(output);
  return output;
}
}  // namespace

// +===========================================================================+
// | [>] body writer is movable but not copyable                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("body writer is movable but not copyable") {
  static_assert(!std::is_copy_constructible_v<body_writer>);
  static_assert(!std::is_copy_assignable_v<body_writer>);
  static_assert(std::is_move_constructible_v<body_writer>);
  static_assert(std::is_move_assignable_v<body_writer>);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] raw writer tracks payload size and releases raw bytes   ( test-case ) |
// +===========================================================================+
DOBA_TEST("raw writer tracks payload size and releases raw bytes") {
  auto value = body_writer::raw();
  DOBA_EXPECT(!value.is_chunked());
  DOBA_EXPECT_EQUAL(value.bytes_written(), 0);
  DOBA_EXPECT(value.write("ab"));
  const std::byte bytes[] = {std::byte{'c'}, std::byte{0}};
  DOBA_EXPECT(value.write(bytes));
  DOBA_EXPECT(value.write(std::string_view{}));
  DOBA_EXPECT_EQUAL(value.bytes_written(), 4);
  DOBA_EXPECT(value.end());
  const std::string output = release(value);
  DOBA_EXPECT_EQUAL(output.size(), 4);
  DOBA_EXPECT_EQUAL(std::string_view(output.data(), 3), "abc");
  DOBA_EXPECT_EQUAL(output[3], '\0');
}
// +===========================================================================+
// | [>] chunked writer tracks payload size excluding framing    ( test-case ) |
// +===========================================================================+
DOBA_TEST("chunked writer tracks payload size excluding framing") {
  auto value = body_writer::chunked();
  DOBA_EXPECT(value.is_chunked());
  DOBA_EXPECT(value.write("hello"));
  DOBA_EXPECT(value.write("world"));
  DOBA_EXPECT_EQUAL(value.bytes_written(), 10);
  DOBA_EXPECT(value.end());
  DOBA_EXPECT(value.end());
  DOBA_EXPECT_EQUAL(release(value), "5\r\nhello\r\n5\r\nworld\r\n0\r\n\r\n");
}
// +===========================================================================+
// | [>] finalized chunked writers reject additional payloads    ( test-case ) |
// +===========================================================================+
DOBA_TEST("finalized chunked writers reject additional payloads") {
  auto value = body_writer::chunked();
  DOBA_EXPECT(value.write("before"));
  DOBA_EXPECT(value.end());
  DOBA_EXPECT(!value.write("after"));
  DOBA_EXPECT_EQUAL(value.bytes_written(), 6);
  DOBA_EXPECT_EQUAL(release(value), "6\r\nbefore\r\n0\r\n\r\n");
}
