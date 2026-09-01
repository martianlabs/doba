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

#ifndef martianlabs_doba_transport_server_completion_mailbox_h
#define martianlabs_doba_transport_server_completion_mailbox_h

#include <deque>
#include <memory>
#include <mutex>
#include <utility>

#include "transport/server/response_completion.h"

#ifdef _WIN32
#include <limits>
#include <new>

#include "platform.h"
#endif

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] completion_mailbox                                          ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Wty>
class completion_mailbox {
 private:
  struct state;

 public:
  // +=========================================================================+
  // | [>] publisher                                                ( class ) |
  // +=========================================================================+
  class publisher {
   public:
    publisher() = default;
    // +=======================================================================+
    // | [>] publish                                                 ( public ) |
    // +=======================================================================+
    bool publish(response_completion completion) const {
      std::shared_ptr<state> target = state_.lock();
      if (!target) return false;
      std::lock_guard<std::mutex> lock(target->mutex);
      if (!target->open) return false;
      bool notify = target->completions.empty();
      try {
        target->completions.emplace_back(std::move(completion));
      } catch (...) {
        return false;
      }
      if (notify && !target->waker()) {
        target->completions.pop_back();
        return false;
      }
      return true;
    }

   private:
    explicit publisher(const std::shared_ptr<state>& in_state)
        : state_{in_state} {}

    std::weak_ptr<state> state_;

    friend class completion_mailbox;
  };
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  explicit completion_mailbox(Wty waker)
      : state_{std::make_shared<state>(std::move(waker))} {}
  completion_mailbox(const completion_mailbox&) = delete;
  completion_mailbox(completion_mailbox&&) noexcept = delete;
  ~completion_mailbox() { close(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  completion_mailbox& operator=(const completion_mailbox&) = delete;
  completion_mailbox& operator=(completion_mailbox&&) noexcept = delete;
  // +=========================================================================+
  // | [>] get_publisher                                            ( public ) |
  // +=========================================================================+
  publisher get_publisher() const { return publisher{state_}; }
  // +=========================================================================+
  // | [>] drain                                                    ( public ) |
  // +=========================================================================+
  std::deque<response_completion> drain() {
    std::deque<response_completion> completions;
    std::lock_guard<std::mutex> lock(state_->mutex);
    completions.swap(state_->completions);
    return completions;
  }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +=========================================================================+
  void close() {
    std::lock_guard<std::mutex> lock(state_->mutex);
    state_->open = false;
    state_->completions.clear();
  }

 private:
  struct state {
    explicit state(Wty in_waker) : waker{std::move(in_waker)} {}

    std::mutex mutex;
    std::deque<response_completion> completions;
    Wty waker;
    bool open{true};
  };

  std::shared_ptr<state> state_;
};

#ifdef _WIN32
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] iocp_response_completion                                   ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
static constexpr inline ULONG_PTR kResponseCompletionKey =
    std::numeric_limits<ULONG_PTR>::max();

struct iocp_response_completion : OVERLAPPED {
  explicit iocp_response_completion(response_completion in_completion)
      : OVERLAPPED{}, completion{std::move(in_completion)} {}

  response_completion completion;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] iocp_completion_mailbox                                     ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class iocp_completion_mailbox {
 private:
  struct state;

 public:
  // +=========================================================================+
  // | [>] publisher                                                ( class ) |
  // +=========================================================================+
  class publisher {
   public:
    publisher() = default;
    // +=======================================================================+
    // | [>] publish                                                 ( public ) |
    // +=======================================================================+
    bool publish(response_completion completion) const {
      std::shared_ptr<state> target = state_.lock();
      if (!target) return false;
      std::unique_ptr<iocp_response_completion> packet{
          new (std::nothrow)
              iocp_response_completion(std::move(completion))};
      if (!packet) return false;
      std::lock_guard<std::mutex> lock(target->mutex);
      if (!target->open) return false;
      if (!PostQueuedCompletionStatus(target->port, 0,
                                      kResponseCompletionKey, packet.get())) {
        return false;
      }
      packet.release();
      return true;
    }

   private:
    explicit publisher(const std::shared_ptr<state>& in_state)
        : state_{in_state} {}

    std::weak_ptr<state> state_;

    friend class iocp_completion_mailbox;
  };
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  iocp_completion_mailbox() = default;
  iocp_completion_mailbox(const iocp_completion_mailbox&) = delete;
  iocp_completion_mailbox(iocp_completion_mailbox&&) noexcept = delete;
  ~iocp_completion_mailbox() { close(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  iocp_completion_mailbox& operator=(const iocp_completion_mailbox&) = delete;
  iocp_completion_mailbox& operator=(
      iocp_completion_mailbox&&) noexcept = delete;
  // +=========================================================================+
  // | [>] open                                                     ( public ) |
  // +=========================================================================+
  void open(HANDLE port) {
    close();
    auto next = std::make_shared<state>(port);
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = std::move(next);
  }
  // +=========================================================================+
  // | [>] get_publisher                                            ( public ) |
  // +=========================================================================+
  publisher get_publisher() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return publisher{state_};
  }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +=========================================================================+
  void close() {
    std::shared_ptr<state> closing;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      closing = std::move(state_);
    }
    if (!closing) return;
    std::lock_guard<std::mutex> lock(closing->mutex);
    closing->open = false;
    closing->port = nullptr;
  }

 private:
  struct state {
    explicit state(HANDLE in_port) : port{in_port} {}

    std::mutex mutex;
    HANDLE port{nullptr};
    bool open{true};
  };

  mutable std::mutex mutex_;
  std::shared_ptr<state> state_;
};
#endif
}  // namespace martianlabs::doba::transport::server::detail

#endif
