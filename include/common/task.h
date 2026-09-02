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

#include <coroutine>
#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] task                                                        ( class ) |
// +---------------------------------------------------------------------------+
// | Lazy, move-only coroutine result with a single consumer.                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Tty>
class task {
 private:
  class awaiter;

 public:
  // +=========================================================================+
  // | [>] promise_type                                             ( public ) |
  // +=========================================================================+
  struct promise_type {
    task get_return_object() noexcept {
      return task(std::coroutine_handle<promise_type>::from_promise(*this));
    }
    std::suspend_always initial_suspend() const noexcept { return {}; }
    struct final_awaiter {
      bool await_ready() const noexcept { return false; }
      std::coroutine_handle<> await_suspend(
          std::coroutine_handle<promise_type> coroutine) const noexcept {
        if (coroutine.promise().continuation_) {
          return coroutine.promise().continuation_;
        }
        return std::noop_coroutine();
      }
      void await_resume() const noexcept {}
    };
    final_awaiter final_suspend() const noexcept { return {}; }
    template <typename Uty>
    void return_value(Uty&& value) {
      value_.emplace(std::forward<Uty>(value));
    }
    void unhandled_exception() noexcept {
      exception_ = std::current_exception();
    }

   private:
    friend class awaiter;
    std::optional<Tty> value_;
    std::exception_ptr exception_;
    std::coroutine_handle<> continuation_;
  };
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  task(const task&) = delete;
  task(task&& in) noexcept
      : coroutine_(std::exchange(in.coroutine_, nullptr)) {}
  ~task() {
    if (coroutine_) coroutine_.destroy();
  }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  task& operator=(const task&) = delete;
  task& operator=(task&& in) noexcept {
    if (this == &in) return *this;
    if (coroutine_) coroutine_.destroy();
    coroutine_ = std::exchange(in.coroutine_, nullptr);
    return *this;
  }
  auto operator co_await() && {
    if (!coroutine_) throw std::logic_error("cannot await an empty task");
    return awaiter(std::exchange(coroutine_, nullptr));
  }
  void operator co_await() & = delete;

 private:
  // +=========================================================================+
  // | [>] awaiter                                                 ( private ) |
  // +=========================================================================+
  class awaiter {
   public:
    explicit awaiter(std::coroutine_handle<promise_type> coroutine) noexcept
        : coroutine_(coroutine) {}
    awaiter(const awaiter&) = delete;
    awaiter(awaiter&& in) noexcept
        : coroutine_(std::exchange(in.coroutine_, nullptr)) {}
    ~awaiter() {
      if (coroutine_) coroutine_.destroy();
    }
    bool await_ready() const noexcept { return false; }
    std::coroutine_handle<> await_suspend(
        std::coroutine_handle<> continuation) noexcept {
      coroutine_.promise().continuation_ = continuation;
      return coroutine_;
    }
    Tty await_resume() {
      auto coroutine = std::exchange(coroutine_, nullptr);
      if (coroutine.promise().exception_) {
        auto exception = coroutine.promise().exception_;
        coroutine.destroy();
        std::rethrow_exception(exception);
      }
      try {
        Tty value(std::move(*coroutine.promise().value_));
        coroutine.destroy();
        coroutine = nullptr;
        return value;
      } catch (...) {
        if (coroutine) coroutine.destroy();
        throw;
      }
    }

   private:
    std::coroutine_handle<promise_type> coroutine_;
  };
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                            ( private ) |
  // +=========================================================================+
  explicit task(std::coroutine_handle<promise_type> coroutine) noexcept
      : coroutine_(coroutine) {}
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::coroutine_handle<promise_type> coroutine_;
};
}  // namespace martianlabs::doba::common

#endif
