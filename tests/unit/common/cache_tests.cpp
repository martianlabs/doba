// Copyright 2026 martianLabs. Licensed under the Apache License, Version 2.0.
#include <atomic>
#include <barrier>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "common/cache.h"
#include "common/reader.h"
#include "../../helpers/concurrency.h"
#include "test_helper.h"

namespace {
using namespace martianlabs::doba::common;
using martianlabs::doba::tests::manual_event;
using martianlabs::doba::tests::scope_exit;
struct clock_stub {
  using duration = std::chrono::milliseconds;
  using time_point = std::chrono::time_point<clock_stub>;
  static inline duration ticks{};
  static time_point now() { return time_point(ticks); }
};
using test_cache = cache<std::string, std::string, clock_stub>;
auto text(std::string value) { return std::make_shared<const std::string>(std::move(value)); }
}

DOBA_TEST("cache bounds retained cost and updates LRU recency") {
  test_cache value({.max_entries = 2, .max_cost = 6});
  value.get_or_load("a", 2, [] { return text("aa"); });
  value.get_or_load("b", 2, [] { return text("bb"); });
  auto pin = value.find("a");
  value.get_or_load("c", 2, [] { return text("cc"); });
  DOBA_EXPECT(!value.find("b"));
  DOBA_EXPECT(value.find("a") != nullptr);
  value.clear();
  DOBA_EXPECT_EQUAL(value.statistics().resident_cost, 0);
  DOBA_EXPECT_EQUAL(value.statistics().live_cost, 2);
  bool called = false;
  DOBA_EXPECT(!value.get_or_load("d", 5, [&] { called = true; return text("12345"); }));
  DOBA_EXPECT(!called);
  pin.reset();
  DOBA_EXPECT(value.get_or_load("d", 5, [] { return text("12345"); }) != nullptr);
}

DOBA_TEST("cache expiration uses its clock and starts after loading") {
  clock_stub::ticks = std::chrono::milliseconds(0);
  test_cache value({.ttl = std::chrono::milliseconds(10)});
  auto first = value.get_or_load("a", 1, [] {
    clock_stub::ticks += std::chrono::milliseconds(20);
    return text("a");
  });
  clock_stub::ticks += std::chrono::milliseconds(9);
  DOBA_EXPECT_EQUAL(value.find("a"), first);
  clock_stub::ticks += std::chrono::milliseconds(1);
  DOBA_EXPECT(!value.find("a"));
  DOBA_EXPECT_EQUAL(*first, "a");
  DOBA_EXPECT_EQUAL(value.statistics().live_cost, 1);
}

DOBA_TEST("cache rejects excess loads and coalesces concurrent same key loads") {
  cache<int, std::string> value({.max_loads = 1});
  manual_event entered, release;
  std::shared_ptr<const std::string> first, second;
  std::atomic<int> calls{0};
  std::jthread a([&] {
    first = value.get_or_load(1, 5, [&] {
      ++calls; entered.open(); release.wait(); return text("hello");
    });
  });
  std::jthread b;
  scope_exit cleanup([&] { release.open(); });
  DOBA_EXPECT(entered.wait_for());
  DOBA_EXPECT(!value.get_or_load(2, 1, [] { return text("x"); }));
  b = std::jthread([&] { second = value.get_or_load(1, 5, [&] { ++calls; return text("bad"); }); });
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  while (!value.statistics().waiters && std::chrono::steady_clock::now() < deadline)
    std::this_thread::yield();
  DOBA_EXPECT_EQUAL(value.statistics().waiters, 1);
  release.open();
  a.join(); b.join();
  DOBA_EXPECT_EQUAL(calls.load(), 1);
  DOBA_EXPECT_EQUAL(first, second);
  DOBA_EXPECT_EQUAL(*first, "hello");
}

DOBA_TEST("cache invalidation prevents obsolete loads from republishing") {
  for (bool clear : {false, true}) {
    cache<int, std::string> value;
    manual_event entered, release;
    std::shared_ptr<const std::string> old;
    std::jthread worker([&] {
      old = value.get_or_load(1, 3, [&] {
        entered.open(); release.wait(); return text("old");
      });
    });
    scope_exit cleanup([&] { release.open(); });
    DOBA_EXPECT(entered.wait_for());
    if (clear) value.clear(); else value.erase(1);
    auto fresh = value.get_or_load(1, 3, [] { return text("new"); });
    release.open(); worker.join();
    DOBA_EXPECT(!old);
    DOBA_EXPECT_EQUAL(value.find(1), fresh);
    DOBA_EXPECT_EQUAL(value.statistics().live_cost, 3);
  }
}

DOBA_TEST("cache releases reservations after failures and detects recursion") {
  cache<int, std::string> value;
  bool failed = false;
  try {
    value.get_or_load(1, 20, []() -> std::shared_ptr<const std::string> {
      throw std::runtime_error("load failed");
    });
  } catch (const std::runtime_error&) { failed = true; }
  DOBA_EXPECT(failed);
  DOBA_EXPECT_EQUAL(value.statistics().live_cost, 0);
  DOBA_EXPECT_EQUAL(value.statistics().loads, 0);
  DOBA_EXPECT(!value.get_or_load(1, 2, [] { return std::shared_ptr<const std::string>{}; }));
  failed = false;
  try {
    value.get_or_load(1, 1, [&] {
      return value.get_or_load(1, 1, [] { return text("x"); });
    });
  } catch (const std::logic_error&) { failed = true; }
  DOBA_EXPECT(failed);
  DOBA_EXPECT(value.get_or_load(1, 1, [] { return text("x"); }) != nullptr);
}

