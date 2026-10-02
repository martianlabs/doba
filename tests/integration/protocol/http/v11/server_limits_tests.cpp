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
#include <string>
#include <string_view>

#include "protocol/http/v11/server.h"
#include "http_test_helper.h"
#include "tcpip_client.h"
#include "test_helper.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] general                                                 ( constants ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
constexpr std::size_t receive_capacity = 5120;
constexpr std::size_t max_query_parameters = 128;
constexpr std::size_t max_chunked_extension_size = 1024;
constexpr std::size_t max_chunked_trailer_size = 4096;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] general                                                    ( usings ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::protocol::http::v11::server;
using martianlabs::doba::tests::integration::receive_http_response;
using martianlabs::doba::tests::integration::tcpip_client;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] read_body                                                ( function ) |
// +---------------------------------------------------------------------------+
// | This function reads the body of an HTTP request using a body reader.      |
// | It appends the read data to a string and sets the response accordingly.   |
// | If there is an error during reading, it responds with a 400 Bad           |
// | Request status. If the reading is complete, it responds with a 200 OK     |
// | status and includes the body in the response.                             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void read_body(const request& req, response& res) {
  if (!req.has_body_reader()) {
    res.bad_request_400();
    return;
  }
  std::array<std::byte, 1024> buffer{};
  std::string body;
  for (;;) {
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
}
}  // namespace

// +===========================================================================+
// | [>] complete head capacity boundaries                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP/1.1 enforces the 4 KiB request head limit") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls{0};
  server<> http_server({.recv_buffer_size = receive_capacity,
                        .ip = "127.0.0.1",
                        .port = std::to_string(port)});
  http_server.add_route("GET", "/ok", [&calls](const request&, response& res) {
    calls.fetch_add(1);
    res.ok_200();
    res.set_body("ok");
    return;
  });
  http_server.start();
  const std::string prefix = "GET /ok HTTP/1.1\r\nHost: example.com\r\nX-Pad: ";
  constexpr std::size_t head_capacity = 4096;
  for (const std::size_t size :
       {head_capacity - 1, head_capacity, head_capacity + 1}) {
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
        const auto split = head_capacity / 2;
        DOBA_EXPECT(
            client.send_all(std::string_view(request_wire).substr(0, split)));
        DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(20)));
        DOBA_EXPECT(
            client.send_all(std::string_view(request_wire).substr(split)));
      } else {
        DOBA_EXPECT(client.send_all(request_wire));
      }
      const auto result = receive_http_response(client);
      DOBA_EXPECT(result.has_value());
      if (result.has_value()) {
        DOBA_EXPECT_EQUAL(result->status,
                          size <= head_capacity
                              ? "HTTP/1.1 200 OK"
                              : "HTTP/1.1 431 Request Header Fields Too Large");
      }
      if (size > head_capacity) {
        DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
      }
      DOBA_EXPECT_EQUAL(calls.load(), before + (size <= head_capacity ? 1 : 0));
      client.close();
    }
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] query storage supported boundaries                      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP/1.1 preserves every query parameter at supported boundaries") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls{0};
  server<> http_server({.recv_buffer_size = receive_capacity,
                        .ip = "127.0.0.1",
                        .port = std::to_string(port)});
  http_server.add_route("GET", "/query",
                        [&calls](const request& req, response& res) {
                          calls.fetch_add(1);
                          res.ok_200();
                          res.set_body(req.get_query_parameters_length());
                          return;
                        });
  http_server.start();
  for (const std::size_t count :
       {max_query_parameters - 1, max_query_parameters,
        max_query_parameters + 1}) {
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
        DOBA_EXPECT_EQUAL(result->status, count <= max_query_parameters
                                              ? "HTTP/1.1 200 OK"
                                              : "HTTP/1.1 400 Bad Request");
        if (count <= max_query_parameters) {
          DOBA_EXPECT_EQUAL(result->body, std::to_string(count));
        }
      }
      if (count > max_query_parameters) {
        DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
      }
      DOBA_EXPECT_EQUAL(calls.load(),
                        before + (count <= max_query_parameters ? 1 : 0));
      client.close();
    }
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] chunk extension and trailer boundaries                  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP/1.1 enforces chunk extension and trailer wire limits") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server({.recv_buffer_size = receive_capacity,
                        .ip = "127.0.0.1",
                        .port = std::to_string(port)});
  http_server.add_route("POST", "/body", [](const request& req, response& res) {
    read_body(req, res);
  });
  http_server.start();
  for (const bool extension : {true, false}) {
    const std::size_t limit =
        extension ? max_chunked_extension_size : max_chunked_trailer_size;
    for (const std::size_t size : {limit - 1, limit, limit + 1}) {
      martianlabs::doba::tests::integration::test_helper::set_context(
          std::string(extension ? "extension " : "trailer ") +
          std::to_string(size));
      const std::string body =
          extension ? "1;" + std::string(size - 1, 'x') + "\r\na\r\n0\r\n\r\n"
                    : "0\r\nX: " + std::string(size - 7, 'x') + "\r\n\r\n";
      const std::string wire =
          "POST /body HTTP/1.1\r\nHost: a\r\n"
          "Transfer-Encoding: chunked\r\n\r\n" +
          body;
      DOBA_EXPECT(client.connect(port));
      DOBA_EXPECT(client.send_all(wire));
      const auto result = receive_http_response(client);
      DOBA_EXPECT(result.has_value());
      if (result.has_value()) {
        DOBA_EXPECT_EQUAL(result->status, size <= limit
                                              ? "HTTP/1.1 200 OK"
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
