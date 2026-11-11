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

#include <chrono>
#include <coroutine>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>

#include "common/task_execution.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::sleep_for;
using martianlabs::doba::common::task;
using martianlabs::doba::common::task_execution;
using martianlabs::doba::common::yield;
using clock_type = task_execution::clock;

// Keep coroutine parameters alive until the frame is destroyed.
task<> finish(std::shared_ptr<int>, int& steps) {
  ++steps;
  co_return;
}

task<> sequence(std::shared_ptr<int>, int& steps) {
  ++steps;
  co_await yield();
  ++steps;
  co_await sleep_for(std::chrono::hours(1));
  ++steps;
}

task<> fail(bool suspended) {
  if (suspended) co_await yield();
  throw std::runtime_error("controlled failure");
}

task<> sleep_once(clock_type::duration duration, int& steps) {
  ++steps;
  co_await sleep_for(duration);
  ++steps;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] foreign_awaiter                                            ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct foreign_awaiter {
  bool& touched;
  bool ready;
  // +=========================================================================+
  // | [>] await_ready                                              ( public ) |
  // +-------------------------------------------------------------------------+
  bool await_ready() const noexcept { touched = true; return ready; }
  // +=========================================================================+
  // | [>] await_suspend                                            ( public ) |
  // +-------------------------------------------------------------------------+
  void await_suspend(std::coroutine_handle<>) const noexcept { touched = true; }
  // +=========================================================================+
  // | [>] await_resume                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void await_resume() const noexcept { touched = true; }
};

task<> foreign(bool& touched, bool ready) {
  co_await foreign_awaiter{touched, ready};
}

task<> yields(int& steps) {
  for (int i = 0; i < 1000; ++i) {
    ++steps;
    co_await yield();
  }
}

task<> local_lifetime(const std::shared_ptr<int>& token, bool fail_at_end) {
  auto retained = token;
  co_await yield();
  if (fail_at_end) throw std::runtime_error("local cleanup");
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] awaitable                                                  ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct awaitable {
  bool& touched;
  foreign_awaiter operator co_await() const noexcept {
    touched = true;
    return {touched, true};
  }
};

task<> foreign_operator(bool& touched) {
  co_await awaitable{touched};
}
}  // namespace

DOBA_TEST("controlled task adopts lazily and retains its completed frame") {
  static_assert(!std::is_copy_constructible_v<task_execution>);
  static_assert(!std::is_move_constructible_v<task_execution>);
  auto token = std::make_shared<int>(0);
  int steps = 0;
  task_execution work(finish(token, steps));
  DOBA_EXPECT_EQUAL(steps, 0);
  DOBA_EXPECT_EQUAL(token.use_count(), 2);
  DOBA_EXPECT(!work.done());
  DOBA_EXPECT(work.ready());
  DOBA_EXPECT(!work.deadline());
  DOBA_EXPECT(!work.error());
  DOBA_EXPECT(work.resume());
  DOBA_EXPECT_EQUAL(steps, 1);
  DOBA_EXPECT(work.done());
  DOBA_EXPECT(!work.ready());
  DOBA_EXPECT(!work.deadline());
  DOBA_EXPECT(!work.error());
  DOBA_EXPECT_EQUAL(token.use_count(), 2);
  DOBA_EXPECT(!work.resume());
  work.cancel();
  DOBA_EXPECT_EQUAL(token.use_count(), 1);
  DOBA_EXPECT(work.done());
  DOBA_EXPECT(!work.resume());
  work.cancel();
  DOBA_EXPECT_EQUAL(token.use_count(), 1);
}

