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
// Copyright 2026 martianLabs
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
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "../../helpers/concurrency.h"
#include "common/cache.h"
#include "common/cache_policies.h"
#include "common/reader.h"
#include "test_helper.h"

namespace {
using namespace martianlabs::doba::common;
using martianlabs::doba::tests::manual_event;
using martianlabs::doba::tests::scope_exit;

auto text(std::string value) {
  return std::make_shared<const std::string>(std::move(value));
}
}  // namespace

// +===========================================================================+
// | [>] resident entries and LRU recency                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("cache bounds resident entries and promotes sequential hits") {
  cache<std::string> value({.max_entries = 2});
  value.try_get_or_load("a", 2, [] { return text("aa"); });
  value.try_get_or_load("b", 2, [] { return text("bb"); });
  auto pinned = value.find("a");
  value.try_get_or_load("c", 2, [] { return text("cc"); });
  DOBA_EXPECT(!value.find("b"));
  DOBA_EXPECT_EQUAL(value.find("a"), pinned);
  DOBA_EXPECT(value.find("c") != nullptr);
}

// +===========================================================================+
// | [>] resident cost and response ownership                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("cache bounds resident cost independently of retained responses") {
  cache<std::string> value({.max_cost = 6});
  auto old = value.try_get_or_load("old", 6, [] { return text("old"); });
  auto fresh = value.try_get_or_load("new", 5, [] { return text("new"); });
  DOBA_EXPECT(fresh != nullptr);
  DOBA_EXPECT(!value.find("old"));
  DOBA_EXPECT_EQUAL(*old, "old");
  value.clear();
  DOBA_EXPECT_EQUAL(*fresh, "new");
  DOBA_EXPECT(!value.find("new"));
  DOBA_EXPECT(value.try_get_or_load("full", 6, [] { return text("full"); }) != nullptr);
}

// +===========================================================================+
// | [>] direct shared ownership and cached hits                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("cache returns the loader ownership directly and reuses hits") {
  cache<std::string> value;
  auto source = text("data");
  auto first = value.try_get_or_load("file", 4, [&] { return source; });
  DOBA_EXPECT_EQUAL(first, source);
  DOBA_EXPECT(!first.owner_before(source) && !source.owner_before(first));
  bool called = false;
  auto hit = value.try_get_or_load(
      "file", std::numeric_limits<std::size_t>::max(), [&] {
        called = true;
        return text("wrong");
      });
  DOBA_EXPECT_EQUAL(hit, source);
  DOBA_EXPECT(!called);
}

// +===========================================================================+
// | [>] disabled and oversized admission                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("cache disabled and oversize admission never invokes the loader") {
  for (auto policies :
       {cache_policies{.max_entries = 0}, cache_policies{.max_cost = 0},
        cache_policies{.max_loads = 0}, cache_policies{.max_cost = 1}}) {
    cache<std::string> value(policies);
    bool called = false;
    DOBA_EXPECT(!value.try_get_or_load("file", 2, [&] {
      called = true;
      return text("xx");
    }));
    DOBA_EXPECT(!called);
  }
  cache<std::string> value({.max_cost = 1});
  auto old = value.try_get_or_load("old", 1, [] { return text("x"); });
  DOBA_EXPECT(!value.try_get_or_load("large", 2, [] { return text("xx"); }));
  DOBA_EXPECT_EQUAL(value.find("old"), old);
}

// +===========================================================================+
// | [>] failed and empty loads                                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST(
    "cache null and failed loads preserve resident entries and permit retry") {
  cache<std::string> value({.max_entries = 1, .max_cost = 4, .max_loads = 1});
  auto old = value.try_get_or_load("old", 4, [] { return text("old"); });
  DOBA_EXPECT(!value.try_get_or_load(
      "new", 4, [] { return std::shared_ptr<const std::string>{}; }));
  bool failed = false;
  try {
    value.try_get_or_load("new", 4, []() -> std::shared_ptr<const std::string> {
      throw std::runtime_error("expected");
    });
  } catch (const std::runtime_error&) {
    failed = true;
  }
  DOBA_EXPECT(failed);
  DOBA_EXPECT_EQUAL(value.find("old"), old);
  DOBA_EXPECT(value.try_get_or_load("new", 4, [] { return text("new"); }) !=
              nullptr);
}

// +===========================================================================+
// | [>] active load bypass                                     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("cache bypasses an active key while distinct keys load") {
  cache<std::string> value({.max_loads = 2});
  manual_event entered, release;
  std::shared_ptr<const std::string> first;
  std::jthread worker([&] {
    first = value.try_get_or_load("a", 1, [&] {
      entered.open();
      release.wait();
      return text("a");
    });
  });
  scope_exit cleanup([&] { release.open(); });
  DOBA_EXPECT(entered.wait_for());
  bool called = false;
  DOBA_EXPECT(!value.try_get_or_load("a", 1, [&] {
    called = true;
    return text("wrong");
  }));
  DOBA_EXPECT(!called);
  DOBA_EXPECT(value.try_get_or_load("b", 1, [] { return text("b"); }) !=
              nullptr);
  release.open();
  worker.join();
  DOBA_EXPECT_EQUAL(value.find("a"), first);
}

