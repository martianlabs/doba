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

#include <memory>
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <string_view>
#include <vector>

#include "protocol/http/v11/engine.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::reader;
using martianlabs::doba::protocol::http::router;
using martianlabs::doba::protocol::http::v11::body::body_writer;
using martianlabs::doba::protocol::http::v11::engine;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::common::task;

response make_response(std::string_view body) {
  auto result = response::ok_200();
  result.set_header("Date", "fixed").set_body(body);
  return result;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] connection                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct connection {
  explicit connection(const router<request, response>& routes,
                      policies configuration = {})
      : value(configuration, routes) {
    value.set_on_send([this](std::unique_ptr<char[]> bytes, std::size_t size,
                             std::optional<reader> source) {
      blocks.push_back(size);
      wire.append(bytes.get(), size);
      if (source) source->read_all(wire);
    });
    value.set_on_close([this]() { closes++; });
  }
  void complete() {
    while (completed < blocks.size()) {
      completed++;
      value.on_send_completed(true);
    }
  }
  std::size_t receive(std::string_view data) {
    return value.on_bytes_received(data.data(), data.size(), 4096);
  }
  engine<request, response> value;
  std::string wire;
  std::vector<std::size_t> blocks;
  std::size_t completed{0};
  int closes{0};
};
}  // namespace

