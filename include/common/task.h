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

#ifndef martianlabs_doba_common_task_h
#define martianlabs_doba_common_task_h

#include <chrono>
#include <coroutine>
#include <exception>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] task_scheduler                                             ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct task_scheduler {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  void* owner{nullptr};
  void (*post)(void*, std::coroutine_handle<>){nullptr};
  void (*schedule)(void*, std::chrono::steady_clock::time_point,
                   std::coroutine_handle<>){nullptr};
};

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

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] task                                                        ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Tty = void>
class task {
  static_assert(std::is_void_v<Tty>);

 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  struct promise_type {
    task get_return_object() noexcept {
      return task{handle_type::from_promise(*this)};
    }
    std::suspend_always initial_suspend() noexcept { return {}; }
    struct final_awaiter {
      promise_type& promise;
      bool await_ready() noexcept {
        auto callback = std::move(promise.completion);
        try {
          if (callback) callback(promise.error);
        } catch (...) {
        }
        return true;
      }
      void await_suspend(std::coroutine_handle<>) noexcept {}
      void await_resume() noexcept {}
    };
    final_awaiter final_suspend() noexcept { return {*this}; }
    void return_void() noexcept {}
    void unhandled_exception() noexcept { error = std::current_exception(); }
    std::function<void(std::exception_ptr)> completion;
    std::exception_ptr error;
    task_scheduler scheduler;
  };
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using handle_type = std::coroutine_handle<promise_type>;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit task(handle_type handle) noexcept : handle_{handle} {}
  task(const task&) = delete;
  task(task&& other) noexcept : handle_{std::exchange(other.handle_, {})} {}
  ~task() {
    if (handle_) handle_.destroy();
  }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  task& operator=(const task&) = delete;
  task& operator=(task&& other) noexcept {
    if (this == &other) return *this;
    if (handle_) handle_.destroy();
    handle_ = std::exchange(other.handle_, {});
    return *this;
  }
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void start(std::function<void(std::exception_ptr)> completion) {
    start(std::move(completion), {});
  }
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void start(std::function<void(std::exception_ptr)> completion,
             task_scheduler scheduler) {
    auto handle = std::exchange(handle_, {});
    handle.promise().completion = std::move(completion);
    handle.promise().scheduler = scheduler;
    handle.resume();
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  handle_type handle_;
};
}  // namespace martianlabs::doba::common

#endif
