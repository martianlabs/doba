//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses-2.0
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


#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <coroutine>
#include <filesystem>
#include <future>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "platform.h"
#include <openssl/ssl.h>

#include "protocol/http/v11/server.h"
#include "common/task.h"
#include "tcpip_client.h"
#include "tls_client.h"
#include "tls_server_policies.h"
#include "test_helper.h"
#include "transport/server/tls.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] general                                                    ( usings ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::protocol::http::v11::server;
using martianlabs::doba::common::task;
using martianlabs::doba::tests::integration::server_policies;
using martianlabs::doba::tests::integration::tcpip_client;
using martianlabs::doba::tests::integration::tls_client;
using martianlabs::doba::transport::server::tls;
using martianlabs::doba::transport::server::tls_policies;
using namespace std::chrono_literals;


// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] receive_response                                         ( function ) |
// +---------------------------------------------------------------------------+
// | This function receives an HTTP response from a TLS client. It reads the   |
// | response headers and body, ensuring that the response is valid and        |
// | complete. The function returns the body of the response as a string if    |
// | successful, or std::nullopt if there is an error or if the response is    |
// | incomplete. It checks for the "Content-Length" header to determine how    |
// | many bytes to read for the body.                                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
std::optional<std::string> receive_response(tls_client& client) {
  std::string wire;
  while (!wire.ends_with("\r\n\r\n")) {
    if (wire.size() >= 16384) return std::nullopt;
    const auto byte = client.receive(1);
    if (!byte) return std::nullopt;
    wire += *byte;
  }
  if (!wire.starts_with("HTTP/1.1 200 OK\r\n")) return std::nullopt;
  const auto field = wire.find("\r\nContent-Length: ");
  if (field == std::string::npos) return std::nullopt;
  const auto begin = field + sizeof("\r\nContent-Length: ") - 1;
  const auto end = wire.find("\r\n", begin);
  if (end == std::string::npos) return std::nullopt;
  std::size_t size = 0;
  const auto parsed =
      std::from_chars(wire.data() + begin, wire.data() + end, size);
  if (parsed.ec != std::errc{} || parsed.ptr != wire.data() + end ||
      size > 1024 * 1024)
    return std::nullopt;
  const auto body = client.receive(size);
  if (!body) return std::nullopt;
  return *body;
}

struct pause_awaiter {
  std::promise<std::coroutine_handle<>>& pending;

  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> handle) const {
    pending.set_value(handle);
  }
  void await_resume() const noexcept {}
};
}  // namespace

// +===========================================================================+
// | [>] tls serves an HTTP response                             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls serves HTTP with configured buffer capacities") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  auto configuration = server_policies(port);
  configuration.encrypted_receive_buffer_size = 8192;
  configuration.network_bio_buffer_size = 4096;
  server<request, response,
         martianlabs::doba::protocol::http::router<request, response>,
         martianlabs::doba::protocol::http::v11::engine<request, response>, tls>
      http_server(configuration);
  http_server.add_route("GET", "/ready", [](const request&, response& res) {
    res.ok_200();
    res.set_body("ready");
    return;
  });
  http_server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  DOBA_EXPECT(client.send_outgoing(
      "GET /ready HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto received = receive_response(client);
  DOBA_EXPECT(received.has_value());
  DOBA_EXPECT_EQUAL(*received, "ready");
  client.socket.close();
  http_server.stop();
}

// +===========================================================================+
// | [>] tls preserves a fragmented binary HTTP body             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls preserves a fragmented binary HTTP body") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  server<request, response,
         martianlabs::doba::protocol::http::router<request, response>,
         martianlabs::doba::protocol::http::v11::engine<request, response>, tls>
      http_server(server_policies(port));
  http_server.add_route("POST", "/echo", [](const request& req, response& res) {
    if (!req.has_body_reader()) {
      res.bad_request_400();
      return;
    }
    std::array<std::byte, 1024> buffer{};
    std::string body;
    for (int i = 0; i < 32; ++i) {
      const auto state = req.get_body_reader()->read(buffer);
      if (state.has_error) {
        res.bad_request_400();
        return;
      }
      body.append(reinterpret_cast<const char*>(buffer.data()), state.produced);
      if (state.complete) {
        res.ok_200();
        res.set_body(body);
        return;
      }
    }
    {
      res.bad_request_400();
      return;
    }
  });
  http_server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  std::string body(513, '\0');
  for (std::size_t i = 0; i < body.size(); ++i) {
    body[i] = static_cast<char>(i % 251);
  }
  DOBA_EXPECT(client.send_outgoing(
      "POST /echo HTTP/1.1\r\nHost: example.com\r\n"));
  DOBA_EXPECT(client.send_outgoing("Content-Length: 513\r\n\r\n"));
  for (std::size_t i = 0; i < body.size(); i += 17) {
    DOBA_EXPECT(client.send_outgoing(std::string_view(body).substr(i, 17)));
  }
  const auto received = receive_response(client);
  DOBA_EXPECT(received.has_value());
  DOBA_EXPECT_EQUAL(*received, body);
  client.socket.close();
  http_server.stop();
}

