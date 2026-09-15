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
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "protocol/http/common/router.h"
#include "test_helper.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] request                                                    ( struct ) |
// +---------------------------------------------------------------------------+
// | Test message representation.                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct request {
  int event{0};
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] response                                                   ( struct ) |
// +---------------------------------------------------------------------------+
// | Test message representation.                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct response {
  std::string value;
};
using martianlabs::doba::protocol::http::router;
using martianlabs::doba::common::task;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] task_probe                                                  ( class ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class task_probe {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
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
  // +=========================================================================+
  // | [>] METHODs                                                 ( private ) |
  // +=========================================================================+
  explicit task_probe(std::coroutine_handle<promise_type> coroutine) noexcept
      : coroutine_(coroutine) {}
  std::coroutine_handle<promise_type> coroutine_;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] manual_event                                                ( class ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class manual_event {
 public:
  // +=========================================================================+
  // | [>] METHODs                                                  ( public ) |
  // +=========================================================================+
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
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::coroutine_handle<> continuation_;
};

template <typename Tty>
task_probe collect(task<Tty> value, std::optional<Tty>& result) {
  result.emplace(co_await std::move(value));
}
}  // namespace

// +===========================================================================+
// | [>] router type is neither copyable nor movable             ( test-case ) |
// +===========================================================================+
DOBA_TEST("router type is neither copyable nor movable") {
  static_assert(!std::is_copy_constructible_v<router<request, response>>);
  static_assert(!std::is_copy_assignable_v<router<request, response>>);
  static_assert(!std::is_move_constructible_v<router<request, response>>);
  static_assert(!std::is_move_assignable_v<router<request, response>>);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] match returns handler without executing it              ( test-case ) |
// +===========================================================================+
DOBA_TEST("match returns handler without executing it") {
  router<request, response> value;
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/items")));
  bool invoked = false;
  value.add("GET", "/items", [&invoked](const request&) {
    response res;
    invoked = true;
    res.value = "matched";
    return res;
  });
  auto match = value.match("GET", "/items");
  DOBA_EXPECT(static_cast<bool>(match));
  DOBA_EXPECT(!invoked);
  request req;
  response res;
  res = match.handler->callback(req);
  DOBA_EXPECT(invoked);
  DOBA_EXPECT_EQUAL(res.value, "matched");
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/Items")));
}
// +===========================================================================+
// | [>] static routes use exact path matching                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("static routes use exact path matching") {
  router<request, response> value;
  auto handler = [](const request&) {
    response res;
    return res;
  };
  value.add("GET", "/assets", handler);
  DOBA_EXPECT(static_cast<bool>(value.match("GET", "/assets")));
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/assets/a")));
}
// +===========================================================================+
// | [>] wildcard routes match their prefix                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("wildcard routes match their prefix") {
  router<request, response> value;
  bool invoked = false;
  value.add("GET", "/assets/*", [&invoked](const request&) {
    response res;
    invoked = true;
    res.value = "wildcard";
    return res;
  });
  constexpr std::string_view matching[] = {
      "/assets/",
      "/assets/a",
      "/assets/a/b",
  };
  for (const auto path : matching) {
    auto match = value.match("GET", path);
    DOBA_EXPECT(match.handler != nullptr);
    DOBA_EXPECT(match.parametrized_handler == nullptr);
  }
  DOBA_EXPECT(!invoked);
  request req;
  response res;
  res = value.match("GET", "/assets/a").handler->callback(req);
  DOBA_EXPECT(invoked);
  DOBA_EXPECT_EQUAL(res.value, "wildcard");
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/assets")));
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/Assets/a")));
}
// +===========================================================================+
// | [>] static routes take precedence over parametrized routes  ( test-case ) |
// +===========================================================================+
DOBA_TEST("static routes take precedence over parametrized routes") {
  router<request, response> value;
  value.add(
      "GET", "/items/:id",
      [](const request&, int) {
        response res;
        res.value = "parametrized";
        return res;
      });
  value.add(
      "GET", "/items/42",
      [](const request&) {
        response res;
        res.value = "static";
        return res;
      });
  auto match = value.match("GET", "/items/42");
  DOBA_EXPECT(match.handler != nullptr);
  DOBA_EXPECT(match.parametrized_handler == nullptr);
  request req;
  response res;
  res = match.handler->callback(req);
  DOBA_EXPECT_EQUAL(res.value, "static");
}
// +===========================================================================+
// | [>] route precedence ends with wildcard routes              ( test-case ) |
// +===========================================================================+
DOBA_TEST("route precedence ends with wildcard routes") {
  router<request, response> value;
  value.add(
      "GET", "/items/*",
      [](const request&) {
        response res;
        res.value = "wildcard";
        return res;
      });
  value.add(
      "GET", "/items/:id",
      [](const request&, int) {
        response res;
        res.value = "parametrized";
        return res;
      });
  value.add(
      "GET", "/items/42",
      [](const request&) {
        response res;
        res.value = "static";
        return res;
      });
  request req;
  response res;
  auto match = value.match("GET", "/items/42");
  res = match.handler->callback(req);
  DOBA_EXPECT_EQUAL(res.value, "static");
  match = value.match("GET", "/items/7");
  res = match.parametrized_handler->invoke(req, "/items/7");
  DOBA_EXPECT_EQUAL(res.value, "parametrized");
  match = value.match("GET", "/items/name");
  res = match.handler->callback(req);
  DOBA_EXPECT_EQUAL(res.value, "wildcard");
}
// +===========================================================================+
// | [>] parametrized routes preserve typed registration order   ( test-case ) |
// +===========================================================================+
DOBA_TEST("parametrized routes preserve typed registration order") {
  router<request, response> value;
  value.add(
      "GET", "/items/:value",
      [](const request&, int) {
        response res;
        res.value = "integer";
        return res;
      });
  value.add(
      "GET", "/items/:value",
      [](const request&, std::string_view) {
        response res;
        res.value = "text";
        return res;
      });
  request req;
  response res;
  auto integer = value.match("GET", "/items/42");
  res = integer.parametrized_handler->invoke(req, "/items/42");
  DOBA_EXPECT_EQUAL(res.value, "integer");
  auto text = value.match("GET", "/items/value");
  res = text.parametrized_handler->invoke(req, "/items/value");
  DOBA_EXPECT_EQUAL(res.value, "text");
}
// +===========================================================================+
// | [>] parametrized routes validate pattern and handler shape  ( test-case ) |
// +===========================================================================+
DOBA_TEST("parametrized routes validate pattern and handler shape") {
  router<request, response> value;
  bool threw = false;
  try {
    value.add("GET", "/items/:id", [](const request&) {
      response res;
      return res;
    });
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  threw = false;
  try {
    value.add("GET", "/items", [](const request&, int) {
      response res;
      return res;
    });
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  threw = false;
  try {
    value.add("GET", "/items/:", [](const request&, int) {
      response res;
      return res;
    });
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
}
// +===========================================================================+
// | [>] wildcard routes validate pattern and handler shape      ( test-case ) |
// +===========================================================================+
DOBA_TEST("wildcard routes validate pattern and handler shape") {
  constexpr std::string_view invalid[] = {
      "*", "/a*", "/a/*/b", "/a/**", "/a/*x",
  };
  for (const auto route : invalid) {
    router<request, response> value;
    bool threw = false;
    try {
      value.add("GET", route, [](const request&) {
        response res;
        return res;
      });
    } catch (const std::invalid_argument&) {
      threw = true;
    }
    DOBA_EXPECT(threw);
  }
  router<request, response> value;
  bool threw = false;
  try {
    value.add("GET", "/items/:id/*", [](const request&) {
      response res;
      return res;
    });
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  threw = false;
  try {
    value.add("GET", "/items/*", [](const request&, int) {
      response res;
      return res;
    });
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
}
// +===========================================================================+
// | [>] match preserves first handler                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("match preserves first handler") {
  router<request, response> value;
  value.add(
      "GET", "/resource",
      [](const request&) {
        response res;
        res.value = "first";
        return res;
      });
  value.add(
      "GET", "/resource",
      [](const request&) {
        response res;
        res.value = "second";
        return res;
      });
  value.add(
      "POST", "/resource",
      [](const request&) {
        response res;
        return res;
      });
  auto get = value.match("GET", "/resource");
  auto post = value.match("POST", "/resource");
  request req;
  response res;
  res = get.handler->callback(req);
  DOBA_EXPECT_EQUAL(res.value, "first");
  DOBA_EXPECT(static_cast<bool>(post));
  DOBA_EXPECT_EQUAL(value.allowed_methods("/resource"), "GET, POST");
  DOBA_EXPECT(value.allowed_methods("/missing").empty());
}
// +===========================================================================+
// | [>] allowed methods include matching parametrized routes    ( test-case ) |
// +===========================================================================+
DOBA_TEST("allowed methods include matching parametrized routes") {
  router<request, response> value;
  value.add("GET", "/items/:id", [](const request&, int) {
    response res;
    return res;
  });
  value.add("POST", "/items/:name", [](const request&, std::string_view) {
    response res;
    return res;
  });
  DOBA_EXPECT_EQUAL(value.allowed_methods("/items/42"), "GET, POST");
  DOBA_EXPECT_EQUAL(value.allowed_methods("/items/name"), "POST");
}
// +===========================================================================+
// | [>] allowed methods include matching wildcard routes        ( test-case ) |
// +===========================================================================+
DOBA_TEST("allowed methods include matching wildcard routes") {
  router<request, response> value;
  value.add("GET", "/assets/logo", [](const request&) {
    response res;
    return res;
  });
  value.add("GET", "/assets/*", [](const request&) {
    response res;
    return res;
  });
  value.add("POST", "/assets/:id", [](const request&, int) {
    response res;
    return res;
  });
  value.add("DELETE", "/assets/*", [](const request&) {
    response res;
    return res;
  });
  DOBA_EXPECT_EQUAL(value.allowed_methods("/assets/logo"), "GET, DELETE");
  DOBA_EXPECT_EQUAL(value.allowed_methods("/assets/42"),
                    "POST, GET, DELETE");
  DOBA_EXPECT(value.allowed_methods("/assets").empty());
}
// +===========================================================================+
// | [>] sync and async routes share one router                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("sync and async routes share one router") {
  router<request, response> value;
  value.add("GET", "/sync", [](const request&) {
    response res;
    return res;
  });
  value.add("GET", "/async",
            [](std::shared_ptr<const request>,
               std::stop_token) -> task<response> {
              co_return response{"async"};
            });
  value.add("GET", "/items/:id",
            [](std::shared_ptr<const request>, std::stop_token,
               int) -> task<response> {
              co_return response{"parametrized"};
            });
  value.add("GET", "/assets/*",
            [](std::shared_ptr<const request>,
               std::stop_token) -> task<response> {
              co_return response{"wildcard"};
            });
  auto sync = value.match("GET", "/sync");
  auto async = value.match("GET", "/async");
  auto parametrized = value.match("GET", "/items/42");
  auto wildcard = value.match("GET", "/assets/logo");
  DOBA_EXPECT(!sync.handler->is_async());
  DOBA_EXPECT(async.handler->is_async());
  DOBA_EXPECT(parametrized.parametrized_handler->is_async());
  DOBA_EXPECT(wildcard.handler->is_async());
}
// +===========================================================================+
// | [>] noexcept handlers register and execute                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("noexcept handlers register and execute") {
  router<request, response> value;
  value.add("GET", "/sync", [](const request&) noexcept {
    return response{"sync"};
  });
  value.add("GET", "/mutable/:id", [](const request&, int id) mutable noexcept {
    return response{std::to_string(id)};
  });
  value.add("GET", "/async",
            [](std::shared_ptr<const request>,
               std::stop_token) noexcept -> task<response> {
              co_return response{"async"};
            });
  value.add("GET", "/async/:id",
            [](std::shared_ptr<const request>, std::stop_token,
               int id) mutable noexcept -> task<response> {
              co_return response{std::to_string(id)};
            });
  request req;
  const auto sync = value.match("GET", "/sync");
  DOBA_EXPECT_EQUAL(sync.handler->callback(req).value, "sync");
  const auto mutable_sync = value.match("GET", "/mutable/42");
  DOBA_EXPECT_EQUAL(
      mutable_sync.parametrized_handler->invoke(req, "/mutable/42").value,
      "42");
  const auto async = value.match("GET", "/async");
  std::optional<response> async_result;
  auto async_probe = collect(
      async.handler->async_callback(std::make_shared<const request>(),
                                    std::stop_token{}),
      async_result);
  DOBA_EXPECT(async_probe.done());
  async_probe.rethrow_if_failed();
  DOBA_EXPECT(async_result.has_value());
  DOBA_EXPECT_EQUAL(async_result->value, "async");
  const auto mutable_async = value.match("GET", "/async/42");
  std::optional<response> mutable_result;
  auto mutable_probe = collect(
      mutable_async.parametrized_handler->invoke_async(
          std::make_shared<const request>(), std::stop_token{}, "/async/42"),
      mutable_result);
  DOBA_EXPECT(mutable_probe.done());
  mutable_probe.rethrow_if_failed();
  DOBA_EXPECT(mutable_result.has_value());
  DOBA_EXPECT_EQUAL(mutable_result->value, "42");
}
// +===========================================================================+
// | [>] static async handler outlives its router                ( test-case ) |
// +===========================================================================+
DOBA_TEST("static async handler outlives its router") {
  manual_event first_event;
  manual_event second_event;
  auto state = std::make_shared<std::string>("static");
  std::weak_ptr<std::string> lifetime = state;
  std::optional<response> first_result;
  std::optional<response> second_result;
  std::optional<task_probe> first_probe;
  std::optional<task_probe> second_probe;
  {
    router<request, response> value;
    value.add(
        "GET", "/async",
        [state, &first_event, &second_event](
            std::shared_ptr<const request> req,
            std::stop_token) -> task<response> {
          if (req->event == 0) {
            co_await first_event;
          } else {
            co_await second_event;
          }
          co_return response{*state};
        });
    state.reset();
    auto match = value.match("GET", "/async");
    first_probe.emplace(collect(
        match.handler->async_callback(
            std::make_shared<const request>(request{0}), std::stop_token{}),
        first_result));
    second_probe.emplace(collect(
        match.handler->async_callback(
            std::make_shared<const request>(request{1}), std::stop_token{}),
        second_result));
    DOBA_EXPECT(!first_probe->done());
    DOBA_EXPECT(!second_probe->done());
  }
  DOBA_EXPECT(!lifetime.expired());
  first_event.resume();
  first_probe->rethrow_if_failed();
  DOBA_EXPECT(first_probe->done());
  DOBA_EXPECT(first_result.has_value());
  DOBA_EXPECT_EQUAL(first_result->value, "static");
  DOBA_EXPECT(!lifetime.expired());
  second_event.resume();
  second_probe->rethrow_if_failed();
  DOBA_EXPECT(second_probe->done());
  DOBA_EXPECT(second_result.has_value());
  DOBA_EXPECT_EQUAL(second_result->value, "static");
  DOBA_EXPECT(lifetime.expired());
}
// +===========================================================================+
// | [>] wildcard async handler propagates late failure          ( test-case ) |
// +===========================================================================+
DOBA_TEST("wildcard async handler propagates late failure") {
  manual_event event;
  auto state = std::make_shared<std::string>("failure");
  std::weak_ptr<std::string> lifetime = state;
  std::optional<response> result;
  std::optional<task_probe> probe;
  {
    router<request, response> value;
    value.add(
        "GET", "/assets/*",
        [state, &event](std::shared_ptr<const request>,
                        std::stop_token) -> task<response> {
          co_await event;
          throw std::runtime_error(*state);
          co_return response{};
        });
    state.reset();
    auto match = value.match("GET", "/assets/item");
    probe.emplace(collect(
        match.handler->async_callback(std::make_shared<const request>(),
                                      std::stop_token{}),
        result));
    DOBA_EXPECT(!probe->done());
  }
  DOBA_EXPECT(!lifetime.expired());
  event.resume();
  DOBA_EXPECT(probe->done());
  bool threw = false;
  try {
    probe->rethrow_if_failed();
  } catch (const std::runtime_error& error) {
    threw = std::string_view(error.what()) == "failure";
  }
  DOBA_EXPECT(threw);
  DOBA_EXPECT(!result.has_value());
  DOBA_EXPECT(lifetime.expired());
}
// +===========================================================================+
// | [>] parametrized async handler outlives its router          ( test-case ) |
// +===========================================================================+
DOBA_TEST("parametrized async handler outlives its router") {
  manual_event event;
  auto state = std::make_shared<std::string>("item");
  std::weak_ptr<std::string> lifetime = state;
  std::optional<response> result;
  std::optional<task_probe> probe;
  {
    router<request, response> value;
    value.add(
        "GET", "/items/:id",
        [state, &event](std::shared_ptr<const request>,
                        std::stop_token,
                        int id) -> task<response> {
          co_await event;
          co_return response{*state + std::to_string(id)};
        });
    state.reset();
    auto match = value.match("GET", "/items/42");
    probe.emplace(collect(
        match.parametrized_handler->invoke_async(
            std::make_shared<const request>(), std::stop_token{},
            "/items/42"),
        result));
    DOBA_EXPECT(!probe->done());
  }
  DOBA_EXPECT(!lifetime.expired());
  event.resume();
  probe->rethrow_if_failed();
  DOBA_EXPECT(probe->done());
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->value, "item42");
  DOBA_EXPECT(lifetime.expired());
}
// +===========================================================================+
// | [>] router accepts move-only async handlers                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("router accepts move-only async handlers") {
  router<request, response> value;
  auto state = std::make_unique<std::string>("move-only");
  value.add(
      "GET", "/move-only",
      [state = std::move(state)](
          std::shared_ptr<const request>,
          std::stop_token) -> task<response> {
        co_return response{*state};
      });
  DOBA_EXPECT(state == nullptr);
  auto match = value.match("GET", "/move-only");
  std::optional<response> result;
  auto probe = collect(
      match.handler->async_callback(std::make_shared<const request>(),
                                    std::stop_token{}),
      result);
  probe.rethrow_if_failed();
  DOBA_EXPECT(probe.done());
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->value, "move-only");
}
// +===========================================================================+
// | [>] async routes validate pattern and handler shape         ( test-case ) |
// +===========================================================================+
DOBA_TEST("async routes validate pattern and handler shape") {
  router<request, response> value;
  bool threw = false;
  try {
    value.add(
        "GET", "/items/:id",
        [](std::shared_ptr<const request>,
           std::stop_token) -> task<response> {
          co_return response{};
        });
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  threw = false;
  try {
    value.add(
        "GET", "/items",
        [](std::shared_ptr<const request>, std::stop_token,
           int) -> task<response> {
          co_return response{};
        });
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  threw = false;
  try {
    value.add(
        "GET", "/items/*",
        [](std::shared_ptr<const request>, std::stop_token,
           int) -> task<response> {
          co_return response{};
        });
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
}
// +===========================================================================+
// | [>] registration owns method and all route pattern forms    ( test-case ) |
// +===========================================================================+
DOBA_TEST("registration owns method and all route pattern forms") {
  router<request, response> value;
  {
    std::string method = "GET";
    std::string fixed = "/fixed";
    std::string parameter = "/value/:id";
    std::string wildcard = "/assets/*";
    value.add(method, fixed, [](const request&) {
      return response{"fixed"};
    });
    value.add(method, parameter, [](const request&, int id) {
      return response{std::to_string(id)};
    });
    value.add(method, wildcard, [](const request&) {
      return response{"wildcard"};
    });
    method.assign(method.size(), 'x');
    fixed.assign(fixed.size(), 'x');
    parameter.assign(parameter.size(), 'x');
    wildcard.assign(wildcard.size(), 'x');
  }
  request req;
  const auto fixed = value.match("GET", "/fixed");
  DOBA_EXPECT(fixed.handler != nullptr);
  DOBA_EXPECT_EQUAL(fixed.handler->callback(req).value, "fixed");
  const auto parameter = value.match("GET", "/value/42");
  DOBA_EXPECT(parameter.parametrized_handler != nullptr);
  DOBA_EXPECT_EQUAL(
      parameter.parametrized_handler->invoke(req, "/value/42").value, "42");
  const auto wildcard = value.match("GET", "/assets/one");
  DOBA_EXPECT(wildcard.handler != nullptr);
  DOBA_EXPECT_EQUAL(wildcard.handler->callback(req).value, "wildcard");
}
// +===========================================================================+
// | [>] invalid registration preserves existing route selection ( test-case ) |
// +===========================================================================+
DOBA_TEST("invalid registration preserves existing route selection") {
  router<request, response> value;
  value.add("GET", "/kept", [](const request&) {
    return response{"kept"};
  });
  for (std::string_view pattern : {"/:", "/x/*/tail", "/x/:id/*"}) {
    bool threw = false;
    try {
      value.add("GET", pattern, [](const request&, int) {
        return response{"invalid"};
      });
    } catch (const std::invalid_argument&) {
      threw = true;
    }
    DOBA_EXPECT(threw);
    const auto match = value.match("GET", "/kept");
    DOBA_EXPECT(match.handler != nullptr);
    DOBA_EXPECT_EQUAL(match.handler->callback(request{}).value, "kept");
    DOBA_EXPECT_EQUAL(value.allowed_methods("/kept"), "GET");
  }
}

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] controller_probe                                            ( class ) |
// +---------------------------------------------------------------------------+
// | Controller implementation.                                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class controller_probe {
 public:
  // +=========================================================================+
  // | [>] METHODs                                                  ( public ) |
  // +=========================================================================+
  controller_probe(std::string prefix, int& alive, manual_event& event)
      : prefix_(std::move(prefix)), alive_(alive), event_(event) { alive_++; }
  controller_probe(const controller_probe&) = delete;
  controller_probe(controller_probe&&) = delete;
  ~controller_probe() { alive_--; }
  template <typename Rty>
  void register_routes(Rty& routes) {
    routes.add("GET", prefix_ + "/count", &controller_probe::count);
    routes.add("GET", prefix_ + "/value/:id", &controller_probe::value);
    routes.add("GET", prefix_ + "/async", &controller_probe::delayed);
    routes.add("GET", prefix_ + "/async/:id", &controller_probe::typed);
    routes.add("GET", prefix_ + "/wild/*", &controller_probe::value_zero);
  }
  response count(const request&) { return {std::to_string(++count_)}; }
  response value(const request&, int id) const noexcept {
    return {std::to_string(count_ + id)};
  }
  response value_zero(const request&) const { return {prefix_}; }
  task<response> delayed(std::shared_ptr<const request> req,
                         std::stop_token token) {
    co_await event_;
    if (req->event == -1) throw std::runtime_error("controller failure");
    co_return response{token.stop_requested() ? "cancelled" : prefix_};
  }
  task<response> typed(std::shared_ptr<const request>,
                       std::stop_token, int id) const noexcept {
    co_return response{std::to_string(id)};
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::string prefix_;
  int& alive_;
  manual_event& event_;
  int count_{0};
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] failing_controller                                         ( struct ) |
// +---------------------------------------------------------------------------+
// | Controller implementation.                                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct failing_controller {
  explicit failing_controller(int mode) : mode_(mode) {
    if (mode == 0) throw std::runtime_error("constructor");
  }
  template <typename Rty>
  void register_routes(Rty& routes) {
    if (mode_ == 1) return;
    routes.add("GET", "/new", &failing_controller::get);
    routes.add("POST", "/new", &failing_controller::get);
    routes.add("GET", "/new/:id", &failing_controller::typed);
    routes.add("GET", "/new/*", &failing_controller::get);
    routes.add("POST", "/new/:id", &failing_controller::typed);
    routes.add("POST", "/new/*", &failing_controller::get);
    routes.add("GET", "/invalid/:id", &failing_controller::get);
  }
  response get(const request&) { return {}; }
  response typed(const request&, int) { return {}; }
  int mode_;
};
}  // namespace

