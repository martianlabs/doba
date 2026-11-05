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

#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <span>
#include <thread>
#include <vector>

#include "protocol/http/v11/policies.h"
#include "tcp_policy_engine.h"
#include "test_helper.h"
#include "transport/server/connection_reservation.h"
#include "transport/server/send_state.h"
#include "transport/server/tcp.h"

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] operating_policies_tests                               ( test-suite ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
namespace {
using namespace martianlabs::doba::transport::server;
using namespace martianlabs::doba::transport::server::detail;
using namespace std::chrono_literals;
} // namespace

DOBA_TEST(
    "operating policies have finite agreed defaults") {
  tcp_policies p;
  DOBA_EXPECT_EQUAL(p.max_connections, 65535);
  DOBA_EXPECT_EQUAL(p.max_buffered_send_bytes, 1024 * 1024);
  martianlabs::doba::protocol::http::v11::policies http;
  DOBA_EXPECT_EQUAL(http.max_pending_requests, 128);
  DOBA_EXPECT_EQUAL(http.max_pending_response_bytes, 1024 * 1024);
  auto factory = [] { return martianlabs::doba::tests::unit::policy_engine{}; };
  p.max_connections = 0;
  tcp<decltype(factory()), decltype(factory)> unlimited(p, factory);
}

DOBA_TEST("connection reservations move release and unwind exactly once") {
  auto budget = std::make_shared<connection_budget>(2);
  {
    auto one = connection_reservation::acquire(budget);
    DOBA_EXPECT(one.has_value());
    try {
      auto two = connection_reservation::acquire(budget);
      DOBA_EXPECT(two.has_value());
      DOBA_EXPECT(!connection_reservation::acquire(budget));
      auto moved = std::move(*two);
      DOBA_EXPECT_EQUAL(budget->used.load(), 2);
      throw 1;
    } catch (int) {
    }
    DOBA_EXPECT_EQUAL(budget->used.load(), 1);
    auto two = connection_reservation::acquire(budget);
    *one = std::move(*two);
    DOBA_EXPECT_EQUAL(budget->used.load(), 1);
  }
  DOBA_EXPECT_EQUAL(budget->used.load(), 0);
  auto unlimited = std::make_shared<connection_budget>(0);
  std::vector<connection_reservation> leases;
  for (int i = 0; i < 128; ++i)
    leases.push_back(std::move(*connection_reservation::acquire(unlimited)));
  DOBA_EXPECT_EQUAL(unlimited->used.load(), 128);
  leases.clear();
  DOBA_EXPECT_EQUAL(unlimited->used.load(), 0);
}

DOBA_TEST("connection admission never exceeds a concurrent global maximum") {
  auto budget = std::make_shared<connection_budget>(3);
  std::atomic<bool> exceeded{false};
  std::vector<std::thread> threads;
  for (int i = 0; i < 8; ++i)
    threads.emplace_back([&] {
      for (int n = 0; n < 2000; ++n) {
        auto lease = connection_reservation::acquire(budget);
        if (budget->used.load() > 3)
          exceeded = true;
        if (lease)
          std::this_thread::yield();
      }
    });
  for (auto &thread : threads)
    thread.join();
  DOBA_EXPECT(!exceeded.load());
  DOBA_EXPECT_EQUAL(budget->used.load(), 0);
}


DOBA_TEST("source blocks use the send capacity after their prefix drains") {
  const std::string bytes = "abcdefgh";
  send_state output(8);
  auto source = std::make_unique<martianlabs::doba::common::reader>(
      martianlabs::doba::common::reader::borrowed(
          std::as_bytes(std::span(bytes))));
  DOBA_EXPECT(output.push("1234567", {}, std::move(source)));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT(output.consume(7));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(output.buffer.size(), 8);
  DOBA_EXPECT(!output.push("x", {}, nullptr));
  DOBA_EXPECT(output.consume(7));
  DOBA_EXPECT(!output.push("x", {}, nullptr));
  DOBA_EXPECT(output.consume(1));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT(output.push("12345678", {}, nullptr));
}

DOBA_TEST("one terminal source retains ownership until its last send drains") {
  auto owner = std::make_shared<const std::string>("abcdef");
  auto source = [&] {
    return std::make_unique<martianlabs::doba::common::reader>(
        martianlabs::doba::common::reader::shared(
            owner, std::as_bytes(std::span(*owner))));
  };
  send_state output(8192);
  DOBA_EXPECT(output.push({}, {}, source()));
  DOBA_EXPECT(!output.push({}, {}, source()));
  DOBA_EXPECT_EQUAL(owner.use_count(), 2);
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT(output.consume(5));
  DOBA_EXPECT(!output.push({}, {}, source()));
  DOBA_EXPECT_EQUAL(owner.use_count(), 2);
  DOBA_EXPECT(output.consume(1));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT_EQUAL(owner.use_count(), 1);
  DOBA_EXPECT(output.push({}, {}, source()));
  output.clear();
  DOBA_EXPECT_EQUAL(owner.use_count(), 1);
}

DOBA_TEST("partial sends retain the active batch in the hard send limit") {
  send_state output(8);
  DOBA_EXPECT(output.push("12345678", {}, nullptr));
  DOBA_EXPECT(output.fill());
  DOBA_EXPECT(output.consume(7));
  DOBA_EXPECT(!output.push("x", {}, nullptr));
  DOBA_EXPECT(output.consume(1));
  DOBA_EXPECT(output.push("12345678", {}, nullptr));
}
