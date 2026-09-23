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

#ifndef martianlabs_doba_transport_server_tcpip_windows_h
#define martianlabs_doba_transport_server_tcpip_windows_h

#include <algorithm>
#include <atomic>
#include <charconv>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <list>
#include <memory>
#include <mutex>
#include <new>
#include <optional>
#include <span>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "network/environment.h"
#include "platform.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] CONSTANTs                                                  ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
static constexpr DWORD kAcceptAddressBytes =
    static_cast<DWORD>(sizeof(sockaddr_storage) + 16);
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] io_type                                                ( enum-class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class io_type : uint8_t { kAccept, kSend, kReceive, kWake };
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] FORWARDs                                                   ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct context;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] overlapped_base                                            ( struct ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct overlapped_base : OVERLAPPED {
  overlapped_base(io_type in_type) : OVERLAPPED{}, type{in_type} {}
  io_type get_type() const { return type; }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  const io_type type;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] overlapped_accept                                          ( struct ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct overlapped_accept : overlapped_base {
  overlapped_accept(SOCKET in_socket)
      : overlapped_base(io_type::kAccept), socket{in_socket} {}
  SOCKET socket{INVALID_SOCKET};
  CHAR addresses[(kAcceptAddressBytes * 2)]{0};
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] overlapped_receive                                         ( struct ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct overlapped_receive : overlapped_base {
  overlapped_receive(std::shared_ptr<context<ENty>> context)
      : overlapped_base(io_type::kReceive), ctx{context} {}
  std::shared_ptr<context<ENty>> ctx;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] overlapped_send                                            ( struct ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct overlapped_send : overlapped_base {
  overlapped_send(std::shared_ptr<context<ENty>> context)
      : overlapped_base(io_type::kSend), ctx{context} {}
  std::shared_ptr<context<ENty>> ctx;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] overlapped_wake                                            ( struct ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct overlapped_wake : overlapped_base {
  explicit overlapped_wake(context<ENty>* value)
      : overlapped_base(io_type::kWake), ctx{value} {}
  context<ENty>* ctx;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] context [windowsTM]                                         ( class ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |  ENty - engine type being used.                                           |
// ----------------------------------------------------------------------------+
// // | This specification holds for the WindowsTM server transport context.   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct context : public std::enable_shared_from_this<context<ENty>> {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  template <protocol::contracts::engine_factory<ENty> FAty>
  context(SOCKET in_socket, std::size_t recv_buffer_size,
          std::size_t send_buffer_size, const FAty& create_engine,
          const std::atomic<bool>& stopping,
          types::on_client_disconnected_delegate on_disconnection,
          std::function<void(context*)> on_retirement = {})
      : socket_{in_socket},
        on_disconnection_{on_disconnection},
        on_retirement_{on_retirement},
        engine_{create_engine()},
        stopping_{stopping},
        send_capacity_{send_buffer_size},
        ovr_buf_{std::make_unique<char[]>(recv_buffer_size)},
        ovr_capacity_{recv_buffer_size} {}
  context(const context&) = delete;
  context(context&&) noexcept = delete;
  ~context() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  context& operator=(const context&) = delete;
  context& operator=(context&&) noexcept = delete;
  // +=========================================================================+
  // | [>] send_completed                                           ( public ) |
  // +=========================================================================+
  void send_completed(std::size_t size) {
    std::lock_guard<std::mutex> engine_lock(engine_mutex_);
    std::unique_lock<std::mutex> sending_lock(sending_mutex_);
    sending_ = false;
    if (socket_ == INVALID_SOCKET) {
      sending_lock.unlock();
      cancel_sends_();
      return;
    }
    send_offset_ += size;
    if (send_offset_ != send_size_) {
      if (size && socket_ != INVALID_SOCKET && send_()) return;
      abort_();
      sending_lock.unlock();
      cancel_sends_();
      return;
    }
    if (!read_source_()) {
      abort_();
      sending_lock.unlock();
      cancel_sends_();
      return;
    }
    if (send_offset_ != send_size_) {
      if (socket_ != INVALID_SOCKET && send_()) return;
      abort_();
      sending_lock.unlock();
      cancel_sends_();
      return;
    }
    send_bytes_ -= send_reserved_;
    send_reserved_ = 0;
    send_buffer_.reset();
    if (socket_ != INVALID_SOCKET && !send_queue_.empty()) {
      send_buffer_ = std::move(std::get<0>(send_queue_.front()));
      send_source_ = std::move(std::get<2>(send_queue_.front()));
      send_size_ = std::get<1>(send_queue_.front());
      send_reserved_ = send_source_
          ? std::max(send_size_, std::min<std::size_t>(8192, send_capacity_))
          : send_size_;
      send_streaming_ = false;
      send_offset_ = 0;
      send_queue_.pop_front();
      if (!read_source_() || !send_()) abort_();
    }
    cleanup_resources_();
    sending_lock.unlock();
    try {
      engine_.on_send_completed(true);
    } catch (...) {
      abort();
      cancel_sends_();
      throw;
    }
    cancel_sends_();
  }
  // +=========================================================================+
  // | [>] send                                                     ( public ) |
  // +=========================================================================+
  void send(std::unique_ptr<char[]> buffer, std::size_t size,
             std::optional<common::reader> source) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    if (closing_ || socket_ == INVALID_SOCKET) return;
    const std::size_t reserved = source
        ? std::max(size, std::min<std::size_t>(8192, send_capacity_)) : size;
    if ((size && !buffer) || (!size && !source) ||
        reserved > send_capacity_ - send_bytes_) {
      abort_();
      return;
    }
    if (send_reserved_) {
      send_queue_.emplace_back(std::move(buffer), size, std::move(source));
    } else {
      send_buffer_ = std::move(buffer);
      send_source_ = std::move(source);
      send_reserved_ = reserved;
      send_streaming_ = false;
      send_size_ = size;
      send_offset_ = 0;
    }
    send_bytes_ += reserved;
    if (!sending_ && (!read_source_() || !send_())) abort_();
  }
  // +=========================================================================+
  // | [>] arm_next_receive_operation                               ( public ) |
  // +=========================================================================+
  bool arm_next_receive_operation() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    processing_receive_ = false;
    if (stopping_.load()) closing_ = true;
    if (closing_) {
      cleanup_resources_();
      return false;
    }
    if (ovr_size_ == ovr_capacity_) {
      abort_();
      return false;
    }
    if (receive_()) return true;
    abort_();
    return false;
  }
  // +=========================================================================+
  // | [>] receive_completed                                        ( public ) |
  // +=========================================================================+
  bool receive_completed(std::size_t size) {
    std::lock_guard<std::mutex> engine_lock(engine_mutex_);
    bool active = false;
    {
      std::lock_guard<std::mutex> sending_lock(sending_mutex_);
      receiving_ = false;
      if (!size) receive_closed_ = true;
      if (stopping_.load()) closing_ = true;
      if (socket_ == INVALID_SOCKET) retire_();
      else if (closing_) cleanup_resources_();
      else {
        active = true;
        processing_receive_ = size != 0;
      }
    }
    if (!active) {
      cancel_sends_();
      return false;
    }
    if (size) {
      ovr_size_ += size;
      std::size_t processed = 0;
      try {
        processed = engine_.on_bytes_received(ovr_buf_.get(), ovr_size_,
                                              ovr_capacity_);
      } catch (...) {
        abort();
        cancel_sends_();
        throw;
      }
      if (processed > ovr_size_) {
        abort();
        cancel_sends_();
        return false;
      }
      cancel_sends_();
      ovr_size_ -= processed;
      if (processed && ovr_size_) {
        std::memmove(ovr_buf_.get(), ovr_buf_.get() + processed, ovr_size_);
      }
      if (ovr_size_ == ovr_capacity_) {
        abort();
        cancel_sends_();
        return false;
      }
    }
    return true;
  }
  // +=========================================================================+
  // | [>] receive_failed                                           ( public ) |
  // +=========================================================================+
  void receive_failed() {
    std::lock_guard<std::mutex> engine_lock(engine_mutex_);
    {
      std::lock_guard<std::mutex> sending_lock(sending_mutex_);
      receiving_ = false;
      abort_();
    }
    cancel_sends_();
  }
  // +=========================================================================+
  // | [>] send_failed                                              ( public ) |
  // +=========================================================================+
  void send_failed() {
    std::lock_guard<std::mutex> engine_lock(engine_mutex_);
    {
      std::lock_guard<std::mutex> sending_lock(sending_mutex_);
      sending_ = false;
      abort_();
    }
    cancel_sends_();
  }
  // +=========================================================================+
  // | [>] abort                                                    ( public ) |
  // +=========================================================================+
  void abort() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    abort_();
  }
  // +=========================================================================+
  // | [>] connected                                                ( public ) |
  // +=========================================================================+
  void connected(HANDLE completion_port) {
    std::lock_guard<std::mutex> engine_lock(engine_mutex_);
    {
      std::lock_guard<std::mutex> sending_lock(sending_mutex_);
      connected_ = true;
      notify_disconnection_();
    }
    try {
      engine_.set_on_wake(
          [weak = this->weak_from_this(), completion_port]() noexcept {
            auto ctx = weak.lock();
            if (!ctx) return;
            std::lock_guard<std::mutex> lock(ctx->sending_mutex_);
            if (ctx->closing_ || ctx->socket_ == INVALID_SOCKET ||
                ctx->wake_pending_) return;
            ctx->wake_pending_ = true;
            ctx->wake_owner_ = ctx;
            if (!PostQueuedCompletionStatus(completion_port, 0, 0,
                                            &ctx->wake_event_)) {
              ctx->wake_pending_ = false;
              ctx->wake_owner_.reset();
              ctx->abort_();
            }
          });
      engine_.set_on_close([this]() { close(); });
      engine_.set_on_send(
          [this](std::unique_ptr<char[]> buffer, std::size_t size,
                 std::optional<common::reader> source) {
            send(std::move(buffer), size, std::move(source));
          });
    } catch (...) {
      abort();
      cancel_sends_();
      throw;
    }
    cancel_sends_();
  }
  // +=========================================================================+
  // | [>] wake                                                     ( public ) |
  // +=========================================================================+
  void wake() {
    std::lock_guard<std::mutex> engine_lock(engine_mutex_);
    bool active = false;
    {
      std::lock_guard<std::mutex> sending_lock(sending_mutex_);
      wake_pending_ = false;
      wake_owner_.reset();
      if (stopping_.load()) closing_ = true;
      active = !closing_ && socket_ != INVALID_SOCKET;
      if (closing_) cleanup_resources_();
    }
    try {
      if (active) engine_.on_wake();
    } catch (...) {
      abort();
    }
    cancel_sends_();
  }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +=========================================================================+
  void close() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    closing_ = true;
    cleanup_resources_();
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    std::lock_guard<std::mutex> engine_lock(engine_mutex_);
    close();
    cancel_sends_();
  }

 private:
  // +=========================================================================+
  // | [>] cancel_sends_                                           ( private ) |
  // +=========================================================================+
  void cancel_sends_() {
    // Called with engine_mutex_ held, after leaving the engine callback.
    std::size_t count = 0;
    bool closing = false;
    {
      std::lock_guard<std::mutex> sending_lock(sending_mutex_);
      closing = closing_;
      if (!sending_ && socket_ == INVALID_SOCKET) {
        count = send_queue_.size() + (send_reserved_ ? 1 : 0);
        send_buffer_.reset();
        send_source_.reset();
        send_reserved_ = 0;
        send_queue_.clear();
        send_bytes_ = 0;
      }
    }
    if (closing && !engine_stopped_) {
      engine_stopped_ = true;
      try {
        engine_.on_stop();
      } catch (...) {
        abort();
      }
    }
    for (std::size_t i = 0; i < count; i++) {
      try {
        engine_.on_send_completed(false);
      } catch (...) {
      }
    }
  }
  // +=========================================================================+
  // | [>] receive_                                                ( private ) |
  // +=========================================================================+
  bool receive_() {
    DWORD f = 0, r = 0;
    overlapped_receive<ENty>* ovr =
        new (std::nothrow) overlapped_receive<ENty>(this->shared_from_this());
    if (!ovr) return false;
    ovr_wsa_.buf = ovr_buf_.get() + ovr_size_;
    ovr_wsa_.len = static_cast<ULONG>(ovr_capacity_ - ovr_size_);
    int res = WSARecv(socket_, &ovr_wsa_, 1, &r, &f, ovr, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      delete ovr;
      return false;
    }
    receiving_ = true;
    return true;
  }
  // +=========================================================================+
  // | [>] read_source_                                            ( private ) |
  // +=========================================================================+
  bool read_source_() {
    if (!send_source_ || send_offset_ != send_size_) return true;
    if (send_source_->failed()) return false;
    if (!send_source_->eof()) {
      const std::size_t capacity = std::min<std::size_t>(8192, send_capacity_);
      if (!send_streaming_) {
        send_buffer_.reset();
        send_buffer_.reset(new (std::nothrow) char[capacity]);
        if (!send_buffer_) return false;
        send_streaming_ = true;
      }
      send_size_ = send_source_->read(std::span<std::byte>(
          reinterpret_cast<std::byte*>(send_buffer_.get()), capacity));
      send_offset_ = 0;
      if (send_source_->failed() || (!send_size_ && !send_source_->eof())) {
        return false;
      }
    }
    if (send_source_->eof()) send_source_.reset();
    return true;
  }
  // +=========================================================================+
  // | [>] send_                                                   ( private ) |
  // +=========================================================================+
  bool send_() {
    DWORD f = 0;
    overlapped_send<ENty>* ovs =
        new (std::nothrow) overlapped_send<ENty>(this->shared_from_this());
    if (!ovs) return false;
    ULONG size = static_cast<ULONG>(std::min<std::size_t>(
        send_size_ - send_offset_, std::numeric_limits<ULONG>::max()));
    WSABUF buffer_view{size, send_buffer_
                                ? send_buffer_.get() + send_offset_ : nullptr};
    int res = WSASend(socket_, &buffer_view, 1, nullptr, f, ovs, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      delete ovs;
      return false;
    }
    sending_ = true;
    return true;
  }
  // +=========================================================================+
  // | [>] cleanup_resources_                                      ( private ) |
  // +=========================================================================+
  void cleanup_resources_() {
    if (!closing_ || sending_) return;
    if (socket_ == INVALID_SOCKET) {
      retire_();
      return;
    }
    if (!socket_shutdown_) {
      if (::shutdown(socket_, SD_SEND) == SOCKET_ERROR) {
        abort_();
        return;
      }
      socket_shutdown_ = true;
    }
    if (stopping_.load() || receive_closed_) {
      close_socket_();
      retire_();
      return;
    }
    // The engine must release the buffer before another receive can use it.
    if (!receiving_ && !processing_receive_) {
      // Discard input until peer EOF to avoid resetting the final response.
      ovr_size_ = 0;
      if (!receive_()) abort_();
    }
  }
  // +=========================================================================+
  // | [>] abort_                                                  ( private ) |
  // +=========================================================================+
  void abort_() {
    closing_ = true;
    close_socket_();
    retire_();
  }
  // +=========================================================================+
  // | [>] close_socket_                                           ( private ) |
  // +=========================================================================+
  void close_socket_() {
    if (socket_ != INVALID_SOCKET) {
      closesocket(socket_);
      socket_ = INVALID_SOCKET;
    }
    notify_disconnection_();
  }
  // +=========================================================================+
  // | [>] notify_disconnection_                                   ( private ) |
  // +=========================================================================+
  void notify_disconnection_() {
    if (!connected_ || disconnected_ || socket_ != INVALID_SOCKET) return;
    disconnected_ = true;
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
  // | [>] retire_                                                 ( private ) |
  // +=========================================================================+
  void retire_() {
    if (retired_ || socket_ != INVALID_SOCKET || receiving_ || sending_ ||
        wake_pending_) return;
    retired_ = true;
    if (on_retirement_) on_retirement_(this);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  // [common] section!
  types::on_client_disconnected_delegate on_disconnection_;
  std::function<void(context*)> on_retirement_;
  SOCKET socket_{INVALID_SOCKET};
  ENty engine_;
  const std::atomic<bool>& stopping_;
  std::mutex engine_mutex_;
  std::unique_ptr<char[]> send_buffer_;
  std::optional<common::reader> send_source_;
  std::list<std::tuple<std::unique_ptr<char[]>, std::size_t,
                       std::optional<common::reader>>> send_queue_;
  const std::size_t send_capacity_;
  std::size_t send_bytes_{0};
  std::size_t send_reserved_{0};
  std::size_t send_size_{0};
  std::size_t send_offset_{0};
  bool send_streaming_{false};
  mutable std::mutex sending_mutex_;
  bool closing_{false};
  bool connected_{false};
  bool engine_stopped_{false};
  bool wake_pending_{false};
  overlapped_wake<ENty> wake_event_{this};
  std::shared_ptr<context> wake_owner_;
  bool disconnected_{false};
  bool receiving_{false};
  bool receive_closed_{false};
  bool processing_receive_{false};
  bool sending_{false};
  bool socket_shutdown_{false};
  bool retired_{false};
  // [overlapped-receive] section!
  std::unique_ptr<char[]> ovr_buf_;
  const std::size_t ovr_capacity_;
  std::size_t ovr_size_{0};
  WSABUF ovr_wsa_{0};
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcpip [windowsTM]                                           ( class ) |
// +---------------------------------------------------------------------------+
// | This specification holds for the WindowsTM server transport.              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
class tcpip {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  using policies_type = policies;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  explicit tcpip(policies_type configuration, FAty create_engine)
      : configuration_{configuration},
        create_engine_{std::move(create_engine)} {
    if (!configuration_.recv_buffer_size ||
        configuration_.recv_buffer_size >
            std::numeric_limits<ULONG>::max()) {
      throw std::runtime_error("Invalid receive buffer size!");
    }
    if (configuration_.worker_count > std::numeric_limits<DWORD>::max()) {
      throw std::runtime_error("Invalid worker count!");
    }
    if (!configuration_.send_buffer_size) {
      throw std::runtime_error("Invalid send buffer size!");
    }
    if (configuration_.listen_backlog < 0) {
      throw std::runtime_error("Invalid listen backlog!");
    }
  }
  tcpip(const tcpip&) = delete;
  tcpip(tcpip&&) noexcept = delete;
  ~tcpip() { stop(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  tcpip& operator=(const tcpip&) = delete;
  tcpip& operator=(tcpip&&) noexcept = delete;
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +=========================================================================+
  void start() {
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      if (starting_ || stopping_ || io_h_ != nullptr) return;
      starting_ = true;
    }
    try {
      in_addr ip_address{};
      if (::InetPtonA(AF_INET, configuration_.ip.c_str(), &ip_address) != 1) {
        throw std::runtime_error("Invalid IPv4 address!");
      }
      uint16_t port_num = parse_port(configuration_.port.c_str());
      std::size_t workers = setup_listener(ip_address, port_num);
      setup_workers(workers);
      setup_accept_pipeline(workers);
    } catch (...) {
      stop_(true);
      throw;
    }
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      starting_ = false;
    }
    lifecycle_cv_.notify_all();
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() { stop_(false); }
  // +=========================================================================+
  // | [>] set_on_connection                                        ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_connection(FNty&& fn) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    ensure_stopped();
    on_connection_ = std::forward<FNty>(fn);
  }
  // +=========================================================================+
  // | [>] set_on_disconnection                                     ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_disconnection(FNty&& fn) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    ensure_stopped();
    on_disconnection_ = std::forward<FNty>(fn);
  }

 private:
  // +=========================================================================+
  // | [>] stop_                                                   ( private ) |
  // +=========================================================================+
  void stop_(bool starting_failure) {
    if (current_transport_ == this) {
      throw std::runtime_error(
          "Transport cannot stop from an I/O worker!");
    }
    HANDLE ioh = nullptr;
    SOCKET listener = INVALID_SOCKET;
    std::vector<std::shared_ptr<context<ENty>>> contexts;
    {
      std::unique_lock<std::mutex> lifecycle_lock(lifecycle_mutex_);
      if (starting_failure) {
        starting_ = false;
        lifecycle_cv_.notify_all();
      } else {
        lifecycle_cv_.wait(lifecycle_lock, [this]() { return !starting_; });
      }
      if (io_h_ == nullptr) return;
      if (stopping_) {
        if (stopping_thread_ == std::this_thread::get_id()) return;
        lifecycle_cv_.wait(lifecycle_lock, [this]() { return !stopping_; });
        return;
      }
      stopping_ = true;
      stopping_thread_ = std::this_thread::get_id();
      ioh = io_h_;
      listener = accept_socket_;
      try {
        contexts.reserve(contexts_.size());
      } catch (...) {
        stopping_ = false;
        stopping_thread_ = {};
        lifecycle_cv_.notify_all();
        throw;
      }
      for (const auto& item : contexts_) contexts.emplace_back(item.second);
      accept_socket_ = INVALID_SOCKET;
    }
    if (listener != INVALID_SOCKET) closesocket(listener);
    for (const auto& ctx : contexts) ctx->stop();
    {
      std::unique_lock<std::mutex> lifecycle_lock(lifecycle_mutex_);
      lifecycle_cv_.wait(lifecycle_lock, [this]() {
        return pending_accepts_ == 0 && contexts_.empty();
      });
    }
    for (std::size_t i = 0; i < workers_.size(); i++) {
      if (!PostQueuedCompletionStatus(ioh, 0, 0, nullptr)) {
        CloseHandle(ioh);
        ioh = nullptr;
        break;
      }
    }
    workers_.clear();
    if (ioh != nullptr) CloseHandle(ioh);
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      io_h_ = nullptr;
      accept_ex_ = nullptr;
      accept_depth_ = 0;
      pending_accepts_ = 0;
      stopping_ = false;
      stopping_thread_ = {};
    }
    lifecycle_cv_.notify_all();
  }
  // +=========================================================================+
  // | [>] parse_port                                              ( private ) |
  // +=========================================================================+
  uint16_t parse_port(const char port[]) const {
    if (!port || !*port) {
      throw std::runtime_error("Invalid port (range from 1 to 65535)!");
    }
    unsigned int port_num = 0;
    const char* end = port + std::strlen(port);
    auto result = std::from_chars(port, end, port_num);
    if (result.ec != std::errc() || result.ptr != end || port_num < 1 ||
        port_num > 65535) {
      throw std::runtime_error("Invalid port (range from 1 to 65535)!");
    }
    return static_cast<uint16_t>(port_num);
  }
  // +=========================================================================+
  // | [>] ensure_stopped                                          ( private ) |
  // +=========================================================================+
  void ensure_stopped() const {
    if (starting_ || stopping_ || io_h_ != nullptr) {
      throw std::runtime_error(
          "Transport callbacks cannot change while active!");
    }
  }
  // +=========================================================================+
  // | [>] setup_listener                                          ( private ) |
  // +=========================================================================+
  std::size_t setup_listener(in_addr ip, uint16_t port_num) {
    std::size_t workers = configuration_.worker_count;
    if (!workers) {
      workers = std::max<std::size_t>(1, std::thread::hardware_concurrency());
    }
    HANDLE ioh = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0,
                                        static_cast<DWORD>(workers));
    if (ioh == NULL) {
      // ((error)) -> Could not create I/O Completion Port!
      throw std::runtime_error("I/O Completion Port could not be created!");
    }
    SOCKET sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                             WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET) {
      // ((error)) -> Could not create socket!
      CloseHandle(ioh);
      throw std::runtime_error("Socket could not be created!");
    }
    sockaddr_in addr = {0};
    addr.sin_addr = ip;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_num);
    if (bind(sock, (const sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
      // ((error)) -> Could not bind socket!
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error("Could not bind socket!");
    }
    if (listen(sock, configuration_.listen_backlog
                         ? configuration_.listen_backlog
                         : SOMAXCONN) == SOCKET_ERROR) {
      // ((error)) -> Could not listen on socket!
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error("Could not listen on socket!");
    }
    GUID acceptex_guid = WSAID_ACCEPTEX;
    DWORD bytes = 0;
    LPFN_ACCEPTEX accept_ex = nullptr;
    if (WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &acceptex_guid,
                 sizeof(acceptex_guid), &accept_ex, sizeof(accept_ex), &bytes,
                 nullptr, nullptr) == SOCKET_ERROR ||
        accept_ex == nullptr) {
      // ((error)) -> Could not load AcceptEx entry point!
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error("AcceptEx entry point could not be loaded!");
    }
    if (!CreateIoCompletionPort((HANDLE)sock, ioh, 0, 0)) {
      // ((error)) -> Could not associate listener socket to IOCP!
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error(
          "Listener socket could not be associated to IOCP!");
    }
    accept_socket_ = sock;
    io_h_ = ioh;
    accept_ex_ = accept_ex;
    return workers;
  }
  // +=========================================================================+
  // | [>] setup_accept_pipeline                                   ( private ) |
  // +=========================================================================+
  void setup_accept_pipeline(std::size_t workers) {
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      accept_depth_ = std::max<std::size_t>(2, workers);
    }
    if (!replenish_accept_pipeline()) {
      // ((error)) -> Could not arm AcceptEx pipeline!
      throw std::runtime_error("AcceptEx pipeline could not be armed!");
    }
  }
  // +=========================================================================+
  // | [>] setup_workers                                           ( private ) |
  // +=========================================================================+
  std::size_t setup_workers(std::size_t number_of_workers) {
    for (std::size_t i = 0; i < number_of_workers; i++) {
      workers_.emplace_back(std::jthread([this]() {
        current_transport_ = this;
        bool stopping = false;
        while (!stopping) {
          ULONG_PTR key = NULL;
          LPOVERLAPPED lpo = NULL;
          DWORD bytes = 0;  // bytes transfered..
          DWORD tout = INFINITE;
          BOOL st = GetQueuedCompletionStatus(io_h_, &bytes, &key, &lpo, tout);
          overlapped_base* ovb = reinterpret_cast<overlapped_base*>(lpo);
          if (st == TRUE && ovb == nullptr) {
            stopping = true;
          } else if (st == TRUE) {
            if (ovb->get_type() == io_type::kWake) {
              // The queued event retains its context until this worker owns it.
              auto ctx = reinterpret_cast<overlapped_wake<ENty>*>(ovb)
                             ->ctx->shared_from_this();
              ctx->wake();
              continue;
            }
            if (ovb->get_type() == io_type::kAccept) {
              auto ova = (overlapped_accept*)ovb;
              handle_accept(ova);
            } else if (ovb->get_type() == io_type::kReceive) {
              auto ovr = (overlapped_receive<ENty>*)ovb;
              handle_receive(ovr->ctx, bytes);
            } else if (ovb->get_type() == io_type::kSend) {
              auto ovs = (overlapped_send<ENty>*)ovb;
              handle_send(ovs, bytes);
            }
          } else if (ovb != nullptr) {
            // If the overlapped operation failed, we need to handle the error
            // based on the type of operation.
            handle_error(ovb);
          } else {
            // If lpo is NULL, it indicates that the completion port is being
            // closed, so we should stop the worker thread.
            stopping = true;
          }
          if (ovb != nullptr) {
            // After handling the overlapped operation, we need to clean up the
            // overlapped structure if it was dynamically allocated.
            handle_overlapped(ovb);
          }
        }
        current_transport_ = nullptr;
      }));
    }
    return workers_.size();
  }
  // +=========================================================================+
  // | [>] get_accept_socket                                       ( private ) |
  // +=========================================================================+
  SOCKET get_accept_socket() {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    if (stopping_) return INVALID_SOCKET;
    return accept_socket_;
  }
  // +=========================================================================+
  // | [>] finish_accept                                           ( private ) |
  // +=========================================================================+
  void finish_accept() {
    bool replenish = false;
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      if (pending_accepts_) pending_accepts_--;
      replenish = !stopping_ && accept_socket_ != INVALID_SOCKET;
    }
    lifecycle_cv_.notify_all();
    if (replenish) replenish_accept_pipeline();
  }
  // +=========================================================================+
  // | [>] register_context                                        ( private ) |
  // +=========================================================================+
  bool register_context(const std::shared_ptr<context<ENty>>& ctx) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    if (stopping_) return false;
    try {
      return contexts_.emplace(ctx.get(), ctx).second;
    } catch (...) {
      return false;
    }
  }
  // +=========================================================================+
  // | [>] retire_context                                          ( private ) |
  // +=========================================================================+
  void retire_context(context<ENty>* ctx) {
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      contexts_.erase(ctx);
    }
    lifecycle_cv_.notify_all();
    replenish_accept_pipeline();
  }
  // +=========================================================================+
  // | [>] handle_accept                                           ( private ) |
  // +=========================================================================+
  void handle_accept(overlapped_accept* ova) {
    SOCKET listener = get_accept_socket();
    if (listener == INVALID_SOCKET) {
      closesocket(ova->socket);
      finish_accept();
      return;
    }
    post_accept(true);
    int result =
        setsockopt(ova->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                   reinterpret_cast<const char*>(&listener), sizeof(listener));
    if (result == SOCKET_ERROR) {
      closesocket(ova->socket);
      finish_accept();
      return;
    }
    ULONG i_mode_flag = 1;
    result = ioctlsocket(ova->socket, FIONBIO, &i_mode_flag);
    if (result != NO_ERROR) {
      closesocket(ova->socket);
      finish_accept();
      return;
    }
    int ndf = 1;
    result = setsockopt(ova->socket, IPPROTO_TCP, TCP_NODELAY,
                        reinterpret_cast<const char*>(&ndf), sizeof(ndf));
    if (result == SOCKET_ERROR) {
      closesocket(ova->socket);
      finish_accept();
      return;
    }
    std::shared_ptr<context<ENty>> ctx;
    try {
      ctx = std::make_shared<context<ENty>>(
          ova->socket, configuration_.recv_buffer_size,
          configuration_.send_buffer_size, create_engine_, stopping_,
          on_disconnection_,
          [this](context<ENty>* context) { retire_context(context); });
    } catch (...) {
      closesocket(ova->socket);
      finish_accept();
      return;
    }
    if (!CreateIoCompletionPort((HANDLE)ova->socket, io_h_, 0, 0)) {
      closesocket(ova->socket);
      ova->socket = INVALID_SOCKET;
      finish_accept();
      return;
    }
    if (!register_context(ctx)) {
      ctx->abort();
      finish_accept();
      return;
    }
    // Let's call user's callback to notify for new connection!
    try {
      on_connection_();
      ctx->connected(io_h_);
    } catch (const std::exception&) {
      ctx->stop();
      finish_accept();
      return;
    } catch (...) {
      ctx->stop();
      finish_accept();
      return;
    }
    // Let's arm next receive operation!
    if (!ctx->arm_next_receive_operation()) {
      ctx->stop();
      finish_accept();
      return;
    }
    finish_accept();
  }
  // +=========================================================================+
  // | [>] handle_receive                                          ( private ) |
  // +=========================================================================+
  void handle_receive(std::shared_ptr<context<ENty>> ctx,
                      DWORD bytes_received) {
    try {
      if (!ctx->receive_completed(bytes_received)) return;
      if (!bytes_received) {
        ctx->stop();
        return;
      }
      if (!ctx->arm_next_receive_operation()) ctx->stop();
    } catch (...) {
      ctx->abort();
      ctx->stop();
    }
  }
  // +=========================================================================+
  // | [>] handle_send                                             ( private ) |
  // +=========================================================================+
  void handle_send(overlapped_send<ENty>* ovs, DWORD bytes_sent) {
    try {
      ovs->ctx->send_completed(bytes_sent);
    } catch (...) {
      ovs->ctx->abort();
      ovs->ctx->stop();
    }
  }
  // +=========================================================================+
  // | [>] handle_error                                            ( private ) |
  // +=========================================================================+
  void handle_error(overlapped_base* ovb) {
    switch (ovb->get_type()) {
      case io_type::kAccept: {
        auto ova = reinterpret_cast<overlapped_accept*>(ovb);
        closesocket(ova->socket);
        finish_accept();
        break;
      }
      case io_type::kReceive:
        reinterpret_cast<overlapped_receive<ENty>*>(ovb)->ctx->receive_failed();
        break;
      case io_type::kSend:
        reinterpret_cast<overlapped_send<ENty>*>(ovb)->ctx->send_failed();
        break;
      case io_type::kWake:
        break;
    }
  }
  // +=========================================================================+
  // | [>] handle_overlapped                                       ( private ) |
  // +=========================================================================+
  void handle_overlapped(overlapped_base* ovb) {
    switch (ovb->get_type()) {
      case io_type::kAccept:
        delete reinterpret_cast<overlapped_accept*>(ovb);
        break;
      case io_type::kReceive:
        delete reinterpret_cast<overlapped_receive<ENty>*>(ovb);
        break;
      case io_type::kSend:
        delete reinterpret_cast<overlapped_send<ENty>*>(ovb);
        break;
      case io_type::kWake:
        break;
    }
  }
  // +=========================================================================+
  // | [>] replenish_accept_pipeline                              ( private )  |
  // +=========================================================================+
  bool replenish_accept_pipeline() {
    for (;;) {
      {
        std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
        if (stopping_ || accept_socket_ == INVALID_SOCKET) return false;
        if (pending_accepts_ >= accept_depth_) return true;
      }
      if (!post_accept(false)) return false;
    }
  }
  // +=========================================================================+
  // | [>] post_accept                                             ( private ) |
  // +=========================================================================+
  bool post_accept(bool replacement) {
    SOCKET soc = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                            WSA_FLAG_OVERLAPPED);
    if (soc == INVALID_SOCKET) return false;
    overlapped_accept* ova = new (std::nothrow) overlapped_accept(soc);
    if (!ova) {
      closesocket(soc);
      return false;
    }
    DWORD received = 0;
    std::unique_lock<std::mutex> lifecycle_lock(lifecycle_mutex_);
    if (stopping_ || accept_socket_ == INVALID_SOCKET) {
      lifecycle_lock.unlock();
      closesocket(soc);
      delete ova;
      return false;
    }
    if (!replacement && pending_accepts_ >= accept_depth_) {
      lifecycle_lock.unlock();
      closesocket(soc);
      delete ova;
      return true;
    }
    pending_accepts_++;
    BOOL accepted_connection =
        accept_ex_(accept_socket_, ova->socket, ova->addresses, 0,
                   kAcceptAddressBytes, kAcceptAddressBytes, &received, ova);
    if (accepted_connection == FALSE && WSAGetLastError() != ERROR_IO_PENDING) {
      // ((error)) -> Could not post AcceptEx operation!
      pending_accepts_--;
      lifecycle_lock.unlock();
      lifecycle_cv_.notify_all();
      closesocket(soc);
      delete ova;
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +=========================================================================+
  const policies_type configuration_;
  const FAty create_engine_;
  network::detail::environment environment_;
  HANDLE io_h_ = nullptr;
  SOCKET accept_socket_ = INVALID_SOCKET;
  LPFN_ACCEPTEX accept_ex_ = nullptr;
  std::mutex lifecycle_mutex_;
  std::condition_variable lifecycle_cv_;
  std::size_t accept_depth_ = 0;
  std::size_t pending_accepts_ = 0;
  bool starting_ = false;
  std::atomic<bool> stopping_{false};
  std::thread::id stopping_thread_{};
  std::vector<std::jthread> workers_;
  inline static thread_local const tcpip* current_transport_{nullptr};
  std::unordered_map<context<ENty>*, std::shared_ptr<context<ENty>>> contexts_;
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
};
}  // namespace martianlabs::doba::transport::server

#endif
