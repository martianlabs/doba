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

#include "../../../helpers/concurrency.h"
#include "common/task.h"
#include "http_server.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "wait_count.h"
#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
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

struct echo_engine {
  using policies_type = int;
  protocol::send_delegate send;
  void set_on_send(protocol::send_delegate callback) { send = std::move(callback); }
  void set_on_close(std::function<void()>) {}
  void on_bytes_received(std::string_view bytes) { send(bytes, {}, nullptr); }
};

struct paused_engine {
  using policies_type = int;
  std::atomic<int>* received;
  std::atomic<bool> paused{false};
  void set_on_send(protocol::send_delegate) {}
  void set_on_close(std::function<void()>) {}
  void on_bytes_received(std::string_view) { paused = true; ++*received; }
  bool input_paused() const { return paused.load(); }
  void resume_input() {}
};

struct plain_peer {
  tcpip_client socket;
  bool connect(std::uint16_t port) { return socket.connect(port); }
  bool send(std::string_view wire) { return socket.send_all(wire); }
  auto receive(std::size_t size) { return socket.receive(size); }
};
#ifdef DOBA_ENABLE_TLS
struct secure_peer {
  tls_client client;
  tcpip_client &socket = client.socket;
  bool connect(std::uint16_t port) {
    return socket.connect(port) && client.negotiate();
  }
  bool send(std::string_view wire) { return client.send_outgoing(wire); }
  auto receive(std::size_t size) { return client.receive(size); }
};
#endif

template <class Peer> bool reply(Peer &peer, std::string_view expected) {
  std::string head;
  while (!head.ends_with("\r\n\r\n") && head.size() < 4096) {
    auto byte = peer.receive(1);
    if (!byte)
      return false;
    head += *byte;
  }
  if (!head.starts_with("HTTP/1.1 200"))
    return false;
  while (!expected.empty()) {
    const auto size = std::min<std::size_t>(65536, expected.size());
    auto body = peer.receive(size);
    if (!body || *body != expected.substr(0, size))
      return false;
    expected.remove_prefix(size);
  }
  return true;
}
bool closed(tcpip_client &client) {
  return client.wait_for_close(3s) || client.connection_reset();
}

template <template <class, class> class Transport, class Peer, class Policies>
void factory_failure(Policies p) {
  tcpip_client failed;
  const auto port = failed.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.max_connections = 1;
  std::atomic<int> attempts{0}, connected{0}, disconnected{0};
  auto factory = [&]() -> echo_engine {
    if (attempts.fetch_add(1) == 0) throw std::runtime_error("factory failed");
    return {};
  };
  Transport<echo_engine, decltype(factory)> server(p, factory);
  server.set_on_connection([&] { ++connected; });
  server.set_on_disconnection([&] { ++disconnected; });
  server.start();
  DOBA_EXPECT(failed.connect(port));
  DOBA_EXPECT(closed(failed));
  Peer healthy;
  DOBA_EXPECT(healthy.connect(port));
  DOBA_EXPECT(healthy.send("ok"));
  const auto result = healthy.receive(2);
  DOBA_EXPECT(result && *result == "ok");
  server.stop();
  DOBA_EXPECT_EQUAL(attempts.load(), 2);
  DOBA_EXPECT_EQUAL(connected.load(), 1);
  DOBA_EXPECT_EQUAL(disconnected.load(), 1);
}

template <template <class, class> class Transport, class Peer, class Policies>
void idle_admission(Policies p) {
  tcpip_client silent;
  const auto port = silent.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.max_connections = 1;
  p.inactivity_period = 300ms;
  std::atomic<int> created{0}, connected{0}, disconnected{0};
  auto factory = [&] { ++created; return echo_engine{}; };
  Transport<echo_engine, decltype(factory)> server(p, factory);
  server.set_on_connection([&] { ++connected; });
  server.set_on_disconnection([&] { ++disconnected; });
  server.start();
  DOBA_EXPECT(silent.connect(port));
  DOBA_EXPECT(wait_count(created, 1));
  tcpip_client excess;
  DOBA_EXPECT(excess.connect(port));
  DOBA_EXPECT(closed(excess));
  DOBA_EXPECT_EQUAL(created.load(), 1);
  DOBA_EXPECT(closed(silent));
  bool recovered = false;
  const auto deadline = std::chrono::steady_clock::now() + 3s;
  while (!recovered && std::chrono::steady_clock::now() < deadline) {
    Peer healthy;
    if (healthy.connect(port) && healthy.send("ok")) {
      auto bytes = healthy.receive(2);
      recovered = bytes && *bytes == "ok";
    }
  }
  DOBA_EXPECT(recovered);
  server.stop();
  DOBA_EXPECT_EQUAL(created.load(), 2);
  constexpr bool secure = requires(Peer& peer) { peer.client; };
  DOBA_EXPECT_EQUAL(connected.load(), secure ? 1 : 2);
  DOBA_EXPECT_EQUAL(disconnected.load(), secure ? 1 : 2);
}

