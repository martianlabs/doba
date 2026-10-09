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

#ifndef martianlabs_doba_common_task_awaiters_h
#define martianlabs_doba_common_task_awaiters_h

#include <chrono>
#include <coroutine>
#include <stdexcept>

#include "common/task_scheduler.h"

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] yield_awaiter                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct yield_awaiter {
  // +=========================================================================+
  // | [>] await_ready                                              ( public ) |
  // +-------------------------------------------------------------------------+
  bool await_ready() const noexcept { return false; }
  // +=========================================================================+
  // | [>] await_suspend                                            ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename Pty>
  void await_suspend(std::coroutine_handle<Pty> handle) const {
    const auto scheduler = handle.promise().scheduler;
    if (!scheduler.post) throw std::runtime_error("No task scheduler");
    scheduler.post(scheduler.owner, handle);
  }
  // +=========================================================================+
  // | [>] await_resume                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void await_resume() const noexcept {}
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] yield                                                    ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
inline yield_awaiter yield() noexcept { return {}; }

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] sleep_awaiter                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct sleep_awaiter {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  std::chrono::steady_clock::duration duration;
  // +=========================================================================+
  // | [>] await_ready                                              ( public ) |
  // +-------------------------------------------------------------------------+
  bool await_ready() const noexcept {
    return duration <= std::chrono::steady_clock::duration::zero();
  }
  // +=========================================================================+
  // | [>] await_suspend                                            ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename Pty>
  void await_suspend(std::coroutine_handle<Pty> handle) const {
    const auto scheduler = handle.promise().scheduler;
    if (!scheduler.schedule) throw std::runtime_error("No task scheduler");
    scheduler.schedule(scheduler.owner,
                       std::chrono::steady_clock::now() + duration, handle);
  }
  // +=========================================================================+
  // | [>] await_resume                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void await_resume() const noexcept {}
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] sleep_for                                                ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
inline sleep_awaiter sleep_for(
    std::chrono::steady_clock::duration duration) noexcept {
  return {duration};
}
}  // namespace martianlabs::doba::common

#endif
