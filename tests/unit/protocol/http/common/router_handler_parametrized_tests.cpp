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

#include <cstdint>
#include <coroutine>
#include <exception>
#include <memory>
#include <limits>
#include <optional>
#include <stop_token>
#include <string>
#include <string_view>
#include <utility>

#include "protocol/http/common/router_handler_parametrized.h"
#include "test_helper.h"

namespace {
struct request {};
struct response {
  std::string value;
};
using martianlabs::doba::protocol::http::make_router_handler_parametrized;
using martianlabs::doba::protocol::http::
    make_router_handler_parametrized_async;
using martianlabs::doba::common::task;

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

class manual_event {
 public:
  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> continuation) noexcept {
    continuation_ = continuation;
  }
  void await_resume() const noexcept {}
  void resume() {
    auto continuation = std::exchange(continuation_, nullptr);
    continuation.resume();
  }

 private:
  std::coroutine_handle<> continuation_;
};

template <typename Tty>
task_probe collect(task<Tty> value, std::optional<Tty>& result) {
  result.emplace(co_await std::move(value));
}
}  // namespace

// +===========================================================================+
// | [>] matches routes with typed parameters                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("matches routes with typed parameters") {
  auto handler = make_router_handler_parametrized<
      request, response, std::uint64_t, bool, double, std::string_view>(
      "/items/:id/:enabled/:score/:name",
      [](const request&, std::uint64_t, bool, double, std::string_view) {
        response res;
        return res;
      });
  DOBA_EXPECT(handler.matches("/items/42/TRUE/1.5/doba"));
  DOBA_EXPECT(!handler.matches("/items/x/true/1.5/doba"));
  DOBA_EXPECT(!handler.matches("/items/42/yes/1.5/doba"));
  DOBA_EXPECT(!handler.matches("/items/42/true/score/doba"));
}
// +===========================================================================+
// | [>] matching requires the complete route shape              ( test-case ) |
// +===========================================================================+
DOBA_TEST("matching requires the complete route shape") {
  auto handler = make_router_handler_parametrized<request, response, int>(
      "/items/:id",
      [](const request&, int) {
        response res;
        return res;
      });
  DOBA_EXPECT(handler.matches("/items/42"));
  DOBA_EXPECT(!handler.matches("/items/"));
  DOBA_EXPECT(!handler.matches("/items/42/"));
  DOBA_EXPECT(!handler.matches("/items/42/details"));
}
// +===========================================================================+
// | [>] invoke passes parsed values to the callback             ( test-case ) |
// +===========================================================================+
DOBA_TEST("invoke passes parsed values to the callback") {
  bool invoked = false;
  auto handler = make_router_handler_parametrized<
      request, response, std::uint64_t, bool, double, std::string>(
      "/items/:id/:enabled/:score/:name",
      [&invoked](const request&, std::uint64_t id, bool enabled,
                 double score, const std::string& name) {
        response res;
        invoked = id == 42 && enabled && score == 1.5 && name == "doba";
        res.value = name;
        return res;
      });
  request req;
  response res;
  res = handler.invoke(req, "/items/42/true/1.5/doba");
  DOBA_EXPECT(invoked);
  DOBA_EXPECT_EQUAL(res.value, "doba");
}
// +===========================================================================+
// | [>] invoke ignores paths with invalid parameters            ( test-case ) |
// +===========================================================================+
DOBA_TEST("invoke ignores paths with invalid parameters") {
  bool invoked = false;
  auto handler = make_router_handler_parametrized<request, response, int>(
      "/items/:id",
      [&invoked](const request&, int) {
        response res;
        invoked = true;
        return res;
      });
  request req;
  response res;
  res = handler.invoke(req, "/items/value");
  DOBA_EXPECT(!invoked);
}
// +===========================================================================+
// | [>] async invoke retains parameters while suspended         ( test-case ) |
// +===========================================================================+
DOBA_TEST("async invoke retains parameters while suspended") {
  manual_event event;
  auto handler = make_router_handler_parametrized_async<
      request, response, std::string>(
      "/items/:name",
      [&event](std::shared_ptr<const request>, std::stop_token,
               const std::string& name) -> task<response> {
        co_await event;
        response res;
        res.value = name;
        co_return res;
      });
  auto req = std::make_shared<const request>();
  std::optional<response> result;
  auto probe = collect(
      handler.invoke_async(req, std::stop_token{}, "/items/doba"), result);
  DOBA_EXPECT(!probe.done());
  DOBA_EXPECT(!result.has_value());
  event.resume();
  probe.rethrow_if_failed();
  DOBA_EXPECT(probe.done());
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->value, "doba");
}
// +===========================================================================+
// | [>] async invoke passes parsed values to the callback       ( test-case ) |
// +===========================================================================+
DOBA_TEST("async invoke passes parsed values to the callback") {
  std::stop_source stop_source;
  stop_source.request_stop();
  auto handler = make_router_handler_parametrized_async<
      request, response, std::uint64_t, bool, double, std::string_view>(
      "/items/:id/:enabled/:score/:name",
      [](std::shared_ptr<const request>, std::stop_token stop_token,
         std::uint64_t id, bool enabled, double score,
         std::string_view name) -> task<response> {
        const bool valid =
            stop_token.stop_requested() && id == 42 && enabled &&
            score == 1.5 && name == "doba";
        co_return response{valid ? "converted" : "invalid"};
      });
  std::optional<response> result;
  auto probe = collect(
      handler.invoke_async(std::make_shared<const request>(),
                           stop_source.get_token(),
                           "/items/42/true/1.5/doba"),
      result);
  probe.rethrow_if_failed();
  DOBA_EXPECT(probe.done());
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->value, "converted");
}
// +===========================================================================+
// | [>] async invoke reports invalid parameters                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("async invoke reports invalid parameters") {
  auto handler = make_router_handler_parametrized_async<
      request, response, int>(
      "/items/:id",
      [](std::shared_ptr<const request>, std::stop_token,
         int) -> task<response> {
        co_return response{};
      });
  std::optional<response> result;
  auto probe = collect(
      handler.invoke_async(std::make_shared<const request>(),
                           std::stop_token{}, "/items/value"),
      result);
  DOBA_EXPECT(probe.done());
  bool threw = false;
  try {
    probe.rethrow_if_failed();
  } catch (const std::runtime_error& error) {
    threw = std::string_view(error.what()) ==
            "The route parameters could not be parsed";
  }
  DOBA_EXPECT(threw);
  DOBA_EXPECT(!result.has_value());
}
// +===========================================================================+
// | [>] async invoke reports an incompatible path               ( test-case ) |
// +===========================================================================+
DOBA_TEST("async invoke reports an incompatible path") {
  auto handler = make_router_handler_parametrized_async<
      request, response, int>(
      "/items/:id",
      [](std::shared_ptr<const request>, std::stop_token,
         int) -> task<response> {
        co_return response{};
      });
  std::optional<response> result;
  auto probe = collect(
      handler.invoke_async(std::make_shared<const request>(),
                           std::stop_token{}, "/other/42"),
      result);
  DOBA_EXPECT(probe.done());
  bool threw = false;
  try {
    probe.rethrow_if_failed();
  } catch (const std::runtime_error& error) {
    threw = std::string_view(error.what()) ==
            "The route parameters could not be extracted";
  }
  DOBA_EXPECT(threw);
  DOBA_EXPECT(!result.has_value());
}

