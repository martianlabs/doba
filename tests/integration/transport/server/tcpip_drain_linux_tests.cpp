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
#include <future>
#include <memory>
#include <string>
#include <utility>

#include "../../../helpers/concurrency.h"
#include "tcp_drain_engine.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "transport/server/tcp.h"

namespace {
// +---------------------------------------------------------------------------+
// | [>] general                                                    ( usings ) |
// +---------------------------------------------------------------------------+
namespace tr = martianlabs::doba::transport::server;
using martianlabs::doba::tests::integration::drain_engine;
using martianlabs::doba::tests::integration::drain_state;
using martianlabs::doba::tests::integration::tcpip_client;
using namespace std::chrono_literals;
}  // namespace

// +===========================================================================+
// | [>] quiesce waits for linux startup                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("quiesce waits for linux startup") {
  struct copy_gate {
    std::promise<void> entered;
    martianlabs::doba::tests::manual_event release;
    std::atomic<bool> enabled{false};
    std::atomic<bool> once{false};
  };
  struct gated_callback {
    std::shared_ptr<copy_gate> gate;

    explicit gated_callback(std::shared_ptr<copy_gate> value)
        : gate(std::move(value)) {}
    gated_callback(const gated_callback& other) : gate(other.gate) {
      if (gate->enabled && !gate->once.exchange(true)) {
        gate->entered.set_value();
        gate->release.wait();
      }
    }
    void operator()() const {}
  };

  auto state = std::make_shared<drain_state>();
  auto gate = std::make_shared<copy_gate>();
  auto copying = gate->entered.get_future();
  auto factory = [state]() -> drain_engine { return drain_engine{state}; };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp_policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcp<drain_engine, decltype(factory)> transport(configuration, factory);
  transport.set_on_connection(gated_callback{gate});
  transport.set_on_disconnection([]() {});
  gate->enabled = true;
  auto starting = std::async(std::launch::async, [&]() { transport.start(); });
  martianlabs::doba::tests::scope_exit release_start(
      [&] { gate->release.open(); });
  DOBA_EXPECT(copying.wait_for(5s) == std::future_status::ready);
  auto quiescing =
      std::async(std::launch::async, [&]() { transport.quiesce(); });
  const bool waited = quiescing.wait_for(200ms) == std::future_status::timeout;
  gate->release.open();
  starting.get();
  quiescing.get();
  transport.stop();
  DOBA_EXPECT(waited);
}
