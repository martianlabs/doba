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

#ifndef martianlabs_doba_transport_server_tcp_windows_context_h
#define martianlabs_doba_transport_server_tcp_windows_context_h

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <new>
#include <string_view>
#include <utility>

#include "platform.h"
#include "transport/server/contracts.h"
#include "transport/server/connection_reservation.h"
#include "transport/server/send_state.h"
#include "transport/server/tcp_connection.h"
#include "transport/server/tcp_windows_overlapped_receive.h"
#include "transport/server/tcp_windows_overlapped_processing.h"
#include "transport/server/tcp_windows_processing_timer.h"
#include "transport/server/tcp_windows_overlapped_send.h"

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] context                                                    ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct keeps track of an engine's TCP connection. It manages         |
// | the socket and input and output buffers, and handles sending and          |
// | receiving data. It also lets you connect, stop, close, and abort the      |
// | connection.                                                               |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// | ENty - engine type being used.                                            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct context : public std::enable_shared_from_this<context<ENty>> {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  template <protocol::contracts::engine_factory<ENty> FAty>
  context(SOCKET in_socket, std::size_t receive_buffer_capacity,
          std::size_t send_buffer_size, const FAty& create_engine,
          const std::atomic<bool>& stopping,
          types::on_client_connected_delegate on_connection,
          types::on_client_disconnected_delegate on_disconnection,
          connection_reservation reservation = {},
          HANDLE completion_port = nullptr,
          windows_processing_timer<ENty>* timer = nullptr)
      : reservation_{std::move(reservation)},
        on_connection_{on_connection},
        on_disconnection_{on_disconnection},
        socket_{in_socket},
        input_{receive_buffer_capacity, create_engine},
        stopping_{stopping},
        output_{send_buffer_size},
        completion_port_{completion_port}, timer_{timer} {}
  context(const context&) = delete;
  context(context&&) noexcept = delete;
  ~context() { close_socket_(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  context& operator=(const context&) = delete;
  context& operator=(context&&) noexcept = delete;
  // +=========================================================================+
  // | [>] send_completed                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Called when a send operation finishes. It takes the number of bytes     |
  // | sent and the total number submitted, then updates the send state and    |
  // | checks the output buffer. If the socket is invalid or something goes    |
  // | wrong, it aborts the connection. Otherwise, it keeps sending any        |
  // | data that’s still waiting.                                              |
  // +-------------------------------------------------------------------------+
  bool send_completed(std::size_t size, std::size_t submitted_size) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    sending_ = false;
    if (socket_ == INVALID_SOCKET) {
      return finished_();
    }
    if (!size || size > submitted_size || !output_.consume(size)) {
      close_socket_();
      return finished_();
    }
    advance_();
    return finished_();
  }
  // +=========================================================================+
  // | [>] start_io                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // Starts I/O after initialization; true means closed with no pending I/O.
  bool start_io() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (stopping_.load()) close();
    advance_();
    return finished_();
  }
  // +=========================================================================+
  // | [>] receive_completed                                        ( public ) |
  // +-------------------------------------------------------------------------+
  // Holds the state lock across engine input and its synchronous callbacks.
  bool receive_completed(std::size_t size) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    receiving_ = false;
    if (!size) {
      receive_closed_ = true;
      close();
    }
    if (stopping_.load()) close();
    if (socket_ == INVALID_SOCKET) {
      return finished_();
    }
    if (state_ == state::kOpen) {
      try {
        progress_ = input_.process(size);
      } catch (...) {
        close_socket_();
        throw;
      }
    }
    advance_();
    return finished_();
  }
  // +=========================================================================+
  // | [>] processing_completed                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // Native packets carry connection identity, never application tasks/frames.
  bool processing_completed(std::uint64_t version = 0) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!version) processing_posted_ = false;
    else if (version != timer_version_) return finished_();
    else scheduled_.reset();
    if (stopping_.load()) close();
    if (state_ == state::kOpen && ready_()) {
      try {
        progress_ = input_.engine.on_processing_turn(output_idle_());
      } catch (...) {
        close_socket_();
      }
    }
    advance_();
    return finished_();
  }
  // +=========================================================================+
  // | [>] receive_failed                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Called when a receive operation fails. It locks the state mutex, marks  |
  // | receiving as closed, and aborts the connection.                         |
  // +-------------------------------------------------------------------------+
  bool receive_failed() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    receiving_ = false;
    close_socket_();
    return finished_();
  }
  // +=========================================================================+
  // | [>] send_failed                                              ( public ) |
  // +-------------------------------------------------------------------------+
  // | Called when a send operation fails. It locks the state mutex, resets    |
  // | the send state to idle, and aborts the connection.                      |
  // +-------------------------------------------------------------------------+
  bool send_failed() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    sending_ = false;
    close_socket_();
    return finished_();
  }
  // +=========================================================================+
  // | [>] abort                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  bool abort() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    close_socket_();
    return finished_();
  }
  // +=========================================================================+
  // | [>] connected                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Installs synchronous engine callbacks for output and close.             |
  // +-------------------------------------------------------------------------+
  void connected() {
    // The engine belongs to this context. Its callbacks are synchronous,
    // non-retained and invoked only during engine entries under state_mutex_.
    input_.engine.set_on_close([this]() { close(); });
    input_.engine.set_on_send([this](
        std::string_view head, std::string_view body,
        std::unique_ptr<common::reader> source) {
      send(head, body, std::move(source));
    });
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Waits for active input and drains output before closing the socket.     |
  // +-------------------------------------------------------------------------+
  bool stop() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    close();
    advance_();
    return finished_();
  }
  // +=========================================================================+
  // | [>] notify_disconnection                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // Only the worker that erases the context from its live set calls this.
  void notify_disconnection() {
    if (!notified_ || !on_disconnection_) return;
    try { on_disconnection_(); } catch (...) {}
  }
  // +=========================================================================+
  // | [>] notify_connection                                        ( public ) |
  // +-------------------------------------------------------------------------+
  // Initialization precedes all I/O and the owner's accept-drain barrier.
  void notify_connection() {
    notified_ = true;
    if (on_connection_) on_connection_();
  }

 private:
  // +=========================================================================+
  // | [>] ENUMs                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  // Close drains output, shuts down TX, then awaits EOF or external stop.
  enum class state { kOpen, kDraining, kShutdown, kAborting, kClosed };
  // +=========================================================================+
  // | [>] close                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  // Marks close inside engine input; its accepted output drains afterwards.
  void close() {
    // Called synchronously by the engine under state_mutex_.
    if (state_ == state::kOpen) {
      state_ = state::kDraining;
      cancel_processing_();
    }
  }
  // +=========================================================================+
  // | [>] send                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  // Copies accepted output under the engine input lock; never posts I/O.
  void send(std::string_view head, std::string_view body,
            std::unique_ptr<common::reader> source) {
    // Called synchronously by the engine under state_mutex_.
    if (state_ != state::kOpen) return;
    if (!output_.push(head, body, std::move(source))) {
      state_ = state::kAborting;
    }
  }
  // +=========================================================================+
  // | [>] advance_                                                ( private ) |
  // +-------------------------------------------------------------------------+
  // | Called under state_mutex_, after engine input or a send completion.     |
  // +-------------------------------------------------------------------------+
  void advance_() {
    if (state_ == state::kAborting) close_socket_();
    if (state_ == state::kClosed) return;
    send_pending_();
    if (state_ == state::kClosed) return;
    if (state_ == state::kOpen) {
      reconcile_();
      if (state_ != state::kOpen) {
        if (state_ == state::kAborting) close_socket_();
        return;
      }
      if (!receive_()) close_socket_();
      return;
    }
    if (sending_ || output_.queued() || !output_.buffer.empty()) return;
    if (state_ == state::kDraining) {
      if (::shutdown(socket_, SD_SEND) == SOCKET_ERROR) {
        close_socket_();
        return;
      }
      state_ = state::kShutdown;
    }
    if (stopping_.load() || receive_closed_) {
      close_socket_();
    } else if (!receive_()) {
      // Discard input until peer EOF to avoid resetting the final response.
      close_socket_();
    }
  }
  // +=========================================================================+
  // | [>] receive_                                                ( private ) |
  // +-------------------------------------------------------------------------+
  // | Starts receiving data by creating an overlapped receive object and      |
  // | calling `WSARecv()`. It sets up the buffer and its size, then checks    |
  // | the result. If `WSARecv()` fails with an error other than               |
  // | `WSA_IO_PENDING`, it deletes the object and returns false. Otherwise,   |
  // | it marks receiving as active and returns true.                          |
  // +-------------------------------------------------------------------------+
  bool receive_() {
    if (receiving_ || output_.has_source()) return true;
    const auto target = input_.receive_buffer();
    if (target.empty()) return false;
    DWORD f = 0, r = 0;
    overlapped_receive<ENty>* ovr = new (std::nothrow)
        overlapped_receive<ENty>(this->shared_from_this());
    if (!ovr) return false;
    ovr_wsa_.buf = target.data();
    ovr_wsa_.len = static_cast<ULONG>(target.size());
    int res = WSARecv(socket_, &ovr_wsa_, 1, &r, &f, ovr, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      delete ovr;
      return false;
    }
    receiving_ = true;
    return true;
  }
  // +=========================================================================+
  // | [>] send_pending_                                           ( private ) |
  // +-------------------------------------------------------------------------+
  // | Posts one batch under state_mutex_. Completion flags are published      |
  // | before unlocking, even if socket I/O completes before the API returns.  |
  // +-------------------------------------------------------------------------+
  void send_pending_() {
    if (sending_) return;
    if (!output_.fill()) {
      close_socket_();
      return;
    }
    if (output_.buffer.empty()) return;
    auto ovs = new (std::nothrow)
        overlapped_send<ENty>(this->shared_from_this());
    if (!ovs) {
      close_socket_();
      return;
    }
    ovs->buffer = {
        static_cast<ULONG>(std::min<std::size_t>(
            output_.buffer.size() - output_.offset,
            std::numeric_limits<ULONG>::max())),
        output_.buffer.data() + output_.offset};
    const int result = WSASend(socket_, &ovs->buffer, 1, nullptr, 0,
                               ovs, nullptr);
    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      delete ovs;
      close_socket_();
      return;
    }
    sending_ = true;
  }
  // +=========================================================================+
  // | [>] output_idle_                                            ( private ) |
  // +-------------------------------------------------------------------------+
  bool output_idle_() const noexcept {
    return !sending_ && !output_.queued() && output_.buffer.empty();
  }
  // +=========================================================================+
  // | [>] ready_                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  bool ready_() const noexcept {
    return progress_.turn_requested ||
        (progress_.turn_after_output_drained && output_idle_()) ||
        (progress_.turn_at && *progress_.turn_at <= clock::now());
  }
  // +=========================================================================+
  // | [>] cancel_processing_                                      ( private ) |
  // +-------------------------------------------------------------------------+
  void cancel_processing_() {
    progress_ = {};
    if (scheduled_) {
      timer_->cancel(this);
      scheduled_.reset();
      ++timer_version_;
    }
  }
  // +=========================================================================+
  // | [>] reconcile_                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Called under the existing context lock. Empty sync snapshots do no native
  // scheduling, allocation, timer locking or clock query.
  void reconcile_() {
    if (progress_.turn_at != scheduled_) {
      if (scheduled_) timer_->cancel(this);
      scheduled_.reset();
      if (++timer_version_ == 0) {
        close_socket_();
        return;
      }
      if (progress_.turn_at && *progress_.turn_at > clock::now()) {
        if (!timer_ || !timer_->replace(this->shared_from_this(),
                *progress_.turn_at, timer_version_)) {
          close_socket_();
          return;
        }
        scheduled_ = progress_.turn_at;
      }
    }
    if (!processing_posted_ && ready_()) {
      auto* packet = new (std::nothrow)
          overlapped_processing<ENty>(this->shared_from_this());
      if (!packet) { close_socket_(); return; }
      processing_posted_ = true;
      // Release publication ownership before another worker can dequeue.
      // This context lock protects the flag through Post's return.
      if (!PostQueuedCompletionStatus(completion_port_, 0, 0, packet)) {
        processing_posted_ = false;
        delete packet;
        close_socket_();
      }
    }
  }
  // +=========================================================================+
  // | [>] close_socket_                                           ( private ) |
  // +-------------------------------------------------------------------------+
  void close_socket_() {
    cancel_processing_();
    state_ = state::kClosed;
    if (socket_ != INVALID_SOCKET) {
      closesocket(socket_);
      socket_ = INVALID_SOCKET;
    }
  }
  // +=========================================================================+
  // | [>] finished_                                               ( private ) |
  // +-------------------------------------------------------------------------+
  // Called under state_mutex_. Outstanding operations still own their bytes.
  bool finished_() const noexcept {
    return state_ == state::kClosed && !receiving_ && !sending_ &&
           !processing_posted_;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  connection_reservation reservation_;
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
  SOCKET socket_{INVALID_SOCKET};
  tcp_connection<ENty> input_;
  const std::atomic<bool>& stopping_;
  // Serializes engine input, TX state, socket completions and external stop.
  std::mutex state_mutex_;
  send_state output_;
  // True only while the kernel owns the active TX batch.
  bool sending_{false};
  // Current connection phase; only state_mutex_ protects transitions.
  state state_{state::kOpen};
  // Initialization emits the connection notification before starting I/O.
  bool notified_{false};
  // True until the one outstanding RX completion is handled.
  bool receiving_{false};
  // Peer EOF permits closing after the accepted output drains.
  bool receive_closed_{false};
  WSABUF ovr_wsa_{};
  // Generic turn state is protected by the same mutex as RX/TX and the engine.
  using clock = protocol::processing_result::clock;
  protocol::processing_result progress_{};
  HANDLE completion_port_{nullptr};
  windows_processing_timer<ENty>* timer_{nullptr};
  // At most one immediate packet, owned by IOCP until its worker handles it.
  bool processing_posted_{false};
  // Lifetime-owned absolute registration and stale-delivery identity.
  std::optional<clock::time_point> scheduled_;
  std::uint64_t timer_version_{0};
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
