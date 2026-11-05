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
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "protocol/processing_result.h"
#include "test_helper.h"
#include "transport/server/tcp_connection.h"

namespace {
using martianlabs::doba::protocol::processing_result;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] usings                                                     ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using martianlabs::doba::protocol::send_delegate;
using martianlabs::doba::common::reader;
using martianlabs::doba::transport::server::detail::tcp_connection;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] test_engine                                                ( struct ) |
// +---------------------------------------------------------------------------+
// | Engine probe used by TCP connection tests.                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct test_engine {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // Processing needs returned by the probe after consuming each full block.
  processing_result progress{};
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
  processing_result on_bytes_received(std::string_view input) {
    const auto* bytes = input.data();
    const auto size = input.size();
    receive(bytes, size);
    return progress;
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool) { return {}; }

  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  std::function<void(const char*, std::size_t)> receive;
  send_delegate send;
  std::function<void()> close;
};
}  // namespace

// +===========================================================================+
// | [>] TCP delivers fresh input blocks                         ( test-case ) |
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
  for (const auto size :
       {std::size_t{5}, std::numeric_limits<std::size_t>::max()}) {
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
// | [>] RX adapter invokes callbacks during input               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("TCP RX adapter invokes callbacks during engine input") {
  auto factory = []() { return test_engine{}; };
  tcp_connection<test_engine> input(4, factory);
  bool sent = false;
  bool closed = false;
  input.engine.set_on_send(
      [&sent](std::string_view head, std::string_view body,
              std::unique_ptr<reader>) { sent = head == "x" && body.empty(); });
  input.engine.set_on_close([&closed]() { closed = true; });
  bool delivered = false;
  input.engine.receive = [&](const char* bytes, std::size_t size) {
    delivered = size == 1 && bytes[0] == 'x';
    input.engine.send(std::string_view(bytes, size), {}, nullptr);
    input.engine.close();
  };
  input.receive_buffer()[0] = 'x';
  input.process(1);
  DOBA_EXPECT(delivered);
  DOBA_EXPECT(sent);
  DOBA_EXPECT(closed);
}


// +===========================================================================+
// | [>] Scheduling does not acknowledge input                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tcp connection returns processing needs without retaining RX") {
  auto factory = [] { return test_engine{}; };
  tcp_connection<test_engine> input(8, factory);
  const auto due = processing_result::clock::time_point{} +
                   std::chrono::seconds(9);
  input.engine.progress = {true, due, true};
  std::string received;
  input.engine.receive = [&](const char* bytes, std::size_t size) {
    received.append(bytes, size);
  };
  const auto* storage = input.receive_buffer().data();
  for (const auto bytes : {"abcdefgh", "ij"}) {
    std::memcpy(input.receive_buffer().data(), bytes, std::strlen(bytes));
    const auto result = input.process(std::strlen(bytes));
    DOBA_EXPECT(result.turn_requested);
    DOBA_EXPECT(result.turn_at && *result.turn_at == due);
    DOBA_EXPECT(result.turn_after_output_drained);
    DOBA_EXPECT_EQUAL(input.receive_buffer().data(), storage);
    DOBA_EXPECT_EQUAL(input.receive_buffer().size(), 8);
  }
  DOBA_EXPECT_EQUAL(received, "abcdefghij");
}
