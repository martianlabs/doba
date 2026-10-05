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

#ifndef martianlabs_doba_tests_integration_wait_count_h
#define martianlabs_doba_tests_integration_wait_count_h

#include <atomic>
#include <chrono>
#include <thread>

namespace martianlabs::doba::tests::integration {
inline bool wait_count(
    const std::atomic<int>& value, int expected,
    std::chrono::steady_clock::duration timeout = std::chrono::seconds(3)) {
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  while (value.load() < expected &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::yield();
  }
  return value.load() == expected;
}
}  // namespace martianlabs::doba::tests::integration

#endif
