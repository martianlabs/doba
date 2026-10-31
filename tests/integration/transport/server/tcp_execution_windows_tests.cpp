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

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "tcpip_client.h"
#include "test_helper.h"
#include "transport/server/tcp.h"

namespace {
namespace tr = martianlabs::doba::transport::server;
using martianlabs::doba::tests::integration::tcpip_client;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] execution_probe                                           ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct execution_probe {
  // Number of engines configured explicitly by their factory.
  std::atomic<unsigned> configured{0};
  // Legacy transport flow hooks must never be configured or called.
  std::atomic<unsigned> flow_hooks{0};
  // The transport must not stop or wait for application tasks.
  std::atomic<unsigned> engine_stops{0};
  // Set false if transport initialization replaces the factory owner.
  std::atomic<bool> owner_preserved{true};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] probe_engine                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct probe_engine {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // +=========================================================================+
  // | [>] set_execution_capacity                                   ( public ) |
  // +-------------------------------------------------------------------------+
  void set_execution_capacity(tr::execution_capacity value) {
    ++probe->configured;
    execution = value;
  }
  // +=========================================================================+
  // | [>] stop                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  bool stop(std::function<void()>) {
    ++probe->engine_stops;
    return true;
  }
  // +=========================================================================+
  // | [>] set_input_flow_control                                  ( public ) |
  // +-------------------------------------------------------------------------+
  void set_input_flow_control(std::function<bool()>, std::function<void()>) {
    ++probe->flow_hooks;
  }
  // +=========================================================================+
  // | [>] input_paused                                            ( public ) |
  // +-------------------------------------------------------------------------+
  bool input_paused() const {
    ++probe->flow_hooks;
    return false;
  }
  // +=========================================================================+
  // | [>] resume_input                                            ( public ) |
  // +-------------------------------------------------------------------------+
  void resume_input() { ++probe->flow_hooks; }
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
  void on_bytes_received(std::string_view input) {
    if (execution.owner != expected_owner) {
      probe->owner_preserved = false;
    }
    output(input, {}, nullptr);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Test-owned counters remain alive until the transport stops.
  std::shared_ptr<execution_probe> probe;
  // Execution belongs to the engine factory, not the TCP transport.
  tr::execution_capacity execution{};
  // Expected factory owner; null when no service is configured.
  void* expected_owner{nullptr};
  // Synchronous transport output callback.
  martianlabs::doba::protocol::send_delegate output;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] exercise                                                 ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void exercise(bool configured) {
  for (const std::size_t workers : {1u, 2u}) {
    auto probe = std::make_shared<execution_probe>();
    auto factory = [probe, configured]() {
      probe_engine engine{probe, {}, nullptr, {}};
      if (configured) {
        engine.expected_owner = probe.get();
        engine.set_execution_capacity({probe.get(), nullptr});
      }
      return engine;
    };
    tcpip_client client;
    const auto port = client.find_available_port();
    DOBA_EXPECT(port != 0);
    tr::tcp<probe_engine, decltype(factory)> transport(
        {.worker_count = workers, .ip = "127.0.0.1",
         .port = std::to_string(port)}, factory);
    transport.set_on_connection([]() {});
    transport.set_on_disconnection([]() {});
    transport.start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("first"));
    const auto first = client.receive(5);
    DOBA_EXPECT(first.has_value());
    DOBA_EXPECT_EQUAL(*first, "first");
    const std::string batch(64 * 1024, 'b');
    DOBA_EXPECT(client.send_all(batch));
    const auto received = client.receive(batch.size());
    DOBA_EXPECT(received.has_value());
    DOBA_EXPECT_EQUAL(*received, batch);
    transport.stop();
    DOBA_EXPECT_EQUAL(probe->configured.load(), configured ? 1u : 0u);
    DOBA_EXPECT(probe->owner_preserved.load());
    DOBA_EXPECT_EQUAL(probe->flow_hooks.load(), 0u);
    DOBA_EXPECT_EQUAL(probe->engine_stops.load(), 0u);
  }
}
}  // namespace

// +===========================================================================+
// | [>] Windows TCP does not inject an execution service       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Windows TCP does not inject an execution service") {
  exercise(false);
}

// +===========================================================================+
// | [>] Windows TCP preserves engine-owned execution            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Windows TCP preserves engine-owned execution") {
  exercise(true);
}
