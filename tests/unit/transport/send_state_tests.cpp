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

#include <limits>
#include <memory>
#include <span>
#include <string>

#include "test_helper.h"
#include "transport/send_state.h"

namespace {
using martianlabs::doba::common::filesystem;
using martianlabs::doba::common::reader;
using martianlabs::doba::transport::detail::send_state;
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
  DOBA_EXPECT(!output.push("later", {}, nullptr));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "oneheadbody");
  DOBA_EXPECT(output.consume(output.buffer.size()));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "source");
  DOBA_EXPECT(output.consume(output.buffer.size()));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT(output.buffer.empty());
  DOBA_EXPECT(output.push("later", {}, nullptr));
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
// | [>] send state keeps exact capacity across partial sends    ( test-case ) |
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
  DOBA_EXPECT(!output.push("ef", {}, nullptr));
  DOBA_EXPECT(!output.push("g", {}, nullptr));
  DOBA_EXPECT(output.consume(2));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT(output.push("ef", {}, nullptr));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "ef");
  DOBA_EXPECT(output.consume(2));
}

// +===========================================================================+
// | [>] bounded source blocks                                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("send state streams source chunks within TX capacity") {
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
  reader source(filesystem{});
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

// +===========================================================================+
// | [>] active socket batch remains stable                      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST(
    "active socket bytes remain stable while the next batch accumulates") {
  send_state output(32768);
  const std::string first(8192, 'a');
  const std::string next(16384, 'b');
  DOBA_EXPECT(output.push(first, {}, nullptr));
  DOBA_EXPECT(output.fill());
  const auto* active = output.buffer.data();
  DOBA_EXPECT(output.push(next, {}, nullptr));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer.data(), active);
  DOBA_EXPECT_EQUAL(output.buffer, first);
  DOBA_EXPECT(output.consume(17));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer.data(), active);
  DOBA_EXPECT_EQUAL(output.buffer.substr(output.offset), first.substr(17));
  DOBA_EXPECT(output.consume(first.size() - 17));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, next);
}

// +===========================================================================+
// | [>] sole reader rejects incompatible output                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("a terminal reader rejects a second reader and subsequent bytes") {
  const std::string body = "body";
  auto source = [&] {
    return std::make_unique<reader>(
        reader::borrowed(std::as_bytes(std::span(body))));
  };
  send_state output(4);
  DOBA_EXPECT(output.push("head", {}, source()));
  DOBA_EXPECT(output.push({}, {}, nullptr));
  DOBA_EXPECT(!output.push({}, {}, source()));
  DOBA_EXPECT(!output.push("x", {}, nullptr));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, "head");
  DOBA_EXPECT(output.consume(4));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer, body);
  DOBA_EXPECT(output.consume(3));
  DOBA_EXPECT(!output.push("x", {}, nullptr));
  DOBA_EXPECT(output.consume(1));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT(!output.has_source());
  DOBA_EXPECT(output.push("next", {}, nullptr));
}

// +===========================================================================+
// | [>] idle TX storage is reused by the next batch             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("idle TX storage is reused by the next accumulated batch") {
  send_state output(32768);
  const std::string bytes(8192, 'x');
  DOBA_EXPECT(output.push(bytes, {}, nullptr));
  DOBA_EXPECT(output.fill());
  const auto* storage = output.buffer.data();
  DOBA_EXPECT(output.consume(bytes.size()));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT(output.buffer.empty());
  DOBA_EXPECT(output.push(bytes, {}, nullptr));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer.data(), storage);
  DOBA_EXPECT_EQUAL(output.buffer, bytes);
}

// +===========================================================================+
// | [>] empty sends preserve source and active bytes            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("send state empty sends preserve source and exact TX capacity") {
  for (const std::string& bytes : {std::string{}, std::string(257, 's')}) {
    auto owner = std::make_shared<const std::string>(bytes);
    send_state output(5);
    DOBA_EXPECT(output.push({}, {}, nullptr));
    DOBA_EXPECT(output.push("pre", {}, nullptr));
    DOBA_EXPECT(output.push({}, {}, nullptr));
    DOBA_EXPECT(output.push("h", "b", std::make_unique<reader>(
        reader::shared(owner, std::as_bytes(std::span(*owner))))));
    DOBA_EXPECT(output.push({}, {}, nullptr));
    DOBA_EXPECT_EQUAL(owner.use_count(), 2);
    DOBA_EXPECT(!output.push("!", {}, nullptr));
    DOBA_EXPECT(!output.push({}, {}, std::make_unique<reader>()));
    DOBA_EXPECT(output.fill());
    DOBA_EXPECT_EQUAL(output.buffer, "prehb");
    const auto address = output.buffer.data();
    std::string received = output.buffer;
    DOBA_EXPECT(output.consume(2));
    DOBA_EXPECT(output.push({}, {}, nullptr));
    DOBA_EXPECT_EQUAL(output.buffer.data(), address);
    DOBA_EXPECT_EQUAL(output.buffer, "prehb");
    DOBA_EXPECT_EQUAL(output.offset, 2);
    DOBA_EXPECT(!output.push("!", {}, nullptr));
    DOBA_EXPECT(output.consume(3));
    for (int turn = 0; output.queued() && turn < 128; ++turn) {
      DOBA_EXPECT(output.push({}, {}, nullptr));
      DOBA_EXPECT(output.fill());
      DOBA_EXPECT(output.buffer.size() <= 5);
      received += output.buffer;
      DOBA_EXPECT(output.consume(output.buffer.size()));
    }
    DOBA_EXPECT(!output.queued());
    DOBA_EXPECT_EQUAL(received, "prehb" + bytes);
    DOBA_EXPECT_EQUAL(owner.use_count(), 1);
    DOBA_EXPECT(output.push("again", {}, nullptr));
    DOBA_EXPECT(output.fill());
    DOBA_EXPECT_EQUAL(output.buffer, "again");
  }
}