template <template <class, class> class Transport, class Peer, class Policies>
void paused_expiry(Policies p) {
  tcpip_client probe;
  const auto port = probe.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.inactivity_period = 300ms;
  std::atomic<int> received{0}, connected{0}, disconnected{0};
  auto factory = [&] { return paused_engine{&received}; };
  Transport<paused_engine, decltype(factory)> server(p, factory);
  server.set_on_connection([&] { ++connected; });
  server.set_on_disconnection([&] { ++disconnected; });
  server.start();
  Peer peer;
  DOBA_EXPECT(peer.connect(port));
  DOBA_EXPECT(peer.send("pause"));
  DOBA_EXPECT(wait_count(received, 1));
  DOBA_EXPECT(!peer.receive(1));
  DOBA_EXPECT(wait_count(disconnected, 1));
  server.stop();
  DOBA_EXPECT_EQUAL(connected.load(), 1);
  DOBA_EXPECT_EQUAL(disconnected.load(), 1);
}

template <class Server, class Peer, class Policies> void admission(Policies p) {
  tcpip_client probe;
  const auto port = probe.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.max_connections = 2;
  p.worker_count = 4;
  Server server(p);
  server.add_route("GET", "/", [](const request &, response &r) {
    r.ok_200().set_body("ok");
  });
  server.start();
  Peer one, two;
  DOBA_EXPECT(one.connect(port));
  DOBA_EXPECT(one.send(get));
  DOBA_EXPECT(reply(one, "ok"));
  DOBA_EXPECT(two.connect(port));
  DOBA_EXPECT(two.send(get));
  DOBA_EXPECT(reply(two, "ok"));
  tcpip_client excess;
  DOBA_EXPECT(excess.connect(port));
  DOBA_EXPECT(closed(excess));
  std::atomic<int> rejected{0};
  std::vector<std::thread> burst;
  for (int i = 0; i < 8; ++i)
    burst.emplace_back([&] {
      tcpip_client client;
      if (client.connect(port) && closed(client))
        ++rejected;
    });
  for (auto &thread : burst)
    thread.join();
  DOBA_EXPECT_EQUAL(rejected.load(), 8);
  // A separate instance still admits while this instance has no credit.
  auto independent_configuration = p;
  const auto independent_port = probe.find_available_port();
  independent_configuration.port = std::to_string(independent_port);
  independent_configuration.max_connections = 1;
  Server independent(independent_configuration);
  independent.add_route("GET", "/", [](const request &, response &r) {
    r.ok_200().set_body("ok");
  });
  independent.start();
  Peer separate;
  DOBA_EXPECT(separate.connect(independent_port));
  DOBA_EXPECT(separate.send(get));
  DOBA_EXPECT(reply(separate, "ok"));
  independent.stop();
  DOBA_EXPECT(one.send(get));
  DOBA_EXPECT(reply(one, "ok"));
  two.socket.close();
  bool recovered = false;
  const auto deadline = std::chrono::steady_clock::now() + 3s;
  while (!recovered && std::chrono::steady_clock::now() < deadline) {
    Peer next;
    recovered = next.connect(port) && next.send(get) && reply(next, "ok");
  }
  DOBA_EXPECT(recovered);
  server.stop();
  server.start();
  Peer next;
  DOBA_EXPECT(next.connect(port));
  DOBA_EXPECT(next.send(get));
  DOBA_EXPECT(reply(next, "ok"));
  server.stop();
}