// +===========================================================================+
// | [>] load slots after invalidation                          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("cache invalidation keeps obsolete loaders in the load limit") {
  for (bool clear : {false, true}) {
    cache<std::string> value({.max_loads = 1});
    manual_event entered, release;
    std::shared_ptr<const std::string> old;
    std::jthread worker([&] {
      old = value.try_get_or_load("a", 1, [&] {
        entered.open();
        release.wait();
        return text("old");
      });
    });
    scope_exit cleanup([&] { release.open(); });
    DOBA_EXPECT(entered.wait_for());
    if (clear) {
      value.clear();
    } else {
      value.erase("a");
    }
    bool called = false;
    DOBA_EXPECT(!value.try_get_or_load("b", 1, [&] {
      called = true;
      return text("b");
    }));
    DOBA_EXPECT(!called);
    release.open();
    worker.join();
    DOBA_EXPECT(!old);
    DOBA_EXPECT(!value.find("a"));
    DOBA_EXPECT(value.try_get_or_load("b", 1, [] { return text("b"); }) !=
                nullptr);
  }
}

// +===========================================================================+
// | [>] obsolete and replacement loaders                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("cache obsolete completion cannot remove or overwrite a newer load") {
  for (bool clear : {false, true}) {
    for (bool fail : {false, true}) {
      cache<std::string> value({.max_loads = 2});
      manual_event old_entered, old_release, new_entered, new_release;
      std::shared_ptr<const std::string> old, fresh;
      std::atomic<bool> failed{false};
      std::jthread first([&] {
        try {
          old = value.try_get_or_load("file", 3, [&] {
            old_entered.open();
            old_release.wait();
            if (fail) throw std::runtime_error("old load failed");
            return text("old");
          });
        } catch (const std::runtime_error&) {
          failed = true;
        }
      });
      std::jthread second;
      scope_exit cleanup([&] {
        old_release.open();
        new_release.open();
      });
      DOBA_EXPECT(old_entered.wait_for());
      if (clear) {
        value.clear();
      } else {
        value.erase("file");
      }
      second = std::jthread([&] {
        fresh = value.try_get_or_load("file", 3, [&] {
          new_entered.open();
          new_release.wait();
          return text("new");
        });
      });
      DOBA_EXPECT(new_entered.wait_for());
      old_release.open();
      first.join();
      DOBA_EXPECT(!old);
      DOBA_EXPECT_EQUAL(failed.load(), fail);
      // The old load frees its slot and leaves the new one alone.
      DOBA_EXPECT(value.try_get_or_load("probe", 1, [] { return text("probe"); }) != nullptr);
      bool called = false;
      DOBA_EXPECT(!value.try_get_or_load("file", 1, [&] {
        called = true;
        return text("wrong");
      }));
      DOBA_EXPECT(!called);
      new_release.open();
      second.join();
      DOBA_EXPECT_EQUAL(value.find("file"), fresh);
      DOBA_EXPECT_EQUAL(*fresh, "new");
    }
  }
}

// +===========================================================================+
// | [>] obsolete completion after publication                  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST(
    "cache obsolete completion preserves an already published replacement") {
  cache<std::string> value;
  manual_event entered, release;
  std::shared_ptr<const std::string> old;
  std::jthread worker([&] {
    old = value.try_get_or_load("file", 3, [&] {
      entered.open();
      release.wait();
      return text("old");
    });
  });
  scope_exit cleanup([&] { release.open(); });
  DOBA_EXPECT(entered.wait_for());
  value.erase("file");
  auto fresh = value.try_get_or_load("file", 3, [] { return text("new"); });
  release.open();
  worker.join();
  DOBA_EXPECT(!old);
  DOBA_EXPECT_EQUAL(value.find("file"), fresh);
}

// +===========================================================================+
// | [>] conditional invalidation                               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("cache conditional erase preserves newer entries and active loads") {
  cache<std::string> value;
  auto old = value.try_get_or_load("file", 3, [] { return text("old"); });
  DOBA_EXPECT(value.erase("file", old));
  manual_event entered, release;
  std::shared_ptr<const std::string> fresh;
  std::jthread worker([&] {
    fresh = value.try_get_or_load("file", 3, [&] {
      entered.open();
      release.wait();
      return text("new");
    });
  });
  scope_exit cleanup([&] { release.open(); });
  DOBA_EXPECT(entered.wait_for());
  DOBA_EXPECT(!value.erase("file", old));
  release.open();
  worker.join();
  DOBA_EXPECT(!value.erase("file", old));
  DOBA_EXPECT_EQUAL(value.find("file"), fresh);
  DOBA_EXPECT_EQUAL(*old, "old");
  DOBA_EXPECT(value.erase("file", fresh));
}

