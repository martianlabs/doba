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
#include <vector>
#include "http_server.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "wait_count.h"
#include "../../../helpers/concurrency.h"
#ifdef DOBA_ENABLE_TLS
#include "https_server.h"
#include "tls_client.h"
#include "tls_server_policies.h"
#endif

namespace {
using namespace martianlabs::doba;
using namespace martianlabs::doba::tests::integration;
using namespace std::chrono_literals;
using protocol::http::v11::request;
using protocol::http::v11::response;
constexpr std::string_view get = "GET / HTTP/1.1\r\nHost: a\r\n\r\n";
struct plain_peer {
  tcpip_client socket;
  bool connect(std::uint16_t port) { return socket.connect(port); }
  bool send(std::string_view input) { return socket.send_all(input); }
  auto receive(std::size_t size) { return socket.receive(size); }
};
#ifdef DOBA_ENABLE_TLS
struct secure_peer {
  tls_client client;
  tcpip_client& socket = client.socket;
  bool connect(std::uint16_t port) {
    return socket.connect(port) && client.negotiate();
  }
  bool send(std::string_view input) { return client.send_outgoing(input); }
  auto receive(std::size_t size) { return client.receive(size); }
};
#endif
template <class Peer>
bool reply(Peer& peer, std::string_view expected) {
  std::string head;
  while (!head.ends_with("\r\n\r\n") && head.size() < 4096) {
    auto byte = peer.receive(1);
    if (!byte) return false;
    head += *byte;
  }
  if (!head.starts_with("HTTP/1.1 200")) return false;
  while (!expected.empty()) {
    auto size = std::min<std::size_t>(65536, expected.size());
    auto bytes = peer.receive(size);
    if (!bytes || *bytes != expected.substr(0, size)) return false;
    expected.remove_prefix(size);
  }
  return true;
}

struct flow_observation {
  tests::manual_event paused;
  std::atomic<bool> seen_pause{false}, resumed{false};
};
struct observed_policies : protocol::http::v11::policies {
  std::shared_ptr<flow_observation> observation;
};
using router_type = protocol::http::router<request, response>;
struct observed_engine : protocol::http::v11::engine<request, response> {
  using base = protocol::http::v11::engine<request, response>;
  using policies_type = observed_policies;
  std::shared_ptr<flow_observation> observation;
  observed_engine(policies_type p, const router_type& routes,
                  std::function<void()> started = {},
                  std::function<void()> finished = {})
      : base(p, routes, std::move(started), std::move(finished)),
        observation(p.observation) {}
  bool input_paused() const {
    const bool paused = base::input_paused();
    if (paused) {
      observation->seen_pause = true;
      observation->paused.open();
    } else if (observation->seen_pause.load())
      observation->resumed = true;
    return paused;
  }
  void on_bytes_received(std::string_view input) {
    base::on_bytes_received(input);
    (void)input_paused();
  }
  void resume_input() {
    base::resume_input();
    (void)input_paused();
  }
};
using observed_http =
    http_server<request, response, router_type, observed_engine>;
#ifdef DOBA_ENABLE_TLS
using observed_https =
    https_server<request, response, router_type, observed_engine>;
#endif

template <class Server, class Peer, class Policies>
void byte_budget(Policies p, bool unlimited) {
  tcpip_client probe;
  const auto port = probe.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.worker_count = 2;
  p.inactivity_period = 0ms;
  observed_policies limits;
  limits.max_pending_requests = 128;
  limits.max_pending_response_bytes = unlimited ? 0 : 4096;
  limits.observation = std::make_shared<flow_observation>();
  auto observation = limits.observation;
  tests::manual_event first;
  std::atomic<int> calls{0};
  Server server(p, limits);
  // Configuration changes after construction must not alter a live engine.
  limits.max_pending_response_bytes = unlimited ? 1 : 0;
  server.add_route("GET", "/",
                   [&](const request&, response& res) -> common::task<void> {
                     if (calls.fetch_add(1) == 0) co_await first;
                     res.ok_200().set_body(std::string(128, 'b'));
                     co_return;
                   });
  server.start();
  tests::scope_exit cleanup([&] {
    first.open();
    server.stop();
  });
  Peer peer;
  DOBA_EXPECT(peer.connect(port));
  std::string batch;
  for (int i = 0; i < 40; ++i) batch += get;
  DOBA_EXPECT(peer.send(batch));
  DOBA_EXPECT(first.wait_for_waiters(1));
  if (unlimited) {
    DOBA_EXPECT(wait_count(calls, 40));
    DOBA_EXPECT(!observation->seen_pause.load());
  } else {
    DOBA_EXPECT(observation->paused.wait_for());
    DOBA_EXPECT(calls.load() > 2 && calls.load() < 40);
    // 40 requests cannot reach the position high watermark of 96. No output
    // has reached the transport, so this pause can only come from HTTP bytes.
  }
  first.open();
  for (int i = 0; i < 40; ++i) DOBA_EXPECT(reply(peer, std::string(128, 'b')));
  DOBA_EXPECT_EQUAL(calls.load(), 40);
  if (!unlimited) DOBA_EXPECT(observation->resumed.load());
}

template <class Server, class Peer, class Policies>
void connection_zero_and_snapshot(Policies p) {
  for (std::size_t maximum : {0u, 1u}) {
    tcpip_client probe;
    auto port = probe.find_available_port();
    p.ip = "127.0.0.1";
    p.port = std::to_string(port);
    p.worker_count = 2;
    p.max_connections = maximum;
    p.inactivity_period = 0ms;
    Server server(p);
    p.max_connections = maximum ? 0 : 1;
    p.inactivity_period = 1ms;
    server.add_route("GET", "/", [](const request&, response& res) {
      res.ok_200().set_body("ok");
    });
    server.start();
    std::vector<std::unique_ptr<Peer>> peers;
    for (int i = 0; i < (maximum ? 1 : 3); ++i) {
      auto peer = std::make_unique<Peer>();
      DOBA_EXPECT(peer->connect(port));
      DOBA_EXPECT(peer->send(get));
      DOBA_EXPECT(reply(*peer, "ok"));
      peers.push_back(std::move(peer));
    }
    if (maximum) {
      tcpip_client excess;
      DOBA_EXPECT(excess.connect(port));
      DOBA_EXPECT(excess.wait_for_close(3s) || excess.connection_reset());
    }
    server.stop();
  }
}

template <class Server, class Peer, class Policies>
void source_zero(Policies p) {
  tcpip_client probe;
  const auto port = probe.find_available_port();
  p.ip = "127.0.0.1"; p.port = std::to_string(port); p.worker_count = 2;
  p.inactivity_period = 0ms; p.max_pending_sources = 0;
  observed_policies limits;
  limits.max_pending_response_bytes = 1;
  limits.observation = std::make_shared<flow_observation>();
  auto observation = limits.observation;
  auto bytes = std::make_shared<const std::string>(1024 * 1024, 's');
  std::atomic<int> calls{0};
  Server server(p, limits);
  p.max_pending_sources = 1;
  server.add_route("GET", "/", [&](const request&, response& res) {
    ++calls;
    res.ok_200().set_body(common::reader::shared(bytes, std::as_bytes(std::span(*bytes))), bytes->size());
  });
  server.start();
  Peer peer;
  DOBA_EXPECT(peer.connect(port));
  DOBA_EXPECT(peer.send(std::string(get) + std::string(get)));
  DOBA_EXPECT(wait_count(calls, 2));
  DOBA_EXPECT(!observation->seen_pause.load());
  // Two logical MiB exceed both HTTP's pending byte budget and TCP's byte
  // budget. Only the queued prefixes and streaming reservation count here.
  DOBA_EXPECT(reply(peer, *bytes));
  DOBA_EXPECT(reply(peer, *bytes));
  server.stop();
}

struct wake_state {
  tests::manual_event entered, release;
  std::atomic<int> active{0}, received{0}, resumed{0};
  std::atomic<bool> reentered{false}, allow_resume{false};
  std::function<void()> ready;
  protocol::send_delegate send;
  bool hold_callback{false}, send_on_resume{true};
};
struct wake_engine {
  using policies_type = int;
  std::shared_ptr<wake_state> state;
  std::atomic<bool> paused{false};
  void set_on_send(protocol::send_delegate value) {
    state->send = std::move(value);
  }
  void set_on_close(std::function<void()>) {}
  void set_input_flow_control(std::function<bool()>,
                              std::function<void()> ready) {
    state->ready = std::move(ready);
  }
  bool input_paused() const { return paused.load(); }
  void on_bytes_received(std::string_view) {
    ++state->active;
    paused = true;
    ++state->received;
    state->entered.open();
    if (state->hold_callback) state->release.wait();
    --state->active;
  }
  void resume_input() {
    if (!state->allow_resume.load() || !paused.exchange(false)) return;
    if (state->active.load()) state->reentered = true;
    ++state->resumed;
    if (state->send_on_resume) state->send("ok", {}, nullptr);
  }
};

template <template <class, class> class Transport, class Peer, class Policies>
void wake_during_receive(Policies p) {
  tcpip_client probe;
  auto port = probe.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.worker_count = 2;
  p.inactivity_period = 0ms;
  auto state = std::make_shared<wake_state>();
  state->hold_callback = true;
  auto factory = [state] { return wake_engine{state, false}; };
  Transport<wake_engine, decltype(factory)> server(p, factory);
  server.set_on_connection([] {});
  server.set_on_disconnection([] {});
  server.start();
  tests::scope_exit cleanup([&] {
    state->release.open();
    server.stop();
  });
  Peer peer;
  DOBA_EXPECT(peer.connect(port));
  DOBA_EXPECT(peer.send("pause"));
  DOBA_EXPECT(state->entered.wait_for());
  state->allow_resume = true;
  state->ready();  // Deliberately while the receive callback is held by a gate.
  DOBA_EXPECT_EQUAL(state->resumed.load(), 0);
  DOBA_EXPECT(!state->reentered.load());
  state->release.open();
  auto result = peer.receive(2);
  DOBA_EXPECT(result && *result == "ok");
  DOBA_EXPECT_EQUAL(state->resumed.load(), 1);
  DOBA_EXPECT(!state->reentered.load());
}

struct sending_engine {
  using policies_type = int;
  std::shared_ptr<wake_state> state;
  void set_on_send(protocol::send_delegate send) {
    state->send = std::move(send);
  }
  void set_on_close(std::function<void()>) {}
  void on_bytes_received(std::string_view) { ++state->received; }
};
template <template <class, class> class Transport, class Peer, class Policies>
void send_progress(Policies p) {
  tcpip_client probe;
  auto port = probe.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.worker_count = 2;
  p.inactivity_period = 400ms;
  auto state = std::make_shared<wake_state>();
  auto factory = [state] { return sending_engine{state}; };
  Transport<sending_engine, decltype(factory)> server(p, factory);
  server.set_on_connection([] {});
  server.set_on_disconnection([] {});
  server.start();
  Peer peer;
  DOBA_EXPECT(peer.connect(port));
  DOBA_EXPECT(peer.send("begin"));
  DOBA_EXPECT(wait_count(state->received, 1));
  for (int i = 0; i < 8; ++i) {
    std::this_thread::sleep_for(100ms);
    state->send("x", {}, nullptr);
    auto byte = peer.receive(1);
    DOBA_EXPECT(byte && *byte == "x");
  }
  DOBA_EXPECT_EQUAL(state->received.load(), 1);  // Only outgoing I/O renews it.
  DOBA_EXPECT(peer.socket.has_data(3s));
  DOBA_EXPECT(!peer.receive(1));
  server.stop();
}

template <class Server, class Policies>
void negative_period(Policies p) {
  tcpip_client probe;
  auto port = probe.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.inactivity_period = -1ms;
  bool rejected = false;
  try {
    Server server(p);
    server.start();
  } catch (const std::runtime_error&) {
    rejected = true;
  }
  DOBA_EXPECT(rejected);
  DOBA_EXPECT(!probe.connect(port));
  p.inactivity_period = 0ms;
  p.worker_count = 2;
  Server valid(p);
  valid.start();
  valid.stop();
}
}  // namespace

