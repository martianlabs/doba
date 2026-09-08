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

#include <array>
#include <atomic>
#include <barrier>
#include <chrono>
#include <coroutine>
#include <cstddef>
#include <exception>
#include <thread>
#include <utility>
#include <vector>

#include "transport/server/executor.h"
#include "test_helper.h"

namespace {
class scheduled_probe {
 public:
  struct promise_type {
    scheduled_probe get_return_object() noexcept {
      return scheduled_probe(
          std::coroutine_handle<promise_type>::from_promise(*this));
    }
    std::suspend_always initial_suspend() const noexcept { return {}; }
    std::suspend_always final_suspend() const noexcept { return {}; }
    void return_void() const noexcept {}
    void unhandled_exception() const noexcept { std::terminate(); }
  };
  scheduled_probe(const scheduled_probe&) = delete;
  scheduled_probe(scheduled_probe&& in) noexcept
      : coroutine_(std::exchange(in.coroutine_, nullptr)) {}
  ~scheduled_probe() {
    if (coroutine_) coroutine_.destroy();
  }
  scheduled_probe& operator=(const scheduled_probe&) = delete;
  scheduled_probe& operator=(scheduled_probe&& in) noexcept {
    if (this == &in) return *this;
    if (coroutine_) coroutine_.destroy();
    coroutine_ = std::exchange(in.coroutine_, nullptr);
    return *this;
  }
  std::coroutine_handle<> get_coroutine() const noexcept { return coroutine_; }

 private:
  explicit scheduled_probe(
      std::coroutine_handle<promise_type> coroutine) noexcept
      : coroutine_(coroutine) {}
  std::coroutine_handle<promise_type> coroutine_;
};

scheduled_probe increment(std::atomic<std::size_t>& value) {
  value.fetch_add(1, std::memory_order_relaxed);
  co_return;
}
}  // namespace

// +===========================================================================+
// | [>] executor resumes a bounded batch exactly once           ( test-case ) |
// +===========================================================================+
DOBA_TEST("executor resumes a bounded batch exactly once") {
  for (std::size_t count : {63, 64, 65}) {
    std::array<std::atomic<std::size_t>, 65> resumed{};
    std::vector<scheduled_probe> probes;
    probes.reserve(count);
    martianlabs::doba::transport::server::detail::executor value;
    for (std::size_t i = 0; i < count; i++) {
      probes.emplace_back(increment(resumed[i]));
      DOBA_EXPECT(value.schedule(probes.back().get_coroutine()));
    }
    DOBA_EXPECT_EQUAL(value.run(), count > 64);
    for (std::size_t i = 0; i < count; i++) {
      DOBA_EXPECT_EQUAL(resumed[i].load(), i < 64 ? 1 : 0);
    }
    DOBA_EXPECT(!value.run());
    DOBA_EXPECT(!value.run());
    for (std::size_t i = 0; i < count; i++) {
      DOBA_EXPECT_EQUAL(resumed[i].load(), 1);
    }
  }
}
// +===========================================================================+
// | [>] executor supports concurrent producers and consumers    ( test-case ) |
// +===========================================================================+
DOBA_TEST("executor supports concurrent producers and consumers") {
  std::array<std::atomic<std::size_t>, 256> resumed{};
  std::atomic<bool> scheduled = true;
  std::atomic<std::size_t> producers_left = 4;
  std::array<bool, 4> overlapped{};
  std::vector<scheduled_probe> probes;
  probes.reserve(resumed.size());
  for (auto& count : resumed) probes.emplace_back(increment(count));
  martianlabs::doba::transport::server::detail::executor value;
  std::barrier start(8);
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::seconds(3);
  std::array<std::jthread, 4> producers;
  std::array<std::jthread, 4> consumers;
  for (std::size_t producer = 0; producer < producers.size(); producer++) {
    producers[producer] = std::jthread([&, producer]() {
      start.arrive_and_wait();
      const std::size_t begin = producer * 64;
      if (!value.schedule(probes[begin].get_coroutine())) scheduled = false;
      while (resumed[begin].load() == 0 &&
             std::chrono::steady_clock::now() < deadline) {
        std::this_thread::yield();
      }
      overlapped[producer] = resumed[begin].load() == 1;
      for (std::size_t i = begin + 1; i < begin + 64; i++) {
        if (!value.schedule(probes[i].get_coroutine())) scheduled = false;
      }
      producers_left.fetch_sub(1);
    });
  }
  for (auto& consumer : consumers) {
    consumer = std::jthread([&]() {
      start.arrive_and_wait();
      while (std::chrono::steady_clock::now() < deadline) {
        const bool finished = producers_left.load() == 0;
        const bool more = value.run();
        if (finished && !more) break;
        std::this_thread::yield();
      }
    });
  }
  for (auto& producer : producers) producer.join();
  for (auto& consumer : consumers) consumer.join();
  DOBA_EXPECT(scheduled.load());
  for (bool overlap : overlapped) DOBA_EXPECT(overlap);
  for (const auto& count : resumed) DOBA_EXPECT_EQUAL(count.load(), 1);
  DOBA_EXPECT(!value.run());
}
// +===========================================================================+
// | [>] executor rejects work while stopped                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("executor rejects work while stopped") {
  martianlabs::doba::transport::server::detail::executor value;
  std::atomic<std::size_t> resumed = 0;
  auto accepted = increment(resumed);
  auto rejected = increment(resumed);
  DOBA_EXPECT(value.schedule(accepted.get_coroutine()));
  value.stop();
  DOBA_EXPECT(!value.schedule(rejected.get_coroutine()));
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT_EQUAL(resumed.load(), 1);
  DOBA_EXPECT(value.start());
  DOBA_EXPECT(value.schedule(rejected.get_coroutine()));
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT_EQUAL(resumed.load(), 2);
}
// +===========================================================================+
// | [>] executor rejects null and completed continuations       ( test-case ) |
// +===========================================================================+
DOBA_TEST("executor rejects null and completed continuations") {
  martianlabs::doba::transport::server::detail::executor value;
  std::atomic<std::size_t> resumed = 0;
  DOBA_EXPECT(!value.schedule({}));
  auto completed = increment(resumed);
  completed.get_coroutine().resume();
  DOBA_EXPECT_EQUAL(resumed.load(), 1);
  DOBA_EXPECT(!value.schedule(completed.get_coroutine()));
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT_EQUAL(resumed.load(), 1);
}
// +===========================================================================+
// | [>] executor start requires an empty queue                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("executor start requires an empty queue") {
  martianlabs::doba::transport::server::detail::executor value;
  std::atomic<std::size_t> resumed = 0;
  auto pending = increment(resumed);
  DOBA_EXPECT(value.schedule(pending.get_coroutine()));
  value.stop();
  DOBA_EXPECT(!value.start());
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT_EQUAL(resumed.load(), 1);
  DOBA_EXPECT(value.start());
}

