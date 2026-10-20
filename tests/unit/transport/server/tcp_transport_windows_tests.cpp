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
#include <limits>
#include <stdexcept>

#include "tcp_policy_engine.h"
#include "test_helper.h"
#include "transport/server/tcp.h"

namespace {
// +---------------------------------------------------------------------------+
// | [>] general                                                    ( usings ) |
// +---------------------------------------------------------------------------+
namespace tr = martianlabs::doba::transport::server;
using martianlabs::doba::tests::unit::policy_engine;
}  // namespace

// +===========================================================================+
// | [>] tcp transport rejects overflowing Windows worker counts ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tcp transport rejects overflowing Windows worker counts") {
  auto factory = [] { return policy_engine{}; };
  tr::tcp_policies configuration;
  configuration.worker_count = std::numeric_limits<std::size_t>::max();
  bool rejected = false;
  try {
    tr::tcp<policy_engine, decltype(factory)> transport(configuration, factory);
  } catch (const std::runtime_error&) {
    rejected = true;
  }
  DOBA_EXPECT(rejected);
}