// +===========================================================================+
// | [>] controllers share one instance per registration         ( test-case ) |
// +===========================================================================+
DOBA_TEST("controllers share one instance per registration") {
  int alive = 0;
  manual_event event;
  {
    router<request, response> value;
    value.add_controller<controller_probe>("/a", alive, event);
    value.add_controller<controller_probe>("/b", alive, event);
    DOBA_EXPECT_EQUAL(alive, 2);
    auto count = value.match("GET", "/a/count");
    DOBA_EXPECT_EQUAL(count.handler->callback({}).value, "1");
    DOBA_EXPECT_EQUAL(count.handler->callback({}).value, "2");
    auto typed = value.match("GET", "/a/value/40");
    DOBA_EXPECT_EQUAL(
        typed.parametrized_handler->invoke({}, "/a/value/40").value, "42");
    DOBA_EXPECT_EQUAL(
        value.match("GET", "/b/count").handler->callback({}).value, "1");
    DOBA_EXPECT_EQUAL(
        value.match("GET", "/a/wild/x").handler->callback({}).value, "/a");
    std::optional<response> result;
    auto match = value.match("GET", "/a/async/42");
    auto probe = collect(match.parametrized_handler->invoke_async(
        std::make_shared<const request>(), {}, "/a/async/42"), result);
    probe.rethrow_if_failed();
    DOBA_EXPECT(probe.done());
    DOBA_EXPECT_EQUAL(result->value, "42");
  }
  DOBA_EXPECT_EQUAL(alive, 0);
}

