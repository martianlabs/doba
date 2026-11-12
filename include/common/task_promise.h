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

#ifndef martianlabs_doba_common_task_promise_h
#define martianlabs_doba_common_task_promise_h

#include <concepts>
#include <coroutine>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "common/task_awaiters.h"
#include "common/task_scheduler.h"

namespace martianlabs::doba::common {
template <typename Tty>
  requires std::same_as<Tty, void>
class task;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] task_promise                                               ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct task_promise {
  // +=========================================================================+
  // | [>] get_return_object                                        ( public ) |
  // +-------------------------------------------------------------------------+
  task<void> get_return_object() noexcept;
  // +=========================================================================+
  // | [>] initial_suspend                                          ( public ) |
  // +-------------------------------------------------------------------------+
  std::suspend_always initial_suspend() noexcept { return {}; }
  // +=========================================================================+
  // | [>] final_suspend                                            ( public ) |
  // +-------------------------------------------------------------------------+
  std::suspend_always final_suspend() noexcept { return {}; }
  // +=========================================================================+
  // | [>] return_void                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void return_void() noexcept {}
  // +=========================================================================+
  // | [>] unhandled_exception                                      ( public ) |
  // +-------------------------------------------------------------------------+
  void unhandled_exception() noexcept { error = std::current_exception(); }
  // +=========================================================================+
  // | [>] await_transform                                          ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename Aty>
  Aty&& await_transform(Aty&& awaiter) {
    if constexpr (std::is_same_v<std::remove_cvref_t<Aty>, yield_awaiter> ||
                  std::is_same_v<std::remove_cvref_t<Aty>, sleep_awaiter>) {
      return std::forward<Aty>(awaiter);
    } else {
      throw std::logic_error("Unsupported task awaiter");
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  std::exception_ptr error;
  task_scheduler scheduler;
};
}  // namespace martianlabs::doba::common

#endif
