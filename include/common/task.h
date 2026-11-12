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

#ifndef martianlabs_doba_common_task_h
#define martianlabs_doba_common_task_h

#include <concepts>
#include <coroutine>
#include <utility>

#include "common/task_promise.h"

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] task                                                        ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Tty = void>
  requires std::same_as<Tty, void>
class task {
 public:
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using promise_type = task_promise;
  using handle_type = std::coroutine_handle<promise_type>;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit task(promise_type& promise) noexcept
      : handle_{handle_type::from_promise(promise)} {}
  task(const task&) = delete;
  task(task&& other) noexcept : handle_{std::exchange(other.handle_, {})} {}
  ~task() {
    if (handle_) {
      handle_.destroy();
    }
  }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  task& operator=(const task&) = delete;
  task& operator=(task&&) = delete;
  // +=========================================================================+
  // | [>] release                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  [[nodiscard]] handle_type release() && noexcept {
    return std::exchange(handle_, {});
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  handle_type handle_;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] get_return_object                                        ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
inline task<void> task_promise::get_return_object() noexcept {
  return task<void>{*this};
}
}  // namespace martianlabs::doba::common

#endif
