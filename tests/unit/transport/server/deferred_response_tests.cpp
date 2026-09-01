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

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

#include "test_helper.h"
#include "transport/server/completion_mailbox.h"
#include "transport/server/deferred_response.h"

namespace {
using martianlabs::doba::protocol::serialization_result;
using martianlabs::doba::transport::server::detail::completion_mailbox;
using martianlabs::doba::transport::server::detail::
    deferred_response_context;

struct test_waker {
  bool operator()() const noexcept {
    (*calls)++;
    return succeeds;
  }

  std::size_t* calls;
  bool succeeds{true};
};

struct test_context {
  uint64_t reserve_response() {
    reservations++;
    return next_position++;
  }
  uint64_t get_connection_key() const { return connection_key; }

  uint64_t connection_key{41};
  uint64_t next_position{13};
  std::size_t reservations{0};
};

using test_mailbox = completion_mailbox<test_waker>;
using test_dispatch = deferred_response_context<test_context, test_mailbox>;
using test_sender = test_dispatch::sender;
}  // namespace

// +===========================================================================+
// | [>] response deferral is lazy and publishes once             ( test-case ) |
// +===========================================================================+
DOBA_TEST("response deferral is lazy and publishes once") {
  std::size_t wakes = 0;
  test_context context;
  test_mailbox mailbox{test_waker{&wakes}};
  test_dispatch dispatch{context, mailbox};

  DOBA_EXPECT(!dispatch.deferred());
  DOBA_EXPECT_EQUAL(context.reservations, 0);
  DOBA_EXPECT_EQUAL(wakes, 0);

  test_sender sender = dispatch.defer();
  DOBA_EXPECT(dispatch.deferred());
  DOBA_EXPECT(sender.valid());
  DOBA_EXPECT_EQUAL(context.reservations, 1);
  DOBA_EXPECT_EQUAL(wakes, 0);
  DOBA_EXPECT(!dispatch.defer().valid());
  DOBA_EXPECT_EQUAL(context.reservations, 1);

  auto response = std::make_unique<serialization_result>();
  response->prefix = "ready";
  DOBA_EXPECT(sender.complete(std::move(response)));
  DOBA_EXPECT(!sender.valid());
  DOBA_EXPECT_EQUAL(wakes, 1);
  DOBA_EXPECT(!sender.complete(std::make_unique<serialization_result>()));

  auto completions = mailbox.drain();
  DOBA_EXPECT_EQUAL(completions.size(), 1);
  DOBA_EXPECT_EQUAL(completions.front().connection_key, 41);
  DOBA_EXPECT_EQUAL(completions.front().position, 13);
  DOBA_EXPECT_EQUAL(completions.front().response->prefix, "ready");
}
// +===========================================================================+
// | [>] response sender transfers ownership and observes revocation ( test-case ) |
// +===========================================================================+
DOBA_TEST("response sender transfers ownership and observes revocation") {
  static_assert(!std::is_copy_constructible_v<test_sender>);
  static_assert(!std::is_copy_assignable_v<test_sender>);
  static_assert(std::is_move_constructible_v<test_sender>);
  static_assert(!std::is_move_assignable_v<test_sender>);

  std::size_t wakes = 0;
  test_context context;
  test_mailbox mailbox{test_waker{&wakes}};
  test_dispatch dispatch{context, mailbox};
  test_sender source = dispatch.defer();
  test_sender sender{std::move(source)};
  DOBA_EXPECT(!source.valid());
  DOBA_EXPECT(sender.valid());
  DOBA_EXPECT(!sender.complete(nullptr));
  DOBA_EXPECT(sender.valid());

  mailbox.close();
  DOBA_EXPECT(!sender.complete(std::make_unique<serialization_result>()));
  DOBA_EXPECT(!sender.valid());
  DOBA_EXPECT_EQUAL(wakes, 0);
}
