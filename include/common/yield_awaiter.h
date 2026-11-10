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

#ifndef martianlabs_doba_common_yield_awaiter_h
#define martianlabs_doba_common_yield_awaiter_h

#include <coroutine>

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
    scheduler.post(scheduler.owner);
  }
  // +=========================================================================+
  // | [>] await_resume                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void await_resume() const noexcept {}
};
}  // namespace martianlabs::doba::common

#endif
