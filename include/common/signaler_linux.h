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

#ifndef martianlabs_doba_common_signaler_linux_h
#define martianlabs_doba_common_signaler_linux_h

#include <atomic>
#include <chrono>
#include <csignal>
#include <stdexcept>
#include <thread>

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] signaler [linux]                                           ( class )  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class signaler {
 public:
  // +=========================================================================+
  // | [>] wait                                                     ( public ) |
  // +=========================================================================+
  static void wait() {
    requested_.store(false);
    auto previous_interrupt = std::signal(SIGINT, &signal_handler);
    if (previous_interrupt == SIG_ERR) {
      throw std::runtime_error("Could not install SIGINT handler");
    }
    auto previous_terminate = std::signal(SIGTERM, &signal_handler);
    if (previous_terminate == SIG_ERR) {
      std::signal(SIGINT, previous_interrupt);
      throw std::runtime_error("Could not install SIGTERM handler");
    }
    while (!requested_.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::signal(SIGTERM, previous_terminate);
    std::signal(SIGINT, previous_interrupt);
  }

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                            ( private ) |
  // +=========================================================================+
  signaler() = delete;
  // +=========================================================================+
  // | [>] signal_handler                                          ( private ) |
  // +=========================================================================+
  static void signal_handler(int) noexcept { requested_.store(true); }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  static_assert(std::atomic<bool>::is_always_lock_free);
  inline static std::atomic<bool> requested_{false};
};
}  // namespace martianlabs::doba::common

#endif
