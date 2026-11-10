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
#include "../../helpers/concurrency.h"

namespace {
using martianlabs::doba::protocol::processing_result;
namespace tr = martianlabs::doba::transport;
using martianlabs::doba::common::reader;
using martianlabs::doba::protocol::send_delegate;
using martianlabs::doba::tests::integration::tcpip_client;
using namespace std::chrono_literals;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] output_case                                                  ( enum ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class output_case {
  kSource, kEmptySource, kCloseSource, kCloseEmptySource,
  kLaterBytes, kSecondReader
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] empty_state                                                ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct empty_state {
  // Shared backing storage; only the fixture owns it after reader release.
  std::shared_ptr<const std::string> bytes;
  // Input count also detects a missing or unexpected additional callback.
  std::atomic<int> calls{0};
  // The reader must be released before the next input call.
  std::atomic<bool> released{true};
  // Output sequence selected before workers start.
  output_case selected{output_case::kSource};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] empty_engine                                               ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct empty_engine {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(send_delegate callback) { send = std::move(callback); }
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
    if (state->calls.fetch_add(1) != 0) {
      state->released = state->bytes.use_count() == 1;
      send("done", {}, nullptr);
      close();
      return {};
    }
    send({}, {}, nullptr);
    send("pre", {}, nullptr);
    send({}, {}, nullptr);
    send("h", "b", std::make_unique<reader>(reader::shared(
        state->bytes, std::as_bytes(std::span(*state->bytes)))));
    send({}, {}, nullptr);
    send({}, {}, nullptr);
    if (state->selected == output_case::kLaterBytes) send("!", {}, nullptr);
    if (state->selected == output_case::kSecondReader)
      send({}, {}, std::make_unique<reader>());
    send({}, {}, nullptr);
    if (state->selected == output_case::kCloseSource ||
        state->selected == output_case::kCloseEmptySource) close();
    return {};
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool) { return {}; }

  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Shared observations and immutable reader storage.
  std::shared_ptr<empty_state> state;
  // Transport callbacks, invoked only during the current input call.
  send_delegate send;
  std::function<void()> close;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] exercise                                                 ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void exercise(output_case selected) {
  for (const std::size_t workers : {1u, 4u}) {
    auto state = std::make_shared<empty_state>();
    state->selected = selected;
    const bool empty = selected == output_case::kEmptySource ||
                       selected == output_case::kCloseEmptySource;
    state->bytes = std::make_shared<const std::string>(empty ? 0 : 257, 's');
    auto factory = [state] { return empty_engine{state, {}, {}}; };
    tcpip_client client;
    const auto port = client.find_available_port();
    DOBA_EXPECT(port != 0);
    tr::tcp<empty_engine, decltype(factory)> transport(
        {.receive_buffer_capacity = 1, .worker_count = workers,
         .max_buffered_send_bytes = 5, .ip = "127.0.0.1",
         .port = std::to_string(port)}, factory);
    martianlabs::doba::tests::scope_exit cleanup([&] {
      client.close();
      transport.stop();
    });
    transport.start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("a"));
    if (selected == output_case::kLaterBytes ||
        selected == output_case::kSecondReader) {
      const auto wire = client.receive_until_close(1, 5s);
      DOBA_EXPECT((wire && wire->empty()) || client.connection_reset());
      DOBA_EXPECT(client.error() != "timeout");
      DOBA_EXPECT_EQUAL(state->calls.load(), 1);
    } else if (selected == output_case::kCloseSource ||
               selected == output_case::kCloseEmptySource) {
      const std::string expected = "prehb" + *state->bytes;
      const auto wire = client.receive(expected.size(), 5s);
      DOBA_EXPECT(wire && *wire == expected);
      DOBA_EXPECT(client.wait_for_close(5s));
      DOBA_EXPECT_EQUAL(state->calls.load(), 1);
    } else {
      const auto prefix = client.receive(5, 5s);
      DOBA_EXPECT(prefix && *prefix == "prehb");
      DOBA_EXPECT(client.send_all("b"));
      const std::string expected = *state->bytes + "done";
      const auto wire = client.receive(expected.size(), 5s);
      DOBA_EXPECT(wire && *wire == expected);
      DOBA_EXPECT(client.wait_for_close(5s));
      DOBA_EXPECT_EQUAL(state->calls.load(), 2);
      DOBA_EXPECT(state->released.load());
    }
    client.close();
    transport.stop();
    DOBA_EXPECT_EQUAL(state->bytes.use_count(), 1);
  }
}
}  // namespace

// +===========================================================================+
// | [>] empty submissions and terminal readers                  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("TCP empty sends preserve prefix source and connection progress") {
  exercise(output_case::kSource);
}
DOBA_TEST("TCP empty sends preserve an empty terminal reader") {
  exercise(output_case::kEmptySource);
}
DOBA_TEST("TCP close drains a source after empty sends") {
  exercise(output_case::kCloseSource);
}
DOBA_TEST("TCP close drains an empty source after empty sends") {
  exercise(output_case::kCloseEmptySource);
}
DOBA_TEST("TCP empty sends do not permit later inline output") {
  exercise(output_case::kLaterBytes);
}
DOBA_TEST("TCP empty sends do not permit a second reader") {
  exercise(output_case::kSecondReader);
}
