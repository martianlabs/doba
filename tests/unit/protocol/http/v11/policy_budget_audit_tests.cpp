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
#include <memory>
#include <span>
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
using common::sleep_for;
using common::task;
using common::yield;
using namespace std::chrono_literals;
}  // namespace

DOBA_TEST("pending request credit uses exact limits and resumes admission") {
  for (std::size_t maximum : {1U, 2U, 3U, 4U}) {
    router routes;
    int calls = 0;
    routes.add("GET", "/", [&](const request&, response& res) -> task<> {
      ++calls;
      co_await yield();
      res.ok_200().set_body("ok");
    });
    policies limits;
    limits.max_pending_requests = maximum;
    probe current(routes, limits);
    std::string batch;
    for (std::size_t i = 0; i < maximum + 1; ++i) batch += probe::get("/");
    current.receive(batch);
    DOBA_EXPECT_EQUAL(calls, static_cast<int>(maximum));
    DOBA_EXPECT_EQUAL(current.closes, 0);
    current.turn();
    DOBA_EXPECT_EQUAL(calls, static_cast<int>(maximum + 1));
    DOBA_EXPECT_EQUAL(current.outputs.size(), maximum);
    current.turn();
    DOBA_EXPECT_EQUAL(current.outputs.size(), maximum + 1);
    DOBA_EXPECT_EQUAL(current.closes, 0);
  }
}

DOBA_TEST(
    "pending sources use request slots without charging logical body size") {
  router routes;
  auto bytes = std::make_shared<const std::string>(1024 * 1024, 's');
  routes.add("GET", "/source", [&](const request&, response& res) {
    res.ok_200().set_body(
        common::reader::shared(bytes, std::as_bytes(std::span(*bytes))),
        bytes->size());
  });
  policies limits;
  limits.max_pending_requests = 1;
  limits.max_pending_response_bytes = 1;
  probe current(routes, limits);
  current.hold_source = true;
  current.receive(probe::get("/source") + probe::get("/source"));
  DOBA_EXPECT_EQUAL(current.sources, 1);
  DOBA_EXPECT_EQUAL(current.closes, 0);
  current.drain();
  DOBA_EXPECT_EQUAL(current.sources, 2);
  current.drain();
  DOBA_EXPECT_EQUAL(current.closes, 0);
  DOBA_EXPECT(!current.needs.turn_after_output_drained);
}

// +===========================================================================+
// | [>] queued source headers retain the pending byte limit     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("queued source headers retain the pending byte limit") {
  router routes;
  auto bytes = std::make_shared<const std::string>(1024 * 1024, 's');
  routes.add("GET", "/source", [&](const request&, response& res) {
    res.ok_200().set_body(
        common::reader::shared(bytes, std::as_bytes(std::span(*bytes))),
        bytes->size());
  });
  policies limits;
  limits.max_pending_requests = 2;
  limits.max_pending_response_bytes = 1;
  probe current(routes, limits);
  current.hold_source = true;
  current.receive(probe::get("/source") + probe::get("/source"));
  DOBA_EXPECT_EQUAL(current.sources, 1);
  DOBA_EXPECT_EQUAL(current.closes, 1);
  current.drain();
  DOBA_EXPECT_EQUAL(current.sources, 1);
  DOBA_EXPECT_EQUAL(current.closes, 1);
}

DOBA_TEST(
    "pending byte overflow cancels all yielded frames without extra sends") {
  router routes;
  auto token = std::make_shared<int>(0);
  routes.add("GET", "/wait", [&](const request&, response&) -> task<> {
    auto retained = token;
    co_await yield();
  });
  routes.add("GET", "/large", [](const request&, response& res) {
    res.ok_200().set_body("larger than the pending byte cap");
  });
  policies limits;
  limits.max_pending_response_bytes = 1;
  probe current(routes, limits);
  current.receive(probe::get("/wait") + probe::get("/large"));
  DOBA_EXPECT_EQUAL(current.closes, 1);
  DOBA_EXPECT(current.outputs.empty());
  DOBA_EXPECT_EQUAL(token.use_count(), 1);
  current.turn();
  current.receive(probe::get("/large"));
  DOBA_EXPECT_EQUAL(current.closes, 1);
  DOBA_EXPECT(current.outputs.empty());
}
