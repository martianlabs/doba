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

#include <coroutine>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "protocol/http/v11/server.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::target;
using martianlabs::doba::protocol::http::v11::decoder;
using martianlabs::doba::protocol::http::v11::rejection_reason;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::protocol::http::v11::server;
using martianlabs::doba::protocol::http::router_async_handler_lambda;
using martianlabs::doba::protocol::http::router_handler_lambda;
using martianlabs::doba::common::task;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] request                                                    ( struct ) |
// +---------------------------------------------------------------------------+
// | Request double used by the server tests.                                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct request {
  // +=========================================================================+
  // | [>] get_method                                               ( public ) |
  // +=========================================================================+
  std::string_view get_method() const { return method; }
  // +=========================================================================+
  // | [>] get_target                                               ( public ) |
  // +=========================================================================+
  target get_target() const { return target_form; }
  // +=========================================================================+
  // | [>] get_absolute_path                                        ( public ) |
  // +=========================================================================+
  std::string_view get_absolute_path() const { return path; }
  // +=========================================================================+
  // | [>] wants_connection_close                                   ( public ) |
  // +=========================================================================+
  bool wants_connection_close() const { return close; }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  std::string_view method = "GET";
  target target_form = target::kOriginForm;
  std::string_view path = "/";
  bool close = false;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] fake_router                                                 ( class ) |
// +---------------------------------------------------------------------------+
// | Router double used by the server tests.                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty>
class fake_router {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  struct handler_data {
    std::function<void(const RQty&, RSty&)> callback;
    std::function<task<RSty>(std::shared_ptr<const RQty>)> async_callback;
    bool is_async() const {
      return static_cast<bool>(async_callback);
    }
  };
  struct parametrized_handler_data {
    void invoke(const RQty&, RSty&, std::string_view) const {}
    task<RSty> invoke_async(std::shared_ptr<const RQty>,
                            std::string_view) const {
      co_return RSty{};
    }
    bool is_async() const { return false; }
  };
  struct route_match {
    const handler_data* handler{nullptr};
    const parametrized_handler_data* parametrized_handler{nullptr};
    explicit operator bool() const {
      return handler != nullptr || parametrized_handler != nullptr;
    }
  };
  // +=========================================================================+
  // | [>] add                                                      ( public ) |
  // +=========================================================================+
  template <typename Hty>
  void add(std::string_view method, std::string_view route, Hty handler) {
    additions++;
    last_method = method;
    last_route = route;
    if constexpr (router_handler_lambda<Hty>) {
      matched_handler = {
          std::function<void(const RQty&, RSty&)>(std::move(handler)), {}};
    } else if constexpr (router_async_handler_lambda<Hty>) {
      async_handler = {
          {}, std::function<task<RSty>(std::shared_ptr<const RQty>)>(
                  std::move(handler))};
    }
  }
  // +=========================================================================+
  // | [>] match                                                    ( public ) |
  // +=========================================================================+
  route_match match(std::string_view method, std::string_view path) {
    matched_method = method;
    matched_path = path;
    if (!match_available) return {};
    return path == "/async" ? route_match{&async_handler, nullptr}
                            : route_match{&matched_handler, nullptr};
  }
  // +=========================================================================+
  // | [>] allowed_methods                                          ( public ) |
  // +=========================================================================+
  std::string allowed_methods(std::string_view) {
    return allowed_methods_result;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  static inline std::size_t additions = 0;
  static inline std::string last_method;
  static inline std::string last_route;
  static inline handler_data matched_handler{};
  static inline handler_data async_handler{};
  static inline bool match_available = true;
  static inline std::string allowed_methods_result;
  static inline std::string matched_method;
  static inline std::string matched_path;
  static inline bool write_body = false;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] fake_transport                                              ( class ) |
// +---------------------------------------------------------------------------+
// | Transport double used by the server tests.                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty,
          template <typename, typename> typename DEty>
class fake_transport {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  using request_callback =
      std::function<std::optional<martianlabs::doba::common::task<RSty>>(
          const std::shared_ptr<RQty>&, RSty&)>;
  using bad_request_callback =
      std::function<void(int, std::string_view, RSty&)>;
  // +=========================================================================+
  // | [>] set_on_request                                           ( public ) |
  // +=========================================================================+
  void set_on_request(request_callback callback) {
    on_request = std::move(callback);
  }
  // +=========================================================================+
  // | [>] set_on_bad_request                                       ( public ) |
  // +=========================================================================+
  void set_on_bad_request(bad_request_callback callback) {
    on_bad_request = std::move(callback);
  }
  // +=========================================================================+
  // | [>] set_on_connection                                        ( public ) |
  // +=========================================================================+
  void set_on_connection(std::function<void()> callback) {
    on_connection = std::move(callback);
  }
  // +=========================================================================+
  // | [>] set_on_disconnection                                     ( public ) |
  // +=========================================================================+
  void set_on_disconnection(std::function<void()> callback) {
    on_disconnection = std::move(callback);
  }
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +=========================================================================+
  void start(const char port[]) {
    started = true;
    started_port = port;
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() { started = false; }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  static inline request_callback on_request;
  static inline bad_request_callback on_bad_request;
  static inline std::function<void()> on_connection;
  static inline std::function<void()> on_disconnection;
  static inline bool started = false;
  static inline std::string started_port;
};

using test_server =
    server<request, response, decoder, fake_transport, fake_router>;
using test_router = fake_router<request, response>;
using test_transport = fake_transport<request, response, decoder>;

class task_probe {
 public:
  struct promise_type {
    task_probe get_return_object() noexcept {
      return task_probe(
          std::coroutine_handle<promise_type>::from_promise(*this));
    }
    std::suspend_never initial_suspend() const noexcept { return {}; }
    std::suspend_always final_suspend() const noexcept { return {}; }
    void return_void() const noexcept {}
    void unhandled_exception() noexcept {
      exception_ = std::current_exception();
    }
    std::exception_ptr exception_;
  };
  task_probe(const task_probe&) = delete;
  task_probe(task_probe&& in) noexcept
      : coroutine_(std::exchange(in.coroutine_, nullptr)) {}
  ~task_probe() {
    if (coroutine_) coroutine_.destroy();
  }
  [[nodiscard]] bool done() const noexcept { return coroutine_.done(); }
  void rethrow_if_failed() const {
    if (coroutine_.promise().exception_) {
      std::rethrow_exception(coroutine_.promise().exception_);
    }
  }

