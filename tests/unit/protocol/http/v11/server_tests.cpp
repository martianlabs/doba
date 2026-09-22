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
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <type_traits>
#include <utility>

#include "protocol/http/v11/server.h"
#include "test_helper.h"

namespace {
namespace http = martianlabs::doba::protocol::http::v11;
using martianlabs::doba::common::task;
using routes_type = martianlabs::doba::protocol::http::router<
    http::request, http::response>;
using engine_type = http::engine<http::request, http::response>;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] memory_transport                                           ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ENty, typename FNty>
struct memory_transport {
  using policies_type = bool;
  memory_transport(bool fail, FNty factory)
      : fail(fail), factory(std::move(factory)) { instance = this; }
  ~memory_transport() { instance = nullptr; }
  void set_on_connection(std::function<void()>) {}
  void set_on_disconnection(std::function<void()>) {}
  void start() {
    if (fail) throw std::runtime_error("start failed");
    connection.reset(new ENty(factory()));
    connection->set_on_send([this](std::unique_ptr<char[]> buffer,
                                   std::size_t size,
                                   std::optional<martianlabs::doba::common::reader>
                                       source) {
      bytes.append(buffer.get(), size);
      if (source) source->read_all(bytes);
    });
    connection->set_on_close([this]() { closed = true; });
    wake = std::make_shared<std::atomic<bool>>(false);
    connection->set_on_wake([wake = wake]() { wake->store(true); });
    closed = false;
  }
  void stop() {
    if (connection) connection->on_stop();
    connection.reset();
  }
  void poll() {
    if (connection && !closed && wake->exchange(false)) connection->on_wake();
  }
  std::string receive(std::string_view wire) {
    bytes.clear();
    connection->on_bytes_received(wire.data(), wire.size(), 8192);
    poll();
    return bytes;
  }
  bool fail;
  FNty factory;
  std::unique_ptr<ENty> connection;
  std::string bytes;
  bool closed{false};
  std::shared_ptr<std::atomic<bool>> wake;
  static inline memory_transport* instance = nullptr;
};
using test_server = http::server<http::request, http::response, routes_type,
                                 engine_type, memory_transport>;
using test_transport = memory_transport<
    engine_type, http::engine_factory<engine_type, routes_type>>;
std::string send_request(std::string_view method = "GET",
                          std::string_view path = "/") {
  return test_transport::instance->receive(
      std::string(method) + " " + std::string(path) +
      " HTTP/1.1\r\nHost: " +
      std::string(method == "CONNECT" ? path : "example.com") + "\r\n\r\n");
}
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] controller                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct controller {
  explicit controller(int& calls) : calls(calls) {}
  template <typename ROty>
  void register_routes(ROty& routes) { routes.add("GET", "/", &controller::get); }
  http::response get(const http::request&) {
    calls++;
    return http::response::ok_200();
  }
  int& calls;
};
}  // namespace
// +===========================================================================+
// | [>] server is neither copyable nor movable                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("server is neither copyable nor movable") {
  static_assert(!std::is_copy_constructible_v<test_server>);
  static_assert(!std::is_move_constructible_v<test_server>);
  static_assert(!std::is_copy_assignable_v<test_server>);
  static_assert(!std::is_move_assignable_v<test_server>);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] lifecycle routing and callbacks cover server behavior   ( test-case ) |
