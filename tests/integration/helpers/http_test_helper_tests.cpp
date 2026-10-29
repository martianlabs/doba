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

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "http_test_helper.h"
#include "native_peer.h"
#include "test_helper.h"

namespace {
using namespace martianlabs::doba::tests::integration;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parse_wire                                               ( function ) |
// +---------------------------------------------------------------------------+
// | Exercise the response oracle with an independent raw TCP peer.            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
std::optional<http_test_response> parse_wire(std::string_view fields,
                                            std::string_view body = "ok") {
  native_peer peer;
  tcpip_client client;
  if (!peer.port() || !client.connect(peer.port()) || !peer.accept())
    throw std::runtime_error("Unable to establish response oracle fixture");
  const std::string wire = "HTTP/1.1 200 OK\r\n" + std::string(fields) +
                           "\r\n" + std::string(body);
  if (!peer.send(wire)) throw std::runtime_error("Unable to send response fixture");
  return receive_http_response(client);
}
constexpr std::string_view date = "Date: Wed, 01 Jan 2020 00:00:00 GMT\r\n";
}

// +===========================================================================+
// | [>] header field names are case insensitive                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP response oracle accepts case insensitive field names") {
  const auto result = parse_wire("date: Wed, 01 Jan 2020 00:00:00 GMT\r\n"
                                 "content-length: 2\r\n");
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->body, "ok");
}

// +===========================================================================+
// | [>] duplicate metadata cannot hide behind case variations    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP response oracle rejects mixed case duplicate singleton fields") {
  for (const std::string& fields : {
       std::string(date) + "Content-Length: 2\r\ncontent-length: 3\r\n",
       std::string(date) + "Content-Length: 2\r\ndate: Wed, 01 Jan 2020 00:00:00 GMT\r\n",
       std::string(date) + "Transfer-Encoding: chunked\r\ntransfer-encoding: chunked\r\n"}) {
    DOBA_EXPECT(!parse_wire(fields, "0\r\n\r\n").has_value());
  }
}
