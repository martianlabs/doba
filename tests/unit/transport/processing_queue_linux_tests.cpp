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

#include <chrono>
#include <cstdint>

#include "test_helper.h"
#include "transport/tcp_linux_processing_queue.h"

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] processing_queue_linux_tests                           ( test-suite ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
namespace {
using queue_type = martianlabs::doba::transport::detail::
    linux_processing_queue;
using result_type = martianlabs::doba::protocol::processing_result;
using clock_type = queue_type::clock;
const auto origin = clock_type::time_point{};
using namespace std::chrono_literals;
}  // namespace

DOBA_TEST(
    "Linux turn snapshots coalesce readiness and preserve future timing") {
  queue_type queue;
  const auto due = origin + 10s;
  for (int i = 0; i < 8; ++i) {
    queue.reconcile(3, {true, due, true}, false, origin + 1s);
  }
  DOBA_EXPECT_EQUAL(queue.ready_count(), 1U);
  DOBA_EXPECT_EQUAL(queue.next_deadline(), due);
  DOBA_EXPECT_EQUAL(queue.take_ready(), 3U);
  DOBA_EXPECT(!queue.take_ready());
  DOBA_EXPECT_EQUAL(queue.next_deadline(), due);
  queue.reconcile(3, {false, due, true}, false, origin + 9s);
  DOBA_EXPECT_EQUAL(queue.next_deadline(), due);
  DOBA_EXPECT_EQUAL(queue.ready_count(), 0U);
  queue.promote(due - clock_type::duration(1));
  DOBA_EXPECT_EQUAL(queue.ready_count(), 0U);
  queue.promote(due);
  DOBA_EXPECT_EQUAL(queue.ready_count(), 1U);
  DOBA_EXPECT_EQUAL(queue.take_ready(), 3U);
  DOBA_EXPECT(!queue.next_deadline());
}

DOBA_TEST(
    "Linux full snapshots withdraw readiness without restarting deadlines") {
  queue_type queue;
  const auto due = origin + 10s;
  queue.reconcile(3, {true, due, false}, true, origin);
  queue.reconcile(3, {false, due, false}, true, origin + 9s);
  DOBA_EXPECT_EQUAL(queue.ready_count(), 0U);
  DOBA_EXPECT_EQUAL(queue.next_deadline(), due);
  queue.reconcile(3, {}, true, origin + 9s);
  queue.promote(origin + 20s);
  DOBA_EXPECT(!queue.take_ready());
  DOBA_EXPECT(!queue.next_deadline());
}

DOBA_TEST("Linux output drain can be ready without a future socket edge") {
  queue_type queue;
  queue.reconcile(3, {false, {}, true}, false, origin);
  DOBA_EXPECT(!queue.take_ready());
  queue.reconcile(3, {false, {}, true}, true, origin);
  DOBA_EXPECT_EQUAL(queue.take_ready(), 3U);
  DOBA_EXPECT(!queue.take_ready());
  // A new request during delivery belongs to a later queued turn.
  queue.reconcile(3, {false, {}, true}, true, origin);
  DOBA_EXPECT_EQUAL(queue.ready_count(), 1U);
  queue.cancel(3);
  DOBA_EXPECT(!queue.take_ready());
}

DOBA_TEST("Linux deadlines progress independently of busy output") {
  queue_type queue;
  const auto due = origin + 1s;
  queue.reconcile(3, {false, due, true}, false, origin);
  queue.promote(due);
  DOBA_EXPECT_EQUAL(queue.take_ready(), 3U);
  DOBA_EXPECT(!queue.next_deadline());
}

DOBA_TEST("Linux deadline replacement eagerly removes earlier registrations") {
  queue_type queue;
  queue.reconcile(3, {false, origin + 1s, false}, false, origin);
  queue.reconcile(3, {false, origin + 3s, false}, false, origin);
  queue.promote(origin + 2s);
  DOBA_EXPECT(!queue.take_ready());
  DOBA_EXPECT_EQUAL(queue.next_deadline(), origin + 3s);
  queue.reconcile(3, {false, origin + 2s, false}, false, origin);
  queue.promote(origin + 2s);
  DOBA_EXPECT_EQUAL(queue.take_ready(), 3U);
  DOBA_EXPECT(!queue.next_deadline());
}

DOBA_TEST("Linux equal deadlines cancel only their own connection identity") {
  queue_type queue;
  for (std::uint64_t i = 3; i <= 5; ++i) {
    queue.reconcile(i, {false, origin + 1s, false}, false, origin);
  }
  queue.cancel(4);
  queue.promote(origin + 1s);
  DOBA_EXPECT_EQUAL(queue.take_ready(), 3U);
  DOBA_EXPECT_EQUAL(queue.take_ready(), 5U);
  DOBA_EXPECT(!queue.take_ready());
}

DOBA_TEST("Linux cancellation removes ready and timed interests together") {
  queue_type queue;
  queue.reconcile(3, {true, origin + 1s, false}, false, origin);
  queue.cancel(3);
  queue.cancel(3);
  queue.reconcile(4, {true, origin + 2s, false}, false, origin);
  queue.promote(origin + 1s);
  DOBA_EXPECT_EQUAL(queue.take_ready(), 4U);
  DOBA_EXPECT_EQUAL(queue.next_deadline(), origin + 2s);
  queue.cancel(4);
  queue.promote(origin + 3s);
  DOBA_EXPECT(!queue.take_ready());
  DOBA_EXPECT(!queue.next_deadline());
}

DOBA_TEST("Linux ready connections are fair when a processed one requeues") {
  queue_type queue;
  for (std::uint64_t i = 3; i <= 5; ++i) {
    queue.reconcile(i, {true, {}, false}, false, origin);
  }
  const auto batch = queue.ready_count();
  for (std::size_t i = 0; i < batch; ++i) {
    const auto identity = queue.take_ready();
    DOBA_EXPECT(identity.has_value());
    DOBA_EXPECT_EQUAL(*identity, i + 3);
    queue.reconcile(*identity, {true, {}, false}, false, origin);
  }
  DOBA_EXPECT_EQUAL(queue.ready_count(), 3U);
  DOBA_EXPECT_EQUAL(queue.take_ready(), 3U);
}

DOBA_TEST("Linux bulk cancellation and shutdown leave no registrations") {
  queue_type queue;
  for (std::uint64_t i = 3; i < 503; ++i) {
    queue.reconcile(i, {true, origin + 1s, false}, false, origin);
  }
  for (std::uint64_t i = 3; i < 503; i += 2) queue.cancel(i);
  DOBA_EXPECT_EQUAL(queue.ready_count(), 250U);
  queue.clear();
  queue.promote(origin + 2s);
  DOBA_EXPECT(!queue.take_ready());
  DOBA_EXPECT(!queue.next_deadline());
  queue.reconcile(504, {true, {}, false}, false, origin);
  DOBA_EXPECT_EQUAL(queue.take_ready(), 504U);
}

DOBA_TEST("Linux deadline promotion is bounded without losing overdue turns") {
  queue_type queue;
  for (std::uint64_t i = 3; i < 68; ++i) {
    queue.reconcile(i, {false, origin + 1s, false}, false, origin);
  }
  queue.promote(origin + 1s);
  DOBA_EXPECT_EQUAL(queue.ready_count(), 64U);
  DOBA_EXPECT_EQUAL(queue.next_deadline(), origin + 1s);
  for (std::uint64_t i = 3; i < 67; ++i) {
    DOBA_EXPECT_EQUAL(queue.take_ready(), i);
  }
  queue.promote(origin + 1s);
  DOBA_EXPECT_EQUAL(queue.take_ready(), 67U);
  DOBA_EXPECT(!queue.next_deadline());
}
