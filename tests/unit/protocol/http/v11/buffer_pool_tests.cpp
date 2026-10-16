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
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>

#include "protocol/http/v11/buffer_pool.h"
#include "protocol/http/v11/policies.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::detail::buffer_pool;
using martianlabs::doba::protocol::http::v11::policies;
}  // namespace

// +===========================================================================+
// | [>] provides configured request and response storage        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("buffer pool provides the configured request and response storage") {
  const policies configuration;
  auto value = buffer_pool::acquire(configuration);
  static_assert(!std::is_copy_constructible_v<decltype(value)>);
  DOBA_EXPECT(static_cast<bool>(value->request));
  DOBA_EXPECT(static_cast<bool>(value->response));
  DOBA_EXPECT_EQUAL(value->request_capacity,
                    configuration.request_head_buffer_capacity +
                        configuration.request_body_inline_capacity);
  DOBA_EXPECT_EQUAL(value->response_capacity,
                    configuration.response_head_buffer_capacity +
                        configuration.response_body_inline_capacity);
  value->request[0] = 'a';
  value->request[value->request_capacity - 1] = 'b';
  value->response[0] = 'c';
  value->response[value->response_capacity - 1] = 'd';
  DOBA_EXPECT_EQUAL(value->request[0], 'a');
  DOBA_EXPECT_EQUAL(value->request[value->request_capacity - 1], 'b');
  DOBA_EXPECT_EQUAL(value->response[0], 'c');
  DOBA_EXPECT_EQUAL(value->response[value->response_capacity - 1], 'd');
  buffer_pool::release(std::move(value));
  DOBA_EXPECT(!value);
}

// +===========================================================================+
// | [>] reuses the pair and both buffers                        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("buffer pool reuses the pair and both buffers") {
  const policies configuration;
  auto first = buffer_pool::acquire(configuration);
  const auto* pair = first.get();
  const auto* request = first->request.get();
  const auto* response = first->response.get();
  buffer_pool::release(std::move(first));
  auto second = buffer_pool::acquire(configuration);
  DOBA_EXPECT(second.get() == pair);
  DOBA_EXPECT(second->request.get() == request);
  DOBA_EXPECT(second->response.get() == response);
  buffer_pool::release(std::move(second));
}

// +===========================================================================+
// | [>] keeps simultaneous loans independent                    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("buffer pool keeps simultaneous loans independent") {
  const policies configuration;
  auto first = buffer_pool::acquire(configuration);
  auto second = buffer_pool::acquire(configuration);
  DOBA_EXPECT(first.get() != second.get());
  DOBA_EXPECT(first->request.get() != second->request.get());
  DOBA_EXPECT(first->response.get() != second->response.get());
  first->request[0] = 'a';
  first->response[0] = 'b';
  second->request[0] = 'c';
  second->response[0] = 'd';
  DOBA_EXPECT_EQUAL(first->request[0], 'a');
  DOBA_EXPECT_EQUAL(first->response[0], 'b');
  buffer_pool::release(std::move(first));
  auto third = buffer_pool::acquire(configuration);
  DOBA_EXPECT(third.get() != second.get());
  DOBA_EXPECT_EQUAL(second->request[0], 'c');
  DOBA_EXPECT_EQUAL(second->response[0], 'd');
  buffer_pool::release(std::move(second));
  buffer_pool::release(std::move(third));
}

// +===========================================================================+
// | [>] selects matching capacities within the free list        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("buffer pool selects matching capacities within the free list") {
  policies first_configuration;
  first_configuration.request_head_buffer_capacity = 64;
  first_configuration.request_body_inline_capacity = 32;
  first_configuration.response_head_buffer_capacity = 128;
  first_configuration.response_body_inline_capacity = 64;
  auto second_configuration = first_configuration;
  second_configuration.request_body_inline_capacity++;
  auto third_configuration = first_configuration;
  third_configuration.response_body_inline_capacity++;
  auto first = buffer_pool::acquire(first_configuration);
  auto second = buffer_pool::acquire(second_configuration);
  auto third = buffer_pool::acquire(third_configuration);
  const auto* first_pair = first.get();
  const auto* second_pair = second.get();
  const auto* third_pair = third.get();
  buffer_pool::release(std::move(first));
  buffer_pool::release(std::move(second));
  buffer_pool::release(std::move(third));
  second = buffer_pool::acquire(second_configuration);
  first = buffer_pool::acquire(first_configuration);
  third = buffer_pool::acquire(third_configuration);
  DOBA_EXPECT(first.get() == first_pair);
  DOBA_EXPECT(second.get() == second_pair);
  DOBA_EXPECT(third.get() == third_pair);
  buffer_pool::release(std::move(first));
  buffer_pool::release(std::move(second));
  buffer_pool::release(std::move(third));
}