// +===========================================================================+
// | [>] accepts every boolean spelling                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("route conversion accepts every boolean spelling") {
  using parameter = bool;
  struct test_case {
    std::string_view input;
    parameter expected;
  };
  const test_case cases[] = {
      {"false", false},
      {"FALSE", false},
      {"FaLsE", false},
      {"0", false},
      {"1", true},
  };
  for (const auto& test : cases) {
    const std::string path = "/value/" + std::string(test.input);
    martianlabs::doba::tests::unit::test_helper::set_context(test.input);
    std::size_t sync_calls = 0;
    std::size_t async_calls = 0;
    parameter sync_value{};
    parameter async_value{};
    auto handler = make_router_handler_parametrized<
        request, response, parameter>(
        "/value/:value",
        [&](const request&, parameter value) {
          response res;
          sync_calls++;
          sync_value = value;
          return res;
        });
    auto async_handler =
        make_router_handler_parametrized_async<request, response, parameter>(
            "/value/:value",
            [&](std::shared_ptr<const request>, std::stop_token,
                parameter value) -> task<response> {
              async_calls++;
              async_value = value;
              co_return response{"converted"};
            });
    DOBA_EXPECT_EQUAL(handler.matches(path), true);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
    request req;
    response res;
    res = handler.invoke(req, path);
    DOBA_EXPECT_EQUAL(sync_calls, 1);
    DOBA_EXPECT_EQUAL(sync_value, test.expected);
    std::optional<response> result;
    auto probe = collect(
        async_handler.invoke_async(std::make_shared<const request>(),
                                    std::stop_token{}, path), result);
    DOBA_EXPECT(probe.done());
    probe.rethrow_if_failed();
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->value, "converted");
    DOBA_EXPECT_EQUAL(async_value, test.expected);
    DOBA_EXPECT_EQUAL(async_calls, 1);
  }
}

