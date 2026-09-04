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
#include <string>

#include "protocol/http/v11/server.h"
#include "tcpip_client.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::protocol::http::v11::server;
using martianlabs::doba::tests::integration::tcpip_client;
}  // namespace

// +===========================================================================+
// | [>] HTTP/1.1 automatic behavior                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 emits interim and automatic response behavior") {
  tcpip_client client;
  uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server http_server;
  http_server.add_route(
      "POST", "/echo",
      [](const request& req, response& res) {
        std::array<std::byte, 16> buffer{};
        const auto state = req.get_body_reader()->read(buffer);
        if (state.has_error || !state.complete) {
          res.bad_request_400();
          return;
        }
        res.ok_200().set_body(
            std::string(reinterpret_cast<const char*>(buffer.data()),
                        state.produced));
      });
  http_server.add_route(
      "GET", "/resource",
      [](const request&, response& res) { res.ok_200().set_body("resource"); });
  std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /echo HTTP/1.1\r\nHost: example.com\r\n"
      "Expect: 100-continue\r\nContent-Length: 4\r\n"
      "Connection: close\r\n\r\n"));
  const auto interim = client.receive(25);
  DOBA_EXPECT(interim.has_value());
  DOBA_EXPECT_EQUAL(*interim, "HTTP/1.1 100 Continue\r\n\r\n");
  DOBA_EXPECT(client.send_all("doba"));
  const auto echoed = client.receive_until_close(4096);
  DOBA_EXPECT(echoed.has_value());
  DOBA_EXPECT(echoed->starts_with("HTTP/1.1 200 OK\r\n"));
  DOBA_EXPECT(echoed->find("Date: ") != std::string::npos);
  DOBA_EXPECT(echoed->find("Connection: close\r\n") != std::string::npos);
  DOBA_EXPECT(echoed->ends_with("doba"));

  constexpr std::string_view conditional_headers[] = {
      "If-Modified-Since: not-a-date\r\n",
      "If-Unmodified-Since: not-a-date\r\n",
  };
  for (const auto header : conditional_headers) {
    DOBA_EXPECT(client.connect(port));
    const std::string conditional =
        "GET /resource HTTP/1.1\r\nHost: example.com\r\n" +
        std::string(header) + "Connection: close\r\n\r\n";
    DOBA_EXPECT(client.send_all(conditional));
    const auto accepted = client.receive_until_close(4096);
    DOBA_EXPECT(accepted.has_value());
    DOBA_EXPECT(accepted->starts_with("HTTP/1.1 200 OK\r\n"));
    DOBA_EXPECT(accepted->ends_with("resource"));
  }

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /resource HTTP/1.1\r\nHost: example.com\r\n"
      "Connection: close\r\n\r\n"));
  const auto rejected = client.receive_until_close(4096);
  DOBA_EXPECT(rejected.has_value());
  DOBA_EXPECT(rejected->starts_with("HTTP/1.1 405 Method Not Allowed\r\n"));
  DOBA_EXPECT(rejected->find("Allow: GET\r\n") != std::string::npos);

  http_server.stop();
}
