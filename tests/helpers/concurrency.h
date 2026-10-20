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

#ifndef martianlabs_doba_tests_concurrency_h
#define martianlabs_doba_tests_concurrency_h

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <coroutine>
#include <mutex>
#include <utility>
#include <vector>

namespace martianlabs::doba::tests {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] scope_exit                                                  ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename F>
class scope_exit {
 public:
  // +-------------------------------------------------------------------------+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit scope_exit(F cleanup) : cleanup_(std::move(cleanup)) {}
  scope_exit(const scope_exit&) = delete;
  scope_exit& operator=(const scope_exit&) = delete;
  ~scope_exit() noexcept { cleanup_(); }

 private:
  // +-------------------------------------------------------------------------+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  F cleanup_;
};

// Open is idempotent and also releases awaiters that arrive after cleanup.
// Keep the event alive until the server and all test threads have stopped.
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] manual_event                                                ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class manual_event {
 public:
  // +-------------------------------------------------------------------------+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  manual_event() = default;
  manual_event(const manual_event&) = delete;
  manual_event& operator=(const manual_event&) = delete;
  // +-------------------------------------------------------------------------+
  // | [>] open                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void open() {
    std::vector<std::coroutine_handle<>> pending;
    {
      std::lock_guard lock(mutex_);
      open_ = true;
      pending.swap(pending_);
      changed_.notify_all();
    }
    for (auto handle : pending) handle.resume();
  }
  // +-------------------------------------------------------------------------+
  // | [>] wait                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void wait() {
    std::unique_lock lock(mutex_);
    changed_.wait(lock, [this] { return open_; });
  }
  // +-------------------------------------------------------------------------+
  // | [>] wait_for                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  bool wait_for(std::chrono::milliseconds timeout = std::chrono::seconds(5)) {
    std::unique_lock lock(mutex_);
    return changed_.wait_for(lock, timeout, [this] { return open_; });
  }
  // +-------------------------------------------------------------------------+
  // | [>] wait_for_waiters                                         ( public ) |
  // +-------------------------------------------------------------------------+
  bool wait_for_waiters(std::size_t count, std::chrono::milliseconds timeout =
                                               std::chrono::seconds(5)) {
    std::unique_lock lock(mutex_);
    return changed_.wait_for(lock, timeout,
                             [this, count] { return arrivals_ >= count; });
  }
  struct awaiter {
    manual_event& owner;
    bool await_ready() const noexcept { return false; }
    bool await_suspend(std::coroutine_handle<> handle) {
      std::lock_guard lock(owner.mutex_);
      ++owner.arrivals_;
      owner.changed_.notify_all();
      if (owner.open_) return false;
      owner.pending_.push_back(handle);
      return true;
    }
    void await_resume() const noexcept {}
  };
  awaiter operator co_await() noexcept { return {*this}; }

 private:
  // +-------------------------------------------------------------------------+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::mutex mutex_;
  std::condition_variable changed_;
  bool open_{false};
  std::size_t arrivals_{0};
  std::vector<std::coroutine_handle<>> pending_;
};
}  // namespace martianlabs::doba::tests
#endif
