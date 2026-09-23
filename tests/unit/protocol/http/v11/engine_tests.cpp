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
  std::size_t receive(std::string_view data) {
    return value.on_bytes_received(data.data(), data.size(), 4096);
  }
  engine<request, response> value;
  std::string wire;
  std::vector<std::size_t> blocks;
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
  const std::vector<std::pair<std::string, std::string>> cases{
      {"GET /missing", "404"}, {"POST /", "405"}, {"GET /fail", "500"},
      {"CONNECT localhost:443", "501"},
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
  DOBA_EXPECT_EQUAL(invalid.blocks.size(), 1);
  DOBA_EXPECT(invalid.wire.starts_with("HTTP/1.1 400 Bad Request\r\n"));
  policies configuration;
  configuration.max_uri_length = 1;
  connection limited(routes, configuration);
  limited.receive("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
  limited.receive("GET /long HTTP/1.1\r\nHost: localhost\r\n\r\n");
  DOBA_EXPECT_EQUAL(limited.closes, 1);
  DOBA_EXPECT(limited.wire.find("HTTP/1.1 414 ") != std::string::npos);
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

// +===========================================================================+
// | [>] engine closes after immediate response failures         ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine closes after immediate response failures") {
  for (const std::string method : {"GET", "HEAD"}) {
    for (const std::string_view scenario : {"throw", "unknown", "framing"}) {
      router<request, response> routes;
      int successors = 0;
      auto fail = [scenario]() -> response {
        if (scenario == "throw") throw std::runtime_error("private detail");
        if (scenario == "unknown") throw 1;
        auto result = make_response("discard");
        result.set_header("Content-Length", "7");
        result.set_header("Transfer-Encoding", "chunked");
        return result;
      };
      routes.add(method, "/fail", [fail](const request&) {
        return fail();
      });

      routes.add("GET", "/next", [&](const request&) {
        successors++;
        return make_response("next");
      });
      connection current(routes);
      const std::string first =
          method + " /fail HTTP/1.1\r\nHost: localhost\r\n\r\n";
      const std::string next =
          "GET /next HTTP/1.1\r\nHost: localhost\r\n\r\n";
      DOBA_EXPECT_EQUAL(current.receive(first + next), first.size());
      DOBA_EXPECT(current.wire.starts_with("HTTP/1.1 500 "));
      DOBA_EXPECT(current.wire.find("Content-Length: 21\r\n") !=
                  std::string::npos);
      DOBA_EXPECT(current.wire.find("Connection: close\r\n") !=
                  std::string::npos);
      const auto body =
          current.wire.substr(current.wire.find("\r\n\r\n") + 4);
      DOBA_EXPECT_EQUAL(body,
                        method == "HEAD" ? "" : "Internal Server Error");
      DOBA_EXPECT(current.wire.find("private detail") == std::string::npos);
      DOBA_EXPECT(current.wire.find("discard") == std::string::npos);
      DOBA_EXPECT_EQUAL(current.blocks.size(), 1);
      DOBA_EXPECT_EQUAL(current.closes, 1);
      DOBA_EXPECT_EQUAL(successors, 0);
      current.receive(next);
      DOBA_EXPECT_EQUAL(successors, 0);
      DOBA_EXPECT_EQUAL(current.blocks.size(), 1);
    }
  }
}
// +===========================================================================+
// | [>] engine never retries a failed transport delivery        ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine never retries a failed transport delivery") {
  for (const std::string_view scenario : {"ok", "throw", "framing"}) {
    router<request, response> routes;
    routes.add("GET", "/", [scenario](const request&) {
      if (scenario == "throw") throw std::runtime_error("private detail");
      auto result = make_response("ok");
      if (scenario == "framing") {
        result.set_header("Transfer-Encoding", "chunked");
        result.set_header("Content-Length", "2");
      }
      return result;
    });
    connection current(routes);
    int deliveries = 0;
    current.value.set_on_send([&](std::unique_ptr<char[]>, std::size_t,
                                  std::optional<reader>) {
      deliveries++;
      throw std::runtime_error("transport failure");
    });
    const std::string bytes = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    current.receive(bytes);
    DOBA_EXPECT_EQUAL(deliveries, 1);
    DOBA_EXPECT_EQUAL(current.closes, 1);
    current.receive(bytes);
    DOBA_EXPECT_EQUAL(deliveries, 1);
  }
}

// +===========================================================================+
// | [>] engine sends one interim for a fragmented body          ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine sends one interim for a fragmented body") {
  for (const bool chunked : {false, true}) {
    router<request, response> routes;
    int calls = 0;
    routes.add("POST", "/", [&](const request&) {
      calls++;
      return make_response("done");
    });
    const std::string head =
        "POST / HTTP/1.1\r\nHost: localhost\r\nExpect: 100-continue\r\n" +
        std::string(chunked ? "Transfer-Encoding: chunked\r\n\r\n"
                            : "Content-Length: 2\r\n\r\n");
    const std::string body = chunked ? "2\r\nxy\r\n0\r\n\r\n" : "xy";
    for (std::size_t split = 1; split < head.size(); split++) {
      connection current(routes);
      calls = 0;
      DOBA_EXPECT_EQUAL(current.receive(head.substr(0, split)), 0);
      DOBA_EXPECT(current.blocks.empty());
      DOBA_EXPECT_EQUAL(current.receive(head), head.size());
      DOBA_EXPECT_EQUAL(current.blocks.size(), 1);
      DOBA_EXPECT(current.wire.starts_with("HTTP/1.1 100 Continue\r\n"));
      DOBA_EXPECT(current.wire.ends_with("\r\n\r\n"));
      DOBA_EXPECT(current.wire.find("Content-Length:") == std::string::npos);
      DOBA_EXPECT(current.wire.find("Transfer-Encoding:") == std::string::npos);
      DOBA_EXPECT_EQUAL(calls, 0);
      std::string buffered;
      for (std::size_t i = 0; i < body.size(); i++) {
        buffered += body[i];
        const auto consumed = current.receive(buffered);
        DOBA_EXPECT(consumed <= buffered.size());
        buffered.erase(0, consumed);
        if (i + 1 < body.size()) {
          DOBA_EXPECT_EQUAL(current.blocks.size(), 1);
          DOBA_EXPECT_EQUAL(calls, 0);
        }
      }
      DOBA_EXPECT(buffered.empty());
      DOBA_EXPECT_EQUAL(calls, 1);
      DOBA_EXPECT_EQUAL(current.blocks.size(), 2);
      DOBA_EXPECT(current.wire.ends_with("\r\n\r\ndone"));
      DOBA_EXPECT_EQUAL(current.closes, 0);
    }
  }
}