template <class Server, class Peer, class Policies>
void inactivity(Policies p) {
  tcpip_client probe;
  const auto port = probe.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.inactivity_period = 300ms;
  p.worker_count = 2;
  Server server(p);
  server.add_route("GET", "/", [](const request &, response &r) {
    r.ok_200().set_body("ok");
  });
  server.start();
  tcpip_client silent;
  DOBA_EXPECT(silent.connect(port));
  DOBA_EXPECT(closed(silent));
  Peer progressing;
  DOBA_EXPECT(progressing.connect(port));
  // Total duration exceeds the period, but every fragment renews it.
  for (auto fragment : {"GET ", "/ ", "HTTP/1.1\r\n", "Host: a\r\n"}) {
    DOBA_EXPECT(progressing.send(fragment));
    std::this_thread::sleep_for(100ms);
  }
  DOBA_EXPECT(progressing.send("\r\n"));
  DOBA_EXPECT(reply(progressing, "ok"));
  DOBA_EXPECT(closed(progressing.socket));
  Peer partial;
  DOBA_EXPECT(partial.connect(port));
  DOBA_EXPECT(partial.send("GET /"));
  DOBA_EXPECT(closed(partial.socket));
  server.stop();
  p.inactivity_period = 0ms;
  Server unlimited(p);
  unlimited.add_route("GET", "/", [](const request &, response &r) {
    r.ok_200().set_body("ok");
  });
  unlimited.start();
  Peer alive;
  DOBA_EXPECT(alive.connect(port));
  std::this_thread::sleep_for(650ms);
  DOBA_EXPECT(alive.send(get));
  DOBA_EXPECT(reply(alive, "ok"));
  unlimited.stop();
}

template <class Server, class Peer, class Policies>
void pipeline(Policies p, bool fragmented = false) {
  tcpip_client probe;
  const auto port = probe.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.worker_count = 2;
  p.max_buffered_send_bytes = 8192;
  protocol::http::v11::policies hp;
  hp.max_pending_requests = 4;
  tests::manual_event gate;
  std::atomic<int> entered{0};
  Server server(p, hp);
  server.add_route("GET", "/",
                   [&](const request &, response &r) -> common::task<void> {
                     if (entered.fetch_add(1) == 0)
                       co_await gate;
                     r.ok_200().set_body(std::string(512, 'x'));
                     co_return;
                   });
  server.start();
  tests::scope_exit cleanup([&] {
    gate.open();
    server.stop();
  });
  Peer peer;
  DOBA_EXPECT(peer.connect(port));
  std::string batch;
  for (int i = 0; i < 20; ++i)
    batch += get;
  if (fragmented) {
    DOBA_EXPECT(peer.send(std::string_view(batch).substr(0, get.size() * 2)));
    DOBA_EXPECT(gate.wait_for_waiters(1));
    DOBA_EXPECT(peer.send(std::string_view(batch).substr(get.size() * 2)));
  } else {
    DOBA_EXPECT(peer.send(batch));
  }
  DOBA_EXPECT(gate.wait_for_waiters(1));
  DOBA_EXPECT(wait_count(entered, 3));
  // No fourth handler is admitted at the high watermark, even in this block.
  DOBA_EXPECT_EQUAL(entered.load(), 3);
  gate.open();
  for (int i = 0; i < 20; ++i)
    DOBA_EXPECT(reply(peer, std::string(512, 'x')));
  DOBA_EXPECT_EQUAL(entered.load(), 20);
}

template <class Server, class Peer, class Policies>
void retained_work(Policies p) {
  tcpip_client probe;
  const auto port = probe.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.max_connections = 1;
  p.inactivity_period = 150ms;
  tests::manual_event gate;
  std::atomic<int> entered{0};
  Server server(p);
  server.add_route("GET", "/",
                   [&](const request &, response &r) -> common::task<void> {
                     if (entered.fetch_add(1) == 0)
                       co_await gate;
                     r.ok_200().set_body("ok");
                     co_return;
                   });
  server.start();
  tests::scope_exit cleanup([&] {
    gate.open();
    server.stop();
  });
  Peer pending;
  DOBA_EXPECT(pending.connect(port));
  DOBA_EXPECT(pending.send(get));
  DOBA_EXPECT(gate.wait_for_waiters(1));
  DOBA_EXPECT(closed(pending.socket));
  tcpip_client excess;
  DOBA_EXPECT(excess.connect(port));
  DOBA_EXPECT(closed(excess));
  DOBA_EXPECT_EQUAL(entered.load(), 1);
  gate.open();
  bool recovered = false;
  const auto deadline = std::chrono::steady_clock::now() + 3s;
  while (!recovered && std::chrono::steady_clock::now() < deadline) {
    Peer next;
    recovered = next.connect(port) && next.send(get) && reply(next, "ok");
  }
  DOBA_EXPECT(recovered);
}

