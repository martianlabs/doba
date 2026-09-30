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
#include <cstddef>
#include <cstring>
#include <memory>
#include <span>
#include <string>
#include <utility>

#include "test_helper.h"
#include "transport/server/output_queue.h"

namespace {
using martianlabs::doba::common::filesystem_file;
using martianlabs::doba::common::reader;
using martianlabs::doba::transport::server::output_queue;

}  // namespace

// +===========================================================================+
// | [>] output queue preserves delivery order                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue preserves delivery order") {
  output_queue queue(8);
  DOBA_EXPECT(queue.push("ab", {}, nullptr));
  DOBA_EXPECT(queue.push("cd", {}, nullptr));
  DOBA_EXPECT(queue.fill());
  DOBA_EXPECT_EQUAL(queue.buffer, "abcd");
  DOBA_EXPECT_EQUAL(queue.offset, 0);
  DOBA_EXPECT(!queue.queued());
}
// +===========================================================================+
// | [>] output queue sends prefixes without copying             ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue keeps prefix buffers through partial sends") {
  output_queue queue(34);
  std::string first(32, 'a');
  const char* const owned = first.data();
  DOBA_EXPECT(queue.push(std::move(first), {}, nullptr));
  DOBA_EXPECT(queue.push("cd", {}, nullptr));
  auto bytes = queue.prefix_bytes();
  DOBA_EXPECT(bytes.data() == owned);
  DOBA_EXPECT_EQUAL(std::string(bytes.data(), bytes.size()),
                    std::string(32, 'a'));
  DOBA_EXPECT(queue.consume_prefixes(1));
  bytes = queue.prefix_bytes();
  DOBA_EXPECT(bytes.data() == owned + 1);
  DOBA_EXPECT_EQUAL(bytes.size(), 31);
  DOBA_EXPECT(!queue.push("ef", {}, nullptr));
  DOBA_EXPECT(queue.consume_prefixes(31));
  DOBA_EXPECT(queue.push("ef", {}, nullptr));
  bytes = queue.prefix_bytes();
  DOBA_EXPECT_EQUAL(std::string(bytes.data(), bytes.size()), "cd");
  DOBA_EXPECT(queue.consume_prefixes(2));
  bytes = queue.prefix_bytes();
  DOBA_EXPECT_EQUAL(std::string(bytes.data(), bytes.size()), "ef");
  DOBA_EXPECT(queue.consume_prefixes(2));
  DOBA_EXPECT(!queue.queued());
}
// +===========================================================================+
// | [>] output queue sends head and body without copying        ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue sends head and body without copying") {
  output_queue queue(64);
  std::string head(24, 'h');
  std::string body(24, 'b');
  const char* head_data = head.data();
  const char* body_data = body.data();
  DOBA_EXPECT(queue.push(std::move(head), std::move(body), nullptr));
  auto bytes = queue.prefix_bytes();
  DOBA_EXPECT(bytes.data() == head_data);
  DOBA_EXPECT(queue.consume_prefixes(24));
  bytes = queue.prefix_bytes();
  DOBA_EXPECT(bytes.data() == body_data);
  DOBA_EXPECT(queue.consume_prefixes(8));
  bytes = queue.prefix_bytes();
  DOBA_EXPECT(bytes.data() == body_data + 8);
  DOBA_EXPECT(queue.consume_prefixes(16));
  DOBA_EXPECT(!queue.queued());
}
// +===========================================================================+
// | [>] output queue batches prefixes in order                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue batches prefixes through partial sends") {
  output_queue queue(128);
  std::string head(24, 'h');
  std::string body(24, 'b');
  const char* head_data = head.data();
  const char* body_data = body.data();
  DOBA_EXPECT(queue.push(std::move(head), std::move(body), nullptr));
  DOBA_EXPECT(queue.push("next", {}, nullptr));
  std::array<std::span<char>, 4> buffers{};
  auto count = queue.prefix_buffers(buffers);
  DOBA_EXPECT_EQUAL(count, 3);
  std::array<std::span<char>, 2> limited{};
  DOBA_EXPECT_EQUAL(queue.prefix_buffers(limited), 2);
  DOBA_EXPECT(buffers[0].data() == head_data);
  DOBA_EXPECT(buffers[1].data() == body_data);
  DOBA_EXPECT_EQUAL(std::string(buffers[2].data(), buffers[2].size()),
                    "next");
  DOBA_EXPECT(queue.consume_prefixes(25));
  count = queue.prefix_buffers(buffers);
  DOBA_EXPECT_EQUAL(count, 2);
  DOBA_EXPECT(buffers[0].data() == body_data + 1);
  DOBA_EXPECT_EQUAL(std::string(buffers[1].data(), buffers[1].size()),
                    "next");
  DOBA_EXPECT(queue.consume_prefixes(27));
  DOBA_EXPECT(!queue.queued());
}
// +===========================================================================+
// | [>] output queue stops batch before source                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue stops a batch at a body source") {
  const std::string source_data = "source";
  output_queue queue(16384);
  DOBA_EXPECT(queue.push("one", {}, nullptr));
  DOBA_EXPECT(queue.push("head", "body",
                         std::make_unique<reader>(reader::borrowed(
                             std::as_bytes(std::span(source_data))))));
  DOBA_EXPECT(queue.push("later", {}, nullptr));
  std::array<std::span<char>, 4> buffers{};
  auto count = queue.prefix_buffers(buffers);
  DOBA_EXPECT_EQUAL(count, 3);
  DOBA_EXPECT_EQUAL(std::string(buffers[0].data(), buffers[0].size()),
                    "one");
  DOBA_EXPECT_EQUAL(std::string(buffers[1].data(), buffers[1].size()),
                    "head");
  DOBA_EXPECT_EQUAL(std::string(buffers[2].data(), buffers[2].size()),
                    "body");
  DOBA_EXPECT(queue.consume_prefixes(11));
  DOBA_EXPECT_EQUAL(queue.prefix_buffers(buffers), 0);
  DOBA_EXPECT(queue.fill());
  DOBA_EXPECT_EQUAL(queue.buffer, "sourcelater");
  DOBA_EXPECT_EQUAL(queue.prefix_buffers(buffers), 0);
}
// +===========================================================================+
// | [>] output queue stops direct send before source            ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue preserves source after direct prefixes") {
  const std::string body = "body";
  output_queue queue(16384);
  DOBA_EXPECT(queue.push("one", {}, nullptr));
  DOBA_EXPECT(queue.push("head", "inline",
                         std::make_unique<reader>(reader::borrowed(
                             std::as_bytes(std::span(body))))));
  const auto bytes = queue.prefix_bytes();
  DOBA_EXPECT_EQUAL(std::string(bytes.data(), bytes.size()), "one");
  DOBA_EXPECT(queue.consume_prefixes(3));
  DOBA_EXPECT(queue.prefix_pending());
  const auto next = queue.prefix_bytes();
  DOBA_EXPECT_EQUAL(std::string(next.data(), next.size()), "head");
  DOBA_EXPECT(queue.consume_prefixes(4));
  const auto inline_bytes = queue.prefix_bytes();
  DOBA_EXPECT_EQUAL(std::string(inline_bytes.data(), inline_bytes.size()),
                    "inline");
  DOBA_EXPECT(queue.consume_prefixes(6));
  DOBA_EXPECT(!queue.prefix_pending());
  DOBA_EXPECT(queue.fill());
  DOBA_EXPECT_EQUAL(queue.buffer, "body");
  DOBA_EXPECT(!queue.queued());
}

