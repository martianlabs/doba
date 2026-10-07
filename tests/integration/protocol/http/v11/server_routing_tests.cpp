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
#include <chrono>
#include <cstddef>
#include <coroutine>
#include <future>
#include <thread>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "protocol/http/v11/server.h"
#include "common/task.h"
#include "http_test_helper.h"
#include "tcpip_client.h"
#include "test_helper.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] general                                                    ( usings ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::protocol::http::v11::server;
using martianlabs::doba::common::task;
using martianlabs::doba::tests::integration::receive_http_response;
using martianlabs::doba::tests::integration::tcpip_client;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] text_response                                            ( function ) |
// +---------------------------------------------------------------------------+
// | This funciton is a helper to send a simple text response with 200 OK      |
// | status. It sets the response body to the provided text and marks the      |
// | response as successful.                                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void text_response(response& res, std::string_view text) {
  res.ok_200();
  res.set_body(text);
}

struct pause_awaiter {
  std::promise<std::coroutine_handle<>>& pending;

  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> handle) const {
    pending.set_value(handle);
  }
  void await_resume() const noexcept {}
};
}  // namespace

DOBA_TEST("HTTP/1.1 orders mixed routes over one connection") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::promise<std::coroutine_handle<>> pending;
  auto suspended = pending.get_future();
  server<> http_server({.ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route("GET", "/slow",
                        [&pending](const request&, response& res)
                            -> task<void> {
                          co_await pause_awaiter{pending};
                          text_response(res, "slow");
                        });
  http_server.add_route("GET", "/fast", [](const request&, response& res) {
    text_response(res, "fast");
  });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  const std::string requests =
      "GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /fast HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT(client.send_all(requests));
  if (suspended.wait_for(std::chrono::seconds(2)) !=
      std::future_status::ready) {
    throw std::runtime_error("asynchronous route did not suspend");
  }
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  suspended.get().resume();
  const auto first = receive_http_response(client);
  const auto second = receive_http_response(client);
  DOBA_EXPECT(first.has_value());
  DOBA_EXPECT(second.has_value());
  if (first && second) {
    DOBA_EXPECT_EQUAL(first->body, "slow");
    DOBA_EXPECT_EQUAL(second->body, "fast");
  }
  DOBA_EXPECT(client.send_all("GET /fast HTTP/1.1\r\nHost: localhost\r\n\r\n"));
  const auto third = receive_http_response(client);
  DOBA_EXPECT(third.has_value());
  if (third) DOBA_EXPECT_EQUAL(third->body, "fast");
  http_server.stop();
}

DOBA_TEST("HTTP/1.1 resumes timed routes on its transport worker") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<bool> same_worker{false};
  server<> http_server({.worker_count = 1, .ip = "127.0.0.1",
                        .port = std::to_string(port)});
  http_server.add_route(
      "GET", "/slow",
      [&same_worker](const request&, response& res) -> task<void> {
        const auto worker = std::this_thread::get_id();
        co_await martianlabs::doba::common::yield();
        co_await martianlabs::doba::common::sleep_for(
            std::chrono::milliseconds(10));
        same_worker = std::this_thread::get_id() == worker;
        text_response(res, "slow");
      });
  http_server.add_route("GET", "/fast", [](const request&, response& res) {
    text_response(res, "fast");
  });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /fast HTTP/1.1\r\nHost: localhost\r\n\r\n"));
  const auto first = receive_http_response(client);
  const auto second = receive_http_response(client);
  DOBA_EXPECT(first.has_value());
  DOBA_EXPECT(second.has_value());
  if (first && second) {
    DOBA_EXPECT_EQUAL(first->body, "slow");
    DOBA_EXPECT_EQUAL(second->body, "fast");
  }
  DOBA_EXPECT(same_worker.load());
  http_server.stop();
}

