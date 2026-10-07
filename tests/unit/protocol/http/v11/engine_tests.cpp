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
#include <atomic>
#include <chrono>
#include <coroutine>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "protocol/http/v11/engine.h"
#include "common/task.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
#include "test_helper.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] usings                                                     ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using martianlabs::doba::common::reader;
using martianlabs::doba::common::task;
using martianlabs::doba::protocol::http::router;
using martianlabs::doba::protocol::http::v11::engine;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::protocol::http::v11::body::body_writer;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] make_response                                            ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void make_response(response& result, std::string_view body) {
  result.ok_200();
  result.set_header("Date", "fixed").set_body(body);
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] connection                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct connection {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit connection(const router<request, response>& routes,
                      policies configuration = {})
      : value(configuration, routes) {
    value.set_on_send([this](std::string_view head, std::string_view body,
                             std::unique_ptr<reader> source) {
      blocks.push_back(head.size() + body.size());
      wire.append(head);
      wire.append(body);
      if (source) source->read_all(wire);
    });
    value.set_on_close([this]() { closes++; });
  }
  // +=========================================================================+
  // | [>] receive                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  std::size_t receive(std::string_view data) {
    return value.on_bytes_received(data.data(), data.size(), 4096);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  engine<request, response> value;
  std::string wire;
  std::vector<std::size_t> blocks;
  int closes{0};
};

struct suspend_once {
  std::coroutine_handle<>& next;

  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> handle) const noexcept {
    next = handle;
  }
  void await_resume() const noexcept {}
};
}  // namespace

DOBA_TEST("engine orders asynchronous and synchronous responses") {
  router<request, response> routes;
  std::coroutine_handle<> next;
  routes.add("POST", "/slow", [&next](const request& req, response& res)
                 -> task<void> {
    co_await suspend_once{next};
    std::array<std::byte, 4> body{};
    const auto state = req.get_body_reader()->read(body);
    if (!state.complete || state.produced != 4) {
      throw std::runtime_error("invalid body");
    }
    make_response(res, std::string_view(
                           reinterpret_cast<const char*>(body.data()), 4));
  });
  routes.add("GET", "/fast", [](const request&, response& res) {
    make_response(res, "fast");
  });
  connection current(routes);
  const std::string slow =
      "POST /slow HTTP/1.1\r\nHost: localhost\r\nContent-Length: 4\r\n"
      "\r\ndata";
  const std::string fast = "GET /fast HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(slow + fast), slow.size() + fast.size());
  DOBA_EXPECT(current.wire.empty());
  next.resume();
  const auto second = current.wire.find("HTTP/1.1", 1);
  DOBA_EXPECT(second != std::string::npos);
  DOBA_EXPECT(current.wire.substr(0, second).ends_with("\r\n\r\ndata"));
  DOBA_EXPECT(current.wire.substr(second).ends_with("\r\n\r\nfast"));
}

DOBA_TEST("engine returns to synchronous delivery after asynchronous work") {
  router<request, response> routes;
  std::coroutine_handle<> pending;
  routes.add("GET", "/slow", [&pending](const request&, response& res)
                 -> task<void> {
    co_await suspend_once{pending};
    make_response(res, "slow");
  });
  routes.add("GET", "/fast", [](const request&, response& res) {
    make_response(res, "fast");
  });
  connection current(routes);
  const std::string fast = "GET /fast HTTP/1.1\r\nHost: localhost\r\n\r\n";
  const std::string slow = "GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n";
  current.receive(fast);
  DOBA_EXPECT_EQUAL(current.blocks.size(), 1);
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\nfast"));
  current.receive(slow);
  current.receive(fast);
  DOBA_EXPECT_EQUAL(current.blocks.size(), 1);
  connection independent(routes);
  independent.receive(fast);
  DOBA_EXPECT_EQUAL(independent.blocks.size(), 1);
  pending.resume();
  DOBA_EXPECT_EQUAL(current.blocks.size(), 3);
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\nfast"));
  current.receive(fast);
  DOBA_EXPECT_EQUAL(current.blocks.size(), 4);
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\nfast"));
  current.receive(slow);
  current.receive(fast);
  DOBA_EXPECT_EQUAL(current.blocks.size(), 4);
  pending.resume();
  DOBA_EXPECT_EQUAL(current.blocks.size(), 6);
  DOBA_EXPECT_EQUAL(current.closes, 0);
}