// +===========================================================================+
// | [>] output queue reads source after prefix                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue reads source after prefix") {
  const std::string body = "body";
  output_queue queue(16);
  DOBA_EXPECT(queue.push("head", {},
                         std::make_unique<reader>(reader::borrowed(
                             std::as_bytes(std::span(body))))));
  DOBA_EXPECT(queue.fill());
  DOBA_EXPECT_EQUAL(queue.buffer, "headbody");
  DOBA_EXPECT(!queue.queued());
}

// +===========================================================================+
// | [>] output queue accepts an empty source                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue accepts an empty source") {
  output_queue queue(16384);
  DOBA_EXPECT(queue.push("x", {}, std::make_unique<reader>()));
  DOBA_EXPECT(queue.push({}, {}, std::make_unique<reader>()));
  DOBA_EXPECT(queue.fill());
  DOBA_EXPECT_EQUAL(queue.buffer, "x");
  DOBA_EXPECT(!queue.queued());
}

// +===========================================================================+
// | [>] output queue enforces its capacity                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue enforces its capacity") {
  output_queue queue(4);
  DOBA_EXPECT(queue.push("ab", "cd", nullptr));
  DOBA_EXPECT(!queue.push("e", {}, nullptr));
  DOBA_EXPECT(queue.fill());
  DOBA_EXPECT_EQUAL(queue.buffer, "abcd");
  DOBA_EXPECT(!queue.push("e", {}, nullptr));
}

// +===========================================================================+
// | [>] output queue reports a failed source after prefix       ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue reports a failed source after prefix") {
  reader source(filesystem_file{});
  std::byte byte{};
  source.read(std::span<std::byte>(&byte, 1));
  DOBA_EXPECT(source.failed());
  output_queue queue(16384);
  DOBA_EXPECT(queue.push("pre", {},
                         std::make_unique<reader>(std::move(source))));
  DOBA_EXPECT(queue.push("later", {}, nullptr));
  DOBA_EXPECT(queue.fill());
  DOBA_EXPECT_EQUAL(queue.buffer, "pre");
  DOBA_EXPECT(queue.queued());
  queue.offset = queue.buffer.size();
  DOBA_EXPECT(!queue.fill());
  DOBA_EXPECT(queue.buffer.empty());
}

// +===========================================================================+
// | [>] output queue rejects hostile sizes and over-consumption ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue rejects hostile sizes and over-consumption") {
  output_queue queue(8);
  DOBA_EXPECT(!queue.push("123456789", {}, nullptr));
  DOBA_EXPECT(!queue.queued());
  DOBA_EXPECT(queue.push("abc", {}, nullptr));
  DOBA_EXPECT(!queue.push("123456", {}, nullptr));
  DOBA_EXPECT(!queue.consume_prefixes(4));
  DOBA_EXPECT(!queue.queued());
  DOBA_EXPECT(!queue.consume_prefixes(1));
  DOBA_EXPECT(queue.push("abcdefgh", {}, nullptr));
  output_queue empty(0);
  DOBA_EXPECT(!empty.push("x", {}, nullptr));
  DOBA_EXPECT(empty.push({}, {}, nullptr));
  DOBA_EXPECT(!empty.queued());
}
