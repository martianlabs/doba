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
// | Keep one TCP connection's engine, socket and RX/TX state together.        |
// | Connect, drain, close or abort it here.                                   |
// +---------------------------------------------------------------------------+
// | ENty is the protocol engine.                                              |
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
  ~context() { close_socket(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  context& operator=(const context&) = delete;
  context& operator=(context&&) noexcept = delete;
  // +=========================================================================+
  // | [>] send_completed                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | On send completion, update TX progress and send what's left. Abort if   |
  // | the socket is invalid or the completion is bad.                         |
  // +-------------------------------------------------------------------------+
  bool send_completed(std::size_t size, std::size_t submitted_size) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    sending_ = false;
    if (socket_ == INVALID_SOCKET) {
      return finished();
    }
    if (!size || size > submitted_size || !output_.consume(size)) {
      close_socket();
      return finished();
    }
    advance();
    return finished();
  }
  // +=========================================================================+
  // | [>] start_io                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // Start I/O after initialization. true means closed with no I/O pending.
  bool start_io() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (stopping_.load()) close();
    advance();
    return finished();
  }
  // +=========================================================================+
  // | [>] receive_completed                                        ( public ) |
  // +-------------------------------------------------------------------------+
  // Keep the state lock through engine input and its synchronous callbacks.
  bool receive_completed(std::size_t size) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    receiving_ = false;
    if (!size) {
      receive_closed_ = true;
      close();
    }
    if (stopping_.load()) close();
    if (socket_ == INVALID_SOCKET) {
      return finished();
    }
    if (state_ == state::kOpen) {
      try {
        progress_ = input_.process(size);
      } catch (...) {
        close_socket();
        throw;
      }
    }
    advance();
    return finished();
  }
  // +=========================================================================+
  // | [>] processing_completed                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // Native packets carry connection IDs, not application tasks or frames.
  bool processing_completed(std::uint64_t version = 0) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!version) processing_posted_ = false;
    else if (version != timer_version_) return finished();
    else scheduled_.reset();
    if (stopping_.load()) close();
    if (state_ == state::kOpen && ready()) {
      try {
        progress_ = input_.engine.on_processing_turn(output_idle());
      } catch (...) {
        close_socket();
      }
    }
    advance();
    return finished();
  }
  // +=========================================================================+
  // | [>] receive_failed                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | On receive failure, lock state, mark RX closed and abort.               |
  // +-------------------------------------------------------------------------+
  bool receive_failed() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    receiving_ = false;
    close_socket();
    return finished();
  }
  // +=========================================================================+
  // | [>] send_failed                                              ( public ) |
  // +-------------------------------------------------------------------------+
  // | On send failure, lock state, reset TX to idle and abort.                |
  // +-------------------------------------------------------------------------+
  bool send_failed() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    sending_ = false;
    close_socket();
    return finished();
  }
  // +=========================================================================+
  // | [>] abort                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  bool abort() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    close_socket();
    return finished();
  }
  // +=========================================================================+
  // | [>] connected                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Install synchronous send and close callbacks on the engine.             |
  // +-------------------------------------------------------------------------+
  void connected() {
    // This context owns the engine. Its callbacks aren't retained: use them
    // only inside engine calls under state_mutex_.
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
  // | Wait for active input and drain output before closing the socket.       |
  // +-------------------------------------------------------------------------+
  bool stop() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    close();
    advance();
    return finished();
  }
  // +=========================================================================+
  // | [>] notify_disconnection                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // Only the worker removing this live context calls here.
  void notify_disconnection() {
    if (!notified_ || !on_disconnection_) return;
    try {
      on_disconnection_();
    } catch (...) {}
  }
  // +=========================================================================+
  // | [>] notify_connection                                        ( public ) |
  // +-------------------------------------------------------------------------+
  // Initialize before I/O and before the owner's accept-drain barrier.
  void notify_connection() {
    notified_ = true;
    if (on_connection_) on_connection_();
  }

 private:
  // +=========================================================================+
  // | [>] ENUMs                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  // Drain output, shut down TX, then wait for peer EOF or external stop.
  enum class state { kOpen, kDraining, kShutdown, kAborting, kClosed };
  // +=========================================================================+
  // | [>] close                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  // Mark close during engine input; drain its accepted output afterwards.
  void close() {
    // The engine calls this synchronously under state_mutex_.
    if (state_ == state::kOpen) {
      state_ = state::kDraining;
      cancel_processing();
    }
  }
  // +=========================================================================+
  // | [>] send                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  // Copy output under the engine input lock; don't post I/O here.
  void send(std::string_view head, std::string_view body,
            std::unique_ptr<common::reader> source) {
    // The engine calls this synchronously under state_mutex_.
    if (state_ != state::kOpen) return;
    if (!output_.push(head, body, std::move(source))) {
      state_ = state::kAborting;
    }
  }
  // +=========================================================================+
  // | [>] advance                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  // | Call under state_mutex_, after engine input or a send completion.       |
  // +-------------------------------------------------------------------------+
  void advance() {
    if (state_ == state::kAborting) close_socket();
    if (state_ == state::kClosed) return;
    send_pending();
    if (state_ == state::kClosed) return;
    if (state_ == state::kOpen) {
      reconcile();
      if (state_ != state::kOpen) {
        if (state_ == state::kAborting) close_socket();
        return;
      }
      if (!receive()) close_socket();
      return;
    }
    if (sending_ || output_.queued() || !output_.buffer.empty()) return;
    if (state_ == state::kDraining) {
      if (::shutdown(socket_, SD_SEND) == SOCKET_ERROR) {
        close_socket();
        return;
      }
      state_ = state::kShutdown;
    }
    if (stopping_.load() || receive_closed_) {
      close_socket();
    } else if (!receive()) {
      // Read and discard until peer EOF; don't reset away the final response.
      close_socket();
    }
  }
  // +=========================================================================+
  // | [>] receive                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  // | Post one WSARecv with an owned overlapped object. WSA_IO_PENDING is     |
  // | fine; other errors free the object and return false. Mark RX active on  |
  // | success.                                                                |
  // +-------------------------------------------------------------------------+
  bool receive() {
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
  // | [>] send_pending                                            ( private ) |
  // +-------------------------------------------------------------------------+
  // | Post one batch under state_mutex_. Publish completion flags before      |
  // | unlocking: I/O may finish before the API returns.                       |
  // +-------------------------------------------------------------------------+
  void send_pending() {
    if (sending_) return;
    if (!output_.fill()) {
      close_socket();
      return;
    }
    if (output_.buffer.empty()) return;
    auto ovs = new (std::nothrow)
        overlapped_send<ENty>(this->shared_from_this());
    if (!ovs) {
      close_socket();
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
      close_socket();
      return;
    }
    sending_ = true;
  }
  // +=========================================================================+
  // | [>] output_idle                                             ( private ) |
  // +-------------------------------------------------------------------------+
  bool output_idle() const noexcept {
    return !sending_ && !output_.queued() && output_.buffer.empty();
  }
  // +=========================================================================+
  // | [>] ready                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  bool ready() const noexcept {
    return progress_.turn_requested ||
        (progress_.turn_after_output_drained && output_idle()) ||
        (progress_.turn_at && *progress_.turn_at <= clock::now());
  }
  // +=========================================================================+
  // | [>] cancel_processing                                       ( private ) |
  // +-------------------------------------------------------------------------+
  void cancel_processing() {
    progress_ = {};
    if (scheduled_) {
      timer_->cancel(this);
      scheduled_.reset();
      ++timer_version_;
    }
  }
  // +=========================================================================+
  // | [>] reconcile                                               ( private ) |
  // +-------------------------------------------------------------------------+
  // Already under the context lock. An empty sync result skips native
  // scheduling, allocation, timer locks and clock queries.
  void reconcile() {
    if (progress_.turn_at != scheduled_) {
      if (scheduled_) timer_->cancel(this);
      scheduled_.reset();
      if (++timer_version_ == 0) {
        close_socket();
        return;
      }
      if (progress_.turn_at && *progress_.turn_at > clock::now()) {
        if (!timer_ || !timer_->replace(this->shared_from_this(),
                *progress_.turn_at, timer_version_)) {
          close_socket();
          return;
        }
        scheduled_ = progress_.turn_at;
      }
    }
    if (!processing_posted_ && ready()) {
      auto* packet = new (std::nothrow)
          overlapped_processing<ENty>(this->shared_from_this());
      if (!packet) {
        close_socket();
        return;
      }
      processing_posted_ = true;
      // Release publication ownership before another worker can dequeue. Keep
      // the context lock through Post's return to protect the flag.
      if (!PostQueuedCompletionStatus(completion_port_, 0, 0, packet)) {
        processing_posted_ = false;
        delete packet;
        close_socket();
      }
    }
  }
  // +=========================================================================+
  // | [>] close_socket                                            ( private ) |
  // +-------------------------------------------------------------------------+
  void close_socket() {
    cancel_processing();
    state_ = state::kClosed;
    if (socket_ != INVALID_SOCKET) {
      closesocket(socket_);
      socket_ = INVALID_SOCKET;
    }
  }
  // +=========================================================================+
  // | [>] finished                                                ( private ) |
  // +-------------------------------------------------------------------------+
  // Call under state_mutex_. Pending operations still own their bytes.
  bool finished() const noexcept {
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
  // Serialize engine input, TX state, completions and external stop.
  std::mutex state_mutex_;
  send_state output_;
  // True while the kernel owns the active TX batch.
  bool sending_{false};
  // Connection phase; change it only under state_mutex_.
  state state_{state::kOpen};
  // Notify the connection before starting I/O.
  bool notified_{false};
  // True until the single pending RX completion is handled.
  bool receiving_{false};
  // After peer EOF, close once accepted output drains.
  bool receive_closed_{false};
  WSABUF ovr_wsa_{};
  // Protect turns with the same mutex as RX, TX and the engine.
  using clock = protocol::processing_result::clock;
  protocol::processing_result progress_{};
  HANDLE completion_port_{nullptr};
  windows_processing_timer<ENty>* timer_{nullptr};
  // At most one immediate packet; IOCP owns it until delivery.
  bool processing_posted_{false};
  // Keep the absolute registration and stale-delivery ID alive with the
  // connection.
  std::optional<clock::time_point> scheduled_;
  std::uint64_t timer_version_{0};
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
