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

#include <limits>
#include <memory>
#include <span>
#include <string>

#include "test_helper.h"
#include "transport/server/tcp_connection.h"

namespace {
using martianlabs::doba::common::filesystem_file;
using martianlabs::doba::common::reader;
using martianlabs::doba::transport::server::send_state;
}  // namespace

// +===========================================================================+
// | [>] send state copies inline bytes before returning         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("send state owns and batches inline bytes") {
  send_state output(16);
  std::string head = "head";
  std::string body = "body";
  DOBA_EXPECT(output.push(head, body, nullptr));
  head.assign("xxxx");
  body.assign("yyyy");
  DOBA_EXPECT(output.push("next", {}, nullptr));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "headbodynext");
  DOBA_EXPECT(!output.queued());
  DOBA_EXPECT(output.consume(5));
  DOBA_EXPECT_EQUAL(output.buffer.substr(output.offset), "odynext");
  DOBA_EXPECT(output.consume(7));
  DOBA_EXPECT(!output.queued());
}

// +===========================================================================+
// | [>] send state preserves source order and ownership         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("send state preserves source order") {
  send_state output(16384);
  std::string source_bytes = "source";
  DOBA_EXPECT(output.push("one", {}, nullptr));
  DOBA_EXPECT(output.push("head", "body",
                          std::make_unique<reader>(reader::borrowed(
                              std::as_bytes(std::span(source_bytes))))));
  DOBA_EXPECT(output.push("later", {}, nullptr));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "oneheadbody");
  DOBA_EXPECT(output.consume(output.buffer.size()));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "source");
  DOBA_EXPECT(output.consume(output.buffer.size()));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "later");
}

// +===========================================================================+
// | [>] send state enforces inline capacity                     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("send state enforces inline capacity") {
  send_state output(8);
  DOBA_EXPECT(!output.push("123456789", {}, nullptr));
  DOBA_EXPECT(output.push("1234", "56", nullptr));
  DOBA_EXPECT(!output.push("789", {}, nullptr));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "123456");
  DOBA_EXPECT(!output.consume(7));
  DOBA_EXPECT(output.consume(6));
  DOBA_EXPECT(output.push("789", {}, nullptr));
}

// +===========================================================================+
// | [>] send state keeps exact capacity across partial sends   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("send state keeps exact capacity across partial sends") {
  send_state output(4);
  DOBA_EXPECT(output.push("ab", "cd", nullptr));
  DOBA_EXPECT(!output.push("e", {}, nullptr));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "abcd");
  DOBA_EXPECT(!output.consume(std::numeric_limits<std::size_t>::max()));
  DOBA_EXPECT_EQUAL(output.offset, 0);
  DOBA_EXPECT(output.consume(2));
  DOBA_EXPECT(output.push("ef", {}, nullptr));
  DOBA_EXPECT(!output.push("g", {}, nullptr));
  DOBA_EXPECT(output.consume(2));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "ef");
  DOBA_EXPECT(output.consume(2));
}

// +===========================================================================+
// | [>] send state reserves capacity for a byte source         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("send state reserves capacity for a byte source") {
  std::string source_bytes = "xy";
  send_state output(4);
  DOBA_EXPECT(output.push(
      "a", {}, std::make_unique<reader>(reader::borrowed(
                   std::as_bytes(std::span(source_bytes))))));
  DOBA_EXPECT(!output.push("b", {}, nullptr));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "a");
  DOBA_EXPECT(output.consume(1));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "xy");
  DOBA_EXPECT(output.consume(2));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT(output.push("bcde", {}, nullptr));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "bcde");
}

// +===========================================================================+
// | [>] send state reports a failed source after its prefix     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("send state reports a failed source after its prefix") {
  reader source(filesystem_file{});
  std::byte byte{};
  source.read(std::span<std::byte>(&byte, 1));
  DOBA_EXPECT(source.failed());
  send_state output(16384);
  DOBA_EXPECT(
      output.push("head", {}, std::make_unique<reader>(std::move(source))));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "head");
  DOBA_EXPECT(output.consume(4));
  DOBA_EXPECT(!output.fill());
}
