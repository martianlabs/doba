//      _       _           
//   __| | ___ | |__   __ _ 
//  / _` |/ _ \| '_ \ / _` |
// | (_| | (_) | |_) | (_| |
//  \__,_|\___/|_.__/ \__,_|
// 
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef martianlabs_doba_common_thread_pool_h
#define martianlabs_doba_common_thread_pool_h

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace martianlabs::doba::common {
// =============================================================================
// thread_pool                                                         ( class )
// -----------------------------------------------------------------------------
// This class holds for a generic (and simple) thread pool.
// -----------------------------------------------------------------------------
// =============================================================================
class thread_pool {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  explicit thread_pool(size_t threads = std::thread::hardware_concurrency()) {
    running_ = true;
    for (size_t i = 0; i < threads; ++i) {
      workers_.emplace([this] {
        for (;;) {
          std::function<void()> task;
          {
            std::unique_lock lock(this->mtx_);
            cv_.wait(lock, [this] { return !running_ || !tasks_.empty(); });
            if (!running_ && tasks_.empty()) {
              return;
            }
            task = std::move(tasks_.front());
            tasks_.pop();
          }
          task();
        }
      });
    }
  }
  thread_pool(const thread_pool&) = delete;
  thread_pool(thread_pool&&) noexcept = delete;
  ~thread_pool() { stop(); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  thread_pool& operator=(const thread_pool&) = delete;
  thread_pool& operator=(thread_pool&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  void stop() {
    {
      std::unique_lock lock(mtx_);
      running_ = false;
    }
    cv_.notify_all();
    while (!workers_.empty()) {
      if (workers_.front().joinable()) {
        workers_.front().join();
      }
      workers_.pop();
    }
  }
  template <class F, class... Args>
  void enqueue(F&& f, Args&&... args) {
    {
      std::unique_lock lock(mtx_);
      if (!running_) return;
      tasks_.emplace([f = std::forward<F>(f),
                      ... args = std::forward<Args>(args)]() mutable {
        f(std::move(args)...);
      });
    }
    cv_.notify_one();
  }

 private:
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  std::queue<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;
  std::mutex mtx_;
  std::condition_variable cv_;
  bool running_;
};
}  // namespace martianlabs::doba::common

#endif
