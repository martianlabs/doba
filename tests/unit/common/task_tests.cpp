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
#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>

#include "common/task.h"
#include "common/task_awaiters.h"
#include "common/task_scheduler.h"
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

task<void> immediate_sleep(int& steps) {
  steps++;
  co_await martianlabs::doba::common::sleep_for(
      std::chrono::steady_clock::duration::zero());
  steps++;
}

task<void> without_scheduler(bool timed) {
  if (timed) {
    co_await martianlabs::doba::common::sleep_for(
        std::chrono::milliseconds(1));
  } else {
    co_await martianlabs::doba::common::yield();
  }
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
  auto completion = [&](std::exception_ptr error) {
    DOBA_EXPECT(!error);
    completed = true;
  };
  moved.start(&completion, [](void* context, std::exception_ptr error) {
    (*static_cast<decltype(completion)*>(context))(error);
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
  auto completion = [&](std::exception_ptr error) {
    DOBA_EXPECT(static_cast<bool>(error));
    try {
      std::rethrow_exception(error);
    } catch (const std::runtime_error&) {
      completed = true;
    }
  };
  value.start(&completion, [](void* context, std::exception_ptr error) {
    (*static_cast<decltype(completion)*>(context))(error);
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
  auto completion = [&](std::exception_ptr error) {
    completions++;
    completed_on = std::this_thread::get_id();
    DOBA_EXPECT(static_cast<bool>(error));
  };
  value.start(&completion, [](void* context, std::exception_ptr error) {
    (*static_cast<decltype(completion)*>(context))(error);
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
  auto completion = [&](std::exception_ptr error) {
    DOBA_EXPECT(!error);
    completed = true;
  };
  value.start(&completion, [](void* context, std::exception_ptr error) {
    (*static_cast<decltype(completion)*>(context))(error);
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

DOBA_TEST("task skips zero sleep without a scheduler") {
  int steps = 0;
  int completions = 0;
  auto value = immediate_sleep(steps);
  auto completion = [&](std::exception_ptr error) {
    DOBA_EXPECT(!error);
    completions++;
  };
  value.start(&completion, [](void* context, std::exception_ptr error) {
    (*static_cast<decltype(completion)*>(context))(error);
  });
  DOBA_EXPECT_EQUAL(steps, 2);
  DOBA_EXPECT_EQUAL(completions, 1);
}

DOBA_TEST("task reports a missing scheduler for yield and sleep") {
  for (bool timed : {false, true}) {
    int completions = 0;
    auto value = without_scheduler(timed);
    auto completion = [&](std::exception_ptr error) {
      DOBA_EXPECT(static_cast<bool>(error));
      try {
        std::rethrow_exception(error);
      } catch (const std::runtime_error& failure) {
        DOBA_EXPECT_EQUAL(std::string_view(failure.what()),
                          "No task scheduler");
      }
      completions++;
    };
    value.start(&completion, [](void* context, std::exception_ptr error) {
      (*static_cast<decltype(completion)*>(context))(error);
    });
    DOBA_EXPECT_EQUAL(completions, 1);
  }
}

DOBA_TEST("task recycles frames after completion") {
  std::coroutine_handle<> next;
  int steps = 0;
  auto first = run(next, steps);
  first.start(nullptr, nullptr);
  const auto address = next.address();
  next.resume();
  auto second = run(next, steps);
  second.start(nullptr, nullptr);
  DOBA_EXPECT(next.address() == address);
  next.resume();
  DOBA_EXPECT_EQUAL(steps, 4);
}

DOBA_TEST("task frame storage preserves size and alignment") {
  using promise = task<void>::promise_type;
  auto* small = promise::operator new(37);
  auto* large = promise::operator new(4097);
  DOBA_EXPECT(small != large);
  DOBA_EXPECT_EQUAL(reinterpret_cast<std::uintptr_t>(small) %
                        __STDCPP_DEFAULT_NEW_ALIGNMENT__,
                    0u);
  DOBA_EXPECT_EQUAL(reinterpret_cast<std::uintptr_t>(large) %
                        __STDCPP_DEFAULT_NEW_ALIGNMENT__,
                    0u);
  static_cast<std::byte*>(small)[36] = std::byte{1};
  static_cast<std::byte*>(large)[4096] = std::byte{2};
  promise::operator delete(small);
  promise::operator delete(large);
  auto* reused_small = promise::operator new(37);
  auto* reused_large = promise::operator new(4097);
  DOBA_EXPECT(reused_small == small);
  DOBA_EXPECT(reused_large == large);
  promise::operator delete(reused_small);
  promise::operator delete(reused_large);
}

DOBA_TEST("task frame allocation rejects size overflow") {
  bool failed = false;
  try {
    auto* storage = task<void>::promise_type::operator new(
        std::numeric_limits<std::size_t>::max());
    task<void>::promise_type::operator delete(storage);
  } catch (const std::bad_alloc&) {
    failed = true;
  }
  DOBA_EXPECT(failed);
}

DOBA_TEST("task returns frames to the resuming thread") {
  std::coroutine_handle<> next;
  int steps = 0;
  int completions = 0;
  auto value = run(next, steps);
  value.start(&completions, [](void* owner, std::exception_ptr error) {
    DOBA_EXPECT(!error);
    ++*static_cast<int*>(owner);
  });
  const auto address = next.address();
  bool reused = false;
  std::jthread worker([&]() {
    next.resume();
    auto second = run(next, steps);
    second.start(&completions, [](void* owner, std::exception_ptr error) {
      DOBA_EXPECT(!error);
      ++*static_cast<int*>(owner);
    });
    reused = next.address() == address;
    next.resume();
  });
  worker.join();
  DOBA_EXPECT(reused);
  DOBA_EXPECT_EQUAL(steps, 4);
  DOBA_EXPECT_EQUAL(completions, 2);
}

DOBA_TEST("task context completion reports immediate and suspended errors") {
  for (bool suspended : {false, true}) {
    std::coroutine_handle<> next;
    int completions = 0;
    auto value = suspended ? fail_after(next) : fail();
    value.start(&completions, [](void* owner, std::exception_ptr error) {
      DOBA_EXPECT(static_cast<bool>(error));
      ++*static_cast<int*>(owner);
    });
    DOBA_EXPECT_EQUAL(completions, suspended ? 0 : 1);
    if (suspended) next.resume();
    DOBA_EXPECT_EQUAL(completions, 1);
  }
}

DOBA_TEST("task context completion contains callback exceptions") {
  int steps = 0;
  int completions = 0;
  auto value = immediate_sleep(steps);
  value.start(&completions, [](void* owner, std::exception_ptr error) {
    DOBA_EXPECT(!error);
    ++*static_cast<int*>(owner);
    throw std::runtime_error("completion failure");
  });
  DOBA_EXPECT_EQUAL(steps, 2);
  DOBA_EXPECT_EQUAL(completions, 1);
}

DOBA_TEST("task completion borrows caller owned context") {
  auto owner = std::make_shared<int>(0);
  std::weak_ptr<int> retained = owner;
  std::coroutine_handle<> next;
  int steps = 0;
  auto value = run(next, steps);
  value.start(owner.get(), [](void* context, std::exception_ptr error) {
    DOBA_EXPECT(!error);
    ++*static_cast<int*>(context);
  });
  DOBA_EXPECT_EQUAL(owner.use_count(), 1);
  next.resume();
  DOBA_EXPECT_EQUAL(*owner, 1);
  DOBA_EXPECT_EQUAL(owner.use_count(), 1);
  owner.reset();
  DOBA_EXPECT(retained.expired());
}

DOBA_TEST("task survives allocation thread exit before completion") {
  std::coroutine_handle<> next;
  int steps = 0;
  int completions = 0;
  {
    std::jthread worker([&]() {
      auto value = run(next, steps);
      value.start(&completions, [](void* owner, std::exception_ptr error) {
        DOBA_EXPECT(!error);
        ++*static_cast<int*>(owner);
      });
    });
  }
  DOBA_EXPECT_EQUAL(steps, 1);
  DOBA_EXPECT_EQUAL(completions, 0);
  next.resume();
  DOBA_EXPECT_EQUAL(steps, 2);
  DOBA_EXPECT_EQUAL(completions, 1);
}

DOBA_TEST("task context completion uses its scheduler") {
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
  int completions = 0;
  auto value = use_scheduler(steps);
  value.start(&completions, [](void* owner, std::exception_ptr error) {
    DOBA_EXPECT(!error);
    ++*static_cast<int*>(owner);
  }, scheduler);
  DOBA_EXPECT_EQUAL(steps, 1);
  DOBA_EXPECT_EQUAL(completions, 0);
  DOBA_EXPECT(static_cast<bool>(handles.ready));
  handles.ready.resume();
  DOBA_EXPECT_EQUAL(steps, 2);
  DOBA_EXPECT_EQUAL(completions, 0);
  DOBA_EXPECT(static_cast<bool>(handles.timed));
  handles.timed.resume();
  DOBA_EXPECT_EQUAL(steps, 3);
  DOBA_EXPECT_EQUAL(completions, 1);
}
