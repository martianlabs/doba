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

std::unique_ptr<char[]> prefix(const std::string& value) {
  auto bytes = std::make_unique_for_overwrite<char[]>(value.size());
  std::memcpy(bytes.get(), value.data(), value.size());
  return bytes;
}
}  // namespace

// +===========================================================================+
// | [>] output queue preserves delivery order                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue preserves delivery order") {
  output_queue queue(8);
  DOBA_EXPECT(queue.push(prefix("ab"), 2, nullptr));
  DOBA_EXPECT(queue.push(prefix("cd"), 2, nullptr));
  DOBA_EXPECT(queue.fill());
  DOBA_EXPECT_EQUAL(queue.buffer, "abcd");
  DOBA_EXPECT_EQUAL(queue.offset, 0);
  DOBA_EXPECT(!queue.queued());
}
// +===========================================================================+
// | [>] output queue sends prefixes without copying             ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue keeps prefix buffers through partial sends") {
  output_queue queue(4);
  auto first = prefix("ab");
  char* const owned = first.get();
  DOBA_EXPECT(queue.push(std::move(first), 2, nullptr));
  DOBA_EXPECT(queue.push(prefix("cd"), 2, nullptr));
  std::array<std::span<char>, 2> segments{};
  DOBA_EXPECT_EQUAL(queue.prefix_segments(segments), 2);
  DOBA_EXPECT(segments[0].data() == owned);
  DOBA_EXPECT_EQUAL(std::string(segments[0].data(), segments[0].size()), "ab");
  DOBA_EXPECT_EQUAL(std::string(segments[1].data(), segments[1].size()), "cd");
  DOBA_EXPECT(queue.consume_prefixes(1));
  DOBA_EXPECT_EQUAL(queue.prefix_segments(segments), 2);
  DOBA_EXPECT(segments[0].data() == owned + 1);
  DOBA_EXPECT_EQUAL(segments[0].size(), 1);
  DOBA_EXPECT(!queue.push(prefix("ef"), 2, nullptr));
  DOBA_EXPECT(queue.consume_prefixes(1));
  DOBA_EXPECT(queue.push(prefix("ef"), 2, nullptr));
  DOBA_EXPECT_EQUAL(queue.prefix_segments(segments), 2);
  DOBA_EXPECT_EQUAL(std::string(segments[0].data(), segments[0].size()), "cd");
  DOBA_EXPECT_EQUAL(std::string(segments[1].data(), segments[1].size()), "ef");
  DOBA_EXPECT(queue.consume_prefixes(4));
  DOBA_EXPECT(!queue.queued());
}
// +===========================================================================+
// | [>] output queue stops direct send before source            ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue preserves source after direct prefixes") {
  const std::string body = "body";
  output_queue queue(16384);
  DOBA_EXPECT(queue.push(prefix("one"), 3, nullptr));
  DOBA_EXPECT(queue.push(prefix("head"), 4,
                         std::make_unique<reader>(reader::borrowed(
                             std::as_bytes(std::span(body))))));
  std::array<std::span<char>, 2> segments{};
  DOBA_EXPECT_EQUAL(queue.prefix_segments(segments), 1);
  DOBA_EXPECT_EQUAL(std::string(segments[0].data(), segments[0].size()), "one");
  DOBA_EXPECT(queue.consume_prefixes(3));
  DOBA_EXPECT(!queue.prefix_pending());
  DOBA_EXPECT(queue.fill());
  DOBA_EXPECT_EQUAL(queue.buffer, "headbody");
  DOBA_EXPECT(!queue.queued());
}

// +===========================================================================+
// | [>] output queue reads source after prefix                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue reads source after prefix") {
  const std::string body = "body";
  output_queue queue(16);
  DOBA_EXPECT(queue.push(prefix("head"), 4,
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
  DOBA_EXPECT(queue.push(prefix("x"), 1, std::make_unique<reader>()));
  DOBA_EXPECT(queue.push(nullptr, 0, std::make_unique<reader>()));
  DOBA_EXPECT(queue.fill());
  DOBA_EXPECT_EQUAL(queue.buffer, "x");
  DOBA_EXPECT(!queue.queued());
}

// +===========================================================================+
// | [>] output queue enforces its capacity                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("output queue enforces its capacity") {
  output_queue queue(4);
  DOBA_EXPECT(queue.push(prefix("abcd"), 4, nullptr));
  DOBA_EXPECT(!queue.push(prefix("e"), 1, nullptr));
  DOBA_EXPECT(!queue.push(nullptr, 1, nullptr));
  DOBA_EXPECT(queue.fill());
  DOBA_EXPECT_EQUAL(queue.buffer, "abcd");
  DOBA_EXPECT(!queue.push(prefix("e"), 1, nullptr));
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
  DOBA_EXPECT(queue.push(prefix("pre"), 3,
                         std::make_unique<reader>(std::move(source))));
  DOBA_EXPECT(queue.push(prefix("later"), 5, nullptr));
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
  DOBA_EXPECT(!queue.push(nullptr, 1, nullptr));
  DOBA_EXPECT(!queue.push(prefix("x"), SIZE_MAX, nullptr));
  DOBA_EXPECT(!queue.push(prefix("x"), 9, nullptr));
  DOBA_EXPECT(!queue.queued());
  DOBA_EXPECT(queue.push(prefix("abc"), 3, nullptr));
  DOBA_EXPECT(!queue.push(prefix("x"), SIZE_MAX - 2, nullptr));
  DOBA_EXPECT(!queue.consume_prefixes(4));
  DOBA_EXPECT(!queue.queued());
  DOBA_EXPECT(!queue.consume_prefixes(1));
  DOBA_EXPECT(queue.push(prefix("abcdefgh"), 8, nullptr));
  output_queue empty(0);
  DOBA_EXPECT(!empty.push(prefix("x"), 1, nullptr));
  DOBA_EXPECT(empty.push(nullptr, 0, nullptr));
  DOBA_EXPECT(!empty.queued());
}