// +===========================================================================+
// | [>] engine resolves synchronous route forms                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine resolves synchronous route forms") {
  router<request, response> routes;
  routes.add("GET", "/static", [](const request&) {
    return make_response("one");
  });
  routes.add("GET", "/item/:id", [](const request&, int id) {
    return make_response(std::to_string(id));
  });
  routes.add("GET", "/assets/*", [](const request&) {
    return make_response("asset");
  });
  const std::vector<std::pair<std::string, std::string>> cases{
      {"/static", "one"}, {"/item/42", "42"}, {"/assets/a/b", "asset"},
      {"http://localhost/static?x=1", "one"}};
  for (const auto& [target, body] : cases) {
    connection current(routes);
    const std::string bytes =
        "GET " + target + " HTTP/1.1\r\nHost: localhost\r\n\r\n";
    DOBA_EXPECT_EQUAL(current.receive(bytes), bytes.size());
    DOBA_EXPECT(current.wire.starts_with("HTTP/1.1 200 OK\r\n"));
    DOBA_EXPECT(current.wire.ends_with("\r\n\r\n" + body));
    current.complete();
    DOBA_EXPECT_EQUAL(current.closes, 0);
  }
}
// +===========================================================================+
// | [>] engine routes only complete requests                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine routes only complete requests") {
  router<request, response> routes;
  int calls = 0;
  routes.add("POST", "/", [&calls](const request&) {
    calls++;
    return make_response("accepted");
  });
  const std::string bytes =
      "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 4\r\n\r\ndata";
  for (std::size_t split = 1; split < bytes.size(); split++) {
    connection current(routes);
    calls = 0;
    std::string buffered = bytes.substr(0, split);
    const auto consumed = current.receive(buffered);
    DOBA_EXPECT(consumed <= buffered.size());
    buffered.erase(0, consumed);
    DOBA_EXPECT_EQUAL(calls, 0);
    buffered += bytes.substr(split);
    DOBA_EXPECT_EQUAL(current.receive(buffered), buffered.size());
    DOBA_EXPECT_EQUAL(calls, 1);
    DOBA_EXPECT(current.wire.ends_with("\r\n\r\naccepted"));
  }
}
// +===========================================================================+
// | [>] engine returns routing and handler errors               ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine returns routing and handler errors") {
  router<request, response> routes;
  routes.add("GET", "/", [](const request&) { return make_response("ok"); });
  routes.add("GET", "/fail", [](const request&) -> response {
    throw std::runtime_error("handler failure");
  });
  bool async_called = false;
  routes.add("GET", "/async",
             [&async_called](std::shared_ptr<const request>,
                             std::stop_token) -> task<response> {
               async_called = true;
               co_return make_response("async");
             });
  const std::vector<std::pair<std::string, std::string>> cases{
      {"GET /missing", "404"}, {"POST /", "405"}, {"GET /fail", "500"},
      {"GET /async", "501"}, {"CONNECT localhost:443", "501"},
      {"OPTIONS *", "200"}};
  for (const auto& [line, status] : cases) {
    connection current(routes);
    const std::string bytes = line + " HTTP/1.1\r\nHost: localhost:443\r\n\r\n";
    DOBA_EXPECT_EQUAL(current.receive(bytes), bytes.size());
    DOBA_EXPECT(current.wire.starts_with("HTTP/1.1 " + status + " "));
    if (status == "405") {
      DOBA_EXPECT(current.wire.find("Allow: GET\r\n") != std::string::npos);
    }
  }
  DOBA_EXPECT(!async_called);
}
// +===========================================================================+
// | [>] engine preserves pipelined response order               ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine preserves pipelined response order") {
  router<request, response> routes;
  int count = 0;
  routes.add("GET", "/", [&count](const request&) {
    return make_response(std::to_string(++count));
  });
  connection current(routes);
  const std::string one = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
  const std::string bytes = one + one + one;
  DOBA_EXPECT_EQUAL(current.receive(bytes), bytes.size());
  DOBA_EXPECT_EQUAL(count, 3);
  DOBA_EXPECT_EQUAL(current.blocks.size(), 3U);
  DOBA_EXPECT(current.wire.find("\r\n\r\n1HTTP/1.1") != std::string::npos);
  DOBA_EXPECT(current.wire.find("\r\n\r\n2HTTP/1.1") != std::string::npos);
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\n3"));
  current.complete();
  DOBA_EXPECT_EQUAL(current.closes, 0);
}
// +===========================================================================+
// | [>] engine sends large and chunked response bodies          ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine sends large and chunked response bodies") {
  router<request, response> routes;
  const std::string body(24001, 'x');
  routes.add("GET", "/large", [&body](const request&) {
    return make_response(body);
  });
  routes.add("GET", "/chunked", [](const request&) {
    auto writer = body_writer::chunked();
    if (!writer.write("abc")) throw std::runtime_error("body write failed");
    auto result = response::ok_200();
    result.set_body(std::move(writer));
    return result;
  });
  connection current(routes);
  const std::string bytes =
      "GET /large HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /chunked HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(bytes), bytes.size());
  const auto begin = current.wire.find("\r\n\r\n") + 4;
  DOBA_EXPECT_EQUAL(current.wire.substr(begin, body.size()), body);
  DOBA_EXPECT(current.wire.substr(begin + body.size()).starts_with(
      "HTTP/1.1 200"));
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\n3\r\nabc\r\n0\r\n\r\n"));
  DOBA_EXPECT_EQUAL(current.blocks.size(), 2U);
  current.complete();
  DOBA_EXPECT_EQUAL(current.closes, 0);
}
// +===========================================================================+
// | [>] engine suppresses HEAD bodies                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine suppresses HEAD bodies") {
  router<request, response> routes;
  routes.add("HEAD", "/", [](const request&) {
    return make_response(std::string(9000, 'h'));
  });
  connection current(routes);
  const std::string bytes = "HEAD / HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(bytes), bytes.size());
  DOBA_EXPECT(current.wire.find("Content-Length: 9000\r\n") !=
              std::string::npos);
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\n"));
  DOBA_EXPECT_EQUAL(current.blocks.size(), 1U);
}
// +===========================================================================+
// | [>] engine requests close without waiting                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine requests close without waiting") {
  router<request, response> routes;
  int calls = 0;
  routes.add("GET", "/", [&calls](const request&) {
    calls++;
    return make_response(std::string(17000, 'c'));
  });
  connection current(routes);
  const std::string one = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
  const std::string closing =
      "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
  const std::string bytes = one + closing + one;
  DOBA_EXPECT_EQUAL(current.receive(bytes), one.size() + closing.size());
  DOBA_EXPECT_EQUAL(calls, 2);
  DOBA_EXPECT_EQUAL(current.closes, 1);
  DOBA_EXPECT_EQUAL(current.receive(one), one.size());
  DOBA_EXPECT_EQUAL(calls, 2);
  current.complete();
  DOBA_EXPECT_EQUAL(current.closes, 1);
}
// +===========================================================================+
// | [>] engine honors response close                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine honors response close") {
  router<request, response> routes;
  routes.add("GET", "/", [](const request&) {
    auto result = make_response("close");
    result.add_header("Connection", "keep-alive");
    result.add_header("Connection", " upgrade, ClOsE ");
    return result;
  });
  const std::string bytes = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
  connection current(routes);
  DOBA_EXPECT_EQUAL(current.receive(bytes + bytes), bytes.size());
  DOBA_EXPECT_EQUAL(current.closes, 1);
  current.complete();
  DOBA_EXPECT_EQUAL(current.closes, 1);

}
// +===========================================================================+
// | [>] engine rejects invalid and policy limited requests      ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine rejects invalid and policy limited requests") {
  router<request, response> routes;
  int calls = 0;
  routes.add("GET", "/", [&calls](const request&) {
    calls++;
    return make_response("ok");
  });
  connection invalid(routes);
  invalid.receive("GET / HTTP/1.0\r\nHost: localhost\r\n\r\n");
  DOBA_EXPECT_EQUAL(invalid.closes, 1);
  DOBA_EXPECT(invalid.blocks.empty());
  policies configuration;
  configuration.max_uri_length = 1;
  connection limited(routes, configuration);
  limited.receive("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
  limited.receive("GET /long HTTP/1.1\r\nHost: localhost\r\n\r\n");
  DOBA_EXPECT_EQUAL(limited.closes, 1);
  DOBA_EXPECT_EQUAL(calls, 1);
}
// +===========================================================================+
// | [>] engine transfers unread sources                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine transfers unread sources") {
  router<request, response> routes;
  const std::string body(17000, 'r');
  routes.add("GET", "/", [&body](const request&) {
    return make_response(body);
  });
  std::string prefix;
  std::optional<reader> pending;
  int deliveries = 0;
  int closes = 0;
  {
    engine<request, response> value({}, routes);
    value.set_on_send(
        [&](std::unique_ptr<char[]> bytes, std::size_t size,
            std::optional<reader> source) {
          deliveries++;
          prefix.assign(bytes.get(), size);
          pending = std::move(source);
        });
    value.set_on_close([&]() { closes++; });
    const std::string bytes =
        "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    DOBA_EXPECT_EQUAL(value.on_bytes_received(bytes.data(), bytes.size(), 4096),
                       bytes.size());
    DOBA_EXPECT_EQUAL(deliveries, 1);
    DOBA_EXPECT_EQUAL(closes, 1);
    DOBA_EXPECT(prefix.ends_with("\r\n\r\n"));
    DOBA_EXPECT(pending.has_value());
    DOBA_EXPECT(!pending->eof());
  }
  std::string received;
  pending->read_all(received);
  DOBA_EXPECT_EQUAL(received, body);
  DOBA_EXPECT(!pending->failed());
}
