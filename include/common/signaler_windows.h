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

#ifndef martianlabs_doba_common_signaler_windows_h
#define martianlabs_doba_common_signaler_windows_h

#include <atomic>
#include <chrono>
#include <system_error>
#include <thread>

#include "platform.h"

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] signaler [windowsTM]                                       ( class )  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class signaler {
 public:
  // +=========================================================================+
  // | [>] wait                                                     ( public ) |
  // +=========================================================================+
  static void wait() {
    requested_.store(false);
    if (!SetConsoleCtrlHandler(&signal_handler, TRUE)) {
      throw std::system_error(static_cast<int>(GetLastError()),
                              std::system_category(), "SetConsoleCtrlHandler");
    }
    while (!requested_.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    SetConsoleCtrlHandler(&signal_handler, FALSE);
  }

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                            ( private ) |
  // +=========================================================================+
  signaler() = delete;
  // +=========================================================================+
  // | [>] signal_handler                                          ( private ) |
  // +=========================================================================+
  static BOOL WINAPI signal_handler(DWORD type) noexcept {
    if (type != CTRL_C_EVENT && type != CTRL_BREAK_EVENT) return FALSE;
    requested_.store(true);
    return TRUE;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  static_assert(std::atomic<bool>::is_always_lock_free);
  inline static std::atomic<bool> requested_{false};
};
}  // namespace martianlabs::doba::common

#endif
