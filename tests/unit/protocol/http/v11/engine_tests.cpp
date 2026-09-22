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
#include <coroutine>
#include <memory>
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <string_view>
#include <thread>
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
    value.set_on_wake([wakes = wakes]() { wakes->fetch_add(1); });
  }
  void poll() {
    if (wakes->exchange(0)) value.on_wake();
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
  std::shared_ptr<std::atomic<int>> wakes{
      std::make_shared<std::atomic<int>>(0)};
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] suspension                                                ( struct )  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct suspension {
  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> value) noexcept {
    pending.store(value);
  }
  void await_resume() const noexcept {}
  void resume() {
    auto value = pending.exchange(nullptr);
    if (value) value.resume();
  }
  std::atomic<std::coroutine_handle<>> pending{nullptr};
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
      {"GET /async", "200"}, {"CONNECT localhost:443", "501"},
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
  DOBA_EXPECT(async_called);
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

// +===========================================================================+
// | [>] engine completes immediate tasks without a wake         ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine completes immediate tasks without a wake") {
  router<request, response> routes;
  routes.add("GET", "/", [](std::shared_ptr<const request>,
                            std::stop_token) -> task<response> {
    co_return make_response("ready");
  });
  connection current(routes);
  const std::string bytes = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
  current.receive(bytes + bytes);
  DOBA_EXPECT_EQUAL(current.blocks.size(), 2);
  DOBA_EXPECT_EQUAL(current.wakes->load(), 0);
  DOBA_EXPECT_EQUAL(current.completed, 0);
}
// +===========================================================================+
// | [>] engine releases mixed responses without send completion( test-case )  |
// +===========================================================================+
DOBA_TEST("engine releases mixed responses without send completion") {
  router<request, response> routes;
  suspension first;
  routes.add("GET", "/first", [&](std::shared_ptr<const request>,
                                 std::stop_token) -> task<response> {
    co_await first;
    co_return make_response("first");
  });
  routes.add("GET", "/second", [](const request&) {
    return make_response("second");
  });
  routes.add("GET", "/third", [](std::shared_ptr<const request>,
                                std::stop_token) -> task<response> {
    co_return make_response("third");
  });
  connection current(routes);
  current.receive("GET /first HTTP/1.1\r\nHost: localhost\r\n\r\n"
                  "GET /second HTTP/1.1\r\nHost: localhost\r\n\r\n"
                  "GET /third HTTP/1.1\r\nHost: localhost\r\n\r\n");
  DOBA_EXPECT(current.blocks.empty());
  std::jthread completing([&]() { first.resume(); });
  completing.join();
  DOBA_EXPECT(current.blocks.empty());
  current.poll();
  DOBA_EXPECT_EQUAL(current.blocks.size(), 3);
  DOBA_EXPECT_EQUAL(current.completed, 0);
  DOBA_EXPECT(current.wire.find("\r\n\r\nfirst") <
              current.wire.find("\r\n\r\nsecond"));
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\nthird"));
  current.receive("GET /second HTTP/1.1\r\nHost: localhost\r\n\r\n");
  DOBA_EXPECT_EQUAL(current.blocks.size(), 4);
  DOBA_EXPECT_EQUAL(current.wakes->load(), 0);
}
// +===========================================================================+
// | [>] engine retains reverse completions until their turn     ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine retains reverse completions until their turn") {
  router<request, response> routes;
  suspension first;
  suspension second;
  routes.add("GET", "/first", [&](std::shared_ptr<const request>,
                                 std::stop_token) -> task<response> {
    co_await first;
    co_return make_response("first");
  });
  routes.add("GET", "/second", [&](std::shared_ptr<const request>,
                                  std::stop_token) -> task<response> {
    co_await second;
    co_return make_response("second");
  });
  connection current(routes);
  current.receive("GET /first HTTP/1.1\r\nHost: localhost\r\n\r\n"
                  "GET /second HTTP/1.1\r\nHost: localhost\r\n\r\n");
  std::jthread completing_second([&]() { second.resume(); });
  completing_second.join();
  current.poll();
  DOBA_EXPECT(current.blocks.empty());
  std::jthread completing_first([&]() { first.resume(); });
  completing_first.join();
  current.poll();
  DOBA_EXPECT_EQUAL(current.blocks.size(), 2);
  DOBA_EXPECT(current.wire.find("\r\n\r\nfirst") <
              current.wire.find("\r\n\r\nsecond"));
}
// +===========================================================================+
// | [>] engine handles completion during initial suspension     ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine handles completion during initial suspension") {
  for (int i = 0; i < 100; i++) {
    router<request, response> routes;
    suspension signal;
    routes.add("GET", "/", [&](std::shared_ptr<const request>,
                              std::stop_token) -> task<response> {
      co_await signal;
      co_return make_response("ready");
    });
    connection current(routes);
    std::jthread completing([&](std::stop_token stop) {
      while (!signal.pending.load()) {
        if (stop.stop_requested()) {
          signal.resume();
          return;
        }
        std::this_thread::yield();
      }
      signal.resume();
    });
    current.receive("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    completing.request_stop();
    completing.join();
    current.poll();
    DOBA_EXPECT_EQUAL(current.blocks.size(), 1);
    DOBA_EXPECT(current.wire.ends_with("\r\n\r\nready"));
    DOBA_EXPECT_EQUAL(current.closes, 0);
  }
}
// +===========================================================================+
// | [>] engine permits late completion after destruction        ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine permits late completion after destruction") {
  router<request, response> routes;
  suspension signal;
  std::stop_token cancellation;
  std::weak_ptr<const request> retained;
  routes.add("GET", "/", [&](std::shared_ptr<const request> input,
                            std::stop_token stop) -> task<response> {
    cancellation = stop;
    retained = input;
    co_await signal;
    co_return make_response(input->get_absolute_path());
  });
  {
    connection current(routes);
    current.receive("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    DOBA_EXPECT(!retained.expired());
  }
  DOBA_EXPECT(cancellation.stop_requested());
  std::jthread completing([&]() { signal.resume(); });
  completing.join();
  DOBA_EXPECT(retained.expired());
}
// +===========================================================================+
// | [>] engine orders a suspended close and ignores successors  ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine orders a suspended close and ignores successors") {
  router<request, response> routes;
  suspension first;
  suspension last;
  int later = 0;
  routes.add("GET", "/first", [&](std::shared_ptr<const request>,
                                 std::stop_token) -> task<response> {
    co_await first;
    co_return make_response("first");
  });
  routes.add("GET", "/last", [&](std::shared_ptr<const request>,
                                std::stop_token) -> task<response> {
    co_await last;
    co_return make_response("last");
  });
  routes.add("GET", "/later", [&](const request&) {
    later++;
    return make_response("later");
  });
  connection current(routes);
  const std::string accepted =
      "GET /first HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /last HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(accepted +
      "GET /later HTTP/1.1\r\nHost: localhost\r\n\r\n"), accepted.size());
  last.resume();
  current.poll();
  DOBA_EXPECT(current.blocks.empty());
  DOBA_EXPECT_EQUAL(current.closes, 0);
  first.resume();
  current.poll();
  DOBA_EXPECT_EQUAL(current.blocks.size(), 2);
  DOBA_EXPECT_EQUAL(current.closes, 1);
  DOBA_EXPECT_EQUAL(later, 0);
  DOBA_EXPECT_EQUAL(current.completed, 0);
}
// +===========================================================================+
// | [>] engine serializes unsafe handlers within a pipeline     ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine serializes unsafe handlers within a pipeline") {
  router<request, response> routes;
  suspension first;
  suspension write;
  std::string executed;
  routes.add("GET", "/first", [&](std::shared_ptr<const request>,
                                 std::stop_token) -> task<response> {
    co_await first;
    executed += 'A';
    co_return make_response("first");
  });
  routes.add("POST", "/write", [&](std::shared_ptr<const request>,
                                  std::stop_token) -> task<response> {
    executed += 'B';
    co_await write;
    executed += 'C';
    co_return make_response("write");
  });
  routes.add("GET", "/last", [&](const request&) {
    executed += 'D';
    return make_response("last");
  });
  connection current(routes);
  current.receive("GET /first HTTP/1.1\r\nHost: localhost\r\n\r\n"
                  "POST /write HTTP/1.1\r\nHost: localhost\r\n"
                  "Content-Length: 0\r\n\r\n"
                  "GET /last HTTP/1.1\r\nHost: localhost\r\n\r\n");
  DOBA_EXPECT(executed.empty());
  first.resume();
  current.poll();
  DOBA_EXPECT_EQUAL(executed, "AB");
  DOBA_EXPECT_EQUAL(current.blocks.size(), 1);
  write.resume();
  current.poll();
  DOBA_EXPECT_EQUAL(executed, "ABCD");
  DOBA_EXPECT_EQUAL(current.blocks.size(), 3);
}
// +===========================================================================+
// | [>] engine preserves the turn of a deferred exception       ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine preserves the turn of a deferred exception") {
  router<request, response> routes;
  suspension signal;
  routes.add("GET", "/fail", [&](std::shared_ptr<const request>,
                                std::stop_token) -> task<response> {
    co_await signal;
    throw std::runtime_error("private diagnostic");
    co_return make_response("unreachable");
  });
  routes.add("GET", "/ok", [](const request&) { return make_response("ok"); });
  connection current(routes);
  current.receive("GET /fail HTTP/1.1\r\nHost: localhost\r\n\r\n"
                  "GET /ok HTTP/1.1\r\nHost: localhost\r\n\r\n");
  DOBA_EXPECT(current.blocks.empty());
  signal.resume();
  current.poll();
  DOBA_EXPECT_EQUAL(current.blocks.size(), 2);
  DOBA_EXPECT(current.wire.starts_with("HTTP/1.1 500 "));
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\nok"));
  DOBA_EXPECT(current.wire.find("private diagnostic") == std::string::npos);
}

// +===========================================================================+
// | [>] engine races cancellation with external completion      ( test-case ) |
// +===========================================================================+
DOBA_TEST("engine races cancellation with external completion") {
  for (int i = 0; i < 100; i++) {
    router<request, response> routes;
    suspension signal;
    std::weak_ptr<const request> retained;
    std::stop_token cancellation;
    routes.add("GET", "/", [&](std::shared_ptr<const request> input,
                              std::stop_token stop) -> task<response> {
      retained = input;
      cancellation = stop;
      std::stop_callback cancel(stop, [&]() { signal.resume(); });
      co_await signal;
      co_return make_response("finished");
    });
    {
      connection current(routes);
      current.receive("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
      std::jthread completing([&]() { signal.resume(); });
      current.value.on_stop();
      completing.join();
      current.poll();
      DOBA_EXPECT(cancellation.stop_requested());
      DOBA_EXPECT(current.blocks.empty());
    }
    DOBA_EXPECT(retained.expired());
  }
}