DOBA_TEST("engine resumes synchronous delivery in a coalesced batch") {
  router<request, response> routes;
  routes.add("GET", "/instant", [](const request&, response& res)
                 -> task<void> {
    make_response(res, "instant");
    co_return;
  });
  routes.add("GET", "/fast", [](const request&, response& res) {
    make_response(res, "fast");
  });
  connection current(routes);
  const std::string instant =
      "GET /instant HTTP/1.1\r\nHost: localhost\r\n\r\n";
  const std::string fast = "GET /fast HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(instant + fast + fast),
                    instant.size() + fast.size() * 2);
  DOBA_EXPECT_EQUAL(current.blocks.size(), 3U);
  const auto second = current.wire.find("HTTP/1.1", 1);
  DOBA_EXPECT(second != std::string::npos);
  const auto third = current.wire.find("HTTP/1.1", second + 1);
  DOBA_EXPECT(third != std::string::npos);
  DOBA_EXPECT(current.wire.substr(0, second).ends_with("\r\n\r\ninstant"));
  DOBA_EXPECT(current.wire.substr(second, third - second)
                  .ends_with("\r\n\r\nfast"));
  DOBA_EXPECT(current.wire.substr(third).ends_with("\r\n\r\nfast"));
}

DOBA_TEST("engine orders two asynchronous completions") {
  router<request, response> routes;
  std::coroutine_handle<> first;
  std::coroutine_handle<> second;
  routes.add("GET", "/first", [&first](const request&, response& res)
                 -> task<void> {
    co_await suspend_once{first};
    make_response(res, "first");
  });
  routes.add("GET", "/second", [&second](const request&, response& res)
                 -> task<void> {
    co_await suspend_once{second};
    make_response(res, "second");
  });
  connection current(routes);
  const std::string requests =
      "GET /first HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /second HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(requests), requests.size());
  second.resume();
  DOBA_EXPECT(current.wire.empty());
  first.resume();
  const auto next = current.wire.find("HTTP/1.1", 1);
  DOBA_EXPECT(next != std::string::npos);
  DOBA_EXPECT(current.wire.substr(0, next).ends_with("\r\n\r\nfirst"));
  DOBA_EXPECT(current.wire.substr(next).ends_with("\r\n\r\nsecond"));
}

DOBA_TEST("engine stops dispatch after a queued asynchronous error") {
  router<request, response> routes;
  std::coroutine_handle<> slow;
  int later_calls = 0;
  routes.add("GET", "/slow", [&slow](const request&, response& res)
                 -> task<void> {
    co_await suspend_once{slow};
    make_response(res, "slow");
  });
  routes.add("GET", "/fail", [](const request&, response&)
                 -> task<void> {
    throw std::runtime_error("before task creation");
  });
  routes.add("GET", "/later", [&later_calls](const request&, response& res) {
    later_calls++;
    make_response(res, "later");
  });
  connection current(routes);
  const std::string requests =
      "GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /fail HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /later HTTP/1.1\r\nHost: localhost\r\n\r\n";
  current.receive(requests);
  DOBA_EXPECT_EQUAL(later_calls, 0);
  slow.resume();
  DOBA_EXPECT_EQUAL(current.closes, 1);
  DOBA_EXPECT(current.wire.find("later") == std::string::npos);
}

DOBA_TEST("engine bounds positions behind a suspended request") {
  router<request, response> routes;
  std::coroutine_handle<> slow;
  int later_calls = 0;
  routes.add("GET", "/slow", [&slow](const request&, response& res)
                 -> task<void> {
    co_await suspend_once{slow};
    make_response(res, "slow");
  });
  routes.add("GET", "/later", [&later_calls](const request&, response& res) {
    later_calls++;
    make_response(res, "later");
  });
  connection current(routes);
  current.receive("GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n");
  for (int i = 0; i < 32; i++) {
    current.receive("GET /later HTTP/1.1\r\nHost: localhost\r\n\r\n");
  }
  DOBA_EXPECT_EQUAL(later_calls, 31);
  DOBA_EXPECT_EQUAL(current.closes, 1);
  slow.resume();
  DOBA_EXPECT(current.wire.empty());
}

DOBA_TEST("engine uses the configured pending request limit") {
  router<request, response> routes;
  std::coroutine_handle<> slow;
  int later_calls = 0;
  routes.add("GET", "/slow", [&slow](const request&, response& res)
                 -> task<void> {
    co_await suspend_once{slow};
    make_response(res, "slow");
  });
  routes.add("GET", "/later", [&later_calls](const request&, response& res) {
    later_calls++;
    make_response(res, "later");
  });
  policies configuration;
  configuration.max_pending_requests = 2;
  connection current(routes, configuration);
  current.receive("GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n");
  current.receive("GET /later HTTP/1.1\r\nHost: localhost\r\n\r\n");
  DOBA_EXPECT_EQUAL(later_calls, 1);
  current.receive("GET /later HTTP/1.1\r\nHost: localhost\r\n\r\n");
  DOBA_EXPECT_EQUAL(later_calls, 1);
  DOBA_EXPECT_EQUAL(current.closes, 1);
  slow.resume();
  DOBA_EXPECT(current.wire.empty());
}