// +===========================================================================+
DOBA_TEST("lifecycle routing and callbacks cover server behavior") {
  test_server value;
  DOBA_EXPECT_EQUAL(&value.add_route("GET", "/", [](const http::request&) {
    auto result = http::response::ok_200();
    result.set_body("body");
    return result;
  }), &value);
  value.start();
  DOBA_EXPECT(send_request().ends_with("\r\n\r\nbody"));
  DOBA_EXPECT(send_request("GET", "/absent").starts_with("HTTP/1.1 404 "));
  const auto wrong_method = send_request("POST");
  DOBA_EXPECT(wrong_method.starts_with("HTTP/1.1 405 "));
  DOBA_EXPECT(wrong_method.find("Allow: GET\r\n") != std::string::npos);
  DOBA_EXPECT(send_request("OPTIONS", "*").starts_with("HTTP/1.1 200 "));
  DOBA_EXPECT(send_request("CONNECT", "example.com:443").starts_with(
      "HTTP/1.1 501 "));
  value.stop();
  value.start();
  DOBA_EXPECT(send_request().ends_with("\r\n\r\nbody"));
}
// +===========================================================================+
// | [>] server suppresses error bodies only for known HEAD requests( test-case ) |
// +===========================================================================+
DOBA_TEST("server suppresses error bodies only for known HEAD requests") {
  test_server value;
  value.start();
  const auto bytes = test_transport::instance->receive(
      "HEAD / HTTP/1.1\r\nHost: example.com\r\nContent-Length: invalid\r\n\r\n");
  DOBA_EXPECT(bytes.starts_with("HTTP/1.1 400 "));
  DOBA_EXPECT(bytes.ends_with("\r\n\r\n"));
  DOBA_EXPECT(test_transport::instance->closed);
}
// +===========================================================================+
// | [>] failed starts release the date server                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("failed starts release the date server") {
  test_server value(true);
  bool failed = false;
  try { value.start(); } catch (const std::runtime_error&) { failed = true; }
  DOBA_EXPECT(failed);
  value.add_route("GET", "/", [](const http::request&) {
    return http::response::ok_200();
  });
  test_transport::instance->fail = false;
  value.start();
  const auto bytes = send_request();
  DOBA_EXPECT(bytes.starts_with("HTTP/1.1 200 "));
  DOBA_EXPECT(bytes.find("Date: ") != std::string::npos);
}
// +===========================================================================+
// | [>] server completes suspended async responses              ( test-case ) |
// +===========================================================================+
DOBA_TEST("server completes suspended async responses") {
  bool invoked = false;
  std::coroutine_handle<> pending;
  test_server value;
  struct awaiter {
    std::coroutine_handle<>& pending;
    bool await_ready() const { return false; }
    void await_suspend(std::coroutine_handle<> value) { pending = value; }
    void await_resume() const {}
  };
  value.add_route("GET", "/", [&](std::shared_ptr<const http::request>,
                                    std::stop_token) -> task<http::response> {
    invoked = true;
    co_await awaiter{pending};
    auto result = http::response::ok_200();
    result.set_body("async");
    co_return result;
  });
  value.start();
  const auto bytes = send_request();
  DOBA_EXPECT(invoked);
  DOBA_EXPECT(bytes.empty());
  DOBA_EXPECT(pending != nullptr);
  pending.resume();
  test_transport::instance->poll();
  DOBA_EXPECT(test_transport::instance->bytes.ends_with("\r\n\r\nasync"));
}
// +===========================================================================+
// | [>] server propagates async handler exceptions              ( test-case ) |
// +===========================================================================+
DOBA_TEST("server propagates async handler exceptions") {
  test_server value;
  value.add_route("GET", "/", [](std::shared_ptr<const http::request>,
                                  std::stop_token) -> task<http::response> {
    throw std::runtime_error("handler failed");
    co_return http::response::ok_200();
  });
  value.start();
  DOBA_EXPECT(send_request().starts_with("HTTP/1.1 500 "));
}
// +===========================================================================+
// | [>] server invokes parametrized async handlers              ( test-case ) |
// +===========================================================================+
DOBA_TEST("server invokes parametrized async handlers") {
  test_server value;
  value.add_route("GET", "/item/:id",
      [](std::shared_ptr<const http::request>, std::stop_token,
          std::string_view id) -> task<http::response> {
        auto result = http::response::ok_200();
        result.set_body(id);
        co_return result;
      });
  value.start();
  DOBA_EXPECT(send_request("GET", "/item/42").ends_with("\r\n\r\n42"));
}
// +===========================================================================+
// | [>] sync handler failure preserves subsequent dispatch      ( test-case ) |
// +===========================================================================+
DOBA_TEST("sync handler failure preserves subsequent dispatch") {
  test_server value;
  value.add_route("GET", "/fail", [](const http::request&) -> http::response {
    throw std::runtime_error("handler failed");
  });
  value.add_route("GET", "/", [](const http::request&) {
    return http::response::ok_200();
  });
  value.start();
  DOBA_EXPECT(send_request("GET", "/fail").starts_with("HTTP/1.1 500 "));
  DOBA_EXPECT(send_request().starts_with("HTTP/1.1 200 "));
}
// +===========================================================================+
// | [>] async handler observes cancellation after suspension    ( test-case ) |
// +===========================================================================+
DOBA_TEST("async handler observes cancellation after suspension") {
  std::stop_token token;
  bool invoked = false;
  std::coroutine_handle<> pending;
  struct awaiter {
    std::coroutine_handle<>& pending;
    bool await_ready() const { return false; }
    void await_suspend(std::coroutine_handle<> value) { pending = value; }
    void await_resume() const {}
  };
  test_server value;
  value.add_route("GET", "/", [&](std::shared_ptr<const http::request>,
                                    std::stop_token stop) -> task<http::response> {
    invoked = true;
    token = stop;
    co_await awaiter{pending};
    co_return http::response::ok_200();
  });
  value.start();
  send_request();
  DOBA_EXPECT(invoked);
  value.stop();
  DOBA_EXPECT(token.stop_requested());
  pending.resume();
}
// +===========================================================================+
// | [>] server retains controllers and rejects live registration( test-case ) |
// +===========================================================================+
DOBA_TEST("server retains controllers and rejects live registration") {
  int calls = 0;
  test_server value;
  value.add_controller<controller>(calls);
  value.start();
  DOBA_EXPECT(send_request().starts_with("HTTP/1.1 200 "));
  DOBA_EXPECT_EQUAL(calls, 1);
  bool rejected = false;
  try { value.add_controller<controller>(calls); }
  catch (const std::runtime_error&) { rejected = true; }
  DOBA_EXPECT(rejected);
  value.stop();
  value.start();
  send_request();
  DOBA_EXPECT_EQUAL(calls, 2);
}