// +===========================================================================+
// | [>] tls orders HTTP responses before closing                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls serves sequential HTTP requests before closing") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  server<request, response,
         martianlabs::doba::protocol::http::router<request, response>,
         martianlabs::doba::protocol::http::v11::engine<request, response>, tls>
      http_server(server_policies(port));
  http_server.add_route("GET", "/one", [](const request&, response& res) {
    res.ok_200();
    res.set_body("one");
    return;
  });
  http_server.add_route("GET", "/two", [](const request&, response& res) {
    res.ok_200();
    res.set_body("two");
    return;
  });
  http_server.add_route("GET", "/three", [](const request&, response& res) {
    res.ok_200();
    res.set_body("three");
    return;
  });
  http_server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  DOBA_EXPECT(client.send_outgoing(
      "GET /one HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto first = receive_response(client);
  DOBA_EXPECT(first.has_value());
  DOBA_EXPECT_EQUAL(*first, "one");
  DOBA_EXPECT(client.send_outgoing(
      "GET /two HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto second = receive_response(client);
  DOBA_EXPECT(second.has_value());
  DOBA_EXPECT_EQUAL(*second, "two");
  DOBA_EXPECT(
      client.send_outgoing("GET /three HTTP/1.1\r\nHost: example.com\r\n"
                  "Connection: close\r\n\r\n"));
  const auto third = receive_response(client);
  DOBA_EXPECT(third.has_value());
  DOBA_EXPECT_EQUAL(*third, "three");
  DOBA_EXPECT(client.receive_close());
  client.socket.close();
  http_server.stop();
}

DOBA_TEST("tls orders asynchronous and synchronous HTTP responses") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  std::promise<std::coroutine_handle<>> pending;
  auto suspended = pending.get_future();
  server<request, response,
         martianlabs::doba::protocol::http::router<request, response>,
         martianlabs::doba::protocol::http::v11::engine<request, response>, tls>
      http_server(server_policies(port));
  http_server.add_route("GET", "/slow",
                        [&pending](const request&, response& res)
                            -> task<void> {
                          co_await pause_awaiter{pending};
                          res.ok_200().set_body("slow");
                        });
  http_server.add_route("GET", "/fast", [](const request&, response& res) {
    res.ok_200().set_body("fast");
  });
  http_server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  DOBA_EXPECT(client.send_outgoing(
      "GET /slow HTTP/1.1\r\nHost: example.com\r\n\r\n"
      "GET /fast HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  if (suspended.wait_for(std::chrono::seconds(2)) !=
      std::future_status::ready) {
    throw std::runtime_error("asynchronous route did not suspend");
  }
  suspended.get().resume();
  const auto first = receive_response(client);
  const auto second = receive_response(client);
  DOBA_EXPECT(first.has_value());
  DOBA_EXPECT(second.has_value());
  if (first && second) {
    DOBA_EXPECT_EQUAL(*first, "slow");
    DOBA_EXPECT_EQUAL(*second, "fast");
  }
  client.socket.close();
  http_server.stop();
}
