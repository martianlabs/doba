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
#include <string>
#include <thread>
#include "protocol/processing_result.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "transport/tcp.h"
#include "../../helpers/concurrency.h"

namespace {
using martianlabs::doba::protocol::processing_result;
using namespace martianlabs::doba;
using namespace martianlabs::doba::tests::integration;
using namespace std::chrono_literals;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] plain_peer                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct plain_peer {
  tcpip_client socket;
  // +-------------------------------------------------------------------------+
  // | [>] connect                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  bool connect(std::uint16_t port) { return socket.connect(port); }
  // +-------------------------------------------------------------------------+
  // | [>] send                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  bool send(std::string_view input) { return socket.send_all(input); }
  // +-------------------------------------------------------------------------+
  // | [>] receive                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  auto receive(std::size_t size) { return socket.receive(size); }
};
const std::string payload(6144, 'p');
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] flow_state                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct flow_state {
  tests::manual_event connected;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] observing_engine                                           ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct observing_engine {
  using policies_type = int;
  std::shared_ptr<flow_state> state;
  protocol::send_delegate output;
  // +-------------------------------------------------------------------------+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()>) {}
  // +-------------------------------------------------------------------------+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(protocol::send_delegate value) { output = std::move(value); }
  // +-------------------------------------------------------------------------+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view) {
    output({}, payload, nullptr);
    return {};
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool) { return {}; }

};
template <template <class, class> class Transport, class Peer, class Policies>
void admission_fairness(Policies p) {
  auto state = std::make_shared<flow_state>();
  std::atomic<int> created{0};
  tcpip_client probe;
  const auto port = probe.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.worker_count = 1;
  p.max_connections = 1;
  p.max_buffered_send_bytes = 8192;
  auto factory = [state, &created] {
    created.fetch_add(1);
    return observing_engine{state, {}};
  };
  Transport<observing_engine, decltype(factory)> server(p, factory);
  server.set_on_connection([&] { state->connected.open(); });
  server.set_on_disconnection([] {});
  server.start();
  Peer admitted;
  DOBA_EXPECT(admitted.connect(port));
  DOBA_EXPECT(state->connected.wait_for());
  std::atomic<bool> stop{false};
  std::thread flooding([&] {
    for (int i = 0; i < 1000 && !stop.load(); ++i) {
      tcpip_client excess;
      excess.connect(port);
    }
  });
  tests::scope_exit cleanup([&] {
    stop = true;
    flooding.join();
  });
  for (int i = 0; i < 20; ++i) {
    DOBA_EXPECT(admitted.send("x"));
    std::string body;
    while (body.size() < payload.size()) {
      auto part = admitted.receive(payload.size() - body.size());
      DOBA_EXPECT(part.has_value());
      if (!part) break;
      body += *part;
    }
    DOBA_EXPECT_EQUAL(body, payload);
  }
  DOBA_EXPECT_EQUAL(created.load(), 1);
}

}  // namespace

DOBA_TEST("TCP admitted traffic progresses while excess peers are rejected") {
  admission_fairness<transport::tcp, plain_peer>(
      transport::tcp_policies{});
}