DOBA_TEST("HTTP/1.1 orders responses after different timer deadlines") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<int> completed{0};
  std::atomic<int> first_order{0};
  std::atomic<int> second_order{0};
  std::atomic<int> third_order{0};
  server<> http_server({.worker_count = 1, .ip = "127.0.0.1",
                        .port = std::to_string(port)});
  http_server.add_route(
      "GET", "/wait/:id/:ms",
      [&](const request&, response& res, int id, int ms) -> task<void> {
        co_await martianlabs::doba::common::sleep_for(
            std::chrono::milliseconds(ms));
        const int order = completed.fetch_add(1) + 1;
        if (id == 1) first_order = order;
        if (id == 2) second_order = order;
        if (id == 3) third_order = order;
        text_response(res, std::to_string(id));
      });
  http_server.add_route("GET", "/fast", [](const request&, response& res) {
    text_response(res, "fast");
  });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /wait/1/80 HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /wait/2/5 HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /wait/3/20 HTTP/1.1\r\nHost: localhost\r\n\r\n"));
  const auto first = receive_http_response(client);
  const auto second = receive_http_response(client);
  const auto third = receive_http_response(client);
  DOBA_EXPECT(first.has_value());
  DOBA_EXPECT(second.has_value());
  DOBA_EXPECT(third.has_value());
  if (first && second && third) {
    DOBA_EXPECT_EQUAL(first->body, "1");
    DOBA_EXPECT_EQUAL(second->body, "2");
    DOBA_EXPECT_EQUAL(third->body, "3");
  }
  DOBA_EXPECT(second_order.load() < third_order.load());
  DOBA_EXPECT(third_order.load() < first_order.load());
  DOBA_EXPECT(client.send_all("GET /fast HTTP/1.1\r\nHost: localhost\r\n\r\n"));
  const auto fast = receive_http_response(client);
  DOBA_EXPECT(fast.has_value());
  if (fast) DOBA_EXPECT_EQUAL(fast->body, "fast");
  http_server.stop();
}

#ifdef _WIN32
DOBA_TEST("HTTP/1.1 runs asynchronous routes with multiple IOCP workers") {
  tcpip_client probe;
  const uint16_t port = probe.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server({.worker_count = 2, .ip = "127.0.0.1",
                        .port = std::to_string(port)});
  http_server.add_route("GET", "/wait",
                        [](const request&, response& res) -> task<void> {
                          co_await martianlabs::doba::common::yield();
                          co_await martianlabs::doba::common::sleep_for(
                              std::chrono::milliseconds(1));
                          text_response(res, "ready");
                        });
  http_server.start();
  std::atomic<int> connected{0};
  std::atomic<int> sent{0};
  std::atomic<int> received{0};
  std::atomic<int> server_errors{0};
  std::atomic<int> passed{0};
  std::vector<std::jthread> clients;
  for (int i = 0; i < 32; i++) {
    clients.emplace_back([&]() {
      tcpip_client client;
      if (!client.connect(port)) return;
      connected++;
      if (!client.send_all("GET /wait HTTP/1.1\r\nHost: localhost\r\n\r\n")) {
        return;
      }
      sent++;
      const auto result = receive_http_response(client);
      if (!result) return;
      received++;
      if (result->status.starts_with("HTTP/1.1 500 ")) server_errors++;
      if (result->status.starts_with("HTTP/1.1 200 ") &&
          result->body == "ready") {
        passed++;
      }
    });
  }
  for (auto& client : clients) client.join();
  DOBA_EXPECT_EQUAL(connected.load(), 32);
  DOBA_EXPECT_EQUAL(sent.load(), 32);
  DOBA_EXPECT_EQUAL(received.load(), 32);
  DOBA_EXPECT_EQUAL(server_errors.load(), 0);
  DOBA_EXPECT_EQUAL(passed.load(), 32);
  http_server.stop();
}
#endif
DOBA_TEST("HTTP/1.1 drains a timed route before stopping workers") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::promise<void> started;
  auto waiting = started.get_future();
  std::atomic<bool> completed{false};
  server<> http_server({.worker_count = 1, .ip = "127.0.0.1",
                        .port = std::to_string(port)});
  http_server.add_route(
      "GET", "/slow",
      [&started, &completed](const request&, response& res) -> task<void> {
        started.set_value();
        co_await martianlabs::doba::common::sleep_for(
            std::chrono::milliseconds(30));
        completed = true;
        text_response(res, "done");
      });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n"));
  DOBA_EXPECT(waiting.wait_for(std::chrono::seconds(2)) ==
              std::future_status::ready);
  http_server.stop();
  DOBA_EXPECT(completed.load());
}

