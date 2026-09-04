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
  martianlabs::doba::transport::server::detail::executor value;
  std::atomic<std::size_t> resumed = 0;
  std::vector<scheduled_probe> probes;
  probes.reserve(65);
  for (std::size_t i = 0; i < 65; i++) {
    probes.emplace_back(increment(resumed));
    DOBA_EXPECT(value.schedule(probes.back().get_coroutine()));
  }
  DOBA_EXPECT(value.run());
  DOBA_EXPECT_EQUAL(resumed.load(), 64);
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT_EQUAL(resumed.load(), 65);
  DOBA_EXPECT(!value.run());
  DOBA_EXPECT_EQUAL(resumed.load(), 65);
}
// +===========================================================================+
// | [>] executor supports concurrent producers and consumers    ( test-case ) |
// +===========================================================================+
DOBA_TEST("executor supports concurrent producers and consumers") {
  martianlabs::doba::transport::server::detail::executor value;
  std::atomic<std::size_t> resumed = 0;
  std::atomic<bool> scheduled = true;
  std::vector<scheduled_probe> probes;
  probes.reserve(256);
  for (std::size_t i = 0; i < 256; i++) {
    probes.emplace_back(increment(resumed));
  }
  std::vector<std::jthread> producers;
  for (std::size_t producer = 0; producer < 4; producer++) {
    producers.emplace_back([&value, &probes, &scheduled, producer]() {
      const std::size_t begin = producer * 64;
      for (std::size_t i = begin; i < begin + 64; i++) {
        if (!value.schedule(probes[i].get_coroutine())) {
          scheduled.store(false);
        }
      }
    });
  }
  producers.clear();
  DOBA_EXPECT(scheduled.load());
  std::vector<std::jthread> consumers;
  for (std::size_t consumer = 0; consumer < 4; consumer++) {
    consumers.emplace_back([&value]() { value.run(); });
  }
  consumers.clear();
  DOBA_EXPECT_EQUAL(resumed.load(), 256);
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
