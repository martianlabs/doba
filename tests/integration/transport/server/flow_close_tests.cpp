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
#include "test_helper.h"
#include "tcpip_client.h"
#include "wait_count.h"
#include "transport/server/tcp.h"
#ifdef DOBA_ENABLE_TLS
#include "tls_client.h"
#include "tls_server_policies.h"
#endif

namespace {
using namespace martianlabs::doba;
using namespace martianlabs::doba::tests::integration;
using namespace std::chrono_literals;
struct flow_state {
  std::atomic<int> received{0};
  std::function<void()> ready;
};
struct close_engine {
  using policies_type = int;
  std::shared_ptr<flow_state> state;
  std::function<void()> close;
  std::atomic<bool> paused{false};
  void set_on_send(protocol::send_delegate) {}
  void set_on_close(std::function<void()> callback) { close = std::move(callback); }
  void set_input_flow_control(std::function<bool()>, std::function<void()> ready) {
    state->ready = std::move(ready);
  }
  bool input_paused() const { return paused.load(); }
  void on_bytes_received(std::string_view) { paused = true; ++state->received; }
  void resume_input() { if (paused.exchange(false)) close(); }
};
template <template <class, class> class Transport, class Policies>
void resume_close(Policies configuration) {
  auto state = std::make_shared<flow_state>();
  auto factory = [state] { return close_engine{state, {}, false}; };
  std::atomic<int> disconnected{0};
  Transport<close_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([] {});
  server.set_on_disconnection([&] { ++disconnected; });
  server.start();
  tcpip_client client;
  DOBA_EXPECT(client.connect(static_cast<std::uint16_t>(std::stoi(configuration.port))));
  DOBA_EXPECT(client.send_all("pause"));
  DOBA_EXPECT(wait_count(state->received, 1));
  state->ready();
  DOBA_EXPECT(client.wait_for_close(3s));
  client.close();
  server.stop();
  DOBA_EXPECT_EQUAL(disconnected.load(), 1);
}
}

DOBA_TEST("TCP resumes a paused engine that closes without producing output") {
  tcpip_client probe;
  transport::server::tcp_policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(probe.find_available_port());
  configuration.inactivity_period = 0ms;
  resume_close<transport::server::tcp>(configuration);
}

#ifdef DOBA_ENABLE_TLS
DOBA_TEST("TLS resumes a paused engine that closes without producing output") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  auto configuration = server_policies(port);
  configuration.inactivity_period = 0ms;
  auto state = std::make_shared<flow_state>();
  auto factory = [state] { return close_engine{state, {}, false}; };
  transport::server::tls<close_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([] {});
  server.set_on_disconnection([] {});
  server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  DOBA_EXPECT(client.send_outgoing("pause"));
  DOBA_EXPECT(wait_count(state->received, 1));
  state->ready();
  DOBA_EXPECT(client.receive_close());
  client.socket.close();
  server.stop();
}
#endif
