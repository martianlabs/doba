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

#include <cstddef>
#include <string>

#include "../../../../../examples/http/v11/expect_continue/echo_handler.h"
#include "http_test_helper.h"
#include "tcpip_client.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::examples::register_echo_route;
using martianlabs::doba::protocol::http::v11::server;
using martianlabs::doba::tests::integration::receive_http_response;
using martianlabs::doba::tests::integration::tcpip_client;
using martianlabs::doba::tests::integration::test_helper;
}  // namespace

// +===========================================================================+
// | [>] unframed requests return an empty echo                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("expect continue example accepts requests without body framing") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  register_echo_route(http_server);
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));

  for (const bool expect : {false, true}) {
    test_helper::set_context(expect ? "with Expect" : "without Expect");
    std::string wire = "POST /echo HTTP/1.1\r\nHost: a\r\n";
    if (expect) wire += "Expect: 100-continue\r\n";
    wire += "\r\n";
    DOBA_EXPECT(client.send_all(wire));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(result->header("Content-Length").value(), "0");
    DOBA_EXPECT_EQUAL(result->body, "");

    DOBA_EXPECT(client.send_all(
        "POST /echo HTTP/1.1\r\nHost: a\r\nContent-Length: 4\r\n\r\nnext"));
    const auto next = receive_http_response(client);
    DOBA_EXPECT(next.has_value());
    DOBA_EXPECT_EQUAL(next->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(next->body, "next");
  }
}

// +===========================================================================+
// | [>] zero content length returns an empty echo               ( test-case ) |
// +===========================================================================+
DOBA_TEST("expect continue example accepts zero content length") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  register_echo_route(http_server);
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));

  for (const bool expect : {false, true}) {
    test_helper::set_context(expect ? "with Expect" : "without Expect");
    std::string wire = "POST /echo HTTP/1.1\r\nHost: a\r\n";
    wire += "Content-Length: 0\r\n";
    if (expect) wire += "Expect: 100-continue\r\n";
    wire += "\r\n";
    DOBA_EXPECT(client.send_all(wire));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(result->header("Content-Length").value(), "0");
    DOBA_EXPECT_EQUAL(result->body, "");

    DOBA_EXPECT(client.send_all(
        "POST /echo HTTP/1.1\r\nHost: a\r\nContent-Length: 4\r\n\r\nnext"));
    const auto next = receive_http_response(client);
    DOBA_EXPECT(next.has_value());
    DOBA_EXPECT_EQUAL(next->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(next->body, "next");
  }
}

// +===========================================================================+
// | [>] empty chunked requests return an empty echo             ( test-case ) |
// +===========================================================================+
DOBA_TEST("expect continue example accepts empty chunked bodies") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  register_echo_route(http_server);
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "POST /echo HTTP/1.1\r\nHost: a\r\n"
      "Transfer-Encoding: chunked\r\n\r\n0\r\n\r\n"));
  const auto result = receive_http_response(client);
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(result->header("Content-Length").value(), "0");
  DOBA_EXPECT_EQUAL(result->body, "");

  DOBA_EXPECT(client.send_all(
      "POST /echo HTTP/1.1\r\nHost: a\r\nContent-Length: 4\r\n\r\nnext"));
  const auto next = receive_http_response(client);
  DOBA_EXPECT(next.has_value());
  DOBA_EXPECT_EQUAL(next->status, "HTTP/1.1 200 OK");
  DOBA_EXPECT_EQUAL(next->body, "next");
}

// +===========================================================================+
// | [>] raw and chunked echo preserve multiple binary reads     ( test-case ) |
// +===========================================================================+
DOBA_TEST("expect continue example preserves raw and chunked binary bodies") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  register_echo_route(http_server);
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));
  std::string body(1025, '\0');
  for (std::size_t i = 0; i < body.size(); i++) {
    body[i] = static_cast<char>(i % 256);
  }

  for (const bool chunked : {false, true}) {
    test_helper::set_context(chunked ? "chunked" : "content length");
    std::string wire = "POST /echo HTTP/1.1\r\nHost: a\r\n";
    if (chunked) {
      wire += "Transfer-Encoding: chunked\r\n\r\n400\r\n" +
              body.substr(0, 1024) + "\r\n1\r\n" + body.substr(1024) +
              "\r\n0\r\n\r\n";
    } else {
      wire += "Content-Length: 1025\r\n\r\n" + body;
    }
    DOBA_EXPECT(client.send_all(wire));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(result->header("Content-Length").value(), "1025");
    DOBA_EXPECT_EQUAL(result->body, body);
  }
}

// +===========================================================================+
// | [>] pending raw and chunked bodies receive 100 Continue     ( test-case ) |
// +===========================================================================+
DOBA_TEST("expect continue example sends interim before reading the body") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  register_echo_route(http_server);
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  DOBA_EXPECT(client.connect(port));

  for (const bool chunked : {false, true}) {
    test_helper::set_context(chunked ? "chunked" : "content length");
    std::string wire =
        "POST /echo HTTP/1.1\r\nHost: a\r\nExpect: 100-continue\r\n";
    wire += chunked ? "Transfer-Encoding: chunked\r\n\r\n"
                    : "Content-Length: 4\r\n\r\n";
    DOBA_EXPECT(client.send_all(wire));
    const auto interim = receive_http_response(client);
    DOBA_EXPECT(interim.has_value());
    DOBA_EXPECT_EQUAL(interim->status, "HTTP/1.1 100 Continue");
    DOBA_EXPECT_EQUAL(interim->body, "");
    DOBA_EXPECT(client.send_all(chunked ? "4\r\ndoba\r\n0\r\n\r\n" : "doba"));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(result->header("Content-Length").value(), "4");
    DOBA_EXPECT_EQUAL(result->body, "doba");
  }
}