DOBA_TEST("engine allows unlimited pending requests when configured") {
  router<request, response> routes;
  std::coroutine_handle<> slow;
  int later_calls = 0;
  routes.add("GET", "/slow", [&slow](const request&, response& res)
                 -> task<void> {
    co_await suspend_once{slow};
    make_response(res, "slow");
  });
  routes.add("GET", "/later", [&later_calls](const request&, response& res) {
    later_calls++;
    make_response(res, "later");
  });
  policies configuration;
  configuration.max_pending_requests = 0;
  connection current(routes, configuration);
  current.receive("GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n");
  for (int i = 0; i < 33; i++) {
    current.receive("GET /later HTTP/1.1\r\nHost: localhost\r\n\r\n");
  }
  DOBA_EXPECT_EQUAL(later_calls, 33);
  DOBA_EXPECT_EQUAL(current.closes, 0);
  slow.resume();
  DOBA_EXPECT_EQUAL(current.blocks.size(), 34);
}

DOBA_TEST("engine uses configured response head and inline capacities") {
  router<request, response> routes;
  routes.add("GET", "/sync", [](const request&, response& res) {
    make_response(res, "ab");
  });
  routes.add("GET", "/async", [](const request&, response& res)
                 -> task<void> {
    make_response(res, "cd");
    co_return;
  });
  policies configuration;
  configuration.max_response_head_size = 128;
  configuration.response_body_inline_capacity = 0;
  engine<request, response> value(configuration, routes);
  int sources = 0;
  std::string wire;
  value.set_on_send([&](std::string_view head, std::string_view body,
                        std::unique_ptr<reader> source) {
    wire.append(head).append(body);
    if (source) {
      sources++;
      source->read_all(wire);
    }
  });
  value.set_on_close([]() {});
  const std::string bytes =
      "GET /sync HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /async HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(value.on_bytes_received(bytes.data(), bytes.size(), 4096),
                    bytes.size());
  DOBA_EXPECT_EQUAL(sources, 2);
  DOBA_EXPECT(wire.find("\r\n\r\nab") != std::string::npos);
  DOBA_EXPECT(wire.ends_with("\r\n\r\ncd"));
}

DOBA_TEST("engine stops dispatch after a queued close response") {
  router<request, response> routes;
  std::coroutine_handle<> slow;
  int later_calls = 0;
  routes.add("GET", "/slow", [&slow](const request&, response& res)
                 -> task<void> {
    co_await suspend_once{slow};
    make_response(res, "slow");
  });
  routes.add("GET", "/close", [](const request&, response& res)
                 -> task<void> {
    make_response(res, "close");
    res.set_header("Connection", "close");
    co_return;
  });
  routes.add("GET", "/later", [&later_calls](const request&, response& res) {
    later_calls++;
    make_response(res, "later");
  });
  connection current(routes);
  current.receive(
      "GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /close HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /later HTTP/1.1\r\nHost: localhost\r\n\r\n");
  DOBA_EXPECT_EQUAL(later_calls, 0);
  slow.resume();
  DOBA_EXPECT_EQUAL(current.closes, 1);
}

DOBA_TEST("engine keeps an asynchronous response written before suspension") {
  router<request, response> routes;
  std::coroutine_handle<> slow;
  routes.add("GET", "/slow", [&slow](const request&, response& res)
                 -> task<void> {
    make_response(res, "before");
    co_await suspend_once{slow};
  });
  routes.add("GET", "/later", [](const request&, response& res) {
    make_response(res, "later");
  });
  connection current(routes);
  current.receive(
      "GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /later HTTP/1.1\r\nHost: localhost\r\n\r\n");
  slow.resume();
  const auto next = current.wire.find("HTTP/1.1", 1);
  DOBA_EXPECT(next != std::string::npos);
  DOBA_EXPECT(current.wire.substr(0, next).ends_with("\r\n\r\nbefore"));
  DOBA_EXPECT(current.wire.substr(next).ends_with("\r\n\r\nlater"));
}

DOBA_TEST("engine serializes concurrent asynchronous sends") {
  router<request, response> routes;
  std::coroutine_handle<> first;
  std::coroutine_handle<> second;
  routes.add("GET", "/first", [&first](const request&, response& res)
                 -> task<void> {
    co_await suspend_once{first};
    make_response(res, "first");
  });
  routes.add("GET", "/second", [&second](const request&, response& res)
                 -> task<void> {
    co_await suspend_once{second};
    make_response(res, "second");
  });
  connection current(routes);
  std::atomic<bool> entered{false};
  std::atomic<bool> release{false};
  std::atomic<int> sending{0};
  std::atomic<bool> overlap{false};
  current.value.set_on_send(
      [&](std::string_view head, std::string_view body,
          std::unique_ptr<reader>) {
        if (sending.fetch_add(1) != 0) overlap = true;
        if (body == "first") {
          entered = true;
          while (!release.load()) std::this_thread::yield();
        }
        current.wire.append(head).append(body);
        sending--;
      });
  current.receive(
      "GET /first HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /second HTTP/1.1\r\nHost: localhost\r\n\r\n");
  std::jthread first_thread([&]() { first.resume(); });
  while (!entered.load()) std::this_thread::yield();
  std::jthread second_thread([&]() { second.resume(); });
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  release = true;
  first_thread.join();
  second_thread.join();
  DOBA_EXPECT(!overlap.load());
  const auto next = current.wire.find("HTTP/1.1", 1);
  DOBA_EXPECT(next != std::string::npos);
  DOBA_EXPECT(current.wire.substr(0, next).ends_with("\r\n\r\nfirst"));
  DOBA_EXPECT(current.wire.substr(next).ends_with("\r\n\r\nsecond"));
}