// +===========================================================================+
// | [>] accepts signed integer boundaries                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("route conversion accepts signed integer boundaries") {
  using parameter = std::int64_t;
  struct test_case {
    std::string_view input;
    parameter expected;
  };
  const test_case cases[] = {
      {"-9223372036854775808", std::numeric_limits<parameter>::min()},
      {"9223372036854775807", std::numeric_limits<parameter>::max()},
      {"-1", -1},
      {"0", 0},
  };
  for (const auto& test : cases) {
    const std::string path = "/value/" + std::string(test.input);
    martianlabs::doba::tests::unit::test_helper::set_context(test.input);
    std::size_t sync_calls = 0;
    std::size_t async_calls = 0;
    parameter sync_value{};
    parameter async_value{};
    auto handler = make_router_handler_parametrized<
        request, response, parameter>(
        "/value/:value",
        [&](const request&, parameter value) {
          response res;
          sync_calls++;
          sync_value = value;
          return res;
        });
    auto async_handler =
        make_router_handler_parametrized_async<request, response, parameter>(
            "/value/:value",
            [&](std::shared_ptr<const request>, std::stop_token,
                parameter value) -> task<response> {
              async_calls++;
              async_value = value;
              co_return response{"converted"};
            });
    DOBA_EXPECT_EQUAL(handler.matches(path), true);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
    request req;
    response res;
    res = handler.invoke(req, path);
    DOBA_EXPECT_EQUAL(sync_calls, 1);
    DOBA_EXPECT_EQUAL(sync_value, test.expected);
    std::optional<response> result;
    auto probe = collect(
        async_handler.invoke_async(std::make_shared<const request>(),
                                    std::stop_token{}, path), result);
    DOBA_EXPECT(probe.done());
    probe.rethrow_if_failed();
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->value, "converted");
    DOBA_EXPECT_EQUAL(async_value, test.expected);
    DOBA_EXPECT_EQUAL(async_calls, 1);
  }
}

// +===========================================================================+
// | [>] accepts unsigned integer boundaries                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("route conversion accepts unsigned integer boundaries") {
  using parameter = std::uint64_t;
  struct test_case {
    std::string_view input;
    parameter expected;
  };
  const test_case cases[] = {
      {"0", 0},
      {"18446744073709551615", std::numeric_limits<parameter>::max()},
  };
  for (const auto& test : cases) {
    const std::string path = "/value/" + std::string(test.input);
    martianlabs::doba::tests::unit::test_helper::set_context(test.input);
    std::size_t sync_calls = 0;
    std::size_t async_calls = 0;
    parameter sync_value{};
    parameter async_value{};
    auto handler = make_router_handler_parametrized<
        request, response, parameter>(
        "/value/:value",
        [&](const request&, parameter value) {
          response res;
          sync_calls++;
          sync_value = value;
          return res;
        });
    auto async_handler =
        make_router_handler_parametrized_async<request, response, parameter>(
            "/value/:value",
            [&](std::shared_ptr<const request>, std::stop_token,
                parameter value) -> task<response> {
              async_calls++;
              async_value = value;
              co_return response{"converted"};
            });
    DOBA_EXPECT_EQUAL(handler.matches(path), true);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
    request req;
    response res;
    res = handler.invoke(req, path);
    DOBA_EXPECT_EQUAL(sync_calls, 1);
    DOBA_EXPECT_EQUAL(sync_value, test.expected);
    std::optional<response> result;
    auto probe = collect(
        async_handler.invoke_async(std::make_shared<const request>(),
                                    std::stop_token{}, path), result);
    DOBA_EXPECT(probe.done());
    probe.rethrow_if_failed();
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->value, "converted");
    DOBA_EXPECT_EQUAL(async_value, test.expected);
    DOBA_EXPECT_EQUAL(async_calls, 1);
  }
}