DOBA_TEST("controlled task yields once per turn and honors sleep eligibility") {
  int steps = 0;
  task_execution work(sequence({}, steps));
  DOBA_EXPECT(work.resume());
  DOBA_EXPECT_EQUAL(steps, 1);
  DOBA_EXPECT(work.ready());
  DOBA_EXPECT(!work.deadline());
  const auto before = clock_type::now();
  DOBA_EXPECT(work.resume());
  const auto after = clock_type::now();
  DOBA_EXPECT_EQUAL(steps, 2);
  DOBA_EXPECT(!work.ready());
  DOBA_EXPECT(work.deadline().has_value());
  const auto due = *work.deadline();
  DOBA_EXPECT(due >= before + std::chrono::hours(1));
  DOBA_EXPECT(due <= after + std::chrono::hours(1));
  DOBA_EXPECT(!work.resume(due - clock_type::duration(1)));
  DOBA_EXPECT_EQUAL(*work.deadline(), due);
  DOBA_EXPECT_EQUAL(steps, 2);
  DOBA_EXPECT(work.resume(due));
  DOBA_EXPECT_EQUAL(steps, 3);
  DOBA_EXPECT(work.done());
  DOBA_EXPECT(!work.error());
}

DOBA_TEST("controlled task cancellation releases every suspended state") {
  for (int turns : {0, 1, 2, 3}) {
    auto token = std::make_shared<int>(0);
    int steps = 0;
    task_execution work(sequence(token, steps));
    for (int i = 0; i < turns; ++i) {
      DOBA_EXPECT(work.resume(work.deadline().value_or(clock_type::now())));
    }
    DOBA_EXPECT_EQUAL(token.use_count(), 2);
    work.cancel();
    DOBA_EXPECT_EQUAL(token.use_count(), 1);
    DOBA_EXPECT(work.done());
    DOBA_EXPECT(!work.ready());
    DOBA_EXPECT(!work.deadline());
    DOBA_EXPECT(!work.error());
    DOBA_EXPECT(!work.resume(clock_type::time_point::max()));
    DOBA_EXPECT_EQUAL(steps, turns);
  }
}

DOBA_TEST("controlled task destruction releases suspended frames") {
  for (int turns : {0, 1, 2, 3}) {
    auto token = std::make_shared<int>(0);
    int steps = 0;
    {
      task_execution work(sequence(token, steps));
      for (int i = 0; i < turns; ++i) {
        DOBA_EXPECT(work.resume(work.deadline().value_or(clock_type::now())));
      }
      DOBA_EXPECT_EQUAL(token.use_count(), 2);
    }
    DOBA_EXPECT_EQUAL(token.use_count(), 1);
    DOBA_EXPECT_EQUAL(steps, turns);
  }
}

DOBA_TEST("controlled task retains exceptions before and after suspension") {
  for (bool suspended : {false, true}) {
    task_execution work(fail(suspended));
    DOBA_EXPECT(work.resume());
    if (suspended) {
      DOBA_EXPECT(!work.done());
      DOBA_EXPECT(!work.error());
      DOBA_EXPECT(work.resume());
    }
    DOBA_EXPECT(work.done());
    DOBA_EXPECT(static_cast<bool>(work.error()));
    bool caught = false;
    try {
      std::rethrow_exception(work.error());
    } catch (const std::runtime_error& error) {
      caught = std::string_view(error.what()) == "controlled failure";
    }
    DOBA_EXPECT(caught);
    DOBA_EXPECT(!work.resume());
    work.cancel();
    DOBA_EXPECT(!work.error());
  }
}

DOBA_TEST("controlled task rejects foreign awaiters before invoking them") {
  for (bool ready : {false, true}) {
    bool touched = false;
    task_execution work(foreign(touched, ready));
    DOBA_EXPECT(work.resume());
    DOBA_EXPECT(work.done());
    DOBA_EXPECT(!touched);
    DOBA_EXPECT(static_cast<bool>(work.error()));
    bool caught = false;
    try {
      std::rethrow_exception(work.error());
    } catch (const std::logic_error& error) {
      caught = std::string_view(error.what()) == "Unsupported task awaiter";
    }
    DOBA_EXPECT(caught);
  }
}