DOBA_TEST("engine closes after an asynchronous handler error") {
  router<request, response> routes;
  std::coroutine_handle<> next;
  routes.add("GET", "/fail", [&next](const request&, response&)
                 -> task<void> {
    co_await suspend_once{next};
    throw std::runtime_error("handler failure");
  });
  routes.add("GET", "/later", [](const request&, response& res) {
    make_response(res, "later");
  });
  connection current(routes);
  const std::string first = "GET /fail HTTP/1.1\r\nHost: localhost\r\n\r\n";
  const std::string later = "GET /later HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(first + later),
                    first.size() + later.size());
  DOBA_EXPECT(current.wire.empty());
  next.resume();
  DOBA_EXPECT(current.wire.starts_with("HTTP/1.1 500 "));
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\nInternal Server Error"));
  DOBA_EXPECT(current.wire.find("later") == std::string::npos);
  DOBA_EXPECT_EQUAL(current.closes, 1);
}

DOBA_TEST("engine orders 100 Continue after an asynchronous response") {
  router<request, response> routes;
  std::coroutine_handle<> next;
  routes.add("GET", "/slow", [&next](const request&, response& res)
                 -> task<void> {
    co_await suspend_once{next};
    make_response(res, "slow");
  });
  routes.add("POST", "/upload", [](const request&, response& res) {
    make_response(res, "done");
  });
  connection current(routes);
  const std::string first = "GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n";
  const std::string head =
      "POST /upload HTTP/1.1\r\nHost: localhost\r\n"
      "Expect: 100-continue\r\nContent-Length: 4\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(first + head), first.size() + head.size());
  DOBA_EXPECT(current.wire.empty());
  next.resume();
  const auto interim = current.wire.find("HTTP/1.1 100 Continue");
  DOBA_EXPECT(interim != std::string::npos);
  DOBA_EXPECT(current.wire.substr(0, interim).ends_with("\r\n\r\nslow"));
  DOBA_EXPECT_EQUAL(current.receive("data"), 4U);
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\ndone"));
}

DOBA_TEST("engine preserves HEAD and 405 for asynchronous routes") {
  router<request, response> routes;
  routes.add("HEAD", "/only", [](const request&, response& res)
                 -> task<void> {
    make_response(res, "body");
    co_return;
  });
  connection head(routes);
  head.receive("HEAD /only HTTP/1.1\r\nHost: localhost\r\n\r\n");
  DOBA_EXPECT(head.wire.starts_with("HTTP/1.1 200 OK\r\n"));
  DOBA_EXPECT(head.wire.find("Content-Length: 4\r\n") !=
              std::string::npos);
  DOBA_EXPECT(head.wire.ends_with("\r\n\r\n"));
  connection method(routes);
  method.receive("POST /only HTTP/1.1\r\nHost: localhost\r\n\r\n");
  DOBA_EXPECT(method.wire.starts_with("HTTP/1.1 405 "));
  DOBA_EXPECT(method.wire.find("Allow: HEAD\r\n") != std::string::npos);
}

DOBA_TEST("engine honors close on an asynchronous request") {
  router<request, response> routes;
  std::coroutine_handle<> next;
  int later_calls = 0;
  routes.add("GET", "/slow", [&next](const request&, response& res)
                 -> task<void> {
    co_await suspend_once{next};
    make_response(res, "slow");
  });
  routes.add("GET", "/later", [&later_calls](const request&, response& res) {
    later_calls++;
    make_response(res, "later");
  });
  connection current(routes);
  current.receive(
      "GET /slow HTTP/1.1\r\nHost: localhost\r\n"
      "Connection: close\r\n\r\n"
      "GET /later HTTP/1.1\r\nHost: localhost\r\n\r\n");
  DOBA_EXPECT_EQUAL(later_calls, 0);
  next.resume();
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\nslow"));
  DOBA_EXPECT_EQUAL(current.closes, 1);
}

