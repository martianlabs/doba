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
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "engine_probe.h"
#include "test_helper.h"

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] operating_limits_tests                                 ( test-suite ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
namespace {
using namespace martianlabs::doba;
using probe = tests::unit::engine_probe;
using request = probe::request;
using response = probe::response;
using router = probe::router;
using policies = probe::policies;
using protocol::http::v11::decoder;
using protocol::deserialization_status;
using clock_type = probe::clock;
using common::task;
using common::yield;
using common::sleep_for;
using namespace std::chrono_literals;
}  // namespace

DOBA_TEST(
    "decoder bounds huge incomplete heads and accepts exact complete heads") {
  policies p;
  p.max_request_head_size = 64;
  for (std::size_t size : {63u, 64u, 65u, 1024u * 1024u}) {
    decoder<request, response> value(p);
    std::string prefix = "GET / HTTP/1.1\r\nHost: a\r\nX: ";
    std::string wire =
        prefix + std::string(size - prefix.size() - 4, 'x') + "\r\n\r\n";
    std::string_view input(wire);
    auto result = value.deserialize(input);
    DOBA_EXPECT_EQUAL(result.code,
                      size <= 64 ? deserialization_status::kSucceeded
                                 : deserialization_status::kInvalidSource);
    if (size > 64) {
      DOBA_EXPECT(result.response.has_value());
      auto output = result.response->serialize();
      DOBA_EXPECT(output.head.starts_with("HTTP/1.1 431"));
    }
  }
  decoder<request, response> incomplete(p);
  std::string wire =
      "GET / HTTP/1.1\r\nHost: a\r\nX: " + std::string(1000000, 'x');
  std::string_view input(wire.data(), 20);
  DOBA_EXPECT_EQUAL(incomplete.deserialize(input).code,
                    deserialization_status::kMoreBytesNeeded);
  input = std::string_view(wire).substr(20);
  auto result = incomplete.deserialize(input);
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
  DOBA_EXPECT(result.response->serialize().head.starts_with("HTTP/1.1 431"));
}

DOBA_TEST("pending HTTP bytes enforce exact boundaries before copying") {
  // Measure the exact wire size using the same fixed headers.
  router routes;
  routes.add("GET", "/wait", [](const request&, response&) -> task<> {
    co_await sleep_for(1h);
  });
  routes.add("GET", "/next", [](const request&, response& res) {
    res.ok_200().set_header("Date", "fixed").set_body("payload");
  });
  probe measured(routes);
  measured.receive(probe::get("/next"));
  DOBA_EXPECT_EQUAL(measured.outputs.size(), 1U);
  const auto size = measured.outputs[0].size();
  for (auto limit : {size - 1, size, size + 1}) {
    policies limits;
    limits.max_pending_response_bytes = limit;
    probe current(routes, limits);
    current.receive(probe::get("/wait") + probe::get("/next"));
    DOBA_EXPECT_EQUAL(current.closes, limit < size ? 1 : 0);
    DOBA_EXPECT(current.outputs.empty());
    current.receive(probe::get("/next"));
    if (limit < size) DOBA_EXPECT_EQUAL(current.closes, 1);
  }
}

DOBA_TEST(
    "deferred raw input enforces exact boundaries while the window is full") {
  for (std::size_t bytes : {7U, 8U, 9U}) {
    router routes;
    int calls = 0;
    routes.add("GET", "/", [&](const request&, response&) -> task<> {
      ++calls;
      co_await sleep_for(1h);
    });
    policies limits;
    limits.max_pending_requests = 1;
    limits.max_deferred_input_size = 8;
    probe current(routes, limits);
    current.receive(probe::get("/"));
    const auto due = current.needs.turn_at;
    current.receive(std::string(bytes, 'x'));
    DOBA_EXPECT_EQUAL(calls, 1);
    DOBA_EXPECT_EQUAL(current.closes, bytes > 8 ? 1 : 0);
    if (bytes <= 8) {
      DOBA_EXPECT_EQUAL(current.needs.turn_at, due);
      DOBA_EXPECT(!current.needs.turn_requested);
    }
  }
}

DOBA_TEST(
    "HTTP processing limits reject zero instead of enabling unlimited work") {
  router routes;
  for (int field = 0; field < 4; ++field) {
    policies limits;
    if (field == 0) limits.max_pending_requests = 0;
    if (field == 1) limits.max_pending_response_bytes = 0;
    if (field == 2) limits.max_deferred_input_size = 0;
    if (field == 3) limits.max_task_steps_per_turn = 0;
    bool rejected = false;
    try {
      probe current(routes, limits);
    } catch (const std::invalid_argument&) {
      rejected = true;
    }
    DOBA_EXPECT(rejected);
  }
}
