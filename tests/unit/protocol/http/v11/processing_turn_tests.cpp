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

#include <array>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "engine_probe.h"
#include "test_helper.h"

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

DOBA_TEST("ready task interests survive repeated RX and serialized turns") {
  router routes;
  int steps = 0;
  routes.add("GET", "/", [&](const request&, response& res) -> task<> {
    ++steps;
    co_await yield();
    ++steps;
    co_await yield();
    ++steps;
    res.ok_200().set_body("done");
  });
  probe current(routes);
  current.receive(probe::get("/"));
  DOBA_EXPECT(current.needs.turn_requested);
  for (int i = 0; i < 3; ++i) current.receive({});
  DOBA_EXPECT_EQUAL(steps, 1);
  DOBA_EXPECT(current.needs.turn_requested);
  current.turn();
  DOBA_EXPECT_EQUAL(steps, 2);
  DOBA_EXPECT(current.needs.turn_requested);
  current.turn();
  DOBA_EXPECT_EQUAL(steps, 3);
  DOBA_EXPECT(!current.needs.turn_requested);
  DOBA_EXPECT_EQUAL(current.outputs.size(), 1U);
}

DOBA_TEST("full in-flight window does not spin for unadmitted raw input") {
  router routes;
  routes.add("GET", "/", [](const request&, response&) -> task<> {
    co_await sleep_for(1h);
  });
  policies limits;
  limits.max_pending_requests = 1;
  probe current(routes, limits);
  current.receive(probe::get("/") + probe::get("/"));
  DOBA_EXPECT(!current.needs.turn_requested);
  DOBA_EXPECT(current.needs.turn_at.has_value());
  const auto due = current.needs.turn_at;
  current.turn();
  DOBA_EXPECT(!current.needs.turn_requested);
  DOBA_EXPECT_EQUAL(current.needs.turn_at, due);
}