DOBA_TEST("engine transfers an asynchronous response source") {
  router<request, response> routes;
  const std::string body(131073, 'a');
  routes.add("GET", "/large", [&body](const request&, response& res)
                 -> task<void> {
    make_response(res, body);
    co_return;
  });
  std::unique_ptr<reader> source;
  engine<request, response> value({}, routes);
  value.set_on_send([&](std::string_view, std::string_view,
                        std::unique_ptr<reader> pending) {
    source = std::move(pending);
  });
  value.set_on_close([]() {});
  const std::string bytes =
      "GET /large HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(value.on_bytes_received(bytes.data(), bytes.size(), 4096),
                    bytes.size());
  DOBA_EXPECT(source != nullptr);
  std::string received;
  source->read_all(received);
  DOBA_EXPECT_EQUAL(received, body);
}

// +===========================================================================+
// | [>] engine resolves synchronous route forms                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine resolves synchronous route forms") {
  router<request, response> routes;
  routes.add("GET", "/static",
             [](const request&, response& res) { make_response(res, "one"); });
  routes.add("POST", "/static",
             [](const request&, response& res) { make_response(res, "post"); });
  routes.add("GET", "/item/:id", [](const request&, response& res, int id) {
    make_response(res, std::to_string(id));
  });
  routes.add("GET", "/assets/*", [](const request&, response& res) {
    make_response(res, "asset");
  });
  const std::vector<std::pair<std::string, std::string>> cases{
      {"/static", "one"},
      {"/item/42", "42"},
      {"/assets/a/b", "asset"},
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
  connection current(routes);
  const std::string bytes = "POST /static HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(bytes), bytes.size());
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\npost"));
}

// +===========================================================================+
// | [>] engine routes only complete requests                    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine routes only complete requests") {
  router<request, response> routes;
  int calls = 0;
  routes.add("POST", "/", [&calls](const request&, response& res) {
    calls++;
    make_response(res, "accepted");
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
// +---------------------------------------------------------------------------+
DOBA_TEST("engine returns routing and handler errors") {
  router<request, response> routes;
  routes.add("GET", "/",
             [](const request&, response& res) { make_response(res, "ok"); });
  routes.add("GET", "/fail", [](const request&, response&) -> void {
    throw std::runtime_error("handler failure");
  });
  const std::vector<std::pair<std::string, std::string>> cases{
      {"GET /missing", "404"},
      {"POST /", "405"},
      {"GET /fail", "500"},
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
// | [>] engine rejects a handler 100 without a final response   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine rejects a handler 100 without a final response") {
  router<request, response> routes;
  routes.add("GET", "/",
             [](const request&, response& res) { res.continue_100(); });
  connection current(routes);
  const std::string one = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(one + one), one.size());
  DOBA_EXPECT(current.wire.starts_with("HTTP/1.1 500 "));
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\nInternal Server Error"));
  DOBA_EXPECT(current.wire.find("Connection: close\r\n") != std::string::npos);
  DOBA_EXPECT_EQUAL(current.blocks.size(), 1U);
  DOBA_EXPECT_EQUAL(current.closes, 1);
}

// +===========================================================================+
// | [>] engine rejects handlers without a status                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine rejects handlers without a status") {
  router<request, response> routes;
  routes.add("GET", "/", [](const request&, response&) {});
  connection current(routes);
  const std::string bytes = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(bytes), bytes.size());
  DOBA_EXPECT(current.wire.starts_with("HTTP/1.1 500 "));
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\nInternal Server Error"));
  DOBA_EXPECT_EQUAL(current.closes, 1);
}

// +===========================================================================+
// | [>] engine processes coalesced requests in order            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine processes coalesced requests in order") {
  router<request, response> routes;
  int count = 0;
  routes.add("GET", "/", [&count](const request&, response& res) {
    make_response(res, std::to_string(++count));
  });
  connection current(routes);
  const std::string one = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
  const std::string bytes = one + one;
  DOBA_EXPECT_EQUAL(current.receive(bytes), bytes.size());
  DOBA_EXPECT_EQUAL(count, 2);
  DOBA_EXPECT_EQUAL(current.blocks.size(), 2U);
  const auto second = current.wire.find("HTTP/1.1", 1);
  DOBA_EXPECT(second != std::string::npos);
  DOBA_EXPECT(current.wire.substr(0, second).ends_with("\r\n\r\n1"));
  DOBA_EXPECT(current.wire.substr(second).ends_with("\r\n\r\n2"));
  DOBA_EXPECT_EQUAL(current.closes, 0);
}

// +===========================================================================+
// | [>] engine sends large and chunked response bodies          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine sends large and chunked response bodies") {
  router<request, response> routes;
  const std::string body(24001, 'x');
  routes.add("GET", "/large", [&body](const request&, response& res) {
    make_response(res, body);
  });
  routes.add("GET", "/chunked", [](const request&, response& res) {
    auto writer = body_writer::chunked();
    if (!writer.write("abc")) throw std::runtime_error("body write failed");
    res.ok_200();
    res.set_body(std::move(writer));
    return;
  });
  connection current(routes);
  const std::string large = "GET /large HTTP/1.1\r\nHost: localhost\r\n\r\n";
  const std::string chunked =
      "GET /chunked HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(large), large.size());
  DOBA_EXPECT_EQUAL(current.receive(chunked), chunked.size());
  const auto begin = current.wire.find("\r\n\r\n") + 4;
  DOBA_EXPECT_EQUAL(current.wire.substr(begin, body.size()), body);
  DOBA_EXPECT(
      current.wire.substr(begin + body.size()).starts_with("HTTP/1.1 200"));
  DOBA_EXPECT(current.wire.ends_with("\r\n\r\n3\r\nabc\r\n0\r\n\r\n"));
  DOBA_EXPECT_EQUAL(current.blocks.size(), 2U);
  DOBA_EXPECT_EQUAL(current.closes, 0);
}

