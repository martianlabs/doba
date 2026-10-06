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

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <coroutine>
#include <mutex>
#include <thread>
#include <vector>

#include "common/signaler.h"
#include "common/task.h"
#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http::v11;

namespace {
class timer {
 public:
  struct awaiter {
    timer& owner;
    std::chrono::steady_clock::time_point due;

    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> handle) const {
      owner.schedule(due, handle);
    }
    void await_resume() const noexcept {}
  };

  timer() : worker_([this](std::stop_token stop) { run(stop); }) {}
  ~timer() {
    worker_.request_stop();
    wake_.notify_one();
  }

  awaiter after(std::chrono::milliseconds duration) {
    return {*this, std::chrono::steady_clock::now() + duration};
  }

 private:
  struct pending {
    std::chrono::steady_clock::time_point due;
    std::coroutine_handle<> handle;
  };

  void schedule(std::chrono::steady_clock::time_point due,
                std::coroutine_handle<> handle) {
    std::lock_guard<std::mutex> lock(mutex_);
    pending_.push_back({due, handle});
    wake_.notify_one();
  }

  void run(std::stop_token stop) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (!stop.stop_requested()) {
      wake_.wait(lock, [this, &stop]() {
        return stop.stop_requested() || !pending_.empty();
      });
      if (stop.stop_requested()) break;
      const auto next = std::min_element(
          pending_.begin(), pending_.end(),
          [](const pending& left, const pending& right) {
            return left.due < right.due;
          });
      const auto due = next->due;
      if (wake_.wait_until(lock, due) == std::cv_status::no_timeout) {
        continue;
      }
      const auto ready = std::min_element(
          pending_.begin(), pending_.end(),
          [](const pending& left, const pending& right) {
            return left.due < right.due;
          });
      if (ready->due > std::chrono::steady_clock::now()) continue;
      const auto handle = ready->handle;
      pending_.erase(ready);
      lock.unlock();
      handle.resume();
      lock.lock();
    }
  }

  std::mutex mutex_;
  std::condition_variable wake_;
  std::vector<pending> pending_;
  std::jthread worker_;
};
}  // namespace

int main() {
  timer delays;
  server<> http_server({.ip = "0.0.0.0", .port = "8080"});
  http_server.add_route("GET", "/slow",
                        [&delays](const request&, response& res)
                            -> task<void> {
                          co_await delays.after(std::chrono::milliseconds(50));
                          res.ok_200().set_body("slow");
                        });
  http_server.add_route("GET", "/fast", [](const request&, response& res) {
    res.ok_200().set_body("fast");
  });
  http_server.start();
  signaler::wait();
  http_server.stop();
  return 0;
}
