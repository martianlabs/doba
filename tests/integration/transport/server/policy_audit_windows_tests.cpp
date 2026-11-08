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
#include <vector>
#include "http_server.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "wait_count.h"
#include "../../../helpers/concurrency.h"

namespace {
using namespace martianlabs::doba;
using namespace martianlabs::doba::tests::integration;
using namespace std::chrono_literals;
using protocol::http::v11::request;
using protocol::http::v11::response;
constexpr std::string_view get = "GET / HTTP/1.1\r\nHost: a\r\n\r\n";
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
template <class Peer>
bool reply(Peer& peer, std::string_view expected) {
  std::string head;
  while (!head.ends_with("\r\n\r\n") && head.size() < 4096) {
    auto byte = peer.receive(1);
    if (!byte) {
      test_helper::set_context(peer.socket.error());
      return false;
    }
    head += *byte;
  }
  if (!head.starts_with("HTTP/1.1 200")) {
    test_helper::set_context(head);
    return false;
  }
  while (!expected.empty()) {
    auto size = std::min<std::size_t>(65536, expected.size());
    auto bytes = peer.receive(size);
    if (!bytes) {
      test_helper::set_context(peer.socket.error());
      return false;
    }
    if (*bytes != expected.substr(0, size)) {
      test_helper::set_context("Unexpected response body bytes");
      return false;
    }
    expected.remove_prefix(size);
  }
  return true;
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
    Server server(p);
    p.max_connections = maximum ? 0 : 1;
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
void terminal_source(Policies p) {
  tcpip_client probe;
  const auto port = probe.find_available_port();
  p.ip = "127.0.0.1";
  p.port = std::to_string(port);
  p.worker_count = 2;
  protocol::http::v11::policies limits;
  // Keep the next request as raw input until this reader drains.
  limits.max_pending_requests = 1;
  limits.max_pending_response_bytes = 1;
  auto bytes = std::make_shared<const std::string>(1024 * 1024, 's');
  std::atomic<int> calls{0};
  Server server(p, limits);
  server.add_route("GET", "/", [&](const request&, response& res) {
    ++calls;
    res.ok_200().set_body(
        common::reader::shared(bytes, std::as_bytes(std::span(*bytes))),
        bytes->size());
  });
  server.start();
  Peer peer;
  DOBA_EXPECT(peer.connect(port));
  DOBA_EXPECT(peer.send(get));
  DOBA_EXPECT(wait_count(calls, 1));
  // Stream the body through bounded TX blocks; don't charge its full length.
  DOBA_EXPECT(reply(peer, *bytes));
  DOBA_EXPECT(peer.send(get));
  DOBA_EXPECT(reply(peer, *bytes));
  DOBA_EXPECT_EQUAL(calls.load(), 2);
  server.stop();
}

}  // namespace

DOBA_TEST("TCP connection zero and policy snapshots apply to a real listener") {
  connection_zero_and_snapshot<http_server<>, plain_peer>(
      transport::server::tcp_policies{});
}
DOBA_TEST(
    "TCP streams one reader and accepts the next request after it drains") {
  terminal_source<http_server<>, plain_peer>(transport::server::tcp_policies{});
}
