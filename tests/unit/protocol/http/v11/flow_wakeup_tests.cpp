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

#include <atomic>
#include <barrier>
#include <thread>
#include "protocol/http/v11/engine_async.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::detail::engine_async;
}

DOBA_TEST(
    "HTTP completion wakes input only while ordered credit is pressured") {
  engine_async pending(1, {}, {}, 8);
  int wakes = 0;
  pending.set_on_input_ready([&] { ++wakes; });
  pending.set_on_send([](auto, auto, auto) {});
  pending.set_on_close([] {});
  DOBA_EXPECT(pending.reserve().has_value());
  pending.submit(0, {"ok", {}, nullptr}, true, false);
  DOBA_EXPECT_EQUAL(wakes, 0);
  DOBA_EXPECT(pending.reserve().has_value());
  DOBA_EXPECT(pending.pressured());
  pending.submit(1, {"ok", {}, nullptr}, true, false);
  DOBA_EXPECT_EQUAL(wakes, 1);
  DOBA_EXPECT(!pending.pressured());
  DOBA_EXPECT(pending.reserve().has_value());
  pending.submit(2, {"ok", {}, nullptr}, true, false);
  DOBA_EXPECT_EQUAL(wakes, 1);
}

DOBA_TEST("HTTP byte credit arms wakeups independently of request pressure") {
  engine_async pending(128, {}, {}, 8);
  int wakes = 0;
  pending.set_on_input_ready([&] { ++wakes; });
  pending.set_on_send([](auto, auto, auto) {});
  pending.set_on_close([] {});
  DOBA_EXPECT(pending.reserve().has_value());
  DOBA_EXPECT(pending.reserve().has_value());
  pending.submit(1, {"123456", {}, nullptr}, true, false);
  DOBA_EXPECT_EQUAL(wakes, 0);
  DOBA_EXPECT(pending.pressured());
  pending.submit(0, {"ok", {}, nullptr}, true, false);
  DOBA_EXPECT_EQUAL(wakes, 1);
  DOBA_EXPECT(!pending.pressured());
}

DOBA_TEST("HTTP subscribing cannot lose request or byte credit completions") {
  for (bool byte_credit : {false, true}) {
    engine_async pending(byte_credit ? 128 : 1, {}, {}, byte_credit ? 8 : 0);
    std::atomic<int> wakes{0};
    std::barrier phase(2);
    constexpr std::size_t rounds = 2048;
    bool missed = false;
    pending.set_on_input_ready([&] { wakes.fetch_add(1); });
    pending.set_on_send([](auto, auto, auto) {});
    pending.set_on_close([] {});
    std::thread completing([&] {
      for (std::size_t i = 0; i < rounds; ++i) {
        phase.arrive_and_wait();
        // Byte mode drains a queued interim after advancing the predecessor.
        // There is no position increment after that byte-credit publication.
        pending.submit(byte_credit ? 2 * i : i, {"ok", {}, nullptr}, true,
                       false);
        phase.arrive_and_wait();
      }
    });
    for (std::size_t i = 0; i < rounds; ++i) {
      if (pending.pressured()) missed = true;
      wakes = 0;
      if (!pending.reserve()) missed = true;
      if (byte_credit) {
        if (!pending.reserve()) missed = true;
        pending.submit(2 * i + 1, {"123456", {}, nullptr}, false, false);
      }
      phase.arrive_and_wait();
      const bool paused = pending.pressured();
      phase.arrive_and_wait();
      if (paused && wakes.load() == 0) missed = true;
      if (byte_credit)
        pending.submit(2 * i + 1, {{}, {}, nullptr}, true, false);
    }
    completing.join();
    DOBA_EXPECT(!missed);
  }
}
