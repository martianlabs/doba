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
#include <cstdio>
#include <ctime>
#include <memory>
#include <mutex>
#include <thread>
#include "transport/server/operating_state.h"

using martianlabs::doba::transport::server::detail::connection_progress;
using namespace std::chrono_literals;

struct slot {
  connection_progress progress;
  std::mutex mutex;
};

int main() {
  std::atomic<bool> done{false};
  std::thread auxiliary([&] { done.wait(false); });
  constexpr std::size_t requests = 1000000;
  for (const std::size_t contexts : {4096u, 32000u}) {
    for (const std::size_t interval_ms : {50u, 100u, 250u, 500u, 1000u}) {
      for (int sample = 0; sample < 5; ++sample) {
        const auto start = std::clock();
        auto states = std::make_unique<slot[]>(contexts);
        const auto created = std::clock();
        std::size_t expired = 0, sweeps = 0;
        for (std::size_t i = 0; i < requests; ++i) {
          auto& state = states[(i * 17) % contexts];
          {
            std::lock_guard lock(state.mutex);
            state.progress.transferred(1);
          }
          // No socket work is simulated. Keep both completion boundaries
          // visible to the compiler, using the already required context lock.
          {
            std::lock_guard lock(state.mutex);
            state.progress.transferred(1);
          }
          // Model one million requests/second, not actual wall-clock pacing.
          if ((i + 1) % (interval_ms * 1000) == 0) {
            ++sweeps;
            for (std::size_t j = 0; j < contexts; ++j) {
              std::lock_guard lock(states[j].mutex);
              expired += states[j].progress.expired(60s);
            }
          }
        }
        const auto end = std::clock();
        const auto ns = [](auto ticks) {
          return double(ticks) * 1e9 / CLOCKS_PER_SEC;
        };
        std::printf(
            "contexts=%zu interval_ms=%zu init_ns=%.3f request_ns=%.3f "
            "sweeps=%zu expired=%zu\n",
            contexts, interval_ms, ns(created - start) / contexts,
            ns(end - created) / requests, sweeps, expired);
      }
    }
  }
  done.store(true);
  done.notify_one();
  auxiliary.join();
}
