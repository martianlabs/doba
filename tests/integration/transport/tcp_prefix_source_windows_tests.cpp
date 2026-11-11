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
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "protocol/processing_result.h"
#include "common/reader.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "transport/tcp.h"
#include "wait_count.h"
#include "../../helpers/concurrency.h"

namespace {
using martianlabs::doba::protocol::processing_result;
namespace tr = martianlabs::doba::transport;
using martianlabs::doba::common::reader;
using martianlabs::doba::protocol::send_delegate;
using martianlabs::doba::tests::integration::tcpip_client;
using martianlabs::doba::tests::integration::wait_count;
using namespace std::chrono_literals;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] source_state                                               ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct source_state {
  // Only the fixture may keep the bytes alive before input resumes.
  std::shared_ptr<const std::string> bytes{
      std::make_shared<const std::string>(8 * 1024 * 1024, 's')};
  // Count input calls.
  std::atomic<int> calls{0};
  // Flag input delivered before the reader finishes.
  std::atomic<bool> source_released{true};
  // 0: valid reader; 1: later inline bytes; 2: a second reader.
  int incompatible{0};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] source_engine                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct source_engine {
  using policies_type = int;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(send_delegate callback) { output = std::move(callback); }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> callback) {
    close = std::move(callback);
  }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view) {
    if (state->calls.fetch_add(1) == 0) {
      output("head", {}, std::make_unique<reader>(reader::shared(
          state->bytes, std::as_bytes(std::span(*state->bytes)))));
      if (state->incompatible == 1) output("later", {}, nullptr);
      if (state->incompatible == 2)
        output({}, {}, std::make_unique<reader>());
    } else {
      state->source_released = state->bytes.use_count() == 1;
      output("done", {}, nullptr);
      close();
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
  // Shared results and the reader backing bytes.
  std::shared_ptr<source_state> state;
  // Send only during an engine call.
  send_delegate output;
  // Close only during an engine call.
  std::function<void()> close;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] exercise                                                 ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void exercise(int incompatible) {
  for (const std::size_t workers : {1u, 4u}) {
    auto state = std::make_shared<source_state>();
    state->incompatible = incompatible;
    auto factory = [state] { return source_engine{state, {}, {}}; };
    tcpip_client client;
    const auto port = client.find_available_port();
    DOBA_EXPECT(port != 0);
    tr::tcp<source_engine, decltype(factory)> transport(
        {.worker_count = workers, .max_buffered_send_bytes = 8192,
         .ip = "127.0.0.1", .port = std::to_string(port)}, factory);
    transport.set_on_connection([] {});
    transport.set_on_disconnection([] {});
    martianlabs::doba::tests::scope_exit cleanup([&] {
      client.close();
      transport.stop();
    });
    DOBA_EXPECT(client.set_receive_buffer_size(4096));
    transport.start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("first"));
    DOBA_EXPECT(wait_count(state->calls, 1));
    if (!incompatible) {
      // Send more input before reading the body. The engine must wait until the
      // reader finishes.
      DOBA_EXPECT(client.send_all("next"));
      const std::string expected = "head" + *state->bytes + "done";
      const auto bytes = client.receive(expected.size(), 15s);
      DOBA_EXPECT(bytes.has_value());
      if (bytes) DOBA_EXPECT_EQUAL(*bytes, expected);
      DOBA_EXPECT(client.wait_for_close(5s));
      DOBA_EXPECT(state->source_released.load());
      DOBA_EXPECT_EQUAL(state->calls.load(), 2);
    } else {
      const auto bytes = client.receive_until_close(32, 5s);
      // Require EOF or reset; hitting the receive limit does not prove closure.
      DOBA_EXPECT((bytes && bytes->empty()) || client.connection_reset());
      DOBA_EXPECT(client.error() != "timeout");
      DOBA_EXPECT_EQUAL(state->calls.load(), 1);
    }
    client.close();
    transport.stop();
    DOBA_EXPECT_EQUAL(state->bytes.use_count(), 1);
  }
}
}  // namespace

// +===========================================================================+
// | [>] terminal reader defers new input                        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Windows prefix source drains before new input reaches the engine") {
  exercise(0);
}

// +===========================================================================+
// | [>] later inline output closes the connection               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Windows terminal reader rejects subsequent inline output") {
  exercise(1);
}

// +===========================================================================+
// | [>] second reader closes the connection                     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Windows terminal reader rejects a second reader") {
  exercise(2);
}
