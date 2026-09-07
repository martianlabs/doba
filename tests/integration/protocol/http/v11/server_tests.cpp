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

#include <array>
#include <atomic>
#include <charconv>
#include <memory>
#include <stop_token>
#include <string>

#include "protocol/http/v11/server.h"
#include "http_test_helper.h"
#include "tcpip_client.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::protocol::http::v11::server;
using martianlabs::doba::tests::integration::tcpip_client;
using martianlabs::doba::tests::integration::receive_http_response;
using martianlabs::doba::tests::integration::http_test_signal;
using martianlabs::doba::tests::integration::wait_for_http_count;
using martianlabs::doba::common::task;

void echo_request(const request& req, response& res) {
  if (!req.has_body_reader()) {
    res.bad_request_400();
    return;
  }
  std::array<std::byte, 4096> buffer{};
  std::string body;
  for (std::size_t i = 0; i < 1024; i++) {
    const auto state = req.get_body_reader()->read(buffer);
    if (state.has_error) {
      res.bad_request_400();
      return;
    }
    body.append(reinterpret_cast<const char*>(buffer.data()), state.produced);
    if (state.complete) {
      res.ok_200().set_body(body);
      return;
    }
  }
  res.bad_request_400();
}
}  // namespace


// +===========================================================================+
// | [>] echoes a body after 100 Continue                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 echoes a body after 100 Continue") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server http_server;
  http_server.add_route(
      "POST", "/echo",
      [](const request& req, response& res) { echo_request(req, res); });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /echo HTTP/1.1\r\nHost: example.com\r\n"
      "Expect: 100-continue\r\nContent-Length: 4\r\n"
      "Connection: close\r\n\r\n"));
  const auto interim = receive_http_response(client);
  DOBA_EXPECT(interim.has_value());
  DOBA_EXPECT_EQUAL(interim->status, "HTTP/1.1 100 Continue");
  DOBA_EXPECT_EQUAL(interim->body, "");
  DOBA_EXPECT(client.send_all("doba"));
  const auto echoed = receive_http_response(client);
  DOBA_EXPECT(echoed.has_value());
  DOBA_EXPECT_EQUAL(echoed->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(echoed->body, "doba");
  DOBA_EXPECT_EQUAL(echoed->header("Connection").value(), "close");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
}

// +===========================================================================+
// | [>] ignores invalid If-Modified-Since                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 ignores invalid If-Modified-Since") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server http_server;
  http_server.add_route(
      "GET", "/resource",
      [](const request&, response& res) { res.ok_200().set_body("resource"); });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /resource HTTP/1.1\r\nHost: example.com\r\n"
      "If-Modified-Since: not-a-date\r\nConnection: close\r\n\r\n"));
  const auto accepted = receive_http_response(client);
  DOBA_EXPECT(accepted.has_value());
  DOBA_EXPECT_EQUAL(accepted->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(accepted->body, "resource");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
}

// +===========================================================================+
// | [>] ignores invalid If-Unmodified-Since                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 ignores invalid If-Unmodified-Since") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server http_server;
  http_server.add_route(
      "GET", "/resource",
      [](const request&, response& res) { res.ok_200().set_body("resource"); });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /resource HTTP/1.1\r\nHost: example.com\r\n"
      "If-Unmodified-Since: not-a-date\r\nConnection: close\r\n\r\n"));
  const auto accepted = receive_http_response(client);
  DOBA_EXPECT(accepted.has_value());
  DOBA_EXPECT_EQUAL(accepted->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(accepted->body, "resource");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
}

// +===========================================================================+
// | [>] reports the allowed method                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 reports the allowed method") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server http_server;
  http_server.add_route(
      "GET", "/resource",
      [](const request&, response& res) { res.ok_200().set_body("resource"); });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /resource HTTP/1.1\r\nHost: example.com\r\n"
      "Connection: close\r\n\r\n"));
  const auto rejected = receive_http_response(client);
  DOBA_EXPECT(rejected.has_value());
  DOBA_EXPECT_EQUAL(rejected->status, "HTTP/1.1 405 Method Not Allowed");
  DOBA_EXPECT_EQUAL(rejected->body, "");
  DOBA_EXPECT_EQUAL(rejected->header("Allow").value(), "GET");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
}

