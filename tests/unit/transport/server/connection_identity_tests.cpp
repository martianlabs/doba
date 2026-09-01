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
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

#include "test_helper.h"
#include "transport/server/connection_identity.h"

namespace {
using martianlabs::doba::transport::server::detail::connection_identity;
}  // namespace

// +===========================================================================+
// | [>] identities are monotonic                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("identities are monotonic") {
  connection_identity identity;
  DOBA_EXPECT_EQUAL(identity.acquire(), 1);
  DOBA_EXPECT_EQUAL(identity.acquire(), 2);
  DOBA_EXPECT_EQUAL(identity.acquire(), 3);
}
// +===========================================================================+
// | [>] concurrent identities are unique                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("concurrent identities are unique") {
  constexpr std::size_t kThreadCount = 4;
  constexpr std::size_t kKeysPerThread = 1024;
  constexpr std::size_t kKeyCount = kThreadCount * kKeysPerThread;
  connection_identity identity;
  std::vector<uint64_t> keys(kKeyCount);
  std::vector<std::thread> threads;
  threads.reserve(kThreadCount);
  for (std::size_t thread = 0; thread < kThreadCount; thread++) {
    threads.emplace_back([&identity, &keys, thread]() {
      std::size_t first = thread * kKeysPerThread;
      for (std::size_t i = 0; i < kKeysPerThread; i++) {
        keys[first + i] = identity.acquire();
      }
    });
  }
  for (auto& thread : threads) thread.join();
  std::sort(keys.begin(), keys.end());
  DOBA_EXPECT_EQUAL(keys.front(), 1);
  DOBA_EXPECT_EQUAL(keys.back(), kKeyCount);
  DOBA_EXPECT(std::adjacent_find(keys.begin(), keys.end()) == keys.end());
}
