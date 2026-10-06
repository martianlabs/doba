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
#include <functional>
#include <type_traits>
#include <utility>

namespace martianlabs::doba::common {
template <typename Tty = void>
class task {
  static_assert(std::is_void_v<Tty>);

 public:
  struct promise_type {
    task get_return_object() noexcept {
      return task{handle_type::from_promise(*this)};
    }
    std::suspend_always initial_suspend() noexcept { return {}; }

    struct final_awaiter {
      promise_type& promise;

      bool await_ready() noexcept {
        auto completion = std::move(promise.completion);
        try {
          if (completion) completion(promise.error);
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
  };

  using handle_type = std::coroutine_handle<promise_type>;

  explicit task(handle_type handle) noexcept : handle_{handle} {}
  task(const task&) = delete;
  task(task&& other) noexcept : handle_{std::exchange(other.handle_, {})} {}
  ~task() {
    if (handle_) handle_.destroy();
  }

  task& operator=(const task&) = delete;
  task& operator=(task&& other) noexcept {
    if (this == &other) return *this;
    if (handle_) handle_.destroy();
    handle_ = std::exchange(other.handle_, {});
    return *this;
  }

  void start(std::function<void(std::exception_ptr)> completion) {
    auto handle = std::exchange(handle_, {});
    handle.promise().completion = std::move(completion);
    handle.resume();
  }

 private:
  handle_type handle_;
};
}  // namespace martianlabs::doba::common

#endif
