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

#include <cstddef>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string_view>

#include "protocol/contracts.h"
#include "tcp_policy_engine.h"
#include "test_helper.h"
#include "transport/server/tcp.h"

namespace {
namespace tr = martianlabs::doba::transport::server;

using martianlabs::doba::tests::unit::policy_engine;
static_assert(!tr::contracts::execution_receiver<policy_engine>);
}  // namespace

// +===========================================================================+
// | [>] tcp transport rejects invalid buffer policies          ( test-case ) |
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
