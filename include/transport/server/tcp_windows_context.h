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

#ifndef martianlabs_doba_transport_server_tcp_windows_context_h
#define martianlabs_doba_transport_server_tcp_windows_context_h

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <exception>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <new>
#include <span>
#include <string_view>
#include <utility>

#include "platform.h"
#include "transport/server/tcp_connection.h"
#include "transport/server/tcp_windows_overlapped_receive.h"
#include "transport/server/tcp_windows_overlapped_send.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] context [windowsTM]                                        ( struct ) |
// +---------------------------------------------------------------------------+
// | Windows server transport connection state.                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty, typename CNty>
struct context : public std::enable_shared_from_this<context<ENty, CNty>> {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  template <protocol::contracts::engine_factory<ENty> FAty>
  context(SOCKET in_socket, std::size_t recv_buffer_size,
          std::size_t send_buffer_size, const FAty& create_engine,
          const std::atomic<bool>& stopping,
          types::on_client_connected_delegate on_connection,
          types::on_client_disconnected_delegate on_disconnection,
          std::function<void(context*)> on_retirement,
          typename CNty::shared_state shared_state)
      : socket_{in_socket},
        on_connection_{on_connection},
        on_disconnection_{on_disconnection},
        on_retirement_{on_retirement},
        input_{recv_buffer_size, send_buffer_size, create_engine,
               shared_state},
        stopping_{stopping},
        output_{send_buffer_size} {}
  context(const context&) = delete;
  context(context&&) noexcept = delete;
  ~context() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  context& operator=(const context&) = delete;
  context& operator=(context&&) noexcept = delete;
  // +=========================================================================+
  // | [>] send_completed                                           ( public ) |
  // +-------------------------------------------------------------------------+
  void send_completed(std::size_t size, std::size_t submitted_size) {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    send_state_ = send_status::kIdle;
    if (socket_ == INVALID_SOCKET) {
      retire_();
      return;
    }
    if (!size || size > submitted_size ||
        !input_.output_sent(output_, size)) {
      abort_();
      return;
    }
    send_pending_();
  }
  // +=========================================================================+
  // | [>] output_ready                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void output_ready() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    send_state_ = send_status::kIdle;
    send_pending_();
  }
  // +=========================================================================+
  // | [>] send                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void send(std::string_view head, std::string_view body,
            std::unique_ptr<common::reader> source) {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    if (closing_ || socket_ == INVALID_SOCKET) return;
    if (head.empty() && body.empty() && !source) return;
    if (!output_.push(head, body, std::move(source))) {
      closing_ = true;
      aborted_ = true;
      ::shutdown(socket_, SD_BOTH);
      request_output_();
      return;
    }
    request_output_();
  }
  // +=========================================================================+
  // | [>] arm_next_receive_operation                               ( public ) |
  // +-------------------------------------------------------------------------+
  bool arm_next_receive_operation() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    processing_receive_ = false;
    if (stopping_.load()) {
      closing_ = true;
      if (!input_.close_output()) abort_();
    }
    send_pending_();
    if (closing_ || socket_ == INVALID_SOCKET) {
      cleanup_resources_();
      return false;
    }
    if (input_.size == input_.capacity || !receive_()) {
      abort_();
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] receive_completed                                        ( public ) |
  // +-------------------------------------------------------------------------+
  bool receive_completed(std::size_t size) {
    std::lock_guard<std::mutex> receive_lock(receive_mutex_);
    {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      receiving_ = false;
      if (!size) receive_closed_ = true;
      if (stopping_.load()) {
        closing_ = true;
        if (!input_.close_output()) abort_();
      }
      if (aborted_) abort_();
      if (socket_ == INVALID_SOCKET) {
        retire_();
        return false;
      }
      if (closing_) {
        cleanup_resources_();
        return false;
      }
      processing_receive_ = size != 0;
    }
    if (!size) return true;
    input_.size += size;
    std::size_t processed;
    try {
      processed = input_.process();
    } catch (...) {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      processing_receive_ = false;
      abort_();
      throw;
    }
    {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      if (!input_.consume(processed) || input_.size == input_.capacity) {
        processing_receive_ = false;
        abort_();
        return false;
      }
      if (input_.peer_closed() && !closing_) {
        closing_ = true;
        if (!input_.close_output()) abort_();
      }
    }
    notify_connection();
    return true;
  }
  // +=========================================================================+
  // | [>] receive_failed                                           ( public ) |
  // +-------------------------------------------------------------------------+
  void receive_failed() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    receiving_ = false;
    abort_();
  }
  // +=========================================================================+
  // | [>] send_failed                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void send_failed() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    send_state_ = send_status::kIdle;
    abort_();
  }
  // +=========================================================================+
  // | [>] abort                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void abort() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    abort_();
  }
  // +=========================================================================+
  // | [>] connected                                                ( public ) |
  // +-------------------------------------------------------------------------+
  void connected(HANDLE completion_port) {
    {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      io_h_ = completion_port;
    }
    input_.engine.set_on_close([weak = this->weak_from_this()]() {
      if (auto ctx = weak.lock()) ctx->close();
    });
    input_.engine.set_on_send([weak = this->weak_from_this()](
        std::string_view head, std::string_view body,
        std::unique_ptr<common::reader> source) {
      if (auto ctx = weak.lock()) {
        ctx->send(head, body, std::move(source));
      }
    });
  }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void close() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    if (closing_ || socket_ == INVALID_SOCKET) return;
    closing_ = true;
    if (!input_.close_output()) {
      abort_();
      return;
    }
    request_output_();
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void stop() {
    // Wait for the active handler before closing the connection.
    {
      std::lock_guard<std::mutex> receive_lock(receive_mutex_);
      std::lock_guard<std::mutex> lock(sending_mutex_);
      closing_ = true;
      if (!input_.close_output()) abort_();
      processing_receive_ = false;
      send_pending_();
      cleanup_resources_();
    }
    notify_disconnection();
  }
  // +=========================================================================+
  // | [>] notify_disconnection                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void notify_disconnection() {
    {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      if (!notified_ || disconnected_ || socket_ != INVALID_SOCKET) return;
      disconnected_ = true;
    }
    try {
      // Let's call user's callback to notify for disconnection!
      on_disconnection_();
    } catch (const std::exception&) {
      // [to-do] -> add support for this!
    } catch (...) {
      // [to-do] -> add support for this!
    }
  }
  // +=========================================================================+
  // | [>] notify_connection                                       ( public )  |
  // +-------------------------------------------------------------------------+
  void notify_connection() {
    {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      if (notified_ || !input_.established()) return;
      notified_ = true;
    }
    on_connection_();
  }
  // +=========================================================================+
  // | [>] eof                                                     ( public )  |
  // +-------------------------------------------------------------------------+
  bool eof() const { return input_.eof(); }

 private:
  // +=========================================================================+
  // | [>] receive_                                                ( private ) |
  // +-------------------------------------------------------------------------+
  bool receive_() {
    DWORD f = 0, r = 0;
    overlapped_receive<ENty, CNty>* ovr = new (std::nothrow)
        overlapped_receive<ENty, CNty>(this->shared_from_this());
    if (!ovr) return false;
    ovr_wsa_.buf = input_.buffer.get() + input_.size;
    ovr_wsa_.len = static_cast<ULONG>(input_.capacity - input_.size);
    int res = WSARecv(socket_, &ovr_wsa_, 1, &r, &f, ovr, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      delete ovr;
      return false;
    }
    receiving_ = true;
    return true;
  }
  // +=========================================================================+
  // | [>] request_output_                                         ( private ) |
  // +-------------------------------------------------------------------------+
  void request_output_() {
    if (processing_receive_ || send_state_ != send_status::kIdle) return;
    auto output = new (std::nothrow) overlapped_send<ENty, CNty>(
        this->shared_from_this(), io_type::kOutput);
    if (output && PostQueuedCompletionStatus(io_h_, 0, 0, output)) {
      send_state_ = send_status::kQueued;
    } else {
      delete output;
      closing_ = true;
      aborted_ = true;
      ::shutdown(socket_, SD_BOTH);
    }
  }
  // +=========================================================================+
  // | [>] send_pending_                                           ( private ) |
  // +-------------------------------------------------------------------------+
  void send_pending_() {
    if (aborted_) {
      abort_();
      return;
    }
    if (send_state_ != send_status::kIdle) return;
    if (socket_ == INVALID_SOCKET) {
      retire_();
      return;
    }
    if (processing_receive_) return;
    if (!input_.prepare_output(output_)) {
      abort_();
      return;
    }
    std::array<std::span<char>, 1> bytes{};
    const std::size_t count = input_.output_buffers(output_, bytes);
    if (!count) {
      cleanup_resources_();
      return;
    }
    auto ovs = new (std::nothrow) overlapped_send<ENty, CNty>(
        this->shared_from_this());
    if (!ovs) {
      abort_();
      return;
    }
    for (std::size_t i = 0; i < count; i++) {
      const std::size_t size = std::min<std::size_t>(
          bytes[i].size(),
          std::numeric_limits<DWORD>::max() - ovs->submitted_size);
      if (!size) break;
      ovs->buffers[i] = {static_cast<ULONG>(size), bytes[i].data()};
      ovs->buffer_count++;
      ovs->submitted_size += size;
    }
    const int result = WSASend(socket_, ovs->buffers.data(),
                               ovs->buffer_count, nullptr, 0, ovs, nullptr);
    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      delete ovs;
      abort_();
      return;
    }
    send_state_ = send_status::kPending;
  }
  // +=========================================================================+
  // | [>] cleanup_resources_                                      ( private ) |
  // +-------------------------------------------------------------------------+
  void cleanup_resources_() {
    if (socket_ == INVALID_SOCKET) {
      retire_();
      return;
    }
    if (!closing_ || processing_receive_ ||
        send_state_ != send_status::kIdle ||
        input_.output_pending(output_)) return;
    if (!socket_shutdown_) {
      if (::shutdown(socket_, SD_SEND) == SOCKET_ERROR) {
        abort_();
        return;
      }
      socket_shutdown_ = true;
    }
    if (stopping_.load() || receive_closed_ || input_.peer_closed()) {
      close_socket_();
      retire_();
      return;
    }
    if (!receiving_) {
      // Discard input until peer EOF to avoid resetting the final response.
      input_.size = 0;
      if (!receive_()) abort_();
    }
  }
  // +=========================================================================+
  // | [>] abort_                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  void abort_() {
    closing_ = true;
    aborted_ = true;
    close_socket_();
    retire_();
  }
  // +=========================================================================+
  // | [>] close_socket_                                           ( private ) |
  // +-------------------------------------------------------------------------+
  void close_socket_() {
    if (socket_ != INVALID_SOCKET) {
      closesocket(socket_);
      socket_ = INVALID_SOCKET;
    }
  }
  // +=========================================================================+
  // | [>] retire_                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  void retire_() {
    if (retired_ || socket_ != INVALID_SOCKET || receiving_ ||
        processing_receive_ || send_state_ != send_status::kIdle) return;
    retired_ = true;
    if (on_retirement_) on_retirement_(this);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  enum class send_status { kIdle, kQueued, kPending };
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
  std::function<void(context*)> on_retirement_;
  SOCKET socket_{INVALID_SOCKET};
  HANDLE io_h_{nullptr};
  CNty input_;
  const std::atomic<bool>& stopping_;
  std::mutex receive_mutex_;
  std::mutex sending_mutex_;
  send_state output_;
  send_status send_state_{send_status::kIdle};
  bool closing_{false};
  bool aborted_{false};
  bool notified_{false};
  bool disconnected_{false};
  bool receiving_{false};
  bool receive_closed_{false};
  bool processing_receive_{false};
  bool socket_shutdown_{false};
  bool retired_{false};
  WSABUF ovr_wsa_{0};
};

}  // namespace martianlabs::doba::transport::server

#endif