// +===========================================================================+
// | [>] rejects integer overflow                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("route conversion rejects integer overflow") {
  {
    using parameter = std::int64_t;
    struct test_case {
      std::string_view input;
    };
    const test_case cases[] = {
        {"-9223372036854775809"},
        {"9223372036854775808"},
    };
    for (const auto& test : cases) {
      const std::string path = "/value/" + std::string(test.input);
      martianlabs::doba::tests::unit::test_helper::set_context(test.input);
      std::size_t sync_calls = 0;
      std::size_t async_calls = 0;
      auto handler = make_router_handler_parametrized<
          request, response, parameter>(
          "/value/:value",
          [&](const request&, parameter) {
            response res;
            sync_calls++;
            return res;
          });
      auto async_handler =
          make_router_handler_parametrized_async<request, response, parameter>(
              "/value/:value",
              [&](std::shared_ptr<const request>, std::stop_token,
                  parameter) -> task<response> {
                async_calls++;
                co_return response{"converted"};
              });
      DOBA_EXPECT_EQUAL(handler.matches(path), false);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
      request req;
      response res;
      res = handler.invoke(req, path);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
      std::optional<response> result;
      auto probe = collect(
          async_handler.invoke_async(std::make_shared<const request>(),
                                      std::stop_token{}, path), result);
      DOBA_EXPECT(probe.done());
      bool threw = false;
      try {
        probe.rethrow_if_failed();
      } catch (const std::runtime_error& error) {
        threw = std::string_view(error.what()) ==
                "The route parameters could not be parsed";
      }
      DOBA_EXPECT(threw);
      DOBA_EXPECT(!result.has_value());
      DOBA_EXPECT_EQUAL(async_calls, 0);
    }
  }
  {
    using parameter = std::uint64_t;
    struct test_case {
      std::string_view input;
    };
    const test_case cases[] = {
        {"18446744073709551616"},
    };
    for (const auto& test : cases) {
      const std::string path = "/value/" + std::string(test.input);
      martianlabs::doba::tests::unit::test_helper::set_context(test.input);
      std::size_t sync_calls = 0;
      std::size_t async_calls = 0;
      auto handler = make_router_handler_parametrized<
          request, response, parameter>(
          "/value/:value",
          [&](const request&, parameter) {
            response res;
            sync_calls++;
            return res;
          });
      auto async_handler =
          make_router_handler_parametrized_async<request, response, parameter>(
              "/value/:value",
              [&](std::shared_ptr<const request>, std::stop_token,
                  parameter) -> task<response> {
                async_calls++;
                co_return response{"converted"};
              });
      DOBA_EXPECT_EQUAL(handler.matches(path), false);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
      request req;
      response res;
      res = handler.invoke(req, path);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
      std::optional<response> result;
      auto probe = collect(
          async_handler.invoke_async(std::make_shared<const request>(),
                                      std::stop_token{}, path), result);
      DOBA_EXPECT(probe.done());
      bool threw = false;
      try {
        probe.rethrow_if_failed();
      } catch (const std::runtime_error& error) {
        threw = std::string_view(error.what()) ==
                "The route parameters could not be parsed";
      }
      DOBA_EXPECT(threw);
      DOBA_EXPECT(!result.has_value());
      DOBA_EXPECT_EQUAL(async_calls, 0);
    }
  }
}

