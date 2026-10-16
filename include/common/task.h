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
#include <cstddef>
#include <cstring>
#include <exception>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#include "common/task_awaiters.h"
#include "common/task_scheduler.h"

namespace martianlabs::doba::common {
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
    static void* operator new(std::size_t size) {
      auto& available = frames().head;
      auto* next = &available;
      while (*next && (*next)->size != size) next = &(*next)->next;
      std::unique_ptr<frame_block> block;
      if (*next) {
        block = std::move(*next);
        *next = std::move(block->next);
      } else {
        if (size > std::numeric_limits<std::size_t>::max() -
                       __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
          throw std::bad_alloc();
        }
        block = std::make_unique<frame_block>();
        block->size = size;
        block->storage = std::make_unique_for_overwrite<std::byte[]>(
            size + __STDCPP_DEFAULT_NEW_ALIGNMENT__);
      }
      auto* storage = block->storage.get();
      // The coroutine runtime owns the block until operator delete recycles it.
      auto* owner = block.release();
      std::memcpy(storage, &owner, sizeof(owner));
      return storage + __STDCPP_DEFAULT_NEW_ALIGNMENT__;
    }
    static void operator delete(void* address) noexcept {
      auto* storage = static_cast<std::byte*>(address) -
                      __STDCPP_DEFAULT_NEW_ALIGNMENT__;
      frame_block* owner;
      std::memcpy(&owner, storage, sizeof(owner));
      std::unique_ptr<frame_block> block{owner};
      auto& available = frames().head;
      block->next = std::move(available);
      available = std::move(block);
    }
    task get_return_object() noexcept {
      return task{handle_type::from_promise(*this)};
    }
    std::suspend_always initial_suspend() noexcept { return {}; }
    struct final_awaiter {
      promise_type& promise;
      bool await_ready() noexcept {
        auto callback = std::exchange(promise.completion, nullptr);
        try {
          if (callback) callback(promise.owner, promise.error);
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
    void* owner{nullptr};
    void (*completion)(void*, std::exception_ptr){nullptr};
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
  void start(void* owner, void (*completion)(void*, std::exception_ptr),
             task_scheduler scheduler = {}) {
    auto handle = std::exchange(handle_, {});
    handle.promise().owner = owner;
    handle.promise().completion = completion;
    handle.promise().scheduler = scheduler;
    handle.resume();
  }

 private:
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  struct frame_block {
    std::unique_ptr<frame_block> next;
    std::size_t size{0};
    std::unique_ptr<std::byte[]> storage;
  };
  struct frame_pool {
    std::unique_ptr<frame_block> head;
    ~frame_pool() {
      while (head) {
        auto block = std::move(head);
        head = std::move(block->next);
      }
    }
  };
  // +=========================================================================+
  // | [>] frames                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  static frame_pool& frames() noexcept {
    thread_local frame_pool value;
    return value;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  handle_type handle_;
};
}  // namespace martianlabs::doba::common

#endif
