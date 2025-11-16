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
//        --- martianLabs Anti-AI Usage and Model-Training Addendum ---
//
// TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
//
// Copyright 2025 martianLabs
//
// Except as otherwise stated in this Addendum, this software is licensed
// under the Apache License, Version 2.0 (the "License"); you may not use
// this file except in compliance with the License.
//
// The following additional terms are hereby added to the Apache License for
// the purpose of restricting the use of this software by Artificial
// Intelligence systems, machine learning models, data-scraping bots, and
// automated systems.
//
// 1.  MACHINE LEARNING AND AI RESTRICTIONS
//     1.1. No entity, organization, or individual may use this software,
//          its source code, object code, or any derivative work for the
//          purpose of training, fine-tuning, evaluating, or improving any
//          machine learning model, artificial intelligence system, large
//          language model, or similar automated system.
//     1.2. No automated system may copy, parse, analyze, index, or
//          otherwise process this software for any AI-related purpose.
//     1.3. Use of this software as input, prompt material, reference
//          material, or evaluation data for AI systems is expressly
//          prohibited.
//
// 2.  SCRAPING AND AUTOMATED ACCESS RESTRICTIONS
//     2.1. No automated crawler, training pipeline, or data-extraction
//          system may collect, store, or incorporate any portion of this
//          software in any dataset used for machine learning or AI
//          training.
//     2.2. Any automated access must comply with this License and with
//          applicable copyright law.
//
// 3.  PROHIBITION ON DERIVATIVE DATASETS
//     3.1. You may not create datasets, corpora, embeddings, vector
//          stores, or similar derivative data intended for use by
//          automated systems, AI models, or machine learning algorithms.
//
// 4.  NO WAIVER OF RIGHTS
//     4.1. These restrictions apply in addition to, and do not limit,
//          the rights and protections provided to the copyright holder
//          under the Apache License Version 2.0 and applicable law.
//
// 5.  ACCEPTANCE
//     5.1. Any use of this software constitutes acceptance of both the
//          Apache License Version 2.0 and this Anti-AI Addendum.
//
// You may obtain a copy of the Apache License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.  See the License for the specific language governing
// permissions and limitations under the Apache License Version 2.0.

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
