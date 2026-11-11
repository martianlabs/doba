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
#include <cstddef>
#include <thread>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "http_server.h"
#include "protocol/http/common/auth/basic.h"
#include "protocol/http/common/auth/bearer.h"
#include "common/task.h"
#include "http_test_helper.h"
#include "tcpip_client.h"
#include "test_helper.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] socket workers                                           ( constant ) |
// +---------------------------------------------------------------------------+
// | Keep connection state separate on both platforms.                         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
constexpr std::size_t kSocketWorkers = 2;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] general                                                    ( usings ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::http_server;
using martianlabs::doba::common::task;
using martianlabs::doba::tests::integration::receive_http_response;
using martianlabs::doba::tests::integration::tcpip_client;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] text_response                                            ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void text_response(response& res, std::string_view text) {
  res.ok_200();
  res.set_body(text);
}

}  // namespace

// +===========================================================================+
// | [>] route precedence over a socket                          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP/1.1 applies static parametrized and wildcard precedence") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  http_server<> http_server({.ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route("GET", "/items/*", [](const request&, response& res) {
    text_response(res, "wildcard");
  });
  http_server.add_route("GET", "/items/:id",
                        [](const request&, response& res, int id) {
                          text_response(res, "parameter:" + std::to_string(id));
                        });
  http_server.add_route("GET", "/items/42", [](const request&, response& res) {
    text_response(res, "static");
  });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  for (const auto& [path, body] :
       {std::pair{"/items/42", "static"}, std::pair{"/items/7", "parameter:7"},
        std::pair{"/items/name", "wildcard"}}) {
    DOBA_EXPECT(client.send_all(std::string("GET ") + path +
                                " HTTP/1.1\r\nHost: a\r\n\r\n"));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    if (result.has_value()) {
      DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
      DOBA_EXPECT_EQUAL(result->body, body);
    }
  }
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  http_server.stop();
}

// +===========================================================================+
// | [>] typed route conversion boundaries                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP/1.1 routes typed parameter boundaries without partial parses") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  http_server<> http_server({.ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route(
      "GET", "/signed/:value",
      [&calls](const request&, response& res, std::int64_t value) {
        calls.fetch_add(1);
        text_response(res, std::to_string(value));
      });
  http_server.add_route(
      "GET", "/unsigned/:value",
      [&calls](const request&, response& res, std::uint64_t value) {
        calls.fetch_add(1);
        text_response(res, std::to_string(value));
      });
  http_server.add_route("GET", "/boolean/:value",
                        [&calls](const request&, response& res, bool value) {
                          calls.fetch_add(1);
                          text_response(res, value ? "true" : "false");
                        });
  http_server.start();
  struct test_case {
    std::string_view path;
    std::string_view status;
    std::string_view body;
  };
  constexpr test_case cases[] = {
      {"/signed/-9223372036854775808", "HTTP/1.1 200 OK",
       "-9223372036854775808"},
      {"/signed/9223372036854775807", "HTTP/1.1 200 OK", "9223372036854775807"},
      {"/unsigned/18446744073709551615", "HTTP/1.1 200 OK",
       "18446744073709551615"},
      {"/boolean/TRUE", "HTTP/1.1 200 OK", "true"},
      {"/boolean/0", "HTTP/1.1 200 OK", "false"},
      {"/signed/9223372036854775808", "HTTP/1.1 404 Not Found", ""},
      {"/signed/42x", "HTTP/1.1 404 Not Found", ""},
      {"/unsigned/-1", "HTTP/1.1 404 Not Found", ""},
      {"/boolean/yes", "HTTP/1.1 404 Not Found", ""},
  };
  for (const auto& test : cases) {
    martianlabs::doba::tests::integration::test_helper::set_context(test.path);
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("GET " + std::string(test.path) +
                                " HTTP/1.1\r\nHost: a\r\n\r\n"));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    if (result.has_value()) {
      DOBA_EXPECT_EQUAL(result->status, test.status);
      DOBA_EXPECT_EQUAL(result->body, test.body);
    }
    client.close();
  }
  DOBA_EXPECT_EQUAL(calls.load(), 5);
  http_server.stop();
}

