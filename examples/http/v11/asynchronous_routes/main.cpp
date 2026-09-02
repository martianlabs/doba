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

#include <condition_variable>
#include <coroutine>
#include <deque>
#include <memory>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>

#include "common/console_logger.h"
#include "common/logo.h"
#include "common/signaler.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http;
using namespace martianlabs::doba::protocol::http::v11;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] background_executor                                        ( class ) |
// +---------------------------------------------------------------------------+
// | Example executor that resumes continuations on one background thread.    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class background_executor {
 public:
  // +=========================================================================+
  // | [>] awaiter                                                  ( class ) |
  // +=========================================================================+
  class awaiter {
   public:
    awaiter(background_executor& executor,
            std::stop_token stop_token) noexcept
        : executor_(executor), stop_token_(stop_token) {}
    bool await_ready() const noexcept {
      return stop_token_.stop_requested();
    }
    void await_suspend(std::coroutine_handle<> continuation) {
      executor_.enqueue(continuation);
    }
    void await_resume() const noexcept {}

   private:
    background_executor& executor_;
    std::stop_token stop_token_;
  };
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  background_executor()
      : worker_([this]() { run(); }) {}
  background_executor(const background_executor&) = delete;
  background_executor(background_executor&&) noexcept = delete;
  ~background_executor() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stopping_ = true;
    }
    condition_.notify_one();
  }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  background_executor& operator=(const background_executor&) = delete;
  background_executor& operator=(background_executor&&) noexcept = delete;
  // +=========================================================================+
  // | [>] schedule                                                 ( public ) |
  // +=========================================================================+
  [[nodiscard]] awaiter schedule(std::stop_token stop_token) noexcept {
    return awaiter(*this, stop_token);
  }

 private:
  // +=========================================================================+
  // | [>] enqueue                                                 ( private ) |
  // +=========================================================================+
  void enqueue(std::coroutine_handle<> continuation) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      continuations_.push_back(continuation);
    }
    condition_.notify_one();
  }
  // +=========================================================================+
  // | [>] run                                                     ( private ) |
  // +=========================================================================+
  void run() {
    for (;;) {
      std::coroutine_handle<> continuation;
      {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this]() {
          return stopping_ || !continuations_.empty();
        });
        if (continuations_.empty()) return;
        continuation = continuations_.front();
        continuations_.pop_front();
      }
      continuation.resume();
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::mutex mutex_;
  std::condition_variable condition_;
  std::deque<std::coroutine_handle<>> continuations_;
  bool stopping_{false};
  std::jthread worker_;
};

int main() {
  background_executor executor;
  server http_server;
  http_server.add_route(
      method_names::kGet, "/health",
      [](const request&, response& res) {
        res.ok_200()
            .add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body("ready");
      });
  http_server.add_route(
      method_names::kGet, "/work/:id",
      [&executor](std::shared_ptr<const request> req,
                  std::stop_token stop_token,
                  int id) -> task<response> {
        co_await executor.schedule(stop_token);
        if (stop_token.stop_requested()) co_return response();
        std::string body = "completed ";
        body += req->get_absolute_path();
        body += " with id ";
        body += std::to_string(id);
        response res;
        res.ok_200()
            .add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body(body);
        co_return res;
      });
  http_server.start("8080");
  signaler::wait();
  http_server.stop();
  return 0;
}
