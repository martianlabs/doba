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
#include <functional>
#include <limits>
#include <stdexcept>
#include <string_view>

#include "protocol/contracts.h"
#include "tcp_policy_engine.h"
#include "test_helper.h"
#include "transport/tcp.h"

namespace {
namespace tr = martianlabs::doba::transport;

using martianlabs::doba::tests::unit::policy_engine;
static_assert(martianlabs::doba::protocol::contracts::engine<policy_engine>);
}  // namespace

// +===========================================================================+
// | [>] tcp transport rejects invalid buffer policies           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tcp transport rejects invalid buffer policies") {
  auto factory = []() { return policy_engine{}; };
  auto rejects = [&](tr::tcp_policies configuration) {
    try {
      tr::tcp<policy_engine, decltype(factory)> transport(configuration,
                                                          factory);
    } catch (const std::runtime_error&) {
      return true;
    }
    return false;
  };
  tr::tcp_policies configuration;
  configuration.receive_buffer_capacity = 0;
  DOBA_EXPECT(rejects(configuration));
  configuration.receive_buffer_capacity =
      std::numeric_limits<std::size_t>::max();
  DOBA_EXPECT(rejects(configuration));
  configuration.receive_buffer_capacity = 1;
  configuration.max_buffered_send_bytes = 0;
  DOBA_EXPECT(rejects(configuration));
  configuration.max_buffered_send_bytes = 1;
  configuration.listen_backlog = -1;
  DOBA_EXPECT(rejects(configuration));
}

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] accepts_policies                                         ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
bool accepts_policies(tr::tcp_policies policies) {
  auto factory = [] { return policy_engine{}; };
  try {
    tr::tcp<policy_engine, decltype(factory)> transport(policies, factory);
    return true;
  } catch (const std::runtime_error&) {
    return false;
  }
}
}  // namespace

// +===========================================================================+
// | [>] declared TCP defaults                                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("TCP policies retain all agreed defaults") {
  const tr::tcp_policies policies;
  DOBA_EXPECT_EQUAL(policies.kDefaultReceiveBufferCapacity, 4096);
  DOBA_EXPECT_EQUAL(policies.receive_buffer_capacity, 4096);
  DOBA_EXPECT_EQUAL(policies.worker_count, 0);
  DOBA_EXPECT_EQUAL(policies.listen_backlog, 0);
  DOBA_EXPECT_EQUAL(policies.max_buffered_send_bytes, 1024 * 1024);
  DOBA_EXPECT_EQUAL(policies.max_connections, 65535);
  DOBA_EXPECT(policies.ip.empty());
  DOBA_EXPECT(policies.port.empty());
}

// +===========================================================================+
// | [>] representable capacities without allocation             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("TCP validates capacity boundaries before allocating connections") {
  tr::tcp_policies policies;
  policies.receive_buffer_capacity = policies.max_buffered_send_bytes = 1;
  DOBA_EXPECT(accepts_policies(policies));
#ifdef _WIN32
  const auto rx_max =
      static_cast<std::size_t>(std::numeric_limits<ULONG>::max());
  const auto tx_max = std::numeric_limits<std::size_t>::max();
#else
  const auto rx_max =
      static_cast<std::size_t>(std::numeric_limits<ssize_t>::max());
  const auto tx_max = rx_max;
#endif
  policies.receive_buffer_capacity = rx_max;
  policies.max_buffered_send_bytes = tx_max;
  DOBA_EXPECT(accepts_policies(policies));
  if (rx_max < std::numeric_limits<std::size_t>::max()) {
    policies.receive_buffer_capacity = rx_max + 1;
    DOBA_EXPECT(!accepts_policies(policies));
  }
  policies.receive_buffer_capacity = 1;
  if (tx_max < std::numeric_limits<std::size_t>::max()) {
    policies.max_buffered_send_bytes = tx_max + 1;
    DOBA_EXPECT(!accepts_policies(policies));
  }
}

// +===========================================================================+
// | [>] zero and finite admission budgets                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("TCP validates worker and admission policy combinations") {
  tr::tcp_policies policies;
  policies.worker_count = 0;
  policies.max_connections = 1;
  DOBA_EXPECT(accepts_policies(policies));
  policies.worker_count = 2;
#ifdef _WIN32
  DOBA_EXPECT(accepts_policies(policies));
#else
  DOBA_EXPECT(!accepts_policies(policies));
#endif
  policies.max_connections = 2;
  DOBA_EXPECT(accepts_policies(policies));
  policies.max_connections = 0;
  DOBA_EXPECT(accepts_policies(policies));
  policies.max_connections = std::numeric_limits<std::size_t>::max();
  DOBA_EXPECT(accepts_policies(policies));
#ifdef _WIN32
  policies.worker_count = std::numeric_limits<DWORD>::max();
  DOBA_EXPECT(accepts_policies(policies));
  if (policies.worker_count < std::numeric_limits<std::size_t>::max()) {
    ++policies.worker_count;
    DOBA_EXPECT(!accepts_policies(policies));
  }
#endif
}

// +===========================================================================+
// | [>] backlog is a nonnegative kernel request                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("TCP validates backlog extremes without allocating connections") {
  tr::tcp_policies policies;
  for (const int backlog : {0, 1, std::numeric_limits<int>::max()}) {
    policies.listen_backlog = backlog;
    DOBA_EXPECT(accepts_policies(policies));
  }
  for (const int backlog : {-1, std::numeric_limits<int>::min()}) {
    policies.listen_backlog = backlog;
    DOBA_EXPECT(!accepts_policies(policies));
  }
}
