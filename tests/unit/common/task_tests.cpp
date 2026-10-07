//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
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

#include <chrono>
#include <coroutine>
#include <exception>
#include <memory>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>

#include "common/task.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::task;
using martianlabs::doba::common::task_scheduler;

struct pause {
  std::coroutine_handle<>& next;

  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> handle) const noexcept {
    next = handle;
  }
  void await_resume() const noexcept {}
};

task<void> run(std::coroutine_handle<>& next, int& steps) {
  steps++;
  co_await pause{next};
  steps++;
}

task<void> fail() {
  throw std::runtime_error("task failure");
  co_return;
}

task<void> retain(std::shared_ptr<int>) {
  co_return;
}

task<void> fail_after(std::coroutine_handle<>& next) {
  co_await pause{next};
  throw std::runtime_error("after suspension");
}

task<void> use_scheduler(int& steps) {
  steps++;
  co_await martianlabs::doba::common::yield();
  steps++;
  co_await martianlabs::doba::common::sleep_for(
      std::chrono::milliseconds(10));
  steps++;
}

struct scheduled_handles {
  std::coroutine_handle<> ready;
  std::coroutine_handle<> timed;
  std::chrono::steady_clock::time_point due;
};
}  // namespace

DOBA_TEST("task starts lazily and completes after resumption") {
  static_assert(!std::is_copy_constructible_v<task<void>>);
  static_assert(std::is_move_constructible_v<task<void>>);
  std::coroutine_handle<> next;
  int steps = 0;
  bool completed = false;
  auto value = run(next, steps);
  DOBA_EXPECT_EQUAL(steps, 0);
  auto moved = std::move(value);
  moved.start([&](std::exception_ptr error) {
    DOBA_EXPECT(!error);
    completed = true;
  });
  DOBA_EXPECT_EQUAL(steps, 1);
  DOBA_EXPECT(!completed);
  next.resume();
  DOBA_EXPECT_EQUAL(steps, 2);
  DOBA_EXPECT(completed);
}

DOBA_TEST("task reports coroutine exceptions") {
  bool completed = false;
  auto value = fail();
  value.start([&](std::exception_ptr error) {
    DOBA_EXPECT(static_cast<bool>(error));
    try {
      std::rethrow_exception(error);
    } catch (const std::runtime_error&) {
      completed = true;
    }
  });
  DOBA_EXPECT(completed);
}

DOBA_TEST("task releases unstarted frames and replaced ownership") {
  auto first = std::make_shared<int>(1);
  auto second = std::make_shared<int>(2);
  {
    auto value = retain(first);
    auto replaced = retain(second);
    DOBA_EXPECT_EQUAL(first.use_count(), 2);
    DOBA_EXPECT_EQUAL(second.use_count(), 2);
    replaced = std::move(value);
    DOBA_EXPECT_EQUAL(second.use_count(), 1);
  }
  DOBA_EXPECT_EQUAL(first.use_count(), 1);
}

DOBA_TEST("task completes once on a different resuming thread") {
  std::coroutine_handle<> next;
  int completions = 0;
  std::thread::id completed_on;
  auto value = fail_after(next);
  value.start([&](std::exception_ptr error) {
    completions++;
    completed_on = std::this_thread::get_id();
    DOBA_EXPECT(static_cast<bool>(error));
  });
  DOBA_EXPECT_EQUAL(completions, 0);
  std::jthread worker([&]() { next.resume(); });
  const auto worker_id = worker.get_id();
  worker.join();
  DOBA_EXPECT_EQUAL(completions, 1);
  DOBA_EXPECT(completed_on == worker_id);
}

DOBA_TEST("task yields and sleeps through its scheduler") {
  scheduled_handles handles;
  task_scheduler scheduler{
      &handles,
      [](void* owner, std::coroutine_handle<> handle) {
        static_cast<scheduled_handles*>(owner)->ready = handle;
      },
      [](void* owner, std::chrono::steady_clock::time_point due,
         std::coroutine_handle<> handle) {
        auto& state = *static_cast<scheduled_handles*>(owner);
        state.due = due;
        state.timed = handle;
      }};
  int steps = 0;
  bool completed = false;
  auto value = use_scheduler(steps);
  value.start([&](std::exception_ptr error) {
    DOBA_EXPECT(!error);
    completed = true;
  }, scheduler);
  DOBA_EXPECT_EQUAL(steps, 1);
  DOBA_EXPECT(static_cast<bool>(handles.ready));
  const auto before = std::chrono::steady_clock::now();
  handles.ready.resume();
  DOBA_EXPECT_EQUAL(steps, 2);
  DOBA_EXPECT(static_cast<bool>(handles.timed));
  DOBA_EXPECT(handles.due > before);
  handles.timed.resume();
  DOBA_EXPECT_EQUAL(steps, 3);
  DOBA_EXPECT(completed);
}
