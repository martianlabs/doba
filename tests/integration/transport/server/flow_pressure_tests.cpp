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
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include "tcpip_client.h"
#include "test_helper.h"
#include "transport/server/tcp.h"
#include "../../../helpers/concurrency.h"
#ifdef DOBA_ENABLE_TLS
#include "tls_client.h"
#include "tls_server_policies.h"
#include "transport/server/tls.h"
#endif

namespace {
using namespace martianlabs::doba;
using namespace martianlabs::doba::tests::integration;
using namespace std::chrono_literals;
struct plain_peer {
  tcpip_client socket;
  bool connect(std::uint16_t port) { return socket.connect(port); }
  bool send(std::string_view input) { return socket.send_all(input); }
  auto receive(std::size_t size) { return socket.receive(size); }
};
#ifdef DOBA_ENABLE_TLS
struct secure_peer {
  tls_client client;
  bool connect(std::uint16_t port) {
    return client.socket.connect(port) && client.negotiate();
  }
  bool send(std::string_view input) { return client.send_outgoing(input); }
  auto receive(std::size_t size) { return client.receive(size); }
};
#endif
const std::string payload(6144, 'p');
struct flow_state {
  std::function<bool()> pressure;
  tests::manual_event connected, sampled, disconnected;
  std::atomic<bool> observed{false};
};
struct observing_engine {
  using policies_type = int;
  std::shared_ptr<flow_state> state;
  protocol::send_delegate output;
  void set_on_close(std::function<void()>) {}
  void set_on_send(protocol::send_delegate value) { output = std::move(value); }
  void set_input_flow_control(std::function<bool()> pressure,
                              std::function<void()>) {
    state->pressure = std::move(pressure);
  }
  bool input_paused() const { return false; }
  void resume_input() {}
  void on_bytes_received(std::string_view) {
    output({}, payload, nullptr);
    state->observed = state->pressure();
    state->sampled.open();
  }
};
template <template <class, class> class Transport, class Peer, class Policies>
void published_pressure(Policies p) {
  auto state = std::make_shared<flow_state>();
  std::function<bool()> surviving_query;
  tcpip_client probe;
  const auto port = probe.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.worker_count = 1;
  p.max_buffered_send_bytes = 8192;
  p.inactivity_period = 0ms;
  {
    auto factory = [state] { return observing_engine{state, {}}; };
    Transport<observing_engine, decltype(factory)> server(p, factory);
    server.set_on_connection([&] { state->connected.open(); });
    server.set_on_disconnection([&] { state->disconnected.open(); });
    server.start();
    {
      Peer peer;
      DOBA_EXPECT(peer.connect(port));
      DOBA_EXPECT(state->connected.wait_for());
      surviving_query = state->pressure;
      DOBA_EXPECT(!surviving_query());
      DOBA_EXPECT(peer.send("x"));
      DOBA_EXPECT(state->sampled.wait_for());
      DOBA_EXPECT(state->observed.load());
      std::string body;
      while (body.size() < payload.size()) {
        auto part = peer.receive(payload.size() - body.size());
        DOBA_EXPECT(part.has_value());
        if (!part) break;
        body += *part;
      }
      DOBA_EXPECT_EQUAL(body, payload);
      const auto deadline = std::chrono::steady_clock::now() + 3s;
      while (surviving_query() && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(1ms);
      DOBA_EXPECT(!surviving_query());
    }
    DOBA_EXPECT(state->disconnected.wait_for());
    DOBA_EXPECT(surviving_query());
    server.stop();
  }
  // The callback owns its observation state, without keeping the context alive.
  DOBA_EXPECT(surviving_query());
}
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
  p.inactivity_period = 0ms;
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
DOBA_TEST(
    "TCP flow pressure publishes enqueue drain and retirement without "
    "retaining its context") {
  published_pressure<transport::server::tcp, plain_peer>(
      transport::server::tcp_policies{});
}
#ifdef DOBA_ENABLE_TLS
DOBA_TEST(
    "TLS flow pressure publishes enqueue drain and retirement without "
    "retaining its context") {
  published_pressure<transport::server::tls, secure_peer>(server_policies(1));
}
#endif

DOBA_TEST(
    "TCP admitted traffic progresses while excess connections are rejected") {
  admission_fairness<transport::server::tcp, plain_peer>(
      transport::server::tcp_policies{});
}
#ifdef DOBA_ENABLE_TLS
DOBA_TEST(
    "TLS admitted traffic progresses while excess connections are rejected "
    "before handshake") {
  admission_fairness<transport::server::tls, secure_peer>(server_policies(1));
}
#endif