// +===========================================================================+
// | [>] controller registration rolls back every route category ( test-case ) |
// +===========================================================================+
DOBA_TEST("controller registration rolls back every route category") {
  router<request, response> value;
  value.add("GET", "/old", [](const request&) { return response{"old"}; });
  value.add("GET", "/old/:id",
            [](const request&, int) { return response{"typed"}; });
  value.add("GET", "/old/*",
            [](const request&) { return response{"wild"}; });
  for (int mode : {0, 1, 2}) {
    bool threw = false;
    try {
      value.add_controller<failing_controller>(mode);
    } catch (const std::exception&) { threw = true; }
    DOBA_EXPECT(threw);
    DOBA_EXPECT(!value.match("GET", "/new"));
    DOBA_EXPECT(!value.match("POST", "/new"));
    DOBA_EXPECT(!value.match("GET", "/new/42"));
    DOBA_EXPECT(!value.match("POST", "/new/x"));
    DOBA_EXPECT_EQUAL(value.allowed_methods("/new/42"), "");
    DOBA_EXPECT_EQUAL(
        value.match("GET", "/old").handler->callback({}).value, "old");
    DOBA_EXPECT(value.match("GET", "/old/42").parametrized_handler);
    DOBA_EXPECT(value.match("GET", "/old/x").handler);
  }
}

