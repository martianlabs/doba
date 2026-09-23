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

#include <algorithm>
#include <array>
#include <atomic>
#include <memory>
#include <stop_token>
#include <string>
#include <string_view>

#include "protocol/http/v11/limits.h"
#include "protocol/http/v11/server.h"
#include "http_test_helper.h"
#include "tcpip_client.h"
#include "test_helper.h"

namespace {
constexpr std::size_t receive_capacity = 5120;
using martianlabs::doba::protocol::http::v11::limits;
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::protocol::http::v11::server;
using martianlabs::doba::tests::integration::receive_http_response;
using martianlabs::doba::tests::integration::tcpip_client;

response read_body(const request& req) {
  if (!req.has_body_reader()) return response::bad_request_400();
  std::array<std::byte, 1024> buffer{};
  std::string body;
  for (;;) {
    const auto state = req.get_body_reader()->read(buffer);
    if (state.has_error) return response::bad_request_400();
    body.append(reinterpret_cast<const char*>(buffer.data()), state.produced);
    if (state.complete) {
      response result = response::ok_200();
      result.set_body(body);
      return result;
    }
  }
}
}  // namespace

// +===========================================================================+
// | [>] complete head capacity boundaries                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 terminates complete heads around decoder capacity") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls{0};
  server<> http_server({.recv_buffer_size = receive_capacity,
                        .ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route("GET", "/ok", [&calls](const request&) {
    calls.fetch_add(1);
    response res = response::ok_200();
    res.set_body("ok");
    return res;
  });
  http_server.start();

  const std::string prefix =
      "GET /ok HTTP/1.1\r\nHost: example.com\r\nX-Pad: ";
  for (const std::size_t size : {receive_capacity - 1,
                                 receive_capacity,
                                 receive_capacity + 1}) {
    for (bool fragmented : {false, true}) {
      martianlabs::doba::tests::integration::test_helper::set_context(
          "head size " + std::to_string(size) +
          (fragmented ? ", fragmented" : ", complete"));
      const std::string request_wire =
          prefix + std::string(size - prefix.size() - 4, 'x') + "\r\n\r\n";
      DOBA_EXPECT_EQUAL(request_wire.size(), size);
      const auto before = calls.load();
      DOBA_EXPECT(client.connect(port));
      if (fragmented) {
        const auto split = receive_capacity / 2;
        DOBA_EXPECT(client.send_all(
            std::string_view(request_wire).substr(0, split)));
        DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(20)));
        // Leave excess bytes unsent to verify rejection at capacity alone.
        const auto end = std::min(size, receive_capacity);
        DOBA_EXPECT(client.send_all(
            std::string_view(request_wire).substr(split, end - split)));
      } else {
        DOBA_EXPECT(client.send_all(request_wire));
      }
      const auto result = receive_http_response(client);
      DOBA_EXPECT(result.has_value());
      if (result.has_value()) {
        DOBA_EXPECT_EQUAL(result->status,
                          size <= receive_capacity
                              ? "HTTP/1.1 200 OK"
                              : "HTTP/1.1 400 Bad Request");
      }
      if (size > receive_capacity) {
        DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
      }
      DOBA_EXPECT_EQUAL(calls.load(),
                        before + (size <= receive_capacity ? 1 : 0));
      client.close();
    }
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] query storage supported boundaries                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 preserves every query parameter at supported boundaries") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls{0};
  server<> http_server({.recv_buffer_size = receive_capacity,
                        .ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route("GET", "/query", [&calls](const request& req) {
    calls.fetch_add(1);
    response res = response::ok_200();
    res.set_body(req.get_query_parameters_length());
    return res;
  });
  http_server.start();

  for (const std::size_t count : {limits::kMaxQueryParameters - 1,
                                  limits::kMaxQueryParameters,
                                  limits::kMaxQueryParameters + 1}) {
    for (bool empty_pairs : {false, true}) {
      std::string wire = empty_pairs ? "GET /query?&&" : "GET /query?";
      for (std::size_t index = 0; index < count; index++) {
        if (index != 0) wire += empty_pairs ? "&&" : "&";
        wire += "p" + std::to_string(index) + "=v";
      }
      if (empty_pairs) wire += "&&";
      wire += " HTTP/1.1\r\nHost: a\r\n\r\n";
      martianlabs::doba::tests::integration::test_helper::set_context(wire);
      const auto before = calls.load();
      DOBA_EXPECT(client.connect(port));
      DOBA_EXPECT(client.send_all(wire));
      const auto result = receive_http_response(client);
      DOBA_EXPECT(result.has_value());
      if (result.has_value()) {
        DOBA_EXPECT_EQUAL(result->status,
                          count <= limits::kMaxQueryParameters
                              ? "HTTP/1.1 200 OK"
                              : "HTTP/1.1 400 Bad Request");
        if (count <= limits::kMaxQueryParameters) {
          DOBA_EXPECT_EQUAL(result->body, std::to_string(count));
        }
      }
      if (count > limits::kMaxQueryParameters) {
        DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
      }
      DOBA_EXPECT_EQUAL(calls.load(),
                        before +
                            (count <= limits::kMaxQueryParameters ? 1 : 0));
      client.close();
    }
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] chunk extension and trailer boundaries                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 enforces chunk extension and trailer wire limits") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server({.recv_buffer_size = receive_capacity,
                        .ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route("POST", "/body", [](const request& req) {
    return read_body(req);
  });
  http_server.start();

  for (const bool extension : {true, false}) {
    const std::size_t limit = extension ? limits::kMaxChunkedExtensionSize
                                        : limits::kMaxChunkedTrailerSize;
    for (const std::size_t size : {limit - 1, limit, limit + 1}) {
      martianlabs::doba::tests::integration::test_helper::set_context(
          std::string(extension ? "extension " : "trailer ") +
          std::to_string(size));
      const std::string body = extension
          ? "1;" + std::string(size - 1, 'x') + "\r\na\r\n0\r\n\r\n"
          : "0\r\nX: " + std::string(size - 7, 'x') + "\r\n\r\n";
      const std::string wire =
          "POST /body HTTP/1.1\r\nHost: a\r\n"
          "Transfer-Encoding: chunked\r\n\r\n" + body;
      DOBA_EXPECT(client.connect(port));
      DOBA_EXPECT(client.send_all(wire));
      const auto result = receive_http_response(client);
      DOBA_EXPECT(result.has_value());
      if (result.has_value()) {
        DOBA_EXPECT_EQUAL(result->status,
                          size <= limit ? "HTTP/1.1 200 OK"
                                        : "HTTP/1.1 400 Bad Request");
        if (size <= limit) {
          DOBA_EXPECT_EQUAL(result->body, extension ? "a" : "");
        }
      }
      if (size > limit) {
        DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
      }
      client.close();
    }
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] capacity rejection follows deferred output              ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 orders a capacity rejection after deferred output") {
  using martianlabs::doba::common::task;
  using martianlabs::doba::tests::integration::http_test_signal;
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto signal = std::make_shared<http_test_signal>();
  const std::string first_body(24001, 'x');
  server<> http_server({.recv_buffer_size = receive_capacity,
                        .ip = "127.0.0.1", .port = std::to_string(port)});
  std::atomic<int> calls{0};
  http_server.add_route("GET", "/first",
      [signal, &first_body](std::shared_ptr<const request>,
                            std::stop_token token) -> task<response> {
        std::stop_callback cancellation(
            token, [signal]() { signal->resume(); });
        co_await *signal;
        auto result = response::ok_200();
        result.set_body(first_body);
        co_return result;
      });
  http_server.add_route("GET", "/sentinel", [&](const request&) {
    calls.fetch_add(1);
    return response::ok_200();
  });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /first HTTP/1.1\r\nHost: a\r\n\r\n"));
  DOBA_EXPECT(signal->wait());
  std::string invalid = "HEAD / HTTP/1.1\r\nHost: a\r\nX-Pad: ";
  invalid.append(receive_capacity - invalid.size(), 'x');
  DOBA_EXPECT(client.send_all(invalid));
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(50)));
  DOBA_EXPECT(client.send_all(
      "\r\n\r\nGET /sentinel HTTP/1.1\r\nHost: a\r\n\r\n"));
  signal->resume();
  const auto first = receive_http_response(client);
  DOBA_EXPECT(first.has_value());
  DOBA_EXPECT_EQUAL(first->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(first->body, first_body);
  const auto rejected = receive_http_response(client, true);
  DOBA_EXPECT(rejected.has_value());
  DOBA_EXPECT_EQUAL(rejected->status, "HTTP/1.1 400 Bad Request");
  DOBA_EXPECT_EQUAL(rejected->body, "");
  DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
  DOBA_EXPECT_EQUAL(calls.load(), 0);
}