DOBA_TEST("HTTP/1.1 waits for an asynchronous route during stop") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::promise<std::coroutine_handle<>> pending;
  auto suspended = pending.get_future();
  server<> http_server({.ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route("GET", "/slow",
                        [&pending](const request&, response& res)
                            -> task<void> {
                          co_await pause_awaiter{pending};
                          text_response(res, "done");
                        });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n"));
  if (suspended.wait_for(std::chrono::seconds(2)) !=
      std::future_status::ready) {
    throw std::runtime_error("asynchronous route did not suspend");
  }
  std::promise<void> stopped;
  auto stop_result = stopped.get_future();
  std::jthread stopper([&]() {
    http_server.stop();
    stopped.set_value();
  });
  DOBA_EXPECT(stop_result.wait_for(std::chrono::milliseconds(100)) ==
              std::future_status::timeout);
  suspended.get().resume();
  DOBA_EXPECT(stop_result.wait_for(std::chrono::seconds(2)) ==
              std::future_status::ready);
}

DOBA_TEST("HTTP/1.1 retains a spilled asynchronous body and parameter") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::promise<std::coroutine_handle<>> pending;
  auto suspended = pending.get_future();
  server<> http_server({.ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route(
      "POST", "/upload/:name",
      [&pending](const request& req, response& res,
                 const std::string& name) -> task<void> {
        co_await pause_awaiter{pending};
        std::vector<std::byte> body(16 * 1024 + 1);
        const auto state = req.get_body_reader()->read(body);
        if (!state.complete || state.produced != body.size() ||
            body.front() != std::byte{'x'} ||
            body.back() != std::byte{'x'}) {
          throw std::runtime_error("invalid spilled body");
        }
        text_response(res, name + ":" + std::to_string(state.produced));
      });
  http_server.add_route("GET", "/next", [](const request&, response& res) {
    text_response(res, "next");
  });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  const std::string body(16 * 1024 + 1, 'x');
  const std::string requests =
      "POST /upload/item HTTP/1.1\r\nHost: localhost\r\nContent-Length: " +
      std::to_string(body.size()) + "\r\n\r\n" + body +
      "GET /next HTTP/1.1\r\nHost: localhost\r\n\r\n";
  DOBA_EXPECT(client.send_all(requests));
  if (suspended.wait_for(std::chrono::seconds(2)) !=
      std::future_status::ready) {
    throw std::runtime_error("asynchronous route did not suspend");
  }
  suspended.get().resume();
  const auto first = receive_http_response(client);
  const auto second = receive_http_response(client);
  DOBA_EXPECT(first.has_value());
  DOBA_EXPECT(second.has_value());
  if (first && second) {
    DOBA_EXPECT_EQUAL(first->body, "item:16385");
    DOBA_EXPECT_EQUAL(second->body, "next");
  }
  http_server.stop();
}

DOBA_TEST("HTTP/1.1 completes after a suspended client disconnects") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::promise<std::coroutine_handle<>> pending;
  auto suspended = pending.get_future();
  std::atomic<bool> finished{false};
  server<> http_server({.ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route("GET", "/slow",
                        [&pending, &finished](const request&, response& res)
                            -> task<void> {
                          co_await pause_awaiter{pending};
                          text_response(res, "done");
                          finished = true;
                        });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("GET /slow HTTP/1.1\r\nHost: localhost\r\n\r\n"));
  if (suspended.wait_for(std::chrono::seconds(2)) !=
      std::future_status::ready) {
    throw std::runtime_error("asynchronous route did not suspend");
  }
  client.close();
  suspended.get().resume();
  DOBA_EXPECT(finished.load());
  http_server.stop();
}

// +===========================================================================+
// | [>] route precedence over a socket                          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP/1.1 applies static parametrized and wildcard precedence") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server({.ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route("GET", "/items/*", [](const request&, response& res) {
    text_response(res, "wildcard");
  });
  http_server.add_route("GET", "/items/:id",
                        [](const request&, response& res, int id) {
                          text_response(res, "parameter:" + std::to_string(id));
                        });
  http_server.add_route("GET", "/items/42", [](const request&, response& res) {
    text_response(res, "static");
  });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  for (const auto& [path, body] :
       {std::pair{"/items/42", "static"}, std::pair{"/items/7", "parameter:7"},
        std::pair{"/items/name", "wildcard"}}) {
    DOBA_EXPECT(client.send_all(std::string("GET ") + path +
                                " HTTP/1.1\r\nHost: a\r\n\r\n"));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    if (result.has_value()) {
      DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
      DOBA_EXPECT_EQUAL(result->body, body);
    }
  }
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  http_server.stop();
}