// +===========================================================================+
// | [>] concurrent stop acceptance                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("executor stop separates accepted and rejected concurrent work") {
  std::array<std::atomic<std::size_t>, 258> resumed{};
  std::array<bool, 258> accepted{};
  std::vector<scheduled_probe> probes;
  probes.reserve(resumed.size());
  for (auto& count : resumed) probes.emplace_back(increment(count));
  martianlabs::doba::transport::server::detail::executor value;
  accepted[0] = value.schedule(probes[0].get_coroutine());
  std::atomic<std::size_t> producers_left = 4;
  std::barrier start(6);
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::seconds(3);
  std::array<std::jthread, 4> producers;
  for (std::size_t producer = 0; producer < producers.size(); producer++) {
    producers[producer] = std::jthread([&, producer]() {
      start.arrive_and_wait();
      for (std::size_t i = 1 + producer * 64; i <= (producer + 1) * 64; i++) {
        accepted[i] = value.schedule(probes[i].get_coroutine());
        std::this_thread::yield();
      }
      producers_left.fetch_sub(1);
    });
  }
  std::jthread consumer([&]() {
    start.arrive_and_wait();
    while (std::chrono::steady_clock::now() < deadline) {
      const bool finished = producers_left.load() == 0;
      const bool more = value.run();
      if (finished && !more) break;
      std::this_thread::yield();
    }
  });
  start.arrive_and_wait();
  value.stop();
  accepted[257] = value.schedule(probes[257].get_coroutine());
  for (auto& producer : producers) producer.join();
  consumer.join();
  for (std::size_t i = 0; i < 5; i++) value.run();
  DOBA_EXPECT(accepted[0]);
  DOBA_EXPECT(!accepted[257]);
  for (std::size_t i = 0; i < resumed.size(); i++) {
    DOBA_EXPECT_EQUAL(resumed[i].load(), accepted[i] ? 1 : 0);
  }
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT(value.start());
  DOBA_EXPECT(value.schedule(probes[257].get_coroutine()));
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT_EQUAL(resumed[257].load(), 1);
}

// +===========================================================================+
// | [>] continuation schedules successor                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("executor continuations can schedule a successor") {
  martianlabs::doba::transport::server::detail::executor value;
  std::atomic<std::size_t> first_count = 0;
  std::atomic<std::size_t> second_count = 0;
  bool accepted = false;
  auto second = increment(second_count);
  const auto schedule_next = [&]() -> scheduled_probe {
    first_count.fetch_add(1);
    accepted = value.schedule(second.get_coroutine());
    co_return;
  };
  auto first = schedule_next();
  DOBA_EXPECT(value.schedule(first.get_coroutine()));
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT(accepted);
  DOBA_EXPECT_EQUAL(first_count.load(), 1);
  DOBA_EXPECT_EQUAL(second_count.load(), 0);
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT_EQUAL(first_count.load(), 1);
  DOBA_EXPECT_EQUAL(second_count.load(), 1);
  DOBA_EXPECT(!value.run());
}