// +===========================================================================+
// | [>] reuses a connection for sequential requests             ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 reuses a connection for sequential requests") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server http_server;
  http_server.add_route(
      "GET", "/one",
      [](const request&, response& res) { res.ok_200().set_body("one"); });
  http_server.add_route(
      "GET", "/two",
      [](const request&, response& res) { res.ok_200().set_body("two"); });
  http_server.add_route(
      "GET", "/three",
      [](const request&, response& res) { res.ok_200().set_body("three"); });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /one HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto first = receive_http_response(client);
  DOBA_EXPECT(first.has_value());
  DOBA_EXPECT_EQUAL(first->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(first->body, "one");
  DOBA_EXPECT(client.send_all(
      "GET /two HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto second = receive_http_response(client);
  DOBA_EXPECT(second.has_value());
  DOBA_EXPECT_EQUAL(second->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(second->body, "two");
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] orders three synchronous pipelined responses            ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 orders three synchronous pipelined responses") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server http_server;
  http_server.add_route(
      "GET", "/one",
      [](const request&, response& res) { res.ok_200().set_body("one"); });
  http_server.add_route(
      "GET", "/two",
      [](const request&, response& res) { res.ok_200().set_body("two"); });
  http_server.add_route(
      "GET", "/three",
      [](const request&, response& res) { res.ok_200().set_body("three"); });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /one HTTP/1.1\r\nHost: example.com\r\n\r\n"
      "GET /two HTTP/1.1\r\nHost: example.com\r\n\r\n"
      "GET /three HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto first = receive_http_response(client);
  DOBA_EXPECT(first.has_value());
  DOBA_EXPECT_EQUAL(first->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(first->body, "one");
  const auto second = receive_http_response(client);
  DOBA_EXPECT(second.has_value());
  DOBA_EXPECT_EQUAL(second->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(second->body, "two");
  const auto third = receive_http_response(client);
  DOBA_EXPECT(third.has_value());
  DOBA_EXPECT_EQUAL(third->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(third->body, "three");
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] keeps synchronous responses behind a suspended handler  ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 keeps synchronous responses behind a suspended handler") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto signal = std::make_shared<http_test_signal>();
  std::atomic<std::size_t> second_calls = 0;
  server http_server;
  http_server.add_route(
      "GET", "/first",
      [signal](std::shared_ptr<const request>,
                   std::stop_token token) -> task<response> {
        std::stop_callback cancellation(
            token, [signal]() { signal->resume(); });
        co_await *signal;
        response res;
        res.ok_200().set_body("first");
        co_return res;
      });
  http_server.add_route(
      "GET", "/second", [&](const request&, response& res) {
        second_calls.fetch_add(1);
        res.ok_200().set_body("second");
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /first HTTP/1.1\r\nHost: example.com\r\n\r\n"
      "GET /second HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  DOBA_EXPECT(signal->wait());
  DOBA_EXPECT(wait_for_http_count(second_calls, 1));
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  signal->resume();
  const auto first = receive_http_response(client);
  DOBA_EXPECT(first.has_value());
  DOBA_EXPECT_EQUAL(first->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(first->body, "first");
  const auto second = receive_http_response(client);
  DOBA_EXPECT(second.has_value());
  DOBA_EXPECT_EQUAL(second->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(second->body, "second");
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] orders asynchronous responses by request order          ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 orders asynchronous responses by request order") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto first_signal = std::make_shared<http_test_signal>();
  auto second_signal = std::make_shared<http_test_signal>();
  std::atomic<std::size_t> second_completed = 0;
  server http_server;
  http_server.add_route(
      "GET", "/first",
      [first_signal](std::shared_ptr<const request>,
                   std::stop_token token) -> task<response> {
        std::stop_callback cancellation(
            token, [first_signal]() { first_signal->resume(); });
        co_await *first_signal;
        response res;
        res.ok_200().set_body("first");
        co_return res;
      });
  http_server.add_route(
      "GET", "/second",
      [second_signal, &second_completed](std::shared_ptr<const request>,
                   std::stop_token token) -> task<response> {
        std::stop_callback cancellation(
            token, [second_signal]() { second_signal->resume(); });
        co_await *second_signal;
        second_completed.fetch_add(1);
        response res;
        res.ok_200().set_body("second");
        co_return res;
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /first HTTP/1.1\r\nHost: example.com\r\n\r\n"
      "GET /second HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  DOBA_EXPECT(first_signal->wait());
  DOBA_EXPECT(second_signal->wait());
  second_signal->resume();
  DOBA_EXPECT(wait_for_http_count(second_completed, 1));
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  first_signal->resume();
  const auto first = receive_http_response(client);
  DOBA_EXPECT(first.has_value());
  DOBA_EXPECT_EQUAL(first->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(first->body, "first");
  const auto second = receive_http_response(client);
  DOBA_EXPECT(second.has_value());
  DOBA_EXPECT_EQUAL(second->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(second->body, "second");
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] preserves pipeline framing after a raw body             ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 preserves pipeline framing after a raw body") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "POST", "/echo", [&](const request& req, response& res) {
        calls.fetch_add(1);
        echo_request(req, res);
      });
  http_server.add_route(
      "GET", "/next",
      [](const request&, response& res) { res.ok_200().set_body("next"); });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /echo HTTP/1.1\r\nHost: example.com\r\n"
      "Content-Length: 7\r\n\r\n"
      "payload"
      "GET /next HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto echoed = receive_http_response(client);
  DOBA_EXPECT(echoed.has_value());
  DOBA_EXPECT_EQUAL(echoed->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(echoed->body, "payload");
  const auto next = receive_http_response(client);
  DOBA_EXPECT(next.has_value());
  DOBA_EXPECT_EQUAL(next->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(next->body, "next");
  DOBA_EXPECT_EQUAL(calls.load(), 1);
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] preserves pipeline framing after a chunked body         ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 preserves pipeline framing after a chunked body") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "POST", "/echo", [&](const request& req, response& res) {
        calls.fetch_add(1);
        echo_request(req, res);
      });
  http_server.add_route(
      "GET", "/next",
      [](const request&, response& res) { res.ok_200().set_body("next"); });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /echo HTTP/1.1\r\nHost: example.com\r\n"
      "Transfer-Encoding: chunked\r\n\r\n"
      "3\r\nabc\r\n0\r\nX: y\r\n\r\n"
      "GET /next HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto echoed = receive_http_response(client);
  DOBA_EXPECT(echoed.has_value());
  DOBA_EXPECT_EQUAL(echoed->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(echoed->body, "abc");
  const auto next = receive_http_response(client);
  DOBA_EXPECT(next.has_value());
  DOBA_EXPECT_EQUAL(next->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(next->body, "next");
  DOBA_EXPECT_EQUAL(calls.load(), 1);
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] waits for the final head delimiter                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 waits for the final head delimiter") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "GET", "/resource", [&](const request&, response& res) {
        calls.fetch_add(1);
        res.ok_200().set_body("resource");
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("GET /resource HTTP/1.1\r"));
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  DOBA_EXPECT_EQUAL(calls.load(), 0);
  DOBA_EXPECT(client.send_all("\nHost: example.com\r\n"));
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  DOBA_EXPECT_EQUAL(calls.load(), 0);
  DOBA_EXPECT(client.send_all("\r\n"));
  const auto result = receive_http_response(client);
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(result->body, "resource");
  DOBA_EXPECT_EQUAL(calls.load(), 1);
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] waits for a complete fragmented raw body                ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 waits for a complete fragmented raw body") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "POST", "/echo", [&](const request& req, response& res) {
        calls.fetch_add(1);
        echo_request(req, res);
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /echo HTTP/1.1\r\nHost: example.com\r\n"
      "Content-Length: 7\r\n\r\n"));
  constexpr std::string_view fragments[] = {
      "pay",
      "loa",
      "d",
  };
  for (std::size_t i = 0; i < std::size(fragments); i++) {
    DOBA_EXPECT(client.send_all(fragments[i]));
    if (i + 1 < std::size(fragments)) {
      DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
      DOBA_EXPECT_EQUAL(calls.load(), 0);
    }
  }
  const auto result = receive_http_response(client);
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(result->body, "payload");
  DOBA_EXPECT_EQUAL(calls.load(), 1);
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] waits for a complete fragmented chunked body            ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 waits for a complete fragmented chunked body") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "POST", "/echo", [&](const request& req, response& res) {
        calls.fetch_add(1);
        echo_request(req, res);
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /echo HTTP/1.1\r\nHost: example.com\r\n"
      "Transfer-Encoding: chunked\r\n\r\n"));
  constexpr std::string_view fragments[] = {
      "3;note=\"a,",
      "b\"\r",
      "\nabc\r\n0\r\nX: y\r",
      "\n",
      "\r\n",
  };
  for (std::size_t i = 0; i < std::size(fragments); i++) {
    DOBA_EXPECT(client.send_all(fragments[i]));
    if (i + 1 < std::size(fragments)) {
      DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
      DOBA_EXPECT_EQUAL(calls.load(), 0);
    }
  }
  const auto result = receive_http_response(client);
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(result->body, "abc");
  DOBA_EXPECT_EQUAL(calls.load(), 1);
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] echoes raw bodies across the spill threshold            ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 echoes raw bodies across the spill threshold") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "POST", "/echo", [&](const request& req, response& res) {
        calls.fetch_add(1);
        echo_request(req, res);
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  for (std::size_t size : {65534, 65535, 65536}) {
    std::string payload(size, '\0');
    for (std::size_t i = 0; i < payload.size(); i++) {
      payload[i] = static_cast<char>((i * 47 + i / 127) % 256);
    }
    const std::string body = payload;
    const std::string head =
        "POST /echo HTTP/1.1\r\nHost: example.com\r\nContent-Length: " +
        std::to_string(payload.size()) + "\r\n\r\n";
    DOBA_EXPECT(client.send_all(head));
    for (std::size_t offset = 0; offset < body.size(); offset += 4096) {
      DOBA_EXPECT(client.send_all(std::string_view(body).substr(offset, 4096)));
    }
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(result->body, payload);
  }
  DOBA_EXPECT_EQUAL(calls.load(), 3);
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] echoes chunked bodies across the spill threshold        ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 echoes chunked bodies across the spill threshold") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "POST", "/echo", [&](const request& req, response& res) {
        calls.fetch_add(1);
        echo_request(req, res);
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  for (std::size_t size : {65534, 65535, 65536}) {
    std::string payload(size - 13, '\0');
    for (std::size_t i = 0; i < payload.size(); i++) {
      payload[i] = static_cast<char>((i * 47 + i / 127) % 256);
    }
    std::array<char, 16> chunk_size{};
    const auto encoded = std::to_chars(
        chunk_size.data(), chunk_size.data() + chunk_size.size(),
        payload.size(), 16);
    DOBA_EXPECT_EQUAL(encoded.ec, std::errc{});
    const std::string body =
        std::string(chunk_size.data(), encoded.ptr) + "\r\n" +
        payload + "\r\n0\r\n\r\n";
    DOBA_EXPECT_EQUAL(body.size(), size);
    const std::string head =
        "POST /echo HTTP/1.1\r\nHost: example.com\r\n"
        "Transfer-Encoding: chunked\r\n\r\n";
    DOBA_EXPECT(client.send_all(head));
    for (std::size_t offset = 0; offset < body.size(); offset += 4096) {
      DOBA_EXPECT(client.send_all(std::string_view(body).substr(offset, 4096)));
    }
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(result->body, payload);
  }
  DOBA_EXPECT_EQUAL(calls.load(), 3);
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] emits no HEAD body before the following GET             ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 emits no HEAD body before the following GET") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server http_server;
  http_server.add_route(
      "GET", "/resource",
      [](const request&, response& res) { res.ok_200().set_body("resource"); });
  http_server.add_route(
      "HEAD", "/resource",
      [](const request&, response& res) { res.ok_200().set_body("resource"); });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "HEAD /resource HTTP/1.1\r\nHost: example.com\r\n\r\n"
      "GET /resource HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto head = receive_http_response(client, true);
  DOBA_EXPECT(head.has_value());
  DOBA_EXPECT_EQUAL(head->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(head->body, "");
  DOBA_EXPECT_EQUAL(head->header("Content-Length").value(), "8");
  const auto get = receive_http_response(client);
  DOBA_EXPECT(get.has_value());
  DOBA_EXPECT_EQUAL(get->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(get->body, "resource");
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] delimits a 204 before a following response              ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 delimits a 204 before a following response") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server http_server;
  http_server.add_route(
      "GET", "/empty",
      [](const request&, response& res) {
        res.no_content_204().set_body("hidden");
      });
  http_server.add_route(
      "GET", "/resource",
      [](const request&, response& res) { res.ok_200().set_body("resource"); });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /empty HTTP/1.1\r\nHost: example.com\r\n\r\n"
      "GET /resource HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto empty = receive_http_response(client);
  DOBA_EXPECT(empty.has_value());
  DOBA_EXPECT_EQUAL(empty->status, "HTTP/1.1 204 No Content");
  DOBA_EXPECT_EQUAL(empty->body, "");
  DOBA_EXPECT(!empty->header("Content-Length").has_value());
  const auto get = receive_http_response(client);
  DOBA_EXPECT(get.has_value());
  DOBA_EXPECT_EQUAL(get->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(get->body, "resource");
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] rejects invalid header syntax and its successor         ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 rejects invalid header syntax and its successor") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "GET", "/resource", [&](const request&, response& res) {
        calls.fetch_add(1);
        res.ok_200().set_body("unexpected");
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /resource HTTP/1.1\r\nHost: example.com\r\n"
      "Bad Name: value\r\n\r\n"
      "GET /resource HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto rejected = receive_http_response(client);
  DOBA_EXPECT(rejected.has_value());
  DOBA_EXPECT_EQUAL(rejected->status, "HTTP/1.1 400 Bad Request");
  DOBA_EXPECT_EQUAL(rejected->body, "Invalid request content!");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
  http_server.stop();
  DOBA_EXPECT_EQUAL(calls.load(), 0);
}