template <class Server, class Peer, class Policies>
void blocked_source(Policies p, bool expire) {
  tcpip_client probe;
  const auto port = probe.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.max_pending_sources = 1;
  p.worker_count = 2;
  p.inactivity_period = expire ? 300ms : 0ms;
  auto bytes = std::make_shared<const std::string>(16 * 1024 * 1024, 'z');
  std::weak_ptr<const std::string> retained;
  tests::manual_event created;
  std::atomic<int> small_calls{0};
  Server server(p);
  server.add_route("GET", "/large", [&](const request &, response &r) {
    auto owner = expire ? std::make_shared<const std::string>(*bytes) : bytes;
    retained = owner;
    r.ok_200().set_body(
        common::reader::shared(owner, std::as_bytes(std::span(*owner))),
        owner->size());
    created.open();
  });
  server.add_route("GET", "/", [&](const request &, response &r) {
    ++small_calls;
    r.ok_200().set_body("ok");
  });
  server.start();
  Peer blocked;
  DOBA_EXPECT(blocked.socket.set_receive_buffer_size(16384));
  DOBA_EXPECT(blocked.connect(port));
  DOBA_EXPECT(
      blocked.send(std::string("GET /large HTTP/1.1\r\nHost: a\r\n\r\n") +
                   std::string(get)));
  DOBA_EXPECT(created.wait_for());
  DOBA_EXPECT(!retained.expired());
  // Source credit pauses the request already in this receive block.
  Peer healthy;
  DOBA_EXPECT(healthy.connect(port));
  DOBA_EXPECT(healthy.send(get));
  DOBA_EXPECT(reply(healthy, "ok"));
  DOBA_EXPECT_EQUAL(small_calls.load(), 1);
  DOBA_EXPECT(!retained.expired());
  if (expire) {
    const auto deadline = std::chrono::steady_clock::now() + 4s;
    while (!retained.expired() && std::chrono::steady_clock::now() < deadline)
      std::this_thread::yield();
    DOBA_EXPECT(retained.expired());
    DOBA_EXPECT_EQUAL(small_calls.load(), 1);
  } else {
    DOBA_EXPECT(reply(blocked, *bytes));
    DOBA_EXPECT(reply(blocked, "ok"));
    DOBA_EXPECT_EQUAL(small_calls.load(), 2);
  }
  server.stop();
}

template <class Server, class Peer, class Policies>
void pending_overflow(Policies p) {
  tcpip_client probe;
  const auto port = probe.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  protocol::http::v11::policies hp;
  hp.max_pending_response_bytes = 512;
  tests::manual_event gate;
  std::atomic<int> dispatched{0};
  Server server(p, hp);
  server.add_route("GET", "/",
                   [&](const request &, response &r) -> common::task<void> {
                     if (dispatched.fetch_add(1) == 0)
                       co_await gate;
                     r.ok_200().set_body(std::string(1024, 'x'));
                     co_return;
                   });
  server.start();
  tests::scope_exit cleanup([&] {
    gate.open();
    server.stop();
  });
  Peer peer;
  DOBA_EXPECT(peer.connect(port));
  DOBA_EXPECT(
      peer.send(std::string(get) + std::string(get) + std::string(get)));
  DOBA_EXPECT(gate.wait_for_waiters(1));
  // The second completed response cannot be retained behind the first.
  DOBA_EXPECT(!peer.receive(1).has_value());
  DOBA_EXPECT_EQUAL(dispatched.load(), 2);
  gate.open();
}
} // namespace