// +===========================================================================+
// | [>] typed route conversion boundaries                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP/1.1 routes typed parameter boundaries without partial parses") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> calls = 0;
  server<> http_server({.ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route(
      "GET", "/signed/:value",
      [&calls](const request&, response& res, std::int64_t value) {
        calls.fetch_add(1);
        text_response(res, std::to_string(value));
      });
  http_server.add_route(
      "GET", "/unsigned/:value",
      [&calls](const request&, response& res, std::uint64_t value) {
        calls.fetch_add(1);
        text_response(res, std::to_string(value));
      });
  http_server.add_route("GET", "/boolean/:value",
                        [&calls](const request&, response& res, bool value) {
                          calls.fetch_add(1);
                          text_response(res, value ? "true" : "false");
                        });
  http_server.start();
  struct test_case {
    std::string_view path;
    std::string_view status;
    std::string_view body;
  };
  constexpr test_case cases[] = {
      {"/signed/-9223372036854775808", "HTTP/1.1 200 OK",
       "-9223372036854775808"},
      {"/signed/9223372036854775807", "HTTP/1.1 200 OK", "9223372036854775807"},
      {"/unsigned/18446744073709551615", "HTTP/1.1 200 OK",
       "18446744073709551615"},
      {"/boolean/TRUE", "HTTP/1.1 200 OK", "true"},
      {"/boolean/0", "HTTP/1.1 200 OK", "false"},
      {"/signed/9223372036854775808", "HTTP/1.1 404 Not Found", ""},
      {"/signed/42x", "HTTP/1.1 404 Not Found", ""},
      {"/unsigned/-1", "HTTP/1.1 404 Not Found", ""},
      {"/boolean/yes", "HTTP/1.1 404 Not Found", ""},
  };
  for (const auto& test : cases) {
    martianlabs::doba::tests::integration::test_helper::set_context(test.path);
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("GET " + std::string(test.path) +
                                " HTTP/1.1\r\nHost: a\r\n\r\n"));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    if (result.has_value()) {
      DOBA_EXPECT_EQUAL(result->status, test.status);
      DOBA_EXPECT_EQUAL(result->body, test.body);
    }
    client.close();
  }
  DOBA_EXPECT_EQUAL(calls.load(), 5);
  http_server.stop();
}

// +===========================================================================+
// | [>] request views reach a routed handler                    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP/1.1 exposes decoded path query headers and cookies to routes") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server({.ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route("GET", "/a b", [](const request& req, response& res) {
    const auto query = req.get_query_parameter("key");
    const auto empty = req.get_query_parameter("empty");
    const auto cookie = req.get_cookie("sid");
    const bool valid = req.get_absolute_path() == "/a b" && query.has_value() &&
                       query->second == "one%20two" && empty.has_value() &&
                       empty->second.empty() &&
                       req.get_header("x-id").second == "value" &&
                       cookie.has_value() && *cookie == "abc=123";
    text_response(res, valid ? "valid" : "invalid");
  });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /a%20b?key=one%20two&empty HTTP/1.1\r\n"
      "Host: a\r\nX-Id: value\r\nCookie: sid=abc=123; other=x\r\n\r\n"));
  const auto result = receive_http_response(client);
  DOBA_EXPECT(result.has_value());
  if (result.has_value()) {
    DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(result->body, "valid");
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] allowed methods include every route kind                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP/1.1 reports allowed methods across all matching route kinds") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server({.ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route(
      "GET", "/assets/logo",
      [](const request&, response& res) { text_response(res, "get"); });
  http_server.add_route(
      "POST", "/assets/:id",
      [](const request&, response& res, int) { text_response(res, "post"); });
  http_server.add_route(
      "DELETE", "/assets/*",
      [](const request&, response& res) { text_response(res, "delete"); });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("PUT /assets/42 HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto result = receive_http_response(client);
  DOBA_EXPECT(result.has_value());
  if (result.has_value()) {
    DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 405 Method Not Allowed");
    DOBA_EXPECT_EQUAL(result->header("Allow").value(), "POST, DELETE");
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] running server rejects route mutation                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST(
    "HTTP/1.1 keeps routes immutable while running and reusable after stop") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server({.ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route("GET", "/first", [](const request&, response& res) {
    text_response(res, "first");
  });
  http_server.start();
  bool threw = false;
  try {
    http_server.add_route("GET", "/late", [](const request&, response& res) {
      text_response(res, "late");
    });
  } catch (const std::runtime_error&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("GET /first HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto first = receive_http_response(client);
  DOBA_EXPECT(first.has_value());
  if (first.has_value()) DOBA_EXPECT_EQUAL(first->body, "first");
  client.close();
  http_server.stop();
  http_server.add_route("GET", "/late", [](const request&, response& res) {
    text_response(res, "late");
  });
  http_server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("GET /late HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto late = receive_http_response(client);
  DOBA_EXPECT(late.has_value());
  if (late.has_value()) DOBA_EXPECT_EQUAL(late->body, "late");
  http_server.stop();
}
