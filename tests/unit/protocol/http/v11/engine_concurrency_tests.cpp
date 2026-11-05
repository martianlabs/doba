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
#include <chrono>
#include <coroutine>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "engine_probe.h"
#include "test_helper.h"

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine_concurrency_tests                               ( test-suite ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
namespace {
using namespace martianlabs::doba;
using probe = tests::unit::engine_probe;
using request = probe::request;
using response = probe::response;
using router = probe::router;
using policies = probe::policies;
using clock_type = probe::clock;
using common::task;
using common::yield;
using common::sleep_for;
using namespace std::chrono_literals;
}  // namespace

DOBA_TEST(
    "same connection starts several independent waits before completion") {
  router routes;
  int started = 0, completed = 0;
  routes.add("GET", "/wait", [&](const request&, response& res) -> task<> {
    ++started;
    co_await sleep_for(5ms);
    ++completed;
    res.ok_200().set_body("done");
  });
  probe current(routes);
  current.receive(probe::get("/wait") + probe::get("/wait") +
                  probe::get("/wait"));
  const auto admitted = clock_type::now();
  DOBA_EXPECT_EQUAL(started, 3);
  DOBA_EXPECT_EQUAL(completed, 0);
  DOBA_EXPECT(current.outputs.empty());
  DOBA_EXPECT(current.needs.turn_at.has_value());
  const auto due = current.needs.turn_at;
  current.receive({});
  DOBA_EXPECT_EQUAL(current.needs.turn_at, due);
  // Every sleep was registered before this bound; no sequential-wait timing
  // assertion is needed to prove all three can finish in one eligible turn.
  std::this_thread::sleep_until(admitted + 5ms);
  current.turn();
  DOBA_EXPECT_EQUAL(completed, 3);
  DOBA_EXPECT_EQUAL(current.outputs.size(), 3U);
  DOBA_EXPECT_EQUAL(current.closes, 0);
}

DOBA_TEST(
    "later completions wait for earlier wire order without stalling sync") {
  router routes;
  std::vector<int> execution;
  routes.add("GET", "/first", [&](const request&, response& res) -> task<> {
    execution.push_back(1);
    co_await yield();
    co_await yield();
    execution.push_back(4);
    res.ok_200().set_body("first");
  });
  routes.add("GET", "/second", [&](const request&, response& res) -> task<> {
    execution.push_back(2);
    co_await yield();
    execution.push_back(3);
    res.ok_200().set_body("second");
  });
  int synchronous = 0;
  routes.add("GET", "/sync", [&](const request&, response& res) {
    ++synchronous;
    res.ok_200().set_body("sync");
  });
  probe current(routes);
  current.receive(probe::get("/first") + probe::get("/second") +
                  probe::get("/sync"));
  DOBA_EXPECT_EQUAL(synchronous, 1);
  DOBA_EXPECT_EQUAL(execution, (std::vector<int>{1, 2}));
  current.turn();
  DOBA_EXPECT_EQUAL(execution, (std::vector<int>{1, 2, 3}));
  DOBA_EXPECT(current.outputs.empty());
  current.turn();
  DOBA_EXPECT_EQUAL(execution, (std::vector<int>{1, 2, 3, 4}));
  DOBA_EXPECT_EQUAL(current.outputs.size(), 3U);
  DOBA_EXPECT(current.outputs[0].ends_with("first"));
  DOBA_EXPECT(current.outputs[1].ends_with("second"));
  DOBA_EXPECT(current.outputs[2].ends_with("sync"));
}

DOBA_TEST("yielded work preserves sibling deadline and other connections") {
  router routes;
  int sleepers = 0, fast = 0;
  routes.add("GET", "/sleep", [&](const request&, response&) -> task<> {
    ++sleepers;
    co_await sleep_for(1h);
  });
  routes.add("GET", "/yield", [](const request&, response& res) -> task<> {
    co_await yield();
    res.ok_200().set_body("yielded");
  });
  routes.add("GET", "/fast", [&](const request&, response& res) {
    ++fast;
    res.ok_200().set_body("fast");
  });
  probe current(routes), other(routes);
  current.receive(probe::get("/sleep"));
  const auto due = current.needs.turn_at;
  DOBA_EXPECT(due.has_value());
  current.receive(probe::get("/yield"));
  DOBA_EXPECT(current.needs.turn_requested);
  DOBA_EXPECT_EQUAL(current.needs.turn_at, due);
  current.turn();
  DOBA_EXPECT(!current.needs.turn_requested);
  DOBA_EXPECT_EQUAL(current.needs.turn_at, due);
  current.receive(probe::get("/fast"));
  other.receive(probe::get("/fast"));
  DOBA_EXPECT_EQUAL(fast, 2);
  DOBA_EXPECT_EQUAL(sleepers, 1);
  DOBA_EXPECT(current.outputs.empty());
  DOBA_EXPECT_EQUAL(other.outputs.size(), 1U);
  DOBA_EXPECT_EQUAL(current.needs.turn_at, due);
}

DOBA_TEST(
    "busy terminal source gates emission but allows eligible task progress") {
  router routes;
  const std::string large(32768, 's');
  int completed = 0;
  routes.add("GET", "/source", [&](const request&, response& res) {
    res.ok_200().set_body(large);
  });
  routes.add("GET", "/later", [&](const request&, response& res) -> task<> {
    co_await yield();
    ++completed;
    res.ok_200().set_body("later");
  });
  probe current(routes);
  current.hold_source = true;
  current.receive(probe::get("/source") + probe::get("/later"));
  DOBA_EXPECT(current.held != nullptr);
  DOBA_EXPECT(current.needs.turn_after_output_drained);
  DOBA_EXPECT(current.needs.turn_requested);
  current.turn(false);
  DOBA_EXPECT_EQUAL(completed, 1);
  DOBA_EXPECT_EQUAL(current.outputs.size(), 1U);
  DOBA_EXPECT(!current.needs.turn_requested);
  DOBA_EXPECT(current.needs.turn_after_output_drained);
  current.drain();
  DOBA_EXPECT_EQUAL(current.outputs.size(), 2U);
  DOBA_EXPECT(current.wire.ends_with(large + current.outputs[1]));
  DOBA_EXPECT(current.outputs[1].ends_with("later"));
  DOBA_EXPECT_EQUAL(current.closes, 0);
}

DOBA_TEST("busy output preserves both runnable work and future deadlines") {
  router routes;
  routes.add("GET", "/source", [](const request&, response& res) {
    res.ok_200().set_body(std::string(32768, 'x'));
  });
  routes.add("GET", "/sleep", [](const request&, response&) -> task<> {
    co_await sleep_for(1h);
  });
  routes.add("GET", "/yield", [](const request&, response& res) -> task<> {
    co_await yield();
    res.ok_200().set_body("done");
  });
  probe current(routes);
  current.hold_source = true;
  current.receive(probe::get("/source") + probe::get("/sleep") +
                  probe::get("/yield"));
  DOBA_EXPECT(current.needs.turn_requested);
  DOBA_EXPECT(current.needs.turn_after_output_drained);
  DOBA_EXPECT(current.needs.turn_at.has_value());
  const auto due = current.needs.turn_at;
  current.turn(false);
  DOBA_EXPECT(!current.needs.turn_requested);
  DOBA_EXPECT(current.needs.turn_after_output_drained);
  DOBA_EXPECT_EQUAL(current.needs.turn_at, due);
  current.drain();
  DOBA_EXPECT(!current.needs.turn_after_output_drained);
  DOBA_EXPECT_EQUAL(current.needs.turn_at, due);
}

DOBA_TEST("turn budget bounds starts and fairly resumes all yielded siblings") {
  for (std::size_t budget : {1U, 2U, 3U}) {
    router routes;
    std::array<int, 3> steps{};
    for (int i = 0; i < 3; ++i) {
      routes.add("GET", "/" + std::to_string(i),
                 [&, i](const request&, response& res) -> task<> {
        for (int j = 0; j < 4; ++j) {
          ++steps[i];
          co_await yield();
        }
        res.ok_200().set_body(std::to_string(i));
      });
    }
    policies limits;
    limits.max_task_steps_per_turn = budget;
    probe current(routes, limits);
    current.receive(probe::get("/0") + probe::get("/1") + probe::get("/2"));
    DOBA_EXPECT_EQUAL(steps[0] + steps[1] + steps[2],
                      static_cast<int>(budget));
    for (int turn = 0; turn < 32 && current.needs.turn_requested; ++turn) {
      const auto before = steps[0] + steps[1] + steps[2];
      current.turn();
      const auto advanced = steps[0] + steps[1] + steps[2] - before;
      DOBA_EXPECT(advanced <= static_cast<int>(budget));
    }
    DOBA_EXPECT_EQUAL(steps, (std::array<int, 3>{4, 4, 4}));
    DOBA_EXPECT_EQUAL(current.outputs.size(), 3U);
    for (int i = 0; i < 3; ++i) {
      DOBA_EXPECT(current.outputs[i].ends_with(std::to_string(i)));
    }
    DOBA_EXPECT(!current.needs.turn_requested);
  }
}

DOBA_TEST("ordered close cancels already started successors exactly once") {
  router routes;
  auto token = std::make_shared<int>(0);
  routes.add("GET", "/first", [](const request&, response& res) -> task<> {
    co_await yield();
    co_await yield();
    res.ok_200().set_body("first");
  });
  routes.add("GET", "/fail", [](const request&, response&) -> task<> {
    co_await yield();
    throw std::runtime_error("failure");
  });
  int started = 0, finished = 0;
  routes.add("GET", "/last", [&](const request&, response& res) -> task<> {
    auto retained = token;
    ++started;
    co_await yield();
    ++finished;
    res.ok_200().set_body("last");
  });
  probe current(routes);
  current.receive(probe::get("/first") + probe::get("/fail") +
                  probe::get("/last"));
  DOBA_EXPECT_EQUAL(started, 1);
  DOBA_EXPECT_EQUAL(token.use_count(), 2);
  current.turn();
  DOBA_EXPECT_EQUAL(finished, 0);
  DOBA_EXPECT_EQUAL(token.use_count(), 1);
  DOBA_EXPECT_EQUAL(current.closes, 0);
  DOBA_EXPECT(current.outputs.empty());
  current.turn();
  DOBA_EXPECT_EQUAL(current.closes, 1);
  DOBA_EXPECT_EQUAL(current.outputs.size(), 2U);
  DOBA_EXPECT(current.outputs[0].ends_with("first"));
  DOBA_EXPECT(current.outputs[1].starts_with("HTTP/1.1 500"));
}

DOBA_TEST("engine destruction cancels all pending frames and late turns stop") {
  router routes;
  auto token = std::make_shared<int>(0);
  int starts = 0, ends = 0;
  routes.add("GET", "/", [&](const request&, response&) -> task<> {
    auto retained = token;
    ++starts;
    co_await sleep_for(1h);
    ++ends;
  });
  {
    probe current(routes);
    current.receive(probe::get("/") + probe::get("/") + probe::get("/"));
    DOBA_EXPECT_EQUAL(starts, 3);
    DOBA_EXPECT_EQUAL(token.use_count(), 4);
  }
  DOBA_EXPECT_EQUAL(token.use_count(), 1);
  DOBA_EXPECT_EQUAL(ends, 0);
}

DOBA_TEST(
    "interim and final responses retain the same ordered request record") {
  router routes;
  routes.add("GET", "/first", [](const request&, response& res) -> task<> {
    co_await yield();
    co_await yield();
    res.ok_200().set_body("first");
  });
  routes.add("POST", "/upload", [](const request&, response& res) {
    res.ok_200().set_body("uploaded");
  });
  probe current(routes);
  current.receive(probe::get("/first") +
      "POST /upload HTTP/1.1\r\nHost: x\r\nExpect: 100-continue\r\n"
      "Content-Length: 4\r\n\r\n");
  current.receive("data");
  DOBA_EXPECT(current.outputs.empty());
  current.turn();
  DOBA_EXPECT(current.outputs.empty());
  current.turn();
  DOBA_EXPECT_EQUAL(current.outputs.size(), 3U);
  DOBA_EXPECT(current.outputs[0].ends_with("first"));
  DOBA_EXPECT(current.outputs[1].starts_with("HTTP/1.1 100 Continue"));
  DOBA_EXPECT(current.outputs[2].ends_with("uploaded"));
}

DOBA_TEST("sibling deadlines keep advancing during another handler resume") {
  router routes;
  clock_type::time_point release_at;
  int completed = 0;
  routes.add("GET", "/work", [&](const request&, response& res) -> task<> {
    co_await yield();
    // Simulate ordinary non-suspending handler work while a sibling sleeps.
    std::this_thread::sleep_until(release_at);
    res.ok_200().set_body("work");
  });
  routes.add("GET", "/sleep", [&](const request&, response& res) -> task<> {
    co_await sleep_for(1ms);
    ++completed;
    res.ok_200().set_body("sleep");
  });
  probe current(routes);
  current.receive(probe::get("/work") + probe::get("/sleep"));
  DOBA_EXPECT_EQUAL(completed, 0);
  release_at = clock_type::now() + 2ms;
  current.turn();
  DOBA_EXPECT_EQUAL(completed, 1);
  DOBA_EXPECT_EQUAL(current.outputs.size(), 2U);
  DOBA_EXPECT(!current.needs.turn_requested);
  DOBA_EXPECT(!current.needs.turn_at);
}

DOBA_TEST(
    "concurrent typed route arguments outlive RX reuse and sibling starts") {
  router routes;
  int started = 0;
  routes.add("GET", "/name/:name",
             [&](const request& req, response& res,
                 const std::string& name) -> task<> {
    ++started;
    const auto host = req.get_host();
    co_await yield();
    res.ok_200().set_body(name + ":" + std::string(host));
  });
  probe current(routes);
  auto bytes = probe::get("/name/alpha") + probe::get("/name/beta");
  current.receive(bytes);
  DOBA_EXPECT_EQUAL(started, 2);
  bytes.assign(bytes.size(), 'x');
  current.turn();
  DOBA_EXPECT_EQUAL(current.outputs.size(), 2U);
  DOBA_EXPECT(current.outputs[0].ends_with("alpha:local"));
  DOBA_EXPECT(current.outputs[1].ends_with("beta:local"));
}

DOBA_TEST("engine rejects unsupported suspension before publishing a handle") {
  router routes;
  routes.add("GET", "/bad", [](const request&, response&) -> task<> {
    co_await std::suspend_always{};
  });
  int later = 0;
  routes.add("GET", "/later", [&](const request&, response& res) {
    ++later;
    res.ok_200().set_body("later");
  });
  probe current(routes);
  current.receive(probe::get("/bad") + probe::get("/later"));
  DOBA_EXPECT_EQUAL(current.closes, 1);
  DOBA_EXPECT_EQUAL(later, 0);
  DOBA_EXPECT_EQUAL(current.outputs.size(), 1U);
  DOBA_EXPECT(current.outputs[0].starts_with("HTTP/1.1 500"));
  DOBA_EXPECT(!current.needs.turn_requested);
  current.turn();
  DOBA_EXPECT_EQUAL(current.closes, 1);
}
