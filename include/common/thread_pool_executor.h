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

#ifndef martianlabs_doba_common_thread_pool_executor_h
#define martianlabs_doba_common_thread_pool_executor_h

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <future>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] thread_pool_executor                                        ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class thread_pool_executor {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  thread_pool_executor(std::size_t worker_count,
                       std::size_t queue_capacity)
      : worker_count_{worker_count}, queue_capacity_{queue_capacity} {
    if (worker_count == 0 || queue_capacity == 0) {
      throw std::invalid_argument(
          "Executor worker count and queue capacity must be positive");
    }
  }
  thread_pool_executor(const thread_pool_executor&) = delete;
  thread_pool_executor(thread_pool_executor&&) noexcept = delete;
  ~thread_pool_executor() { stop(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  thread_pool_executor& operator=(const thread_pool_executor&) = delete;
  thread_pool_executor& operator=(thread_pool_executor&&) noexcept = delete;
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +=========================================================================+
  void start() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (accepting_ || stopping_) return;
    accepting_ = true;
    try {
      workers_.reserve(worker_count_);
      for (std::size_t i = 0; i < worker_count_; i++) {
        workers_.emplace_back([this]() { run_(); });
      }
    } catch (...) {
      accepting_ = false;
      condition_.notify_all();
      lock.unlock();
      for (auto& worker : workers_) worker.join();
      workers_.clear();
      throw;
    }
  }
  // +=========================================================================+
  // | [>] try_submit                                               ( public ) |
  // +=========================================================================+
  template <typename FNty>
  bool try_submit(FNty&& fn) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (!accepting_ || tasks_.size() >= queue_capacity_) return false;
      tasks_.emplace_back(std::forward<FNty>(fn));
    }
    condition_.notify_one();
    return true;
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    std::vector<std::thread> workers;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (workers_.empty()) return;
      accepting_ = false;
      stopping_ = true;
      workers.swap(workers_);
    }
    condition_.notify_all();
    for (auto& worker : workers) worker.join();
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stopping_ = false;
    }
  }

 private:
  // +=========================================================================+
  // | [>] run_                                                     ( private ) |
  // +=========================================================================+
  void run_() {
    for (;;) {
      std::packaged_task<void()> task;
      {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock,
                        [this]() { return !accepting_ || !tasks_.empty(); });
        if (tasks_.empty()) return;
        task = std::move(tasks_.front());
        tasks_.pop_front();
      }
      task();
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  const std::size_t worker_count_;
  const std::size_t queue_capacity_;
  std::mutex mutex_;
  std::condition_variable condition_;
  std::deque<std::packaged_task<void()>> tasks_;
  std::vector<std::thread> workers_;
  bool accepting_{false};
  bool stopping_{false};
};
}  // namespace martianlabs::doba::common

#endif