// +===========================================================================+
// | [>] invalid field value rejects pipelined successor         ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 rejects invalid dispatched header values and its successor") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "GET", "/resource", [&](const request&, response& res) {
        calls.fetch_add(1);
        res.ok_200().set_body("unexpected");
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /resource HTTP/1.1\r\nHost: example.com\r\n"
      "Content-Type: text\r\n\r\n"
      "GET /resource HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto rejected = receive_http_response(client);
  DOBA_EXPECT(rejected.has_value());
  DOBA_EXPECT_EQUAL(rejected->status, "HTTP/1.1 400 Bad Request");
  DOBA_EXPECT_EQUAL(rejected->body, "Invalid request content!");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
  http_server.stop();
  DOBA_EXPECT_EQUAL(calls.load(), 0);
}

// +===========================================================================+
// | [>] retains suspended request views across later heads      ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 retains suspended request views across later heads") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto signal = std::make_shared<http_test_signal>();
  std::atomic<std::size_t> later_calls = 0;
  server http_server;
  http_server.add_route(
      "GET", "/original",
      [signal](std::shared_ptr<const request> req,
               std::stop_token token) -> task<response> {
        std::stop_callback cancellation(
            token, [signal]() { signal->resume(); });
        co_await *signal;
        const auto query = req->get_query_parameter("key");
        const bool valid = req->get_absolute_path() == "/original" &&
                           query.has_value() && query->second == "value" &&
                           req->get_header("X-Id").second == "first" &&
                           req->get_header("Host").second == "first.example";
        response res;
        res.ok_200().set_body(valid ? "retained" : "corrupt");
        co_return res;
      });
  http_server.add_route(
      "GET", "/later", [&](const request& req, response& res) {
        later_calls.fetch_add(1);
        res.ok_200().set_body(req.get_header("X-Id").second);
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /original?key=value HTTP/1.1\r\nHost: first.example\r\n"
      "X-Id: first\r\n\r\n"));
  DOBA_EXPECT(signal->wait());
  for (std::size_t i = 1; i <= 2; i++) {
    const std::string later =
        "GET /later HTTP/1.1\r\nHost: later.example\r\nX-Id: " +
        std::to_string(i) + "\r\nX-Pad: " + std::string(4000, 'x') + "\r\n\r\n";
    DOBA_EXPECT(client.send_all(later));
    DOBA_EXPECT(wait_for_http_count(later_calls, i));
  }
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  signal->resume();
  const auto first = receive_http_response(client);
  DOBA_EXPECT(first.has_value());
  DOBA_EXPECT_EQUAL(first->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(first->body, "retained");
  const auto second = receive_http_response(client);
  DOBA_EXPECT(second.has_value());
  DOBA_EXPECT_EQUAL(second->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(second->body, "1");
  const auto third = receive_http_response(client);
  DOBA_EXPECT(third.has_value());
  DOBA_EXPECT_EQUAL(third->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(third->body, "2");
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] drains a complete request after a half close            ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 drains a complete request after a half close") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "POST", "/echo", [&](const request& req, response& res) {
        calls.fetch_add(1);
        echo_request(req, res);
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /echo HTTP/1.1\r\nHost: example.com\r\n"
      "Content-Length: 7\r\n\r\npayload"));
  DOBA_EXPECT(client.shutdown_write());
  const auto result = receive_http_response(client);
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(result->body, "payload");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
  http_server.stop();
  DOBA_EXPECT_EQUAL(calls.load(), 1);
}

