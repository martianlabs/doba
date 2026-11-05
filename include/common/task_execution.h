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

#ifndef martianlabs_doba_common_task_execution_h
#define martianlabs_doba_common_task_execution_h

#include <chrono>
#include <coroutine>
#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>

#include "common/task.h"

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] task_execution                                              ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class task_execution {
 public:
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using clock = std::chrono::steady_clock;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit task_execution(task<void> value)
      : handle_{std::exchange(value.handle_, {})} {
    if (!handle_) throw std::invalid_argument("Empty task");
    auto& promise = handle_.promise();
    promise.controlled = true;
    promise.scheduler = {
        this,
        [](void* owner, std::coroutine_handle<>) {
          static_cast<task_execution*>(owner)->ready_ = true;
        },
        [](void* owner, clock::time_point due, std::coroutine_handle<>) {
          static_cast<task_execution*>(owner)->deadline_ = due;
        }};
  }
  task_execution(const task_execution&) = delete;
  task_execution(task_execution&&) = delete;
  ~task_execution() { cancel(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  task_execution& operator=(const task_execution&) = delete;
  task_execution& operator=(task_execution&&) = delete;
  // +=========================================================================+
  // | [>] resume                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  bool resume(clock::time_point now = clock::now()) {
    if (done() || (!ready_ && (!deadline_ || now < *deadline_))) return false;
    ready_ = false;
    deadline_.reset();
    handle_.resume();
    return true;
  }
  // +=========================================================================+
  // | [>] cancel                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  void cancel() noexcept {
    ready_ = false;
    deadline_.reset();
    if (auto handle = std::exchange(handle_, {})) handle.destroy();
  }
  // +=========================================================================+
  // | [>] done                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  bool done() const noexcept { return !handle_ || handle_.done(); }
  // +=========================================================================+
  // | [>] ready                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  bool ready() const noexcept { return ready_; }
  // +=========================================================================+
  // | [>] deadline                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  std::optional<clock::time_point> deadline() const noexcept {
    return deadline_;
  }
  // +=========================================================================+
  // | [>] error                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  std::exception_ptr error() const noexcept {
    return handle_ && handle_.done() ? handle_.promise().error : nullptr;
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  task<void>::handle_type handle_;  // Sole owner of the suspended frame.
  bool ready_{true};               // Initial entry or a requested yield.
  std::optional<clock::time_point> deadline_;  // Positive sleep, if pending.
};
}  // namespace martianlabs::doba::common

#endif