// +===========================================================================+
// | [>] request views reach a routed handler                    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP/1.1 exposes decoded path query headers and cookies to routes") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  http_server<> http_server({.ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route("GET", "/a b", [](const request& req, response& res) {
    const auto query = req.get_query_parameter("key");
    const auto empty = req.get_query_parameter("empty");
    const auto cookie = req.get_cookie("sid");
    const bool valid = req.get_absolute_path() == "/a b" && query.has_value() &&
                       query->second == "one%20two" && empty.has_value() &&
                       empty->second.empty() &&
                       req.get_header("x-id").second == "value" &&
                       cookie.has_value() && *cookie == "abc=123";
    text_response(res, valid ? "valid" : "invalid");
  });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /a%20b?key=one%20two&empty HTTP/1.1\r\n"
      "Host: a\r\nX-Id: value\r\nCookie: sid=abc=123; other=x\r\n\r\n"));
  const auto result = receive_http_response(client);
  DOBA_EXPECT(result.has_value());
  if (result.has_value()) {
    DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(result->body, "valid");
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] allowed methods include every route kind                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP/1.1 reports allowed methods across all matching route kinds") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  http_server<> http_server({.ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route(
      "GET", "/assets/logo",
      [](const request&, response& res) { text_response(res, "get"); });
  http_server.add_route(
      "POST", "/assets/:id",
      [](const request&, response& res, int) { text_response(res, "post"); });
  http_server.add_route(
      "DELETE", "/assets/*",
      [](const request&, response& res) { text_response(res, "delete"); });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("PUT /assets/42 HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto result = receive_http_response(client);
  DOBA_EXPECT(result.has_value());
  if (result.has_value()) {
    DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 405 Method Not Allowed");
    DOBA_EXPECT_EQUAL(result->header("Allow").value(), "POST, DELETE");
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] running server rejects route mutation                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST(
    "HTTP/1.1 keeps routes immutable while running and reusable after stop") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  http_server<> http_server({.ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route("GET", "/first", [](const request&, response& res) {
    text_response(res, "first");
  });
  http_server.start();
  bool threw = false;
  try {
    http_server.add_route("GET", "/late", [](const request&, response& res) {
      text_response(res, "late");
    });
  } catch (const std::runtime_error&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("GET /first HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto first = receive_http_response(client);
  DOBA_EXPECT(first.has_value());
  if (first.has_value()) DOBA_EXPECT_EQUAL(first->body, "first");
  client.close();
  http_server.stop();
  http_server.add_route("GET", "/late", [](const request&, response& res) {
    text_response(res, "late");
  });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("GET /late HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto late = receive_http_response(client);
  DOBA_EXPECT(late.has_value());
  if (late.has_value()) DOBA_EXPECT_EQUAL(late->body, "late");
  http_server.stop();
}

#ifdef _WIN32
DOBA_TEST("HTTP/1.1 authenticates Basic and Bearer over persistent TCP") {
  namespace doba = martianlabs::doba;
  namespace auth = doba::protocol::http::auth;
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<int> calls{0};
  doba::http_server<request, response,
                    doba::protocol::http::router<request, response>,
                    doba::protocol::http::v11::engine<request, response>,
                    doba::transport::tcp, auth::basic, auth::bearer>
      server({.ip = "127.0.0.1", .port = std::to_string(port)}, {},
             std::tuple{auth::basic{"demo",
                                    [](auto user, auto password) {
                                      return user == "alice" &&
                                             password == "secret";
                                    }},
                        auth::bearer{"demo", [](auto token) {
                                       return token == "demo-token";
                                     }}});
  server.add_route<auth::basic>("POST", "/basic",
      [&](const request&, response& res) {
        calls++;
        text_response(res, "basic");
      });
  server.add_route<auth::bearer>("GET", "/bearer",
      [&](const request&, response& res) -> task<void> {
        calls++;
        text_response(res, "bearer");
        co_return;
      });
  server.add_route("GET", "/public", [](const request&, response& res) {
    text_response(res, "public");
  });
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /basic HTTP/1.1\r\nHost: localhost\r\n"
      "Content-Length: 4\r\n\r\ndata"
      "GET /public HTTP/1.1\r\nHost: localhost\r\n\r\n"));
  const auto rejected = receive_http_response(client);
  const auto public_response = receive_http_response(client);
  DOBA_EXPECT(rejected.has_value());
  DOBA_EXPECT(public_response.has_value());
  if (rejected) {
    DOBA_EXPECT_EQUAL(rejected->status, "HTTP/1.1 401 Unauthorized");
    DOBA_EXPECT(rejected->header("WWW-Authenticate") ==
                "Basic realm=\"demo\"");
  }
  if (public_response) DOBA_EXPECT_EQUAL(public_response->body, "public");
  DOBA_EXPECT_EQUAL(calls.load(), 0);
  DOBA_EXPECT(client.send_all(
      "POST /basic HTTP/1.1\r\nHost: localhost\r\n"
      "Authorization: Basic YWxpY2U6c2VjcmV0\r\n"
      "Content-Length: 4\r\n\r\ndata"
      "GET /bearer HTTP/1.1\r\nHost: localhost\r\n"
      "Authorization: Bearer demo-token\r\n\r\n"
      "GET /bearer HTTP/1.1\r\nHost: localhost\r\n"
      "Authorization: Bearer wrong\r\n\r\n"));
  const auto basic = receive_http_response(client);
  const auto bearer = receive_http_response(client);
  const auto invalid = receive_http_response(client);
  DOBA_EXPECT(basic.has_value());
  DOBA_EXPECT(bearer.has_value());
  DOBA_EXPECT(invalid.has_value());
  if (basic) DOBA_EXPECT_EQUAL(basic->body, "basic");
  if (bearer) DOBA_EXPECT_EQUAL(bearer->body, "bearer");
  if (invalid) {
    DOBA_EXPECT_EQUAL(invalid->status, "HTTP/1.1 401 Unauthorized");
    DOBA_EXPECT(invalid->header("WWW-Authenticate") ==
        "Bearer realm=\"demo\", error=\"invalid_token\"");
  }
  DOBA_EXPECT_EQUAL(calls.load(), 2);
  server.stop();
}


#endif

DOBA_TEST("HTTP/1.1 shares a Bearer provider across concurrent connections") {
  namespace doba = martianlabs::doba;
  namespace auth = doba::protocol::http::auth;
  tcpip_client probe;
  const uint16_t port = probe.find_available_port();
  std::atomic<int> checks{0};
  std::atomic<int> received{0};
  doba::http_server<request, response,
                    doba::protocol::http::router<request, response>,
                    doba::protocol::http::v11::engine<request, response>,
                    doba::transport::tcp, auth::bearer>
      server(
          {.worker_count = kSocketWorkers, .ip = "127.0.0.1",
           .port = std::to_string(port)},
          {}, std::tuple{auth::bearer{"demo", [&](auto token) {
                                        checks++;
                                        return token == "demo-token";
                                      }}});
  server.add_route<auth::bearer>("GET", "/",
      [](const request&, response& res) { text_response(res, "accepted"); });
  server.start();
  std::vector<std::jthread> clients;
  for (int i = 0; i < 16; i++) {
    clients.emplace_back([&]() {
      tcpip_client client;
      if (!client.connect(port)) return;
      if (!client.send_all(
          "GET / HTTP/1.1\r\nHost: localhost\r\n"
          "Authorization: Bearer demo-token\r\n\r\n")) return;
      const auto result = receive_http_response(client);
      if (result && result->body == "accepted") received++;
    });
  }
  clients.clear();
  server.stop();
  DOBA_EXPECT_EQUAL(checks.load(), 16);
  DOBA_EXPECT_EQUAL(received.load(), 16);
}
