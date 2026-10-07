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

#ifndef martianlabs_doba_protocol_http_v11_engine_async_h
#define martianlabs_doba_protocol_http_v11_engine_async_h

#include <atomic>
#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

#include "protocol/send_delegate.h"
#include "protocol/serialization.h"

namespace martianlabs::doba::protocol::http::v11 {
class engine_async {
 public:
  explicit engine_async(std::size_t max_pending_requests)
      : max_pending_requests_{max_pending_requests} {}

  void set_on_send(protocol::send_delegate output) {
    on_send_ = std::move(output);
  }
  void set_on_close(std::function<void()> close) {
    on_close_ = std::move(close);
  }
  std::optional<std::size_t> reserve() {
    if (!accepting()) return std::nullopt;
    if (max_pending_requests_ &&
        next_request_ - next_send_.load(std::memory_order_acquire) >=
            max_pending_requests_) {
      close_now();
      return std::nullopt;
    }
    return next_request_++;
  }
  void begin() noexcept { active_.fetch_add(1, std::memory_order_release); }
  void end() noexcept { active_.fetch_sub(1, std::memory_order_release); }
  bool closing() const noexcept {
    return closing_.load(std::memory_order_acquire);
  }
  bool accepting() const noexcept {
    return accepting_.load(std::memory_order_acquire);
  }
  bool idle() const noexcept {
    return accepting() &&
           active_.load(std::memory_order_acquire) == 0 &&
           next_request_ == next_send_.load(std::memory_order_acquire);
  }

  void submit(std::size_t position, protocol::serialization_result result,
              bool final, bool close) {
    if (close) accepting_.store(false, std::memory_order_release);
    if (active_.load(std::memory_order_acquire) == 0 &&
        position == next_send_.load(std::memory_order_acquire) &&
        pending_.empty()) {
      if (closing()) return;
      on_send_(result.head, result.body, std::move(result.source));
      if (final) {
        next_send_++;
        if (close) close_now();
      } else {
        pending_.try_emplace(position);
      }
      return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (closing()) return;
    if (final && position == next_send_.load(std::memory_order_relaxed) &&
        pending_.empty()) {
      on_send_(result.head, result.body, std::move(result.source));
      next_send_++;
      if (close) close_now();
      return;
    }
    auto& slot = pending_[position];
    slot.outputs.push_back({std::string(result.head), std::string(result.body),
                            std::move(result.source)});
    slot.complete = final;
    slot.close = close;
    for (;;) {
      auto current = pending_.find(next_send_.load());
      if (current == pending_.end()) break;
      while (!current->second.outputs.empty()) {
        auto output = std::move(current->second.outputs.front());
        current->second.outputs.pop_front();
        on_send_(output.head, output.body, std::move(output.source));
      }
      if (!current->second.complete) break;
      const bool should_close = current->second.close;
      pending_.erase(current);
      next_send_++;
      if (should_close) {
        pending_.clear();
        close_now();
        break;
      }
    }
  }

  void close_now() {
    accepting_.store(false, std::memory_order_release);
    if (closing_.exchange(true)) return;
    on_close_();
  }

 private:
  struct output {
    std::string head;
    std::string body;
    std::unique_ptr<common::reader> source;
  };
  struct slot {
    std::deque<output> outputs;
    bool complete{false};
    bool close{false};
  };

  protocol::send_delegate on_send_;
  std::function<void()> on_close_;
  const std::size_t max_pending_requests_;
  std::atomic<std::size_t> active_{0};
  std::atomic<bool> accepting_{true};
  std::atomic<bool> closing_{false};
  std::size_t next_request_{0};
  std::atomic<std::size_t> next_send_{0};
  std::mutex mutex_;
  std::map<std::size_t, slot> pending_;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