// +===========================================================================+
// | [>] engine suppresses HEAD bodies                           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine suppresses HEAD bodies") {
  router<request, response> routes;
  routes.add("HEAD", "/", [](const request&, response& res) {
    make_response(res, std::string(9000, 'h'));
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
// +---------------------------------------------------------------------------+
DOBA_TEST("engine requests close without waiting") {
  router<request, response> routes;
  int calls = 0;
  routes.add("GET", "/", [&calls](const request&, response& res) {
    calls++;
    make_response(res, std::string(17000, 'c'));
  });
  connection current(routes);
  const std::string one = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
  const std::string closing =
      "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(one), one.size());
  DOBA_EXPECT_EQUAL(current.receive(closing), closing.size());
  DOBA_EXPECT_EQUAL(calls, 2);
  DOBA_EXPECT_EQUAL(current.closes, 1);
  DOBA_EXPECT_EQUAL(current.receive(one), one.size());
  DOBA_EXPECT_EQUAL(calls, 2);
  DOBA_EXPECT_EQUAL(current.closes, 1);
}

// +===========================================================================+
// | [>] engine honors response close                            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine honors response close") {
  router<request, response> routes;
  routes.add("GET", "/", [](const request&, response& res) {
    make_response(res, "close");
    res.add_header("Connection", "keep-alive");
    res.add_header("Connection", " upgrade, ClOsE ");
    return;
  });
  const std::string bytes = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
  connection current(routes);
  DOBA_EXPECT_EQUAL(current.receive(bytes + bytes), bytes.size());
  DOBA_EXPECT_EQUAL(current.closes, 1);
  DOBA_EXPECT_EQUAL(current.closes, 1);
}

// +===========================================================================+
// | [>] engine rejects invalid and policy limited requests      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine rejects invalid and policy limited requests") {
  router<request, response> routes;
  int calls = 0;
  routes.add("GET", "/", [&calls](const request&, response& res) {
    calls++;
    make_response(res, "ok");
  });
  connection invalid(routes);
  invalid.receive("GET / HTTP/1.0\r\nHost: localhost\r\n\r\n");
  DOBA_EXPECT_EQUAL(invalid.closes, 1);
  DOBA_EXPECT_EQUAL(invalid.blocks.size(), 1);
  DOBA_EXPECT(invalid.wire.starts_with("HTTP/1.1 400 Bad Request\r\n"));
  policies configuration;
  configuration.max_content_length = 1;
  connection limited(routes, configuration);
  limited.receive("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
  limited.receive(
      "POST / HTTP/1.1\r\nHost: localhost\r\n"
      "Content-Length: 2\r\n\r\n");
  DOBA_EXPECT_EQUAL(limited.closes, 1);
  DOBA_EXPECT(limited.wire.find("HTTP/1.1 413 ") != std::string::npos);
  DOBA_EXPECT_EQUAL(calls, 1);
}

// +===========================================================================+
// | [>] engine transfers unread sources                         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine transfers unread sources") {
  router<request, response> routes;
  const std::string body(131073, 'r');
  routes.add("GET", "/", [&body](const request&, response& res) {
    make_response(res, body);
  });
  std::string prefix;
  std::unique_ptr<reader> pending;
  int deliveries = 0;
  int closes = 0;
  {
    engine<request, response> value({}, routes);
    value.set_on_send([&](std::string_view head, std::string_view body,
                          std::unique_ptr<reader> source) {
      deliveries++;
      prefix = std::string(head) + std::string(body);
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
    DOBA_EXPECT(pending != nullptr);
    DOBA_EXPECT(!pending->eof());
  }
  std::string received;
  pending->read_all(received);
  DOBA_EXPECT_EQUAL(received, body);
  DOBA_EXPECT(!pending->failed());
}

// +===========================================================================+
// | [>] engine closes after immediate response failures         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine closes after immediate response failures") {
  for (const std::string method : {"GET", "HEAD"}) {
    for (const std::string_view scenario : {"throw", "unknown", "framing"}) {
      router<request, response> routes;
      int successors = 0;
      auto fail = [scenario](response& res) {
        if (scenario == "throw") throw std::runtime_error("private detail");
        if (scenario == "unknown") throw 1;
        make_response(res, "discard");
        res.set_header("Content-Length", "7");
        res.set_header("Transfer-Encoding", "chunked");
        return;
      };
      routes.add(method, "/fail",
                 [fail](const request&, response& res) { fail(res); });

      routes.add("GET", "/next", [&](const request&, response& res) {
        successors++;
        make_response(res, "next");
      });
      connection current(routes);
      const std::string first =
          method + " /fail HTTP/1.1\r\nHost: localhost\r\n\r\n";
      const std::string next = "GET /next HTTP/1.1\r\nHost: localhost\r\n\r\n";
      DOBA_EXPECT_EQUAL(current.receive(first + next), first.size());
      DOBA_EXPECT(current.wire.starts_with("HTTP/1.1 500 "));
      DOBA_EXPECT(current.wire.find("Content-Length: 21\r\n") !=
                  std::string::npos);
      DOBA_EXPECT(current.wire.find("Connection: close\r\n") !=
                  std::string::npos);
      const auto body = current.wire.substr(current.wire.find("\r\n\r\n") + 4);
      DOBA_EXPECT_EQUAL(body, method == "HEAD" ? "" : "Internal Server Error");
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
// +---------------------------------------------------------------------------+
DOBA_TEST("engine never retries a failed transport delivery") {
  for (const std::string_view scenario : {"ok", "throw", "framing"}) {
    router<request, response> routes;
    routes.add("GET", "/", [scenario](const request&, response& res) {
      if (scenario == "throw") throw std::runtime_error("private detail");
      make_response(res, "ok");
      if (scenario == "framing") {
        res.set_header("Transfer-Encoding", "chunked");
        res.set_header("Content-Length", "2");
      }
      return;
    });
    connection current(routes);
    int deliveries = 0;
    current.value.set_on_send(
        [&](std::string_view, std::string_view, std::unique_ptr<reader>) {
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
// +---------------------------------------------------------------------------+
DOBA_TEST("engine sends one interim for a fragmented body") {
  for (const bool chunked : {false, true}) {
    router<request, response> routes;
    int calls = 0;
    routes.add("POST", "/", [&](const request&, response& res) {
      calls++;
      make_response(res, "done");
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
// +---------------------------------------------------------------------------+
DOBA_TEST("engine closes when interim delivery fails") {
  router<request, response> routes;
  connection current(routes);
  int deliveries = 0;
  current.value.set_on_send(
      [&](std::string_view, std::string_view, std::unique_ptr<reader>) {
        deliveries++;
        throw std::runtime_error("transport failure");
      });
  current.receive(
      "POST / HTTP/1.1\r\nHost: localhost\r\n"
      "Expect: 100-continue\r\nContent-Length: 1\r\n\r\n");
  DOBA_EXPECT_EQUAL(deliveries, 1);
  DOBA_EXPECT_EQUAL(current.closes, 1);
  current.receive("x");
  DOBA_EXPECT_EQUAL(deliveries, 1);
}

// +===========================================================================+
// | [>] engine emits a terminal rejection without dispatch      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine emits a terminal rejection without dispatch") {
  router<request, response> routes;
  int calls = 0;
  routes.add("GET", "/", [&](const request&, response& res) {
    calls++;
    make_response(res, "unexpected");
  });
  for (const bool head : {false, true}) {
    connection current(routes);
    const std::string bytes =
        std::string(head ? "HEAD" : "GET") +
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
// | [>] engine never dispatches a smuggled request              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine never dispatches a smuggled request") {
  constexpr std::string_view smuggled =
      "GET /admin HTTP/1.1\r\nHost: localhost\r\n\r\n";
  const std::string vectors[] = {
      "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 4\r\n"
      "Transfer-Encoding: chunked\r\n\r\n0\r\n\r\n" +
          std::string(smuggled),
      "POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n"
      "Content-Length: 4\r\n\r\n0\r\n\r\n" +
          std::string(smuggled),
      "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n"
      "Content-Length: 40\r\n\r\n" +
          std::string(smuggled),
      "POST / HTTP/1.1\r\nHost: localhost\r\n"
      "Transfer-Encoding: chunked, identity\r\n\r\n" +
          std::string(smuggled),
      "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length : 0\r\n\r\n" +
          std::string(smuggled),
  };
  for (const auto& bytes : vectors) {
    martianlabs::doba::tests::unit::test_helper::set_context(bytes);
    router<request, response> routes;
    int posts = 0;
    int admin = 0;
    routes.add("POST", "/", [&](const request&, response& res) {
      posts++;
      make_response(res, "post");
    });
    routes.add("GET", "/admin", [&](const request&, response& res) {
      admin++;
      make_response(res, "admin");
    });
    connection current(routes);
    current.receive(bytes);
    DOBA_EXPECT_EQUAL(posts, 0);
    DOBA_EXPECT_EQUAL(admin, 0);
    DOBA_EXPECT_EQUAL(current.blocks.size(), 1);
    DOBA_EXPECT(current.wire.starts_with("HTTP/1.1 400 "));
    DOBA_EXPECT(current.wire.find("Connection: close\r\n") !=
                std::string::npos);
    DOBA_EXPECT(current.wire.find("admin") == std::string::npos);
    DOBA_EXPECT_EQUAL(current.closes, 1);
    current.receive(smuggled);
    DOBA_EXPECT_EQUAL(admin, 0);
    DOBA_EXPECT_EQUAL(current.blocks.size(), 1);
  }
}

// +===========================================================================+
// | [>] engine ignores trailer fields for later requests        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine ignores trailer fields for later requests") {
  router<request, response> routes;
  int posts = 0;
  int admin = 0;
  std::string host;
  std::size_t headers = 0;
  routes.add("POST", "/", [&](const request& req, response& res) {
    posts++;
    host = req.get_header("Host").second;
    headers = req.get_headers_length();
    make_response(res, "post");
  });
  routes.add("GET", "/admin", [&](const request&, response& res) {
    admin++;
    make_response(res, "admin");
  });
  connection current(routes);
  const std::string post =
      "POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n"
      "\r\n0\r\nContent-Length: 40\r\n\r\n";
  const std::string get = "GET /admin HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(post), post.size());
  DOBA_EXPECT_EQUAL(current.receive(get), get.size());
  DOBA_EXPECT_EQUAL(posts, 1);
  DOBA_EXPECT_EQUAL(admin, 1);
  DOBA_EXPECT_EQUAL(host, "localhost");
  DOBA_EXPECT_EQUAL(headers, 2);
  DOBA_EXPECT_EQUAL(current.blocks.size(), 2);
  DOBA_EXPECT_EQUAL(current.closes, 0);
}

// +===========================================================================+
// | [>] engine rejects an invalid coalesced successor           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine rejects an invalid coalesced successor") {
  router<request, response> routes;
  int calls = 0;
  routes.add("GET", "/", [&](const request&, response& res) {
    calls++;
    make_response(res, "first");
  });
  connection current(routes);
  const std::string bytes =
      "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET / HTTP/1.1\r\nHost: localhost\r\nHost: other\r\n\r\n"
      "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT_EQUAL(current.receive(bytes), bytes.size());
  DOBA_EXPECT_EQUAL(calls, 1);
  DOBA_EXPECT_EQUAL(current.blocks.size(), 2U);
  DOBA_EXPECT(current.wire.starts_with("HTTP/1.1 200 OK\r\n"));
  DOBA_EXPECT(current.wire.find("HTTP/1.1 400 ") != std::string::npos);
  DOBA_EXPECT(current.wire.find("first") != std::string::npos);
  DOBA_EXPECT_EQUAL(current.closes, 1);
}

// +===========================================================================+
// | [>] engine does not retry failed rejection delivery         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine does not retry failed rejection delivery") {
  router<request, response> routes;
  connection current(routes);
  int deliveries = 0;
  current.value.set_on_send(
      [&](std::string_view, std::string_view, std::unique_ptr<reader>) {
        deliveries++;
        throw std::runtime_error("transport failure");
      });
  current.receive("GET / HTTP/1.1\r\n\r\n");
  DOBA_EXPECT_EQUAL(deliveries, 1);
  DOBA_EXPECT_EQUAL(current.closes, 1);
}

// +===========================================================================+
// | [>] engine response close stops immediate dispatch          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine response close stops immediate dispatch") {
  router<request, response> routes;
  int calls = 0;
  auto closing = [&calls](response& res) {
    calls++;
    make_response(res, "last");
    res.set_header("Connection", "close");
    return;
  };
  routes.add("GET", "/",
             [closing](const request&, response& res) { closing(res); });
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
// +---------------------------------------------------------------------------+
DOBA_TEST("engine matches only complete close options") {
  for (const std::string_view option :
       {"keep-alive", "x-close", "keep-alive, close-later"}) {
    router<request, response> routes;
    routes.add("GET", "/", [option](const request&, response& res) {
      make_response(res, "ok");
      res.set_header("Connection", option);
      return;
    });
    connection current(routes);
    const std::string bytes = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    DOBA_EXPECT_EQUAL(current.receive(bytes), bytes.size());
    DOBA_EXPECT_EQUAL(current.receive(bytes), bytes.size());
    DOBA_EXPECT_EQUAL(current.blocks.size(), 2);
    DOBA_EXPECT_EQUAL(current.closes, 0);
    current.receive(
        "GET / HTTP/1.1\r\nHost: localhost\r\n"
        "Connection: close\r\n\r\n");
    DOBA_EXPECT_EQUAL(current.blocks.size(), 3);
    DOBA_EXPECT(current.wire.find("Connection: close\r\n") !=
                std::string::npos);
    DOBA_EXPECT_EQUAL(current.closes, 1);
  }
}