// +===========================================================================+
// | [>] controller lifetime and cancellation                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("suspended controller survives router and observes cancellation") {
  manual_event event;
  int alive = 0;
  std::stop_source stop;
  std::optional<response> result;
  std::optional<task_probe> probe;
  {
    router<request, response> value;
    value.add_controller<controller_probe>("/a", alive, event);
    probe.emplace(collect(
        value.match("GET", "/a/async").handler->async_callback(
            std::make_shared<const request>(), stop.get_token()), result));
    DOBA_EXPECT(!probe->done());
  }
  DOBA_EXPECT_EQUAL(alive, 1);
  stop.request_stop();
  event.resume();
  probe->rethrow_if_failed();
  DOBA_EXPECT_EQUAL(result->value, "cancelled");
  DOBA_EXPECT_EQUAL(alive, 0);
}

// +===========================================================================+
// | [>] suspended controller releases ownership after failure   ( test-case ) |
// +===========================================================================+
DOBA_TEST("suspended controller releases ownership after failure") {
  manual_event event;
  int alive = 0;
  std::optional<response> result;
  std::optional<task_probe> probe;
  {
    router<request, response> value;
    value.add_controller<controller_probe>("/a", alive, event);
    probe.emplace(collect(
        value.match("GET", "/a/async").handler->async_callback(
            std::make_shared<const request>(request{-1}), {}), result));
  }
  DOBA_EXPECT_EQUAL(alive, 1);
  event.resume();
  bool threw = false;
  try { probe->rethrow_if_failed(); }
  catch (const std::runtime_error&) { threw = true; }
  DOBA_EXPECT(threw);
  DOBA_EXPECT_EQUAL(alive, 0);
}