DOBA_TEST("controlled task skips nonpositive sleeps in the same turn") {
  for (auto duration : {clock_type::duration::zero(),
                        clock_type::duration(-1),
                        clock_type::duration::min()}) {
    int steps = 0;
    task_execution work(sleep_once(duration, steps));
    DOBA_EXPECT(work.resume());
    DOBA_EXPECT(work.done());
    DOBA_EXPECT_EQUAL(steps, 2);
    DOBA_EXPECT(!work.deadline());
    DOBA_EXPECT(!work.error());
  }
}

DOBA_TEST("task sleep saturates overflowing positive deadlines") {
  int steps = 0;
  task_execution work(sleep_once(clock_type::duration::max(), steps));
  DOBA_EXPECT(work.resume());
  DOBA_EXPECT_EQUAL(steps, 1);
  DOBA_EXPECT(work.deadline().has_value());
  DOBA_EXPECT_EQUAL(*work.deadline(), clock_type::time_point::max());
  DOBA_EXPECT(!work.resume(clock_type::time_point::max() -
                           clock_type::duration(1)));
  work.cancel();
  DOBA_EXPECT_EQUAL(steps, 1);
}

DOBA_TEST("controlled task has no recursive yield resumption") {
  int steps = 0;
  task_execution work(yields(steps));
  for (int i = 0; i < 1000; ++i) {
    DOBA_EXPECT(work.resume());
    DOBA_EXPECT_EQUAL(steps, i + 1);
    DOBA_EXPECT(work.ready());
    DOBA_EXPECT(!work.done());
  }
  DOBA_EXPECT(work.resume());
  DOBA_EXPECT(work.done());
  DOBA_EXPECT_EQUAL(steps, 1000);
}

DOBA_TEST("controlled task permits serialized migration between workers") {
  int steps = 0;
  task_execution work(sequence({}, steps));
  bool resumed = false;
  std::jthread first([&]() { resumed = work.resume(); });
  first.join();
  DOBA_EXPECT(resumed);
  DOBA_EXPECT_EQUAL(steps, 1);
  std::jthread second([&]() { resumed = work.resume(); });
  second.join();
  DOBA_EXPECT(resumed);
  DOBA_EXPECT_EQUAL(steps, 2);
  const auto due = work.deadline();
  DOBA_EXPECT(due.has_value());
  std::jthread third([&]() { resumed = work.resume(*due); });
  third.join();
  DOBA_EXPECT(resumed);
  DOBA_EXPECT_EQUAL(steps, 3);
  DOBA_EXPECT(work.done());
}

DOBA_TEST("task rejects repeated transfer to an execution owner") {
  int steps = 0;
  auto value = finish({}, steps);
  task_execution work(std::move(value));
  bool rejected = false;
  try {
    task_execution invalid(std::move(value));
  } catch (const std::invalid_argument&) {
    rejected = true;
  }
  DOBA_EXPECT(rejected);
  DOBA_EXPECT(work.resume());
  DOBA_EXPECT_EQUAL(steps, 1);
}

DOBA_TEST("controlled task cleans body locals on cancellation and completion") {
  for (bool fail_at_end : {false, true}) {
    for (bool cancel : {false, true}) {
      auto token = std::make_shared<int>(0);
      task_execution work(local_lifetime(token, fail_at_end));
      DOBA_EXPECT_EQUAL(token.use_count(), 1);
      DOBA_EXPECT(work.resume());
      DOBA_EXPECT_EQUAL(token.use_count(), 2);
      if (cancel) {
        work.cancel();
        DOBA_EXPECT(!work.error());
      } else {
        DOBA_EXPECT(work.resume());
        DOBA_EXPECT(static_cast<bool>(work.error()) == fail_at_end);
      }
      DOBA_EXPECT(work.done());
      DOBA_EXPECT_EQUAL(token.use_count(), 1);
    }
  }
}

DOBA_TEST("controlled task rejects operator co_await before it publishes") {
  bool touched = false;
  task_execution work(foreign_operator(touched));
  DOBA_EXPECT(work.resume());
  DOBA_EXPECT(work.done());
  DOBA_EXPECT(work.error() != nullptr);
  DOBA_EXPECT(!touched);
}
