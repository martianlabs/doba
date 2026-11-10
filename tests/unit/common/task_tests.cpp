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

#include <memory>
#include <type_traits>
#include <utility>

#include "../../helpers/concurrency.h"
#include "common/task_execution.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::task;
using martianlabs::doba::common::task_execution;

template <typename Tty>
concept task_available = requires { typename task<Tty>; };

task<> run(int& steps) {
  ++steps;
  co_await martianlabs::doba::common::yield();
  ++steps;
}

task<> retain(std::shared_ptr<int>) { co_return; }
}  // namespace

// +===========================================================================+
// | [>] task result constraint                                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("task constraints reject unsupported result types") {
  DOBA_EXPECT(task_available<void>);
  DOBA_EXPECT(!task_available<int>);
  DOBA_EXPECT(!task_available<void*>);
  DOBA_EXPECT(!task_available<const void>);
}

// +===========================================================================+
// | [>] lazy task ownership                                     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("task transfers a lazy frame to its execution owner") {
  static_assert(!std::is_copy_constructible_v<task<>>);
  static_assert(std::is_move_constructible_v<task<>>);
  int steps = 0;
  auto value = run(steps);
  DOBA_EXPECT_EQUAL(steps, 0);
  auto moved = std::move(value);
  task_execution work(std::move(moved));
  DOBA_EXPECT_EQUAL(steps, 0);
  DOBA_EXPECT(work.resume());
  DOBA_EXPECT_EQUAL(steps, 1);
  DOBA_EXPECT(!work.done());
  DOBA_EXPECT(work.resume());
  DOBA_EXPECT_EQUAL(steps, 2);
  DOBA_EXPECT(work.done());
  DOBA_EXPECT(!work.error());
}

// +===========================================================================+
// | [>] abandoned task cleanup                                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("task releases an unstarted frame after ownership moves") {
  auto token = std::make_shared<int>(1);
  {
    auto value = retain(token);
    DOBA_EXPECT_EQUAL(token.use_count(), 2);
    auto moved = std::move(value);
    DOBA_EXPECT_EQUAL(token.use_count(), 2);
  }
  DOBA_EXPECT_EQUAL(token.use_count(), 1);
}

// +===========================================================================+
// | [>] explicit frame transfer                                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("released task frame survives the original task") {
  auto token = std::make_shared<int>(1);
  task<>::handle_type handle;
  martianlabs::doba::tests::scope_exit cleanup([&] {
    if (handle) {
      handle.destroy();
    }
  });
  {
    auto value = retain(token);
    handle = std::move(value).release();
    DOBA_EXPECT(static_cast<bool>(handle));
    DOBA_EXPECT(!std::move(value).release());
  }
  DOBA_EXPECT_EQUAL(token.use_count(), 2);
  handle.destroy();
  handle = {};
  DOBA_EXPECT_EQUAL(token.use_count(), 1);
}
