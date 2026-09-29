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
#include <functional>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>

#include "test_helper.h"
#include "transport/server/tcp_connection.h"

namespace {
using martianlabs::doba::common::send_delegate;
using martianlabs::doba::common::reader;
using martianlabs::doba::transport::server::tcp_connection;
using martianlabs::doba::transport::server::output_queue;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] test_engine                                               ( struct )  |
// +---------------------------------------------------------------------------+
// | Engine probe used by TCP connection tests.                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct test_engine {
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  using policies_type = int;
  // +=========================================================================+
  // | [>] METHODs                                                  ( public ) |
  // +=========================================================================+
  void set_on_send(send_delegate value) { send = std::move(value); }
  void set_on_close(std::function<void()> value) { close = std::move(value); }
  std::size_t on_bytes_received(const char* bytes, std::size_t size,
                                std::size_t capacity) {
    return receive(bytes, size, capacity);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  std::function<std::size_t(const char*, std::size_t, std::size_t)> receive;
  send_delegate send;
  std::function<void()> close;
};
}  // namespace

// +===========================================================================+
// | [>] tcp connection retains unconsumed fragments             ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcp connection retains unconsumed fragments") {
  auto factory = []() { return test_engine{}; };
  tcp_connection<test_engine> input(8, factory);
  std::string received;
  std::size_t received_capacity = 0;
  input.engine.receive = [&](const char* bytes, std::size_t size,
                             std::size_t capacity) {
    received.assign(bytes, size);
    received_capacity = capacity;
    return std::size_t{2};
  };
  std::memcpy(input.buffer.get(), "ab", 2);
  input.size = 2;
  DOBA_EXPECT(input.consume(0));
  std::memcpy(input.buffer.get() + input.size, "cd", 2);
  input.size += 2;
  DOBA_EXPECT(input.consume(input.process()));
  DOBA_EXPECT_EQUAL(received, "abcd");
  DOBA_EXPECT_EQUAL(received_capacity, 8);
  DOBA_EXPECT_EQUAL(input.size, 2);
  DOBA_EXPECT_EQUAL(std::string(input.buffer.get(), input.size), "cd");
  DOBA_EXPECT(input.consume(2));
  DOBA_EXPECT_EQUAL(input.size, 0);
}

// +===========================================================================+
// | [>] tcp connection rejects invalid consumption              ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcp connection rejects invalid consumption") {
  auto factory = []() { return test_engine{}; };
  tcp_connection<test_engine> input(4, factory);
  std::memcpy(input.buffer.get(), "abcd", 4);
  input.size = 4;
  DOBA_EXPECT_EQUAL(input.size, input.capacity);
  DOBA_EXPECT(!input.consume(5));
  DOBA_EXPECT_EQUAL(input.size, 4);
  DOBA_EXPECT_EQUAL(std::string(input.buffer.get(), input.size), "abcd");
}

// +===========================================================================+
// | [>] tcp connection propagates engine errors                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcp connection propagates engine errors") {
  auto factory = []() { return test_engine{}; };
  tcp_connection<test_engine> input(4, factory);
  input.engine.receive = [](const char*, std::size_t, std::size_t)
      -> std::size_t { throw std::runtime_error("receive failed"); };
  std::memcpy(input.buffer.get(), "x", 1);
  input.size = 1;
  bool caught = false;
  try {
    input.process();
  } catch (const std::runtime_error&) {
    caught = true;
  }
  DOBA_EXPECT(caught);
  DOBA_EXPECT_EQUAL(input.size, 1);
}

// +===========================================================================+
// | [>] tcp connection keeps engine callbacks                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcp connection keeps engine callbacks") {
  auto factory = []() { return test_engine{}; };
  tcp_connection<test_engine> input(4, factory);
  bool sent = false;
  bool closed = false;
  input.engine.set_on_send([&sent](std::unique_ptr<char[]> buffer,
                                   std::size_t size,
                                   std::unique_ptr<reader>) {
    sent = size == 1 && buffer[0] == 'x';
  });
  input.engine.set_on_close([&closed]() { closed = true; });
  auto buffer = std::make_unique<char[]>(1);
  buffer[0] = 'x';
  input.engine.send(std::move(buffer), 1, nullptr);
  input.engine.close();
  DOBA_EXPECT(sent);
  DOBA_EXPECT(closed);
}

// +===========================================================================+
// | [>] tcp connection sends queue bytes directly               ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcp connection sends queue bytes directly") {
  auto factory = []() { return test_engine{}; };
  tcp_connection<test_engine> input(8, 8, factory, nullptr);
  DOBA_EXPECT(input.established());
  DOBA_EXPECT(!input.peer_closed());
  DOBA_EXPECT(input.eof());
  DOBA_EXPECT(input.close_output());
  output_queue output(8);
  auto prefix = std::make_unique<char[]>(3);
  std::memcpy(prefix.get(), "abc", 3);
  DOBA_EXPECT(output.push(std::move(prefix), 3, nullptr));
  DOBA_EXPECT(input.prepare_output(output));
  std::array<std::span<char>, 2> segments{};
  DOBA_EXPECT_EQUAL(input.output_segments(output, segments), 1);
  DOBA_EXPECT_EQUAL(std::string(segments[0].data(), segments[0].size()), "abc");
  DOBA_EXPECT(input.output_pending(output));
  DOBA_EXPECT(input.output_sent(output, 2));
  DOBA_EXPECT_EQUAL(input.output_segments(output, segments), 1);
  DOBA_EXPECT_EQUAL(std::string(segments[0].data(), segments[0].size()), "c");
  DOBA_EXPECT(input.output_sent(output, 1));
  DOBA_EXPECT(!input.output_pending(output));
}
