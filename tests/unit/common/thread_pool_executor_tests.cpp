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
#include <chrono>
#include <cstddef>
#include <future>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "common/thread_pool_executor.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::thread_pool_executor;

struct move_only_task {
  explicit move_only_task(bool& in_executed) : executed{&in_executed} {}
  move_only_task(const move_only_task&) = delete;
  move_only_task(move_only_task&& other) noexcept
      : executed{std::exchange(other.executed, nullptr)} {}
  move_only_task& operator=(const move_only_task&) = delete;
  move_only_task& operator=(move_only_task&&) noexcept = delete;

  void operator()() { *executed = true; }
  bool valid() const { return executed != nullptr; }

  bool* executed;
};
}  // namespace

// +===========================================================================+
// | [>] executor validates configuration and ownership           ( test-case ) |
// +===========================================================================+
DOBA_TEST("executor validates configuration and ownership") {
  static_assert(!std::is_copy_constructible_v<thread_pool_executor>);
  static_assert(!std::is_copy_assignable_v<thread_pool_executor>);
  static_assert(!std::is_move_constructible_v<thread_pool_executor>);
  static_assert(!std::is_move_assignable_v<thread_pool_executor>);
  bool zero_workers = false;
  bool zero_capacity = false;
  try {
    thread_pool_executor value{0, 1};
  } catch (const std::invalid_argument&) {
    zero_workers = true;
  }
  try {
    thread_pool_executor value{1, 0};
  } catch (const std::invalid_argument&) {
    zero_capacity = true;
  }
  DOBA_EXPECT(zero_workers);
  DOBA_EXPECT(zero_capacity);
}
// +===========================================================================+
// | [>] executor runs accepted tasks exactly once                ( test-case ) |
// +===========================================================================+
DOBA_TEST("executor runs accepted tasks exactly once") {
  constexpr std::size_t kProducerCount = 4;
  constexpr std::size_t kTasksPerProducer = 256;
  constexpr std::size_t kTaskCount = kProducerCount * kTasksPerProducer;
  thread_pool_executor value{4, kTaskCount};
  std::atomic<std::size_t> accepted{0};
  std::atomic<std::size_t> executed{0};
  value.start();
  std::vector<std::thread> producers;
  producers.reserve(kProducerCount);
  for (std::size_t producer = 0; producer < kProducerCount; producer++) {
    producers.emplace_back([&value, &accepted, &executed]() {
      for (std::size_t i = 0; i < kTasksPerProducer; i++) {
        if (value.try_submit([&executed]() { executed++; })) accepted++;
      }
    });
  }
  for (auto& producer : producers) producer.join();
  value.stop();
  DOBA_EXPECT_EQUAL(accepted.load(), kTaskCount);
  DOBA_EXPECT_EQUAL(executed.load(), kTaskCount);
}
// +===========================================================================+
// | [>] executor rejects work when stopped or saturated          ( test-case ) |
// +===========================================================================+
DOBA_TEST("executor rejects work when stopped or saturated") {
  thread_pool_executor value{1, 2};
  std::promise<void> started;
  std::promise<void> release;
  std::shared_future<void> released = release.get_future().share();
  bool executed = false;
  move_only_task rejected_before{executed};
  bool before_start = value.try_submit(std::move(rejected_before));
  value.start();
  bool blocker = value.try_submit([&started, released]() {
    started.set_value();
    released.wait();
  });
  std::future<void> started_future = started.get_future();
  std::future_status started_status =
      started_future.wait_for(std::chrono::seconds(2));
  bool first = value.try_submit([]() {});
  bool second = value.try_submit([]() {});
  move_only_task rejected_full{executed};
  bool saturated = value.try_submit(std::move(rejected_full));
  release.set_value();
  value.stop();
  move_only_task rejected_after{executed};
  bool after_stop = value.try_submit(std::move(rejected_after));
  DOBA_EXPECT(!before_start);
  DOBA_EXPECT(rejected_before.valid());
  DOBA_EXPECT(blocker);
  DOBA_EXPECT(started_status == std::future_status::ready);
  DOBA_EXPECT(first);
  DOBA_EXPECT(second);
  DOBA_EXPECT(!saturated);
  DOBA_EXPECT(rejected_full.valid());
  DOBA_EXPECT(!after_stop);
  DOBA_EXPECT(rejected_after.valid());
  DOBA_EXPECT(!executed);
}
// +===========================================================================+
// | [>] executor drains tasks and survives task exceptions       ( test-case ) |
// +===========================================================================+
DOBA_TEST("executor drains tasks and survives task exceptions") {
  constexpr std::size_t kTaskCount = 128;
  thread_pool_executor value{2, kTaskCount + 1};
  std::atomic<std::size_t> executed{0};
  value.start();
  bool throwing = value.try_submit([]() { throw std::runtime_error("task"); });
  bool accepted = true;
  for (std::size_t i = 0; i < kTaskCount; i++) {
    accepted = value.try_submit([&executed]() { executed++; }) && accepted;
  }
  value.stop();
  DOBA_EXPECT(throwing);
  DOBA_EXPECT(accepted);
  DOBA_EXPECT_EQUAL(executed.load(), kTaskCount);
}
// +===========================================================================+
// | [>] executor can restart after stopping                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("executor can restart after stopping") {
  thread_pool_executor value{1, 1};
  std::atomic<std::size_t> executed{0};
  value.start();
  bool first = value.try_submit([&executed]() { executed++; });
  value.stop();
  value.start();
  bool second = value.try_submit([&executed]() { executed++; });
  value.stop();
  DOBA_EXPECT(first);
  DOBA_EXPECT(second);
  DOBA_EXPECT_EQUAL(executed.load(), 2);
}