// +===========================================================================+
// | [>] closes incomplete raw input without dispatch            ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 closes incomplete raw input without dispatch") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "POST", "/echo", [&](const request& req, response& res) {
        calls.fetch_add(1);
        echo_request(req, res);
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /echo HTTP/1.1\r\nHost: example.com\r\n"
      "Content-Length: 7\r\n\r\n"
      "pay"));
  DOBA_EXPECT(client.shutdown_write());
  const auto received = client.receive_until_close(4096);
  DOBA_EXPECT(received.has_value());
  DOBA_EXPECT_EQUAL(client.error(), "eof");
  http_server.stop();
  DOBA_EXPECT_EQUAL(calls.load(), 0);
}

// +===========================================================================+
// | [>] closes incomplete chunked input without dispatch        ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 closes incomplete chunked input without dispatch") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "POST", "/echo", [&](const request& req, response& res) {
        calls.fetch_add(1);
        echo_request(req, res);
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /echo HTTP/1.1\r\nHost: example.com\r\n"
      "Transfer-Encoding: chunked\r\n\r\n"
      "3\r\nabc\r\n"));
  DOBA_EXPECT(client.shutdown_write());
  const auto received = client.receive_until_close(4096);
  DOBA_EXPECT(received.has_value());
  DOBA_EXPECT_EQUAL(client.error(), "eof");
  http_server.stop();
  DOBA_EXPECT_EQUAL(calls.load(), 0);
}

