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
      [](const request&, response&, std::uint64_t, bool, double,
         std::string_view) {});
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
      "/items/:id", [](const request&, response&, int) {});
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
      [&invoked](const request&, response& res, std::uint64_t id, bool enabled,
                 double score, const std::string& name) {
        invoked = id == 42 && enabled && score == 1.5 && name == "doba";
        res.value = name;
      });
  request req;
  response res;
  handler.invoke(req, res, "/items/42/true/1.5/doba");
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
      [&invoked](const request&, response&, int) { invoked = true; });
  request req;
  response res;
  handler.invoke(req, res, "/items/value");
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
      [&event](std::shared_ptr<const request>,
               const std::string& name) -> task<response> {
        co_await event;
        response res;
        res.value = name;
        co_return res;
      });
  auto req = std::make_shared<const request>();
  std::optional<response> result;
  auto probe = collect(handler.invoke_async(req, "/items/doba"), result);
  DOBA_EXPECT(!probe.done());
  DOBA_EXPECT(!result.has_value());
  event.resume();
  probe.rethrow_if_failed();
  DOBA_EXPECT(probe.done());
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->value, "doba");
}
