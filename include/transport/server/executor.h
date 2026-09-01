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

#ifndef martianlabs_doba_transport_server_executor_h
#define martianlabs_doba_transport_server_executor_h

#include <array>
#include <coroutine>
#include <cstddef>
#include <deque>
#include <mutex>

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] executor                                                    ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class executor {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  executor() = default;
  executor(const executor&) = delete;
  executor(executor&&) noexcept = delete;
  ~executor() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  executor& operator=(const executor&) = delete;
  executor& operator=(executor&&) noexcept = delete;
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +=========================================================================+
  bool start() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!continuations_.empty()) return false;
    accepting_ = true;
    return true;
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    accepting_ = false;
  }
  // +=========================================================================+
  // | [>] schedule                                                 ( public ) |
  // +=========================================================================+
  bool schedule(std::coroutine_handle<> continuation) {
    if (!continuation || continuation.done()) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!accepting_) return false;
    continuations_.push_back(continuation);
    return true;
  }
  // +=========================================================================+
  // | [>] run                                                      ( public ) |
  // +=========================================================================+
  bool run() {
    std::array<std::coroutine_handle<>, kBatchSize> batch{};
    std::size_t count = 0;
    bool pending = false;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      while (count < batch.size() && !continuations_.empty()) {
        batch[count++] = continuations_.front();
        continuations_.pop_front();
      }
      pending = !continuations_.empty();
    }
    for (std::size_t i = 0; i < count; i++) {
      if (!batch[i].done()) batch[i].resume();
    }
    return pending;
  }

 private:
  // +=========================================================================+
  // | [>] CONSTANTs                                               ( private ) |
  // +=========================================================================+
  static constexpr std::size_t kBatchSize = 64;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::mutex mutex_;
  std::deque<std::coroutine_handle<>> continuations_;
  bool accepting_{true};
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