// +===========================================================================+
// | [>] rejects partial numbers spaces and plus signs           ( test-case ) |
// +===========================================================================+
DOBA_TEST("route conversion rejects partial numbers spaces and plus signs") {
  {
    using parameter = int;
    struct test_case {
      std::string_view input;
    };
    const test_case cases[] = {
        {"42x"},
        {" 42"},
        {"42 "},
        {"+42"},
    };
    for (const auto& test : cases) {
      const std::string path = "/value/" + std::string(test.input);
      martianlabs::doba::tests::unit::test_helper::set_context(test.input);
      std::size_t sync_calls = 0;
      std::size_t async_calls = 0;
      auto handler = make_router_handler_parametrized<
          request, response, parameter>(
          "/value/:value",
          [&](const request&, parameter) {
            response res;
            sync_calls++;
            return res;
          });
      auto async_handler =
          make_router_handler_parametrized_async<request, response, parameter>(
              "/value/:value",
              [&](std::shared_ptr<const request>, std::stop_token,
                  parameter) -> task<response> {
                async_calls++;
                co_return response{"converted"};
              });
      DOBA_EXPECT_EQUAL(handler.matches(path), false);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
      request req;
      response res;
      res = handler.invoke(req, path);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
      std::optional<response> result;
      auto probe = collect(
          async_handler.invoke_async(std::make_shared<const request>(),
                                      std::stop_token{}, path), result);
      DOBA_EXPECT(probe.done());
      bool threw = false;
      try {
        probe.rethrow_if_failed();
      } catch (const std::runtime_error& error) {
        threw = std::string_view(error.what()) ==
                "The route parameters could not be parsed";
      }
      DOBA_EXPECT(threw);
      DOBA_EXPECT(!result.has_value());
      DOBA_EXPECT_EQUAL(async_calls, 0);
    }
  }
  {
    using parameter = double;
    struct test_case {
      std::string_view input;
    };
    const test_case cases[] = {
        {"1.5x"},
        {"+1.5"},
    };
    for (const auto& test : cases) {
      const std::string path = "/value/" + std::string(test.input);
      martianlabs::doba::tests::unit::test_helper::set_context(test.input);
      std::size_t sync_calls = 0;
      std::size_t async_calls = 0;
      auto handler = make_router_handler_parametrized<
          request, response, parameter>(
          "/value/:value",
          [&](const request&, parameter) {
            response res;
            sync_calls++;
            return res;
          });
      auto async_handler =
          make_router_handler_parametrized_async<request, response, parameter>(
              "/value/:value",
              [&](std::shared_ptr<const request>, std::stop_token,
                  parameter) -> task<response> {
                async_calls++;
                co_return response{"converted"};
              });
      DOBA_EXPECT_EQUAL(handler.matches(path), false);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
      request req;
      response res;
      res = handler.invoke(req, path);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
      std::optional<response> result;
      auto probe = collect(
          async_handler.invoke_async(std::make_shared<const request>(),
                                      std::stop_token{}, path), result);
      DOBA_EXPECT(probe.done());
      bool threw = false;
      try {
        probe.rethrow_if_failed();
      } catch (const std::runtime_error& error) {
        threw = std::string_view(error.what()) ==
                "The route parameters could not be parsed";
      }
      DOBA_EXPECT(threw);
      DOBA_EXPECT(!result.has_value());
      DOBA_EXPECT_EQUAL(async_calls, 0);
    }
  }
}

// +===========================================================================+
// | [>] rejects negative unsigned values                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("route conversion rejects negative unsigned values") {
  using parameter = std::uint64_t;
  struct test_case {
    std::string_view input;
  };
  const test_case cases[] = {
      {"-1"},
      {"-0"},
  };
  for (const auto& test : cases) {
    const std::string path = "/value/" + std::string(test.input);
    martianlabs::doba::tests::unit::test_helper::set_context(test.input);
    std::size_t sync_calls = 0;
    std::size_t async_calls = 0;
    auto handler = make_router_handler_parametrized<
        request, response, parameter>(
        "/value/:value",
        [&](const request&, parameter) {
          response res;
          sync_calls++;
          return res;
        });
    auto async_handler =
        make_router_handler_parametrized_async<request, response, parameter>(
            "/value/:value",
            [&](std::shared_ptr<const request>, std::stop_token,
                parameter) -> task<response> {
              async_calls++;
              co_return response{"converted"};
            });
    DOBA_EXPECT_EQUAL(handler.matches(path), false);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
    request req;
    response res;
    res = handler.invoke(req, path);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
    std::optional<response> result;
    auto probe = collect(
        async_handler.invoke_async(std::make_shared<const request>(),
                                    std::stop_token{}, path), result);
    DOBA_EXPECT(probe.done());
    bool threw = false;
    try {
      probe.rethrow_if_failed();
    } catch (const std::runtime_error& error) {
      threw = std::string_view(error.what()) ==
              "The route parameters could not be parsed";
    }
    DOBA_EXPECT(threw);
    DOBA_EXPECT(!result.has_value());
    DOBA_EXPECT_EQUAL(async_calls, 0);
  }
}

