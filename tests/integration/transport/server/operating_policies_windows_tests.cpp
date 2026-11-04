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

#include "protocol/processing_result.h"
#include "../../../helpers/concurrency.h"
#include "http_server.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "wait_count.h"
#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

namespace {
using martianlabs::doba::protocol::processing_result;
using namespace martianlabs::doba;
using namespace martianlabs::doba::tests::integration;
using namespace std::chrono_literals;
using protocol::http::v11::request;
using protocol::http::v11::response;
constexpr std::string_view get = "GET / HTTP/1.1\r\nHost: a\r\n\r\n";

struct echo_engine {
  using policies_type = int;
  protocol::send_delegate send;
  void set_on_send(protocol::send_delegate callback) {
    send = std::move(callback);
  }
  void set_on_close(std::function<void()>) {}
  processing_result on_bytes_received(std::string_view bytes) {
    send(bytes, {}, nullptr);
    return {};
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool) { return {}; }

};

struct plain_peer {
  tcpip_client socket;
  bool connect(std::uint16_t port) { return socket.connect(port); }
  bool send(std::string_view wire) { return socket.send_all(wire); }
  auto receive(std::size_t size) { return socket.receive(size); }
};

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

}  // namespace

DOBA_TEST("TCP policies enforce global admission and recover on restart") {
  admission<http_server<>, plain_peer>(transport::server::tcp_policies{});
}
DOBA_TEST("TCP admission releases its reservation on engine factory failure") {
  factory_failure<transport::server::tcp, plain_peer>(
      transport::server::tcp_policies{});
}
