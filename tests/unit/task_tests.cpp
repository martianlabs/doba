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
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "common/task.h"
#include "test_helper.h"

namespace {
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

task<int> delayed_value(manual_event& event, int& started) {
  started++;
  co_await event;
  co_return 42;
}

task<std::unique_ptr<int>> movable_value() {
  co_return std::make_unique<int>(7);
}

task<int> failed_value() {
  throw std::runtime_error("failure");
  co_return 0;
}

task<int> owned_value(std::shared_ptr<int> value) {
  co_return *value;
}

task<int> add_one(task<int> value, int& resumed) {
  const int result = co_await std::move(value);
  resumed++;
  co_return result + 1;
}

task<int> await_moved_source(task<int> value) {
  [[maybe_unused]] task<int> moved(std::move(value));
  co_return co_await std::move(value);
}
}  // namespace

// +===========================================================================+
// | [>] task is movable but not copyable                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("task is movable but not copyable") {
  static_assert(!std::is_copy_constructible_v<task<int>>);
  static_assert(!std::is_copy_assignable_v<task<int>>);
  static_assert(std::is_nothrow_move_constructible_v<task<int>>);
  static_assert(std::is_nothrow_move_assignable_v<task<int>>);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] task starts lazily and resumes its continuation         ( test-case ) |
// +===========================================================================+
DOBA_TEST("task starts lazily and resumes its continuation") {
  manual_event event;
  int started = 0;
  auto value = delayed_value(event, started);
  DOBA_EXPECT_EQUAL(started, 0);
  std::optional<int> result;
  auto probe = collect(std::move(value), result);
  DOBA_EXPECT_EQUAL(started, 1);
  DOBA_EXPECT(!probe.done());
  DOBA_EXPECT(!result.has_value());
  event.resume();
  DOBA_EXPECT(probe.done());
  DOBA_EXPECT_EQUAL(*result, 42);
}
// +===========================================================================+
// | [>] task returns move-only values                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("task returns move-only values") {
  std::optional<std::unique_ptr<int>> result;
  auto probe = collect(movable_value(), result);
  DOBA_EXPECT(probe.done());
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(**result, 7);
}
// +===========================================================================+
// | [>] task propagates unhandled exceptions                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("task propagates unhandled exceptions") {
  std::optional<int> result;
  auto probe = collect(failed_value(), result);
  DOBA_EXPECT(probe.done());
  bool threw = false;
  try {
    probe.rethrow_if_failed();
  } catch (const std::runtime_error& error) {
    threw = std::string_view(error.what()) == "failure";
  }
  DOBA_EXPECT(threw);
  DOBA_EXPECT(!result.has_value());
}
// +===========================================================================+
// | [>] task destroys a frame that was never started            ( test-case ) |
// +===========================================================================+
DOBA_TEST("task destroys a frame that was never started") {
  auto owner = std::make_shared<int>(1);
  std::weak_ptr<int> lifetime = owner;
  {
    auto value = owned_value(std::move(owner));
    DOBA_EXPECT(!lifetime.expired());
  }
  DOBA_EXPECT(lifetime.expired());
}
// +===========================================================================+
// | [>] task move assignment releases the previous frame        ( test-case ) |
// +===========================================================================+
DOBA_TEST("task move assignment releases the previous frame") {
  auto first_owner = std::make_shared<int>(1);
  auto second_owner = std::make_shared<int>(2);
  std::weak_ptr<int> first_lifetime = first_owner;
  std::weak_ptr<int> second_lifetime = second_owner;
  {
    auto first = owned_value(std::move(first_owner));
    auto second = owned_value(std::move(second_owner));
    first = std::move(second);
    DOBA_EXPECT(first_lifetime.expired());
    DOBA_EXPECT(!second_lifetime.expired());
  }
  DOBA_EXPECT(second_lifetime.expired());
}
// +===========================================================================+
// | [>] awaiting a moved task reports an error                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("awaiting a moved task reports an error") {
  std::optional<int> result;
  auto probe =
      collect(await_moved_source(owned_value(std::make_shared<int>(7))), result);
  DOBA_EXPECT(probe.done());
  bool threw = false;
  try {
    probe.rethrow_if_failed();
  } catch (const std::logic_error& error) {
    threw = std::string_view(error.what()) == "cannot await an empty task";
  }
  DOBA_EXPECT(threw);
  DOBA_EXPECT(!result.has_value());
}
// +===========================================================================+
// | [>] nested tasks resume each continuation once              ( test-case ) |
// +===========================================================================+
DOBA_TEST("nested tasks resume each continuation once") {
  manual_event event;
  int started = 0;
  int resumed = 0;
  std::optional<int> result;
  auto probe = collect(add_one(delayed_value(event, started), resumed),
                       result);
  DOBA_EXPECT_EQUAL(started, 1);
  DOBA_EXPECT_EQUAL(resumed, 0);
  DOBA_EXPECT(!probe.done());
  event.resume();
  probe.rethrow_if_failed();
  DOBA_EXPECT(probe.done());
  DOBA_EXPECT_EQUAL(resumed, 1);
  DOBA_EXPECT_EQUAL(*result, 43);
}
