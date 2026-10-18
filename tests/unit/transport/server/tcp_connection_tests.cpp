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
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "test_helper.h"
#include "transport/server/tcp_connection.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] usings                                                    ( public )  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using martianlabs::doba::protocol::send_delegate;
using martianlabs::doba::common::reader;
using martianlabs::doba::transport::server::tcp_connection;
using martianlabs::doba::transport::server::send_state;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] test_engine                                               ( struct )  |
// +---------------------------------------------------------------------------+
// | Engine probe used by TCP connection tests.                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct test_engine {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(send_delegate value) { send = std::move(value); }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> value) { close = std::move(value); }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  void on_bytes_received(const char* bytes, std::size_t size) {
    receive(bytes, size);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  std::function<void(const char*, std::size_t)> receive;
  send_delegate send;
  std::function<void()> close;
};
}  // namespace

// +===========================================================================+
// | [>] TCP delivers fresh input blocks                          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tcp connection delivers fresh blocks and reuses all receive space") {
  auto factory = []() { return test_engine{}; };
  tcp_connection<test_engine> input(8, factory);
  std::string received;
  input.engine.receive = [&](const char* bytes, std::size_t size) {
    received.append(bytes, size);
  };
  for (const auto bytes : {"abcdefgh", "ij"}) {
    auto target = input.receive_buffer();
    DOBA_EXPECT_EQUAL(target.size(), 8);
    DOBA_EXPECT_EQUAL(target.data(), input.buffer.get());
    std::memcpy(target.data(), bytes, std::strlen(bytes));
    input.process(std::strlen(bytes));
  }
  DOBA_EXPECT_EQUAL(received, "abcdefghij");
}

DOBA_TEST("tcp connection rejects receive sizes above the buffer") {
  auto factory = []() { return test_engine{}; };
  tcp_connection<test_engine> input(4, factory);
  bool delivered = false;
  input.engine.receive = [&](const char*, std::size_t) { delivered = true; };
  for (const auto size : {std::size_t{5}, std::numeric_limits<std::size_t>::max()}) {
    bool caught = false;
    try {
      input.process(size);
    } catch (const std::out_of_range&) {
      caught = true;
    }
    DOBA_EXPECT(caught);
  }
  DOBA_EXPECT(!delivered);
}

DOBA_TEST("tcp connection reuses a one byte buffer") {
  auto factory = []() { return test_engine{}; };
  tcp_connection<test_engine> input(1, factory);
  std::string received;
  input.engine.receive = [&](const char* bytes, std::size_t size) {
    received.append(bytes, size);
  };
  for (const char value : {'x', 'y'}) {
    auto target = input.receive_buffer();
    DOBA_EXPECT_EQUAL(target.size(), 1);
    target[0] = value;
    input.process(1);
  }
  DOBA_EXPECT_EQUAL(received, "xy");
}

DOBA_TEST("tcp connection propagates engine errors") {
  auto factory = []() { return test_engine{}; };
  tcp_connection<test_engine> input(4, factory);
  input.engine.receive = [](const char*, std::size_t) {
    throw std::runtime_error("receive failed");
  };
  bool caught = false;
  try {
    input.process(1);
  } catch (const std::runtime_error&) {
    caught = true;
  }
  DOBA_EXPECT(caught);
}

// +===========================================================================+
// | [>] tcp connection keeps engine callbacks                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tcp connection keeps engine callbacks") {
  auto factory = []() { return test_engine{}; };
  tcp_connection<test_engine> input(4, factory);
  bool sent = false;
  bool closed = false;
  input.engine.set_on_send(
      [&sent](std::string_view head, std::string_view body,
              std::unique_ptr<reader>) { sent = head == "x" && body.empty(); });
  input.engine.set_on_close([&closed]() { closed = true; });
  input.engine.send("x", {}, nullptr);
  input.engine.close();
  DOBA_EXPECT(sent);
  DOBA_EXPECT(closed);
}

// +===========================================================================+
// | [>] tcp connection sends queue bytes directly               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tcp connection sends queue bytes directly") {
  auto factory = []() { return test_engine{}; };
  tcp_connection<test_engine> input(8, 8, factory, nullptr);
  DOBA_EXPECT(input.established());
  DOBA_EXPECT(!input.peer_closed());
  DOBA_EXPECT(input.eof());
  DOBA_EXPECT(input.close_output());
  send_state output(8);
  DOBA_EXPECT(output.push("abc", {}, nullptr));
  DOBA_EXPECT(input.prepare_output(output));
  auto bytes = input.output_bytes(output);
  DOBA_EXPECT_EQUAL(std::string(bytes.data(), bytes.size()), "abc");
  DOBA_EXPECT(input.output_pending(output));
  DOBA_EXPECT(input.output_sent(output, 2));
  bytes = input.output_bytes(output);
  DOBA_EXPECT_EQUAL(std::string(bytes.data(), bytes.size()), "c");
  DOBA_EXPECT(input.output_sent(output, 1));
  DOBA_EXPECT(!input.output_pending(output));
}

// +===========================================================================+
// | [>] tcp connection batches queued output                    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tcp connection batches queued output") {
  auto factory = []() { return test_engine{}; };
  tcp_connection<test_engine> input(8, 8, factory, nullptr);
  send_state output(32);
  DOBA_EXPECT(output.push("head", "body", nullptr));
  DOBA_EXPECT(output.push("next", {}, nullptr));
  DOBA_EXPECT(input.prepare_output(output));
  std::array<std::span<char>, 4> buffers{};
  auto count = input.output_buffers(output, buffers);
  DOBA_EXPECT_EQUAL(count, 1);
  DOBA_EXPECT_EQUAL(std::string(buffers[0].data(), buffers[0].size()),
                    "headbodynext");
  DOBA_EXPECT(!input.output_sent(output, 13));
  DOBA_EXPECT_EQUAL(output.offset, 0);
  DOBA_EXPECT(input.output_sent(output, 5));
  count = input.output_buffers(output, buffers);
  DOBA_EXPECT_EQUAL(count, 1);
  DOBA_EXPECT_EQUAL(std::string(buffers[0].data(), buffers[0].size()),
                    "odynext");
  DOBA_EXPECT(input.output_sent(output, 7));
  DOBA_EXPECT(!input.output_pending(output));
  DOBA_EXPECT_EQUAL(input.output_buffers(output, {}), 0);
}
