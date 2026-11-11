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

#include <string>

#include "../../../../helpers/concurrency.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/v11/engine.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
#include "test_helper.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] general                                                    ( usings ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using namespace martianlabs::doba;
using namespace martianlabs::doba::protocol::http::v11;
using protocol::http::router;
constexpr std::string_view next = "GET /next HTTP/1.1\r\nHost: x\r\n\r\n";
}  // namespace

// +===========================================================================+
// | [>] body one byte above the limit                           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("detection rejects a body one byte above its configured limit") {
  policies configuration;
  configuration.max_request_body_size = 4;
  decoder<request, response> parser(configuration);
  const std::string wire =
      "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 5\r\n\r\n12345";
  std::string_view input(wire);
  const auto result = parser.deserialize(input);
  DOBA_EXPECT(result.code == protocol::deserialization_status::kInvalidSource);
  DOBA_EXPECT(!result.request.has_value());
  decoder<request, response> exact(configuration);
  const std::string allowed =
      "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 4\r\n\r\n1234";
  input = allowed;
  DOBA_EXPECT(exact.deserialize(input).code ==
              protocol::deserialization_status::kSucceeded);
}

// +===========================================================================+
// | [>] hold the next response until the first finishes         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("detection queues a successor response until the first completes") {
  router<request, response> routes;
  routes.add("GET", "/first",
             [&](const request&, response& res) -> common::task<void> {
               co_await common::yield();
               res.ok_200().set_body("first");
             });
  routes.add("GET", "/next", [](const request&, response& res) {
    res.ok_200().set_body("second");
  });
  engine<request, response> current({}, routes);
  int sends = 0;
  std::string wire;
  current.set_on_send([&](std::string_view head, std::string_view body,
                          std::unique_ptr<common::reader>) {
    ++sends;
    wire.append(head).append(body);
  });
  current.set_on_close([] {});
  current.on_bytes_received("GET /first HTTP/1.1\r\nHost: x\r\n\r\n" +
                            std::string(next));
  // Keep the next response while the first task is suspended.
  DOBA_EXPECT_EQUAL(sends, 0);
  current.on_processing_turn(true);
  DOBA_EXPECT_EQUAL(sends, 2);
  const auto first = wire.find("\r\n\r\nfirst");
  const auto second = wire.find("\r\n\r\nsecond");
  DOBA_EXPECT(first != std::string::npos && second != std::string::npos &&
              first < second);
}

// +===========================================================================+
// | [>] no dispatch after terminal rejection                    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("detection never dispatches a successor after terminal rejection") {
  router<request, response> routes;
  int dispatched = 0, closes = 0;
  routes.add("GET", "/next", [&](const request&, response& res) {
    ++dispatched;
    res.ok_200().set_body("forbidden");
  });
  engine<request, response> current({}, routes);
  current.set_on_send([](std::string_view, std::string_view,
                         std::unique_ptr<common::reader>) {});
  current.set_on_close([&] { ++closes; });
  current.on_bytes_received("GET /invalid HTTP/1.1\r\n\r\n" +
                            std::string(next));
  DOBA_EXPECT_EQUAL(dispatched, 0);
  DOBA_EXPECT_EQUAL(closes, 1);
  current.on_bytes_received(next);
  DOBA_EXPECT_EQUAL(dispatched, 0);
  DOBA_EXPECT_EQUAL(closes, 1);
}
