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

#ifndef martianlabs_doba_transport_server_tcpip_h
#define martianlabs_doba_transport_server_tcpip_h

#include <coroutine>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <stop_token>
#include <utility>

#include "common/task.h"
#include "platform.h"

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] PLATFORM-INDEPENDENT-TYPEs                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
namespace martianlabs::doba::transport::server {
struct types {
  template <typename RQty, typename RSty>
  using on_request_delegate =
      std::function<std::optional<common::task<RSty>>(
          const std::shared_ptr<RQty>&, RSty&,
          const std::stop_token&)>;
  template <typename RSty>
  using on_bad_request_delegate =
      std::function<void(int, std::string_view, RSty&)>;
  using on_client_connected_delegate = std::function<void()>;
  using on_client_disconnected_delegate = std::function<void()>;
};
}  // namespace martianlabs::doba::transport::server

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] detached_operation                                         ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class detached_operation {
 public:
  // +=========================================================================+
  // | [>] promise_type                                             ( public ) |
  // +=========================================================================+
  struct promise_type {
    detached_operation get_return_object() noexcept {
      return detached_operation(
          std::coroutine_handle<promise_type>::from_promise(*this));
    }
    std::suspend_always initial_suspend() const noexcept { return {}; }
    std::suspend_never final_suspend() const noexcept { return {}; }
    void return_void() const noexcept {}
    void unhandled_exception() const noexcept { std::terminate(); }
  };
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  detached_operation(const detached_operation&) = delete;
  detached_operation(detached_operation&& in) noexcept
      : coroutine_(std::exchange(in.coroutine_, nullptr)) {}
  ~detached_operation() {
    if (coroutine_) coroutine_.destroy();
  }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  detached_operation& operator=(const detached_operation&) = delete;
  detached_operation& operator=(detached_operation&& in) noexcept {
    if (this == &in) return *this;
    if (coroutine_) coroutine_.destroy();
    coroutine_ = std::exchange(in.coroutine_, nullptr);
    return *this;
  }
  // +=========================================================================+
  // | [>] get_coroutine                                            ( public ) |
  // +=========================================================================+
  std::coroutine_handle<> get_coroutine() const noexcept {
    return coroutine_;
  }
  // +=========================================================================+
  // | [>] release                                                  ( public ) |
  // +=========================================================================+
  void release() noexcept { coroutine_ = nullptr; }

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                            ( private ) |
  // +=========================================================================+
  explicit detached_operation(
      std::coroutine_handle<promise_type> coroutine) noexcept
      : coroutine_(coroutine) {}
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::coroutine_handle<promise_type> coroutine_;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] resume_on                                                   ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename STty>
class resume_on {
 public:
  explicit resume_on(std::weak_ptr<STty> scheduler) noexcept
      : scheduler_(std::move(scheduler)) {}
  bool await_ready() const noexcept { return false; }
  bool await_suspend(std::coroutine_handle<> continuation) {
    auto scheduler = scheduler_.lock();
    if (!scheduler) return false;
    scheduled_ = true;
    if (scheduler->schedule(continuation)) return true;
    scheduled_ = false;
    return false;
  }
  bool await_resume() const noexcept { return scheduled_; }

 private:
  std::weak_ptr<STty> scheduler_;
  bool scheduled_{false};
};
}  // namespace martianlabs::doba::transport::server::detail

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] PLATFORM-DEPENDENT-INCLUDEs                               ( section ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include "transport/server/tcpip_windows.h"
#elif __linux__
#include "transport/server/tcpip_linux.h"
#endif

#endif