// +===========================================================================+
// | [>] engine closes when interim delivery fails               ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine closes when interim delivery fails") {
  router<request, response> routes;
  connection current(routes);
  int deliveries = 0;
  current.value.set_on_send([&](std::unique_ptr<char[]>, std::size_t,
                                std::optional<reader>) {
    deliveries++;
    throw std::runtime_error("transport failure");
  });
  current.receive("POST / HTTP/1.1\r\nHost: localhost\r\n"
                  "Expect: 100-continue\r\nContent-Length: 1\r\n\r\n");
  DOBA_EXPECT_EQUAL(deliveries, 1);
  DOBA_EXPECT_EQUAL(current.closes, 1);
  current.receive("x");
  DOBA_EXPECT_EQUAL(deliveries, 1);
}

// +===========================================================================+
// | [>] engine emits a terminal rejection without dispatch      ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine emits a terminal rejection without dispatch") {
  router<request, response> routes;
  int calls = 0;
  routes.add("GET", "/", [&](const request&) {
    calls++;
    return make_response("unexpected");
  });
  for (const bool head : {false, true}) {
    connection current(routes);
    const std::string bytes = std::string(head ? "HEAD" : "GET") +
        " / HTTP/1.1\r\nHost: localhost\r\nContent-Length: invalid\r\n\r\n"
        "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    DOBA_EXPECT_EQUAL(current.receive(bytes), bytes.size());
    DOBA_EXPECT_EQUAL(current.blocks.size(), 1);
    DOBA_EXPECT(current.wire.starts_with("HTTP/1.1 400 Bad Request\r\n"));
    DOBA_EXPECT(current.wire.find("Connection: close\r\n") !=
                std::string::npos);
    DOBA_EXPECT(current.wire.find("Content-Length: 24\r\n") !=
                std::string::npos);
    DOBA_EXPECT(current.wire.ends_with(
        head ? "\r\n\r\n" : "\r\n\r\nInvalid request content!"));
    DOBA_EXPECT_EQUAL(current.closes, 1);
    current.receive("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    DOBA_EXPECT_EQUAL(current.blocks.size(), 1);
    DOBA_EXPECT_EQUAL(calls, 0);
  }
}

// +===========================================================================+
// | [>] engine does not retry failed rejection delivery         ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine does not retry failed rejection delivery") {
  router<request, response> routes;
  connection current(routes);
  int deliveries = 0;
  current.value.set_on_send([&](std::unique_ptr<char[]>, std::size_t,
                                std::optional<reader>) {
    deliveries++;
    throw std::runtime_error("transport failure");
  });
  current.receive("GET / HTTP/1.1\r\n\r\n");
  DOBA_EXPECT_EQUAL(deliveries, 1);
  DOBA_EXPECT_EQUAL(current.closes, 1);
}

// +===========================================================================+
// | [>] engine response close stops immediate dispatch          ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine response close stops immediate dispatch") {
  router<request, response> routes;
  int calls = 0;
  auto closing = [&calls]() {
    calls++;
    auto result = make_response("last");
    result.set_header("Connection", "close");
    return result;
  };
  routes.add("GET", "/", [closing](const request&) { return closing(); });

  connection current(routes);
  const std::string bytes = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(bytes + bytes), bytes.size());
  DOBA_EXPECT_EQUAL(calls, 1);
  DOBA_EXPECT_EQUAL(current.blocks.size(), 1);
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\nlast"));
  DOBA_EXPECT_EQUAL(current.closes, 1);
  DOBA_EXPECT_EQUAL(current.receive(bytes), bytes.size());
  DOBA_EXPECT_EQUAL(calls, 1);
  DOBA_EXPECT_EQUAL(current.closes, 1);
}

// +===========================================================================+
// | [>] engine matches only complete close options              ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine matches only complete close options") {
  for (const std::string_view option : {"keep-alive", "x-close",
                                        "keep-alive, close-later"}) {
    router<request, response> routes;
    routes.add("GET", "/", [option](const request&) {
      auto result = make_response("ok");
      result.set_header("Connection", option);
      return result;
    });
    connection current(routes);
    const std::string bytes = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    DOBA_EXPECT_EQUAL(current.receive(bytes + bytes), bytes.size() * 2);
    DOBA_EXPECT_EQUAL(current.blocks.size(), 2);
    DOBA_EXPECT_EQUAL(current.closes, 0);
    current.receive("GET / HTTP/1.1\r\nHost: localhost\r\n"
                    "Connection: close\r\n\r\n");
    DOBA_EXPECT_EQUAL(current.blocks.size(), 3);
    DOBA_EXPECT(current.wire.find("Connection: close\r\n") !=
                std::string::npos);
    DOBA_EXPECT_EQUAL(current.closes, 1);
  }
}

