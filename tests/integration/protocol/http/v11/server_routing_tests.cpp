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
#include <cstdint>
#include <memory>
#include <stop_token>
#include <stdexcept>
#include <string>
#include <string_view>

#include "protocol/http/v11/server.h"
#include "http_test_helper.h"
#include "tcpip_client.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::protocol::http::v11::server;
using martianlabs::doba::common::task;
using martianlabs::doba::tests::integration::receive_http_response;
using martianlabs::doba::tests::integration::tcpip_client;

response text_response(std::string_view text) {
  response result = response::ok_200();
  result.set_body(text);
  return result;
}
}  // namespace

// +===========================================================================+
// | [>] route precedence over a socket                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 applies static parametrized and wildcard precedence") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  http_server.add_route("GET", "/items/*", [](const request&) {
    return text_response("wildcard");
  });
  http_server.add_route("GET", "/items/:id", [](const request&, int id) {
    return text_response("parameter:" + std::to_string(id));
  });
  http_server.add_route("GET", "/items/42", [](const request&) {
    return text_response("static");
  });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /items/42 HTTP/1.1\r\nHost: a\r\n\r\n"
      "GET /items/7 HTTP/1.1\r\nHost: a\r\n\r\n"
      "GET /items/name HTTP/1.1\r\nHost: a\r\n\r\n"));
  for (const std::string_view body : {"static", "parameter:7", "wildcard"}) {
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
// +===========================================================================+
DOBA_TEST("HTTP/1.1 routes typed parameter boundaries without partial parses") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server<> http_server;
  http_server.add_route(
      "GET", "/signed/:value",
      [&calls](const request&, std::int64_t value) {
        calls.fetch_add(1);
        return text_response(std::to_string(value));
      });
  http_server.add_route(
      "GET", "/unsigned/:value",
      [&calls](const request&, std::uint64_t value) {
        calls.fetch_add(1);
        return text_response(std::to_string(value));
      });
  http_server.add_route("GET", "/boolean/:value",
                        [&calls](const request&, bool value) {
    calls.fetch_add(1);
    return text_response(value ? "true" : "false");
  });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

  // +=========================================================================+
  // | [>] test_case                                                ( struct ) |
  // +=========================================================================+
  struct test_case {
    // +=======================================================================+
    // | [>] ATTRIBUTEs                                             ( public ) |
    // +=======================================================================+
    std::string_view path;
    std::string_view status;
    std::string_view body;
  };
  constexpr test_case cases[] = {
      {"/signed/-9223372036854775808", "HTTP/1.1 200 OK",
       "-9223372036854775808"},
      {"/signed/9223372036854775807", "HTTP/1.1 200 OK",
       "9223372036854775807"},
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
// +===========================================================================+
DOBA_TEST("HTTP/1.1 exposes decoded path query headers and cookies to routes") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  http_server.add_route("GET", "/a b", [](const request& req) {
    const auto query = req.get_query_parameter("key");
    const auto empty = req.get_query_parameter("empty");
    const auto cookie = req.get_cookie("sid");
    const bool valid = req.get_absolute_path() == "/a b" &&
                       query.has_value() && query->second == "one%20two" &&
                       empty.has_value() && empty->second.empty() &&
                       req.get_header("x-id").second == "value" &&
                       cookie.has_value() && *cookie == "abc=123";
    return text_response(valid ? "valid" : "invalid");
  });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

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
// +===========================================================================+
DOBA_TEST("HTTP/1.1 reports allowed methods across all matching route kinds") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  http_server.add_route("GET", "/assets/logo", [](const request&) {
    return text_response("get");
  });
  http_server.add_route("POST", "/assets/:id",
                        [](const request&, int) {
    return text_response("post");
  });
  http_server.add_route("DELETE", "/assets/*", [](const request&) {
    return text_response("delete");
  });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "PUT /assets/42 HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto result = receive_http_response(client);
  DOBA_EXPECT(result.has_value());
  if (result.has_value()) {
    DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 405 Method Not Allowed");
    DOBA_EXPECT_EQUAL(result->header("Allow").value(), "POST, DELETE");
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] asynchronous typed route crosses the transport          ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 invokes an asynchronous parametrized route over TCP") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  http_server.add_route(
      "GET", "/async/:id",
      [](std::shared_ptr<const request>, std::stop_token,
         std::uint64_t id) -> task<response> {
        response result = response::ok_200();
        result.set_body(id);
        co_return result;
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /async/18446744073709551615 HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto result = receive_http_response(client);
  DOBA_EXPECT(result.has_value());
  if (result.has_value()) {
    DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(result->body, "18446744073709551615");
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] running server rejects route mutation                   ( test-case ) |
// +===========================================================================+
DOBA_TEST(
    "HTTP/1.1 keeps routes immutable while running and reusable after stop") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  http_server.add_route("GET", "/first", [](const request&) {
    return text_response("first");
  });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

  bool threw = false;
  try {
    http_server.add_route("GET", "/late", [](const request&) {
      return text_response("late");
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

  http_server.add_route("GET", "/late", [](const request&) {
    return text_response("late");
  });
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("GET /late HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto late = receive_http_response(client);
  DOBA_EXPECT(late.has_value());
  if (late.has_value()) DOBA_EXPECT_EQUAL(late->body, "late");
  http_server.stop();
}