// +===========================================================================+
// | [>] accepts finite floating point values                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("route conversion accepts finite floating point values") {
  using parameter = double;
  struct test_case {
    std::string_view input;
    parameter expected;
  };
  const test_case cases[] = {
      {"-1.5", -1.5},
      {"1e2", 100.0},
      {"0", 0.0},
      {"1.5e-2", 0.015},
  };
  for (const auto& test : cases) {
    const std::string path = "/value/" + std::string(test.input);
    martianlabs::doba::tests::unit::test_helper::set_context(test.input);
    std::size_t sync_calls = 0;
    std::size_t async_calls = 0;
    parameter sync_value{};
    parameter async_value{};
    auto handler = make_router_handler_parametrized<
        request, response, parameter>(
        "/value/:value",
        [&](const request&, parameter value) {
          response res;
          sync_calls++;
          sync_value = value;
          return res;
        });
    auto async_handler =
        make_router_handler_parametrized_async<request, response, parameter>(
            "/value/:value",
            [&](std::shared_ptr<const request>, std::stop_token,
                parameter value) -> task<response> {
              async_calls++;
              async_value = value;
              co_return response{"converted"};
            });
    DOBA_EXPECT_EQUAL(handler.matches(path), true);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
    request req;
    response res;
    res = handler.invoke(req, path);
    DOBA_EXPECT_EQUAL(sync_calls, 1);
    DOBA_EXPECT_EQUAL(sync_value, test.expected);
    std::optional<response> result;
    auto probe = collect(
        async_handler.invoke_async(std::make_shared<const request>(),
                                    std::stop_token{}, path), result);
    DOBA_EXPECT(probe.done());
    probe.rethrow_if_failed();
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->value, "converted");
    DOBA_EXPECT_EQUAL(async_value, test.expected);
    DOBA_EXPECT_EQUAL(async_calls, 1);
  }
}

// +===========================================================================+
// | [>] rejects floating point range errors                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("route conversion rejects floating point range errors") {
  using parameter = double;
  struct test_case {
    std::string_view input;
  };
  const test_case cases[] = {
      {"1e9999"},
      {"1e-9999"},
  };
  for (const auto& test : cases) {
    const std::string path = "/value/" + std::string(test.input);
    martianlabs::doba::tests::unit::test_helper::set_context(test.input);
    std::size_t sync_calls = 0;
    std::size_t async_calls = 0;
    auto handler = make_router_handler_parametrized<
        request, response, parameter>(
        "/value/:value",
        [&](const request&, parameter) {
          response res;
          sync_calls++;
          return res;
        });
    auto async_handler =
        make_router_handler_parametrized_async<request, response, parameter>(
            "/value/:value",
            [&](std::shared_ptr<const request>, std::stop_token,
                parameter) -> task<response> {
              async_calls++;
              co_return response{"converted"};
            });
    DOBA_EXPECT_EQUAL(handler.matches(path), false);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
    request req;
    response res;
    res = handler.invoke(req, path);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
    std::optional<response> result;
    auto probe = collect(
        async_handler.invoke_async(std::make_shared<const request>(),
                                    std::stop_token{}, path), result);
    DOBA_EXPECT(probe.done());
    bool threw = false;
    try {
      probe.rethrow_if_failed();
    } catch (const std::runtime_error& error) {
      threw = std::string_view(error.what()) ==
              "The route parameters could not be parsed";
    }
    DOBA_EXPECT(threw);
    DOBA_EXPECT(!result.has_value());
    DOBA_EXPECT_EQUAL(async_calls, 0);
  }
}