DOBA_TEST(
    "TCP HTTP bytes alone pause and resume while zero disables that budget") {
  byte_budget<observed_http, plain_peer>(transport::server::tcp_policies{},
                                         false);
  byte_budget<observed_http, plain_peer>(transport::server::tcp_policies{},
                                         true);
}
DOBA_TEST("TCP connection zero and policy snapshots apply to a real listener") {
  connection_zero_and_snapshot<http_server<>, plain_peer>(
      transport::server::tcp_policies{});
}
DOBA_TEST("TCP zero source limit streams multiple responses without charging logical length") {
  source_zero<observed_http, plain_peer>(transport::server::tcp_policies{});
}
DOBA_TEST(
    "TCP input wakeups during a held receive are serialized and not lost") {
  wake_during_receive<transport::server::tcp, plain_peer>(
      transport::server::tcp_policies{});
}
DOBA_TEST("TCP positive sends renew inactivity without any further receive") {
  send_progress<transport::server::tcp, plain_peer>(
      transport::server::tcp_policies{});
}
DOBA_TEST("TCP rejects a negative inactivity period before listening") {
  negative_period<http_server<>>(transport::server::tcp_policies{});
}
#ifdef DOBA_ENABLE_TLS
DOBA_TEST(
    "TLS HTTP bytes alone pause and resume while zero disables that budget") {
  byte_budget<observed_https, secure_peer>(server_policies(1), false);
  byte_budget<observed_https, secure_peer>(server_policies(1), true);
}
DOBA_TEST("TLS connection zero and policy snapshots apply to a real listener") {
  connection_zero_and_snapshot<https_server<>, secure_peer>(server_policies(1));
}
DOBA_TEST("TLS zero source limit streams multiple responses without charging logical length") {
  source_zero<observed_https, secure_peer>(server_policies(1));
}
DOBA_TEST(
    "TLS input wakeups during a held receive are serialized and not lost") {
  wake_during_receive<transport::server::tls, secure_peer>(server_policies(1));
}
DOBA_TEST("TLS positive sends renew inactivity without any further receive") {
  send_progress<transport::server::tls, secure_peer>(server_policies(1));
}
DOBA_TEST("TLS rejects a negative inactivity period before listening") {
  negative_period<https_server<>>(server_policies(1));
}
DOBA_TEST(
    "TLS resumes a buffered peer close notify without waiting for TCP EOF") {
  tls_client peer;
  auto port = peer.socket.find_available_port();
  auto p = server_policies(port);
  p.inactivity_period = 0ms;
  auto state = std::make_shared<wake_state>();
  state->send_on_resume = false;
  auto factory = [state] { return wake_engine{state, false}; };
  transport::server::tls<wake_engine, decltype(factory)> server(p, factory);
  server.set_on_connection([] {});
  server.set_on_disconnection([] {});
  server.start();
  DOBA_EXPECT(peer.socket.connect(port));
  DOBA_EXPECT(peer.negotiate());
  std::size_t written = 0;
  DOBA_EXPECT(SSL_write_ex(peer.ssl.get(), "pause", 5, &written) == 1 &&
              written == 5);
  DOBA_EXPECT(SSL_shutdown(peer.ssl.get()) == 0);
  std::string encrypted(BIO_ctrl_pending(peer.network.get()), '\0');
  std::size_t size = 0;
  DOBA_EXPECT(BIO_read_ex(peer.network.get(), encrypted.data(),
                          encrypted.size(), &size) == 1);
  DOBA_EXPECT(peer.socket.send_all(std::string_view(encrypted.data(), size)));
  DOBA_EXPECT(state->entered.wait_for());
  state->allow_resume = true;
  state->ready();
  DOBA_EXPECT(peer.receive_close());
  DOBA_EXPECT_EQUAL(state->resumed.load(), 1);
  peer.socket.close();
  server.stop();
}
#endif