// +===========================================================================+
// | [>] reader ownership after cache destruction               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("shared readers retain evicted values after cache destruction") {
  reader first, second;
  std::weak_ptr<const std::string> weak;
  {
    cache<std::string> value;
    auto data = value.try_get_or_load("file", 4, [] { return text("data"); });
    weak = data;
    const auto bytes = std::as_bytes(std::span(data->data(), data->size()));
    first = reader::shared(data, bytes);
    second = reader::shared(data, bytes.subspan(1, 2));
    value.clear();
  }
  DOBA_EXPECT(!weak.expired());
  reader moved(std::move(first));
  std::string a, b;
  moved.read_all(a);
  second.read_all(b);
  DOBA_EXPECT_EQUAL(a, "data");
  DOBA_EXPECT_EQUAL(b, "at");
  moved = reader{};
  second = reader{};
  DOBA_EXPECT(weak.expired());
}

// +===========================================================================+
// | [>] concurrent publication bounds                          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("cache concurrent publication respects resident and loader bounds") {
  cache<int> value({.max_entries = 3, .max_cost = 5, .max_loads = 4});
  std::atomic<bool> good{true};
  std::atomic<unsigned> active{0};
  std::vector<std::jthread> workers;
  for (int thread = 0; thread < 8; ++thread)
    workers.emplace_back([&, thread] {
      const auto key = std::to_string(thread);
      for (int i = 0; i < 100; ++i) {
        auto result = value.try_get_or_load(key, 2, [&] {
          if (active.fetch_add(1) >= 4) good = false;
          std::this_thread::yield();
          active.fetch_sub(1);
          return std::make_shared<const int>(thread);
        });
        if (result && *result != thread) good = false;
        if (i % 7 == 0) value.erase(key);
        if (i % 29 == 0) value.clear();
      }
    });
  workers.clear();
  DOBA_EXPECT(good.load());
  value.clear();
  for (int key = 0; key < 8; ++key)
    DOBA_EXPECT(!value.find(std::to_string(key)));
  DOBA_EXPECT(value.try_get_or_load("after", 2, [] {
    return std::make_shared<const int>(42);
  }) != nullptr);

  // Finish eight loads together. Only two values should fit.
  cache<int> bounded({.max_entries = 3, .max_cost = 5, .max_loads = 8});
  manual_event entered, release;
  std::atomic<unsigned> arrivals{0};
  scope_exit cleanup([&] { release.open(); workers.clear(); });
  for (int key = 0; key < 8; ++key)
    workers.emplace_back([&, key] {
      bounded.try_get_or_load(std::to_string(key), 2, [&] {
        if (arrivals.fetch_add(1) + 1 == 8) entered.open();
        release.wait();
        return std::make_shared<const int>(key);
      });
    });
  DOBA_EXPECT(entered.wait_for());
  release.open();
  workers.clear();
  unsigned residents = 0;
  for (int key = 0; key < 8; ++key)
    if (bounded.find(std::to_string(key))) ++residents;
  DOBA_EXPECT_EQUAL(residents, 2);
}

// +===========================================================================+
// | [>] concurrent hits and invalidation                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("cache concurrent hits retain valid values across invalidation") {
  cache<std::string> value;
  auto pinned = value.try_get_or_load("file", 4, [] { return text("data"); });
  manual_event start;
  std::atomic<bool> good{true};
  std::vector<std::jthread> workers;
  for (int i = 0; i < 8; ++i)
    workers.emplace_back([&] {
      start.wait();
      for (int j = 0; j < 1000; ++j) {
        auto hit = value.find("file");
        if (hit && hit != pinned) good = false;
      }
    });
  start.open();
  value.erase("file");
  workers.clear();
  DOBA_EXPECT(good.load());
  DOBA_EXPECT(!value.find("file"));
  DOBA_EXPECT_EQUAL(*pinned, "data");
}

// +===========================================================================+
// | [>] recursive load bypass                                  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("cache recursive same key load bypasses instead of waiting") {
  cache<std::string> value;
  bool called = false;
  auto result = value.try_get_or_load("file", 1, [&] {
    auto nested = value.try_get_or_load("file", 1, [&] {
      called = true;
      return text("wrong");
    });
    return nested ? nested : text("ok");
  });
  DOBA_EXPECT(!called);
  DOBA_EXPECT_EQUAL(*result, "ok");
}