 private:
  explicit task_probe(std::coroutine_handle<promise_type> coroutine) noexcept
      : coroutine_(coroutine) {}
  std::coroutine_handle<promise_type> coroutine_;
};

template <typename Tty>
task_probe collect(task<Tty> value, std::optional<Tty>& result) {
  result.emplace(co_await std::move(value));
}

// +===========================================================================+
// | [>] send_request                                             ( function ) |
// +===========================================================================+
std::string send_request(const request& req) {
  response res;
  auto shared_request = std::make_shared<request>(req);
  auto response_task = test_transport::on_request(shared_request, res);
  if (response_task) {
    std::optional<response> result;
    auto probe = collect(std::move(*response_task), result);
    probe.rethrow_if_failed();
    if (!probe.done() || !result) {
      throw std::runtime_error("The asynchronous response did not complete");
    }
    res = std::move(*result);
  }
  res.set_header("Date", "fixed");
  return res.serialize()->prefix;
}
}  // namespace

// +===========================================================================+
// | [>] server is neither copyable nor movable                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("server is neither copyable nor movable") {
  static_assert(!std::is_copy_constructible_v<test_server>);
  static_assert(!std::is_copy_assignable_v<test_server>);
  static_assert(!std::is_move_constructible_v<test_server>);
  static_assert(!std::is_move_assignable_v<test_server>);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] lifecycle routing and callbacks cover server behavior   ( test-case ) |