// +===========================================================================+
// | [>] preserves binary raw payload bytes                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 preserves binary raw payload bytes") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "POST", "/echo", [&](const request& req, response& res) {
        calls.fetch_add(1);
        echo_request(req, res);
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  std::string payload = "a";
  payload.push_back('\0');
  payload += "\r\n";
  payload.push_back(static_cast<char>(0x80));
  payload += "z";
  const std::string source =
      "POST /echo HTTP/1.1\r\nHost: example.com\r\nContent-Length: " +
      std::to_string(payload.size()) + "\r\n\r\n" + payload;
  DOBA_EXPECT(client.send_all(source));
  const auto result = receive_http_response(client);
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(result->body, payload);
  DOBA_EXPECT_EQUAL(calls.load(), 1);
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] complete body omits interim response                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 omits interim responses when the body is already complete") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "POST", "/echo", [&](const request& req, response& res) {
        calls.fetch_add(1);
        echo_request(req, res);
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /echo HTTP/1.1\r\nHost: example.com\r\n"
      "Expect: 100-continue\r\nContent-Length: 7\r\n\r\npayload"));
  const auto result = receive_http_response(client);
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(result->body, "payload");
  DOBA_EXPECT_EQUAL(calls.load(), 1);
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] sends one interim while a fragmented body is pending    ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 sends one interim while a fragmented body is pending") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server http_server;
  http_server.add_route(
      "POST", "/echo", [&](const request& req, response& res) {
        calls.fetch_add(1);
        echo_request(req, res);
      });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /echo HTTP/1.1\r\nHost: example.com\r\n"
      "Expect: 100-continue\r\nContent-Length: 7\r\n\r\n"));
  const auto interim = receive_http_response(client);
  DOBA_EXPECT(interim.has_value());
  DOBA_EXPECT_EQUAL(interim->status, "HTTP/1.1 100 Continue");
  DOBA_EXPECT_EQUAL(interim->body, "");
  DOBA_EXPECT(client.send_all("pay"));
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  DOBA_EXPECT_EQUAL(calls.load(), 0);
  DOBA_EXPECT(client.send_all("loa"));
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  DOBA_EXPECT_EQUAL(calls.load(), 0);
  DOBA_EXPECT(client.send_all("d"));
  const auto result = receive_http_response(client);
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(result->body, "payload");
  DOBA_EXPECT_EQUAL(calls.load(), 1);
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] completes a large response before a short successor     ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 completes a large response before a short successor") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::string payload(131089, '\0');
  for (std::size_t i = 0; i < payload.size(); i++) {
    payload[i] = static_cast<char>((i * 53 + i / 131) % 256);
  }
  server http_server;
  http_server.add_route(
      "GET", "/large",
      [&](const request&, response& res) { res.ok_200().set_body(payload); });
  http_server.add_route(
      "GET", "/next",
      [](const request&, response& res) { res.ok_200().set_body("next"); });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /large HTTP/1.1\r\nHost: example.com\r\n\r\n"
      "GET /next HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto large = receive_http_response(client);
  DOBA_EXPECT(large.has_value());
  DOBA_EXPECT_EQUAL(large->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(large->body, payload);
  const auto next = receive_http_response(client);
  DOBA_EXPECT(next.has_value());
  DOBA_EXPECT_EQUAL(next->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(next->body, "next");
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
}

// +===========================================================================+
// | [>] cancels a suspended request after client reset          ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 cancels a suspended request after client reset") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto signal = std::make_shared<http_test_signal>();
  std::atomic<std::size_t> cancelled = 0;
  std::atomic<std::size_t> completed = 0;
  std::weak_ptr<const request> retained;
  server http_server;
  http_server.add_route(
      "GET", "/suspended",
      [&](std::shared_ptr<const request> req,
          std::stop_token token) -> task<response> {
        retained = req;
        std::stop_callback cancellation(token, [signal, &cancelled]() {
          cancelled.fetch_add(1);
          signal->resume();
        });
        co_await *signal;
        completed.fetch_add(1);
        response res;
        res.ok_200().set_body("cancelled");
        co_return res;
      });
  http_server.add_route(
      "GET", "/next",
      [](const request&, response& res) { res.ok_200().set_body("next"); });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /suspended HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  DOBA_EXPECT(signal->wait());
  DOBA_EXPECT(!retained.expired());
  client.abort();
  DOBA_EXPECT(wait_for_http_count(cancelled, 1));
  DOBA_EXPECT(wait_for_http_count(completed, 1));
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::seconds(3);
  while (!retained.expired() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::yield();
  }
  DOBA_EXPECT(retained.expired());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /next HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto next = receive_http_response(client);
  DOBA_EXPECT(next.has_value());
  DOBA_EXPECT_EQUAL(next->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(next->body, "next");
  http_server.stop();
  DOBA_EXPECT_EQUAL(cancelled.load(), 1);
  DOBA_EXPECT_EQUAL(completed.load(), 1);
}