DOBA_TEST("TCP policies enforce global admission and recover on restart") {
  admission<http_server<>, plain_peer>(transport::server::tcp_policies{});
}
DOBA_TEST("TCP admission releases its sole reservation when engine construction fails") {
  factory_failure<transport::server::tcp, plain_peer>(transport::server::tcp_policies{});
}
DOBA_TEST("TCP idle connections occupy admission until expiry and notify once") {
  idle_admission<transport::server::tcp, plain_peer>(transport::server::tcp_policies{});
}
DOBA_TEST("TCP expiration notifies once when input is paused without output") {
  paused_expiry<transport::server::tcp, plain_peer>(transport::server::tcp_policies{});
}
DOBA_TEST(
    "TCP inactivity expires idle peers and preserves progressing fragments") {
  inactivity<http_server<>, plain_peer>(transport::server::tcp_policies{});
}
DOBA_TEST("TCP policies pause a received pipeline and resume ordered replies") {
  pipeline<http_server<>, plain_peer>(transport::server::tcp_policies{});
  pipeline<http_server<>, plain_peer>(transport::server::tcp_policies{}, true);
}
DOBA_TEST(
    "TCP admission retains expired connections until accepted work completes") {
  retained_work<http_server<>, plain_peer>(transport::server::tcp_policies{});
}
DOBA_TEST("TCP source pressure serves another client then resumes the blocked "
          "pipeline") {
  blocked_source<http_server<>, plain_peer>(transport::server::tcp_policies{},
                                            false);
}
DOBA_TEST("TCP inactivity releases a source retained by a nonreading peer") {
  blocked_source<http_server<>, plain_peer>(transport::server::tcp_policies{},
                                            true);
}
DOBA_TEST(
    "TCP pending response overflow closes and suppresses successor dispatch") {
  pending_overflow<http_server<>, plain_peer>(
      transport::server::tcp_policies{});
}
#ifdef DOBA_ENABLE_TLS
DOBA_TEST("TLS admission releases its sole reservation when engine construction fails") {
  factory_failure<transport::server::tls, secure_peer>(server_policies(1));
}
DOBA_TEST("TLS incomplete handshakes occupy admission until expiry without callbacks") {
  idle_admission<transport::server::tls, secure_peer>(server_policies(1));
}
DOBA_TEST("TLS expiration notifies once when input is paused without output") {
  paused_expiry<transport::server::tls, secure_peer>(server_policies(1));
}
DOBA_TEST("TLS handshake progresses while a small output BIO drains") {
  tls_client peer;
  const auto port = peer.socket.find_available_port();
  auto p = server_policies(port);
  p.max_buffered_send_bytes = 2048;
  https_server<> server(p);
  server.add_route("GET", "/", [](const request &, response &r) {
    r.ok_200().set_body("ok");
  });
  server.start();
  DOBA_EXPECT(peer.socket.connect(port));
  DOBA_EXPECT(peer.negotiate());
  DOBA_EXPECT(peer.send_outgoing(get));
  struct adapter {
    tls_client &client;
    auto receive(std::size_t size) { return client.receive(size); }
  } input{peer};
  DOBA_EXPECT(reply(input, "ok"));
  server.stop();
}
DOBA_TEST("TLS policies enforce admission before allocating handshake state") {
  admission<https_server<>, secure_peer>(server_policies(1));
}
DOBA_TEST("TLS inactivity expires handshakes idle peers and partial requests") {
  inactivity<https_server<>, secure_peer>(server_policies(1));
}
DOBA_TEST("TLS policies pause a received pipeline and resume ordered replies") {
  pipeline<https_server<>, secure_peer>(server_policies(1));
  pipeline<https_server<>, secure_peer>(server_policies(1), true);
}
DOBA_TEST(
    "TLS admission retains expired connections until accepted work completes") {
  retained_work<https_server<>, secure_peer>(server_policies(1));
}
DOBA_TEST("TLS source pressure serves another client then resumes the blocked "
          "pipeline") {
  blocked_source<https_server<>, secure_peer>(server_policies(1), false);
}
DOBA_TEST("TLS inactivity releases a source retained by a nonreading peer") {
  blocked_source<https_server<>, secure_peer>(server_policies(1), true);
}
DOBA_TEST(
    "TLS pending response overflow closes and suppresses successor dispatch") {
  pending_overflow<https_server<>, secure_peer>(server_policies(1));
}
#endif