DOBA_TEST("shared readers retain evicted values after cache destruction") {
  reader first, second;
  std::weak_ptr<const std::string> weak;
  {
    cache<int, std::string> value;
    auto data = value.get_or_load(1, 4, [] { return text("data"); });
    weak = data;
    const auto bytes = std::as_bytes(std::span(data->data(), data->size()));
    first = reader::shared(data, bytes);
    second = reader::shared(data, bytes.subspan(1, 2));
    value.clear();
  }
  DOBA_EXPECT(!weak.expired());
  reader moved(std::move(first));
  std::string a, b;
  moved.read_all(a); second.read_all(b);
  DOBA_EXPECT_EQUAL(a, "data");
  DOBA_EXPECT_EQUAL(b, "at");
  moved = reader{}; second = reader{};
  DOBA_EXPECT(weak.expired());
}

DOBA_TEST("cache handles concurrent distinct keys with bounded live cost") {
  cache<int, int> value({.max_entries = 8, .max_cost = 8, .max_loads = 4});
  std::atomic<bool> good{true};
  std::vector<std::jthread> workers;
  for (int thread = 0; thread < 8; ++thread) workers.emplace_back([&, thread] {
    for (int i = 0; i < 100; ++i) {
      auto result = value.get_or_load(thread, 1, [thread] { return std::make_shared<const int>(thread); });
      if (result && *result != thread) good = false;
      if (value.statistics().live_cost > 8) good = false;
      if (i % 7 == 0) value.erase(thread);
      if (i % 29 == 0) value.clear();
    }
  });
  workers.clear();
  DOBA_EXPECT(good.load());
  value.clear();
  DOBA_EXPECT_EQUAL(value.statistics().live_cost, 0);
}

DOBA_TEST("cache shares loader exceptions with waiters and permits retry") {
  cache<int, std::string> value;
  manual_event entered, release;
  std::atomic<unsigned> failures{0};
  std::jthread first([&] {
    try {
      value.get_or_load(1, 5, [&]() -> std::shared_ptr<const std::string> {
        entered.open(); release.wait(); throw std::runtime_error("expected");
      });
    } catch (const std::runtime_error&) { ++failures; }
  });
  std::jthread second;
  scope_exit cleanup([&] { release.open(); });
  DOBA_EXPECT(entered.wait_for());
  second = std::jthread([&] {
    try { value.get_or_load(1, 5, [] { return text("wrong"); }); }
    catch (const std::runtime_error&) { ++failures; }
  });
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  while (!value.statistics().waiters && std::chrono::steady_clock::now() < deadline)
    std::this_thread::yield();
  DOBA_EXPECT_EQUAL(value.statistics().waiters, 1);
  release.open(); first.join(); second.join();
  DOBA_EXPECT_EQUAL(failures.load(), 2);
  DOBA_EXPECT_EQUAL(value.statistics().live_cost, 0);
  DOBA_EXPECT_EQUAL(*value.get_or_load(1, 2, [] { return text("ok"); }), "ok");
}

DOBA_TEST("cache invalidation wakes waiters before an obsolete loader finishes") {
  cache<int, std::string> value;
  manual_event entered, release, waiter_done;
  std::shared_ptr<const std::string> old, waiting;
  std::jthread first([&] {
    old = value.get_or_load(1, 5, [&] { entered.open(); release.wait(); return text("old"); });
  });
  std::jthread second;
  scope_exit cleanup([&] { release.open(); });
  DOBA_EXPECT(entered.wait_for());
  second = std::jthread([&] {
    waiting = value.get_or_load(1, 5, [] { return text("wrong"); });
    waiter_done.open();
  });
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  while (!value.statistics().waiters && std::chrono::steady_clock::now() < deadline)
    std::this_thread::yield();
  DOBA_EXPECT_EQUAL(value.statistics().waiters, 1);
  value.clear();
  DOBA_EXPECT(waiter_done.wait_for());
  second.join();
  DOBA_EXPECT(!waiting);
  DOBA_EXPECT_EQUAL(value.statistics().loads, 1);
  auto fresh = value.get_or_load(1, 3, [] { return text("new"); });
  release.open(); first.join();
  DOBA_EXPECT(!old);
  DOBA_EXPECT_EQUAL(value.find(1), fresh);
  DOBA_EXPECT_EQUAL(value.statistics().live_cost, 3);
}

DOBA_TEST("cache disabled admission and extreme TTL preserve contracts") {
  for (const auto policies : {cache_policies{.max_entries = 0},
                             cache_policies{.max_cost = 0},
                             cache_policies{.max_loads = 0}}) {
    cache<int, int> value(policies);
    bool called = false;
    DOBA_EXPECT(!value.get_or_load(1, 0, [&] { called = true; return std::make_shared<const int>(1); }));
    DOBA_EXPECT(!called);
  }
  clock_stub::ticks = std::chrono::milliseconds(-100);
  test_cache value({.ttl = std::chrono::milliseconds::max()});
  auto pinned = value.get_or_load("a", 1, [] { return text("a"); });
  clock_stub::ticks = std::chrono::milliseconds::max() - std::chrono::milliseconds(1);
  DOBA_EXPECT_EQUAL(value.find("a"), pinned);
}