// +===========================================================================+
DOBA_TEST("lifecycle routing and callbacks cover server behavior") {
  // ---------------------------------------------------------------------------
  // Route registration
  // ---------------------------------------------------------------------------
  test_router::additions = 0;
  test_server value;
  auto handler = [](const request&, response& res) {
    res.ok_200();
    if (test_router::write_body) res.set_body("body");
  };
  DOBA_EXPECT_EQUAL(&value.add_route("GET", "/", handler), &value);
  DOBA_EXPECT_EQUAL(test_router::additions, 1);
  DOBA_EXPECT_EQUAL(test_router::last_method, "GET");
  DOBA_EXPECT_EQUAL(test_router::last_route, "/");
  auto async_handler =
      [](std::shared_ptr<const request>) -> task<response> {
    response res;
    res.ok_200().set_body("async");
    co_return res;
  };
  DOBA_EXPECT_EQUAL(
      &value.add_route("GET", "/async", async_handler), &value);
  DOBA_EXPECT_EQUAL(test_router::additions, 2);
  DOBA_EXPECT_EQUAL(test_router::last_method, "GET");
  DOBA_EXPECT_EQUAL(test_router::last_route, "/async");
  // ---------------------------------------------------------------------------
  // Lifecycle
  // ---------------------------------------------------------------------------
  value.start("8080");
  DOBA_EXPECT(test_transport::started);
  DOBA_EXPECT_EQUAL(test_transport::started_port, "8080");
  // ---------------------------------------------------------------------------
  // Route locking
  // ---------------------------------------------------------------------------
  bool threw = false;
  try {
    value.add_route("POST", "/", handler);
  } catch (const std::runtime_error&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  DOBA_EXPECT_EQUAL(test_router::additions, 2);
  // ---------------------------------------------------------------------------
  // Router results
  // ---------------------------------------------------------------------------
  test_router::write_body = false;
  struct route_case {
    bool matched;
    std::string_view allowed_methods;
    std::string_view status;
  };
  constexpr route_case route_cases[] = {
      {true, "", "HTTP/1.1 200 OK\r\n"},
      {false, "", "HTTP/1.1 404 Not Found\r\n"},
      {false, "GET", "HTTP/1.1 405 Method Not Allowed\r\n"},
  };
  for (const auto& test : route_cases) {
    test_router::match_available = test.matched;
    test_router::allowed_methods_result = test.allowed_methods;
    const std::string output = send_request(request{});
    DOBA_EXPECT(std::string_view(output).starts_with(test.status));
    if (test.allowed_methods.empty()) {
      DOBA_EXPECT(output.find("Allow:") == std::string::npos);
    } else {
      DOBA_EXPECT(output.find("Allow: GET\r\n") != std::string::npos);
    }
    DOBA_EXPECT_EQUAL(test_router::matched_method, "GET");
    DOBA_EXPECT_EQUAL(test_router::matched_path, "/");
  }
  // ---------------------------------------------------------------------------
  // Request target forms
  // ---------------------------------------------------------------------------
  struct target_case {
    target target_form;
    std::string_view status;
  };
  constexpr target_case target_cases[] = {
      {target::kAuthorityForm, "HTTP/1.1 501 Not Implemented\r\n"},
      {target::kAsteriskForm, "HTTP/1.1 200 OK\r\n"},
      {target::kUnknown, "HTTP/1.1 400 Bad Request\r\n"},
  };
  for (const auto& test : target_cases) {
    request req;
    req.target_form = test.target_form;
    const std::string output = send_request(req);
    DOBA_EXPECT(std::string_view(output).starts_with(test.status));
  }
  // ---------------------------------------------------------------------------
  // Connection close
  // ---------------------------------------------------------------------------
  test_router::match_available = true;
  test_router::allowed_methods_result.clear();
  request req;
  req.close = true;
  std::string output = send_request(req);
  DOBA_EXPECT(output.find("Connection: close\r\n") != std::string::npos);
  // ---------------------------------------------------------------------------
  // HEAD response
  // ---------------------------------------------------------------------------
  test_router::write_body = true;
  req = {};
  req.method = "HEAD";
  output = send_request(req);
  DOBA_EXPECT(output.find("Content-Length: 4\r\n") != std::string::npos);
  DOBA_EXPECT(!std::string_view(output).ends_with("body"));
  test_router::write_body = false;
  // ---------------------------------------------------------------------------
  // Asynchronous response rules
  // ---------------------------------------------------------------------------
  req = {};
  req.method = "HEAD";
  req.path = "/async";
  req.close = true;
  output = send_request(req);
  DOBA_EXPECT(output.find("Connection: close\r\n") != std::string::npos);
  DOBA_EXPECT(output.find("Content-Length: 5\r\n") != std::string::npos);
  DOBA_EXPECT(!std::string_view(output).ends_with("async"));
  // ---------------------------------------------------------------------------
  // Rejection reasons
  // ---------------------------------------------------------------------------
  struct rejection_case {
    rejection_reason reason;
    std::string_view status;
  };
  constexpr rejection_case rejection_cases[] = {
      {rejection_reason::kNone, "HTTP/1.1 400 Bad Request\r\n"},
      {rejection_reason::kSyntax, "HTTP/1.1 400 Bad Request\r\n"},
      {rejection_reason::kPayloadTooLarge,
       "HTTP/1.1 413 Content Too Large\r\n"},
      {rejection_reason::kUnsupportedFeature,
       "HTTP/1.1 501 Not Implemented\r\n"},
      {rejection_reason::kVersionNotSupported,
       "HTTP/1.1 505 HTTP Version Not Supported\r\n"},
      {rejection_reason::kUriTooLong, "HTTP/1.1 414 URI Too Long\r\n"},
      {rejection_reason::kHeaderFieldsTooLarge,
       "HTTP/1.1 431 Request Header Fields Too Large\r\n"},
      {rejection_reason::kHandlerError,
       "HTTP/1.1 500 Internal Server Error\r\n"},
      {rejection_reason::kExpectationFailed,
       "HTTP/1.1 417 Expectation Failed\r\n"},
  };
  for (const auto& test : rejection_cases) {
    response res;
    test_transport::on_bad_request(static_cast<int>(test.reason), "reason",
                                   res);
    res.set_header("Date", "fixed");
    const auto serialized = res.serialize();
    DOBA_EXPECT(serialized->prefix.starts_with(test.status));
    DOBA_EXPECT(serialized->prefix.ends_with("reason"));
  }
  // ---------------------------------------------------------------------------
  // Stop
  // ---------------------------------------------------------------------------
  value.stop();
  DOBA_EXPECT(!test_transport::started);
}
