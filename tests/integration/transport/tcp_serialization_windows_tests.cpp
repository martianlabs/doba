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

#include <atomic>
#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "protocol/processing_result.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "transport/tcp.h"
#include "../../helpers/concurrency.h"

namespace {
using martianlabs::doba::protocol::processing_result;
namespace tr = martianlabs::doba::transport;
using martianlabs::doba::tests::integration::tcpip_client;
using namespace std::chrono_literals;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] observations                                               ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct observations {
  // Hold one engine call on a separate connection.
  martianlabs::doba::tests::manual_event held, release;
  // Flag overlapping engine calls on the same connection.
  std::atomic<bool> overlap{false};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] serial_engine                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct serial_engine {
  using policies_type = int;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit serial_engine(std::shared_ptr<observations> value)
      : state{std::move(value)} {}
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(martianlabs::doba::protocol::send_delegate value) {
    output = std::move(value);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()>) {}
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view bytes) {
    if (active.exchange(true)) state->overlap = true;
    martianlabs::doba::tests::scope_exit leave([&] { active = false; });
    for (char byte : bytes) {
      if (byte == 'h') {
        state->held.open();
        if (!state->release.wait_for(5s)) {
          throw std::runtime_error("Test did not release its engine");
        }
      }
      const std::string body(256, byte);
      for (int i = 0; i < 16; ++i) output({}, body, nullptr);
    }
    return {};
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool) { return {}; }

  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Share results, but give each engine its own entry guard.
  std::shared_ptr<observations> state;
  // Check overlap without using the transport lock.
  std::atomic<bool> active{false};
  // Send only during an engine call.
  martianlabs::doba::protocol::send_delegate output;
};
}  // namespace

// +===========================================================================+
// | [>] multiple output calls preserve byte order               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Windows serializes engine input and batches synchronous callbacks") {
  for (const std::size_t workers : {1u, 4u, 8u}) {
    auto state = std::make_shared<observations>();
    auto factory = [state] { return serial_engine{state}; };
    tcpip_client client;
    const auto port = client.find_available_port();
    tr::tcp<serial_engine, decltype(factory)> transport(
        {.worker_count = workers, .ip = "127.0.0.1",
         .port = std::to_string(port)}, factory);
    transport.set_on_connection([] {});
    transport.set_on_disconnection([] {});
    martianlabs::doba::tests::scope_exit cleanup([&] {
      client.close();
      transport.stop();
    });
    transport.start();
    DOBA_EXPECT(client.connect(port));
    for (int i = 0; i < 32; ++i) {
      DOBA_EXPECT(client.send_all("abc"));
      DOBA_EXPECT(client.send_all("def"));
      std::string expected;
      for (char byte : std::string_view("abcdef"))
        expected.append(4096, byte);
      const auto bytes = client.receive(expected.size(), 5s);
      DOBA_EXPECT(bytes.has_value());
      if (bytes) DOBA_EXPECT_EQUAL(*bytes, expected);
    }
    client.close();
    transport.stop();
    DOBA_EXPECT(!state->overlap.load());
  }
}

// +===========================================================================+
// | [>] connection locks are independent                        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Windows engine lock does not block an independent connection") {
  auto state = std::make_shared<observations>();
  auto factory = [state] { return serial_engine{state}; };
  tcpip_client held, fast;
  const auto port = held.find_available_port();
  tr::tcp<serial_engine, decltype(factory)> transport(
      {.worker_count = 4, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  transport.set_on_connection([] {});
  transport.set_on_disconnection([] {});
  martianlabs::doba::tests::scope_exit cleanup([&] {
    state->release.open();
    held.close();
    fast.close();
    transport.stop();
  });
  transport.start();
  DOBA_EXPECT(held.connect(port));
  DOBA_EXPECT(held.send_all("h"));
  DOBA_EXPECT(state->held.wait_for(5s));
  DOBA_EXPECT(fast.connect(port));
  DOBA_EXPECT(fast.send_all("f"));
  const auto fast_bytes = fast.receive(4096, 5s);
  DOBA_EXPECT(fast_bytes.has_value());
  if (fast_bytes) DOBA_EXPECT_EQUAL(*fast_bytes, std::string(4096, 'f'));
  state->release.open();
  const auto held_bytes = held.receive(4096, 5s);
  DOBA_EXPECT(held_bytes.has_value());
  if (held_bytes) DOBA_EXPECT_EQUAL(*held_bytes, std::string(4096, 'h'));
  DOBA_EXPECT(!state->overlap.load());
}