// +===========================================================================+
// | [>] compatibility depends on total storage capacities       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("buffer pool compatibility depends on total storage capacities") {
  policies configuration;
  auto first = buffer_pool::acquire(configuration);
  const auto* pair = first.get();
  buffer_pool::release(std::move(first));
  configuration.request_head_buffer_capacity++;
  configuration.request_body_inline_capacity--;
  configuration.response_head_buffer_capacity++;
  configuration.response_body_inline_capacity--;
  configuration.max_request_body_size = 1;
  auto second = buffer_pool::acquire(configuration);
  DOBA_EXPECT(second.get() == pair);
  buffer_pool::release(std::move(second));
}

// +===========================================================================+
// | [>] supports zero inline capacities and minimal heads       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("buffer pool supports zero inline capacities and minimal heads") {
  policies configuration;
  configuration.request_head_buffer_capacity = 1;
  configuration.request_body_inline_capacity = 0;
  configuration.response_head_buffer_capacity = 1;
  configuration.response_body_inline_capacity = 0;
  auto value = buffer_pool::acquire(configuration);
  DOBA_EXPECT_EQUAL(value->request_capacity, 1u);
  DOBA_EXPECT_EQUAL(value->response_capacity, 1u);
  value->request[0] = 'a';
  value->response[0] = 'b';
  DOBA_EXPECT_EQUAL(value->request[0], 'a');
  DOBA_EXPECT_EQUAL(value->response[0], 'b');
  buffer_pool::release(std::move(value));
}

// +===========================================================================+
// | [>] rejects zero head capacities                            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("buffer pool rejects zero head capacities") {
  for (const bool request : {false, true}) {
    policies configuration;
    if (request) {
      configuration.request_head_buffer_capacity = 0;
    } else {
      configuration.response_head_buffer_capacity = 0;
    }
    bool rejected = false;
    try {
      auto value = buffer_pool::acquire(configuration);
    } catch (const std::invalid_argument&) {
      rejected = true;
    }
    DOBA_EXPECT(rejected);
  }
}

// +===========================================================================+
// | [>] rejects overflowing capacities                          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("buffer pool rejects overflowing capacities") {
  for (const bool request : {false, true}) {
    policies configuration;
    if (request) {
      configuration.request_head_buffer_capacity =
          std::numeric_limits<std::size_t>::max();
      configuration.request_body_inline_capacity = 1;
    } else {
      configuration.response_head_buffer_capacity =
          std::numeric_limits<std::size_t>::max();
      configuration.response_body_inline_capacity = 1;
    }
    bool rejected = false;
    try {
      auto value = buffer_pool::acquire(configuration);
    } catch (const std::invalid_argument&) {
      rejected = true;
    }
    DOBA_EXPECT(rejected);
  }
}

// +===========================================================================+
// | [>] preserves available pairs after allocation failure      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("buffer pool preserves available pairs after allocation failure") {
  const policies configuration;
  for (const bool request : {false, true}) {
    auto first = buffer_pool::acquire(configuration);
    const auto* pair = first.get();
    buffer_pool::release(std::move(first));
    auto oversized = configuration;
    if (request) {
      oversized.request_head_buffer_capacity =
          std::numeric_limits<std::size_t>::max();
      oversized.request_body_inline_capacity = 0;
    } else {
      oversized.response_head_buffer_capacity =
          std::numeric_limits<std::size_t>::max();
      oversized.response_body_inline_capacity = 0;
    }
    bool failed = false;
    try {
      auto value = buffer_pool::acquire(oversized);
    } catch (const std::bad_alloc&) {
      failed = true;
    }
    DOBA_EXPECT(failed);
    auto second = buffer_pool::acquire(configuration);
    DOBA_EXPECT(second.get() == pair);
    buffer_pool::release(std::move(second));
  }
}

// +===========================================================================+
// | [>] isolates available pairs between threads                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("buffer pool isolates available pairs between threads") {
  const policies configuration;
  auto first = buffer_pool::acquire(configuration);
  const auto pair = reinterpret_cast<std::uintptr_t>(first.get());
  const auto request =
      reinterpret_cast<std::uintptr_t>(first->request.get());
  const auto response =
      reinterpret_cast<std::uintptr_t>(first->response.get());
  buffer_pool::release(std::move(first));
  bool isolated = false;
  bool reused = false;
  std::jthread worker([&]() {
    auto second = buffer_pool::acquire(configuration);
    const auto* worker_pair = second.get();
    const auto* worker_request = second->request.get();
    const auto* worker_response = second->response.get();
    isolated = reinterpret_cast<std::uintptr_t>(worker_pair) != pair &&
               reinterpret_cast<std::uintptr_t>(worker_request) != request &&
               reinterpret_cast<std::uintptr_t>(worker_response) != response;
    buffer_pool::release(std::move(second));
    auto third = buffer_pool::acquire(configuration);
    reused = third.get() == worker_pair &&
             third->request.get() == worker_request &&
             third->response.get() == worker_response;
    buffer_pool::release(std::move(third));
  });
  worker.join();
  DOBA_EXPECT(isolated);
  DOBA_EXPECT(reused);
  auto fourth = buffer_pool::acquire(configuration);
  DOBA_EXPECT(reinterpret_cast<std::uintptr_t>(fourth.get()) == pair);
  buffer_pool::release(std::move(fourth));
}
