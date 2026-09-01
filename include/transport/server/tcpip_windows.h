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

#include <array>
#include <atomic>
#include <charconv>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <new>
#include <span>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "network/environment.h"
#include "platform.h"
#include "protocol/deserialization.h"
#include "protocol/serialization.h"
#include "transport/server/completion_mailbox.h"
#include "transport/server/connection_identity.h"
#include "transport/server/deferred_response.h"
#include "transport/server/response_scheduler.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] CONSTANTs                                                  ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
static constexpr DWORD kAcceptAddressBytes =
    static_cast<DWORD>(sizeof(sockaddr_storage) + 16);
static constexpr inline std::size_t kReceiveBufferSz = 8192;
static constexpr inline std::size_t kSendBufferMaxSz = 65536;
static constexpr inline std::size_t kSendChunkSz = 8192;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] io_type                                                ( enum-class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class io_type : uint8_t { kAccept, kSend, kReceive };
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] FORWARDs                                                   ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty,
          template <typename, typename> class DEty>
struct context;
// +---------------------------------------------------------------------------+
// | [>] overlapped_base                                            ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct overlapped_base : OVERLAPPED {
  overlapped_base(io_type in_type) : OVERLAPPED{}, type{in_type} {}
  io_type get_type() const { return type; }

 private:
  const io_type type;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] overlapped_accept                                          ( struct ) |
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
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty,
          template <typename, typename> class DEty>
struct overlapped_receive : overlapped_base {
  overlapped_receive(std::shared_ptr<context<RQty, RSty, DEty>> context)
      : overlapped_base(io_type::kReceive), ctx{context} {}
  std::shared_ptr<context<RQty, RSty, DEty>> ctx;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] overlapped_send                                            ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty,
          template <typename, typename> class DEty>
struct overlapped_send : overlapped_base {
  overlapped_send(std::shared_ptr<context<RQty, RSty, DEty>> context)
      : overlapped_base(io_type::kSend), ctx{context} {}
  std::shared_ptr<context<RQty, RSty, DEty>> ctx;
};
// +---------------------------------------------------------------------------+
// | [>] context [windowsTM]                                         ( class ) |
// +---------------------------------------------------------------------------+
// | This specification holds for the WindowsTM server transport context.      |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used.                                              |
// |   RSty - response being used.                                             |
// |   DEty - decoder (deserializer) being used.                               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty,
          template <typename, typename> class DEty>
struct context
    : public std::enable_shared_from_this<context<RQty, RSty, DEty>> {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  context(uint64_t connection_key, SOCKET in_socket,
          types::on_client_disconnected_delegate on_disconnection,
          std::function<void(uint64_t)> on_retirement = {})
      : on_disconnection_{on_disconnection},
        on_retirement_{on_retirement},
        socket_{in_socket},
        connection_key_{connection_key} {}
  context(const context&) = delete;
  context(context&&) noexcept = delete;
  ~context() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  context& operator=(const context&) = delete;
  context& operator=(context&&) noexcept = delete;
  // +=========================================================================+
  // | [>] get_connection_key                                       ( public ) |
  // +=========================================================================+
  uint64_t get_connection_key() const { return connection_key_; }
  // +=========================================================================+
  // | [>] reserve_response                                         ( public ) |
  // +=========================================================================+
  uint64_t reserve_response() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    return scheduler_.reserve();
  }
  // +=========================================================================+
  // | [>] accumulate                                               ( public ) |
  // +=========================================================================+
  std::size_t accumulate(std::size_t bytes_received) {
    std::size_t bytes_accumulated =
        decoder_.accumulate(ovr_wsa_.buf + receive_offset_, bytes_received);
    receive_offset_ += bytes_accumulated;
    return bytes_accumulated;
  }
  // +=========================================================================+
  // | [>] deserialize                                              ( public ) |
  // +=========================================================================+
  protocol::deserialization_result<RQty> deserialize() {
    return decoder_.deserialize();
  }
  // | [>] enqueue_response                                         ( public ) |
  // +=========================================================================+
  bool enqueue_response(
      std::unique_ptr<protocol::serialization_result> response) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    return enqueue_response_(std::move(response));
  }
  // +=========================================================================+
  // | [>] complete_response                                        ( public ) |
  // +=========================================================================+
  bool complete_response(
      uint64_t position,
      std::unique_ptr<protocol::serialization_result> response) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    if (socket_ == INVALID_SOCKET) return false;
    if (!scheduler_.complete(position, std::move(response))) return false;
    arm_next_send_operation_();
    resume_receive_operation_();
    return true;
  }
  // +=========================================================================+
  // | [>] enqueue_error_response                                   ( public ) |
  // +=========================================================================+
  bool enqueue_error_response(
      std::unique_ptr<protocol::serialization_result> response) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    if (!enqueue_response_(std::move(response))) {
      fail_response_();
      return false;
    }
    closing_ = true;
    arm_next_send_operation_();
    return true;
  }
  // +=========================================================================+
  // | [>] fail_response                                            ( public ) |
  // +=========================================================================+
  void fail_response() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    try {
      fail_response_();
    } catch (...) {
      abort_();
    }
  }
  // +=========================================================================+
  // | [>] check_sending_buffer_and_arm                             ( public ) |
  // +=========================================================================+
  void check_sending_buffer_and_arm(std::size_t bytes_sent) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    sending_ = false;
    if (socket_ == INVALID_SOCKET) {
      sending_buffer_.clear();
      sending_offset_ = 0;
      retire_();
      return;
    }
    if (!bytes_sent || sending_offset_ > sending_buffer_.size() ||
        bytes_sent > sending_buffer_.size() - sending_offset_) {
      abort_();
      return;
    }
    sending_offset_ += bytes_sent;
    arm_next_send_operation_();
    resume_receive_operation_();
  }
  // +=========================================================================+
  // | [>] arm_next_receive_operation                               ( public ) |
  // +=========================================================================+
  bool arm_next_receive_operation() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    return arm_next_receive_operation_();
  }
  // +=========================================================================+
  // | [>] arm_next_send_operation                                  ( public ) |
  // +=========================================================================+
  void arm_next_send_operation() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    arm_next_send_operation_();
  }
  // +=========================================================================+
  // | [>] receive_completed                                        ( public ) |
  // +=========================================================================+
  bool receive_completed() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    receiving_ = false;
    if (socket_ == INVALID_SOCKET) {
      retire_();
      return false;
    }
    return !closing_;
  }
  // +=========================================================================+
  // | [>] receive_failed                                           ( public ) |
  // +=========================================================================+
  void receive_failed() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    receiving_ = false;
    abort_();
  }
  // +=========================================================================+
  // | [>] send_failed                                              ( public ) |
  // +=========================================================================+
  void send_failed() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    sending_ = false;
    abort_();
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
  void connected() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    connected_ = true;
    notify_disconnection_();
  }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +=========================================================================+
  void close() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    closing_ = true;
    arm_next_send_operation_();
  }

 private:
  // +=========================================================================+
  // | [>] enqueue_response_                                       ( private ) |
  // +=========================================================================+
  bool enqueue_response_(
      std::unique_ptr<protocol::serialization_result> response) {
    if (!response || socket_ == INVALID_SOCKET) return false;
    scheduler_.push_ready(std::move(response));
    return true;
  }
  // +=========================================================================+
  // | [>] fail_response_                                          ( private ) |
  // +=========================================================================+
  void fail_response_() {
    closing_ = true;
    arm_next_send_operation_();
  }
  // +=========================================================================+
  // | [>] arm_next_receive_operation_                             ( private ) |
  // +=========================================================================+
  bool arm_next_receive_operation_() {
    if (closing_) return false;
    if (scheduler_.saturated()) {
      receive_paused_ = true;
      return true;
    }
    receive_paused_ = false;
    if (receive_()) return true;
    abort_();
    return false;
  }
  // +=========================================================================+
  // | [>] resume_receive_operation_                               ( private ) |
  // +=========================================================================+
  void resume_receive_operation_() {
    if (!receive_paused_ || closing_ || scheduler_.saturated()) return;
    arm_next_receive_operation_();
  }
  // +=========================================================================+
  // | [>] arm_next_send_operation_                                ( private ) |
  // +=========================================================================+
  void arm_next_send_operation_() {
    if (closing_) cleanup_resources_();
    if (socket_ == INVALID_SOCKET) return;
    if (sending_) return;
    if (sending_offset_ == sending_buffer_.size()) {
      sending_buffer_.clear();
      sending_offset_ = 0;
      while (!scheduler_.empty()) {
        if (sending_buffer_.size() >= kSendBufferMaxSz) break;
        detail::response_data& data = scheduler_.front();
        if (!data.ready()) break;
        if (!data.prefix_written) {
          sending_buffer_.append(data.response->prefix);
          data.prefix_written = true;
          continue;
        }
        auto& source = data.response->source;
        if (source.has_value() && !source->eof()) {
          // Let's pour, at most, the remaining outgoing buffer capacity!
          std::byte chunk[kSendChunkSz];
          std::size_t room = kSendBufferMaxSz - sending_buffer_.size();
          if (room > kSendChunkSz) room = kSendChunkSz;
          std::size_t read = source->read(std::span<std::byte>(chunk, room));
          if (source->failed()) {
            abort_();
            return;
          }
          if (!read) {
            // Sources are synchronous readers, so a zero-byte read means there
            // is nothing else to pour: let's retire this response right below!
            source.reset();
            continue;
          }
          sending_buffer_.append(reinterpret_cast<const char*>(chunk), read);
          continue;
        }
        scheduler_.pop_front();
      }
    }
    if (sending_buffer_.empty()) {
      cleanup_resources_();
      return;
    }
    if (!send_()) {
      abort_();
      return;
    }
    sending_ = true;
  }
  // +=========================================================================+
  // | [>] receive_                                                ( private ) |
  // +=========================================================================+
  bool receive_() {
    DWORD f = 0, r = 0;
    overlapped_receive<RQty, RSty, DEty>* ovr =
        new (std::nothrow)
            overlapped_receive<RQty, RSty, DEty>(this->shared_from_this());
    if (!ovr) return false;
    receive_offset_ = 0;
    ovr_wsa_.buf = ovr_buf_;
    ovr_wsa_.len = kReceiveBufferSz;
    int res = WSARecv(socket_, &ovr_wsa_, 1, &r, &f, ovr, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      delete ovr;
      return false;
    }
    receiving_ = true;
    return true;
  }
  // +=========================================================================+
  // | [>] send_                                                   ( private ) |
  // +=========================================================================+
  bool send_() {
    DWORD f = 0, snt = 0;
    overlapped_send<RQty, RSty, DEty>* ovs =
        new (std::nothrow)
            overlapped_send<RQty, RSty, DEty>(this->shared_from_this());
    if (!ovs) return false;
    ovs_wsa_.buf = sending_buffer_.data() + sending_offset_;
    ovs_wsa_.len =
        static_cast<ULONG>(sending_buffer_.size() - sending_offset_);
    int res = WSASend(socket_, &ovs_wsa_, 1, &snt, f, ovs, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      delete ovs;
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] cleanup_resources_                                      ( private ) |
  // +=========================================================================+
  void cleanup_resources_() {
    if (!closing_) return;
    if (sending_ || sending_offset_ != sending_buffer_.size() ||
        !scheduler_.empty()) {
      return;
    }
    sending_buffer_.clear();
    sending_offset_ = 0;
    close_socket_();
    retire_();
  }
  // +=========================================================================+
  // | [>] abort_                                                  ( private ) |
  // +=========================================================================+
  void abort_() {
    closing_ = true;
    scheduler_.clear();
    if (!sending_) {
      sending_buffer_.clear();
      sending_offset_ = 0;
    }
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
    if (retired_ || socket_ != INVALID_SOCKET || receiving_ || sending_) return;
    retired_ = true;
    if (on_retirement_) on_retirement_(connection_key_);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  // [common] section!
  types::on_client_disconnected_delegate on_disconnection_;
  std::function<void(uint64_t)> on_retirement_;
  SOCKET socket_{INVALID_SOCKET};
  mutable std::mutex sending_mutex_;
  bool closing_{false};
  bool connected_{false};
  bool disconnected_{false};
  bool receiving_{false};
  bool receive_paused_{false};
  bool sending_{false};
  bool retired_{false};
  // [decoder] section!
  DEty<RQty, RSty> decoder_{};
  // [overlapped-receive] section!
  CHAR ovr_buf_[kReceiveBufferSz]{0};
  WSABUF ovr_wsa_{0};
  std::size_t receive_offset_{0};
  // [overlapped-send] section!
  std::string sending_buffer_;
  std::size_t sending_offset_{0};
  WSABUF ovs_wsa_{0};
  // [responses] section!
  detail::response_scheduler scheduler_;
  const uint64_t connection_key_;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcpip [windowsTM]                                           ( class ) |
// +---------------------------------------------------------------------------+
// | This specification holds for the WindowsTM server transport.              |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used.                                              |
// |   RSty - response being used.                                             |
// |   DEty - decoder (deserializer) being used.                               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty,
          template <typename, typename> class DEty>
class tcpip {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  tcpip() = default;
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
  void start(const char port[]) {
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      if (starting_ || stopping_ || io_h_ != nullptr) return;
      starting_ = true;
    }
    try {
      uint16_t port_num = parse_port(port);
      std::size_t workers = setup_listener(port_num);
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
  // | [>] set_on_request                                           ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_request(FNty&& fn) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    ensure_stopped();
    using request_context = detail::deferred_response_context<
        context<RQty, RSty, DEty>, detail::iocp_completion_mailbox>;
    if constexpr (std::is_invocable_v<
                      FNty, const std::shared_ptr<RQty>&, RSty&,
                      request_context&>) {
      on_request_ = std::forward<FNty>(fn);
    } else {
      static_assert(std::is_invocable_v<FNty, const RQty&, RSty&>);
      on_request_ = [callback = std::forward<FNty>(fn)](
                        const std::shared_ptr<RQty>& request,
                        RSty& response, request_context&) mutable {
        callback(*request, response);
      };
    }
  }
  // +=========================================================================+
  // | [>] set_on_bad_request                                       ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_bad_request(FNty&& fn) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    ensure_stopped();
    on_bad_request_ = std::forward<FNty>(fn);
  }
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
    HANDLE ioh = nullptr;
    SOCKET listener = INVALID_SOCKET;
    std::vector<std::shared_ptr<context<RQty, RSty, DEty>>> contexts;
    {
      std::unique_lock<std::mutex> lifecycle_lock(lifecycle_mutex_);
      if (starting_failure) {
        starting_ = false;
        lifecycle_cv_.notify_all();
      } else {
        lifecycle_cv_.wait(lifecycle_lock, [this]() { return !starting_; });
      }
      if (io_h_ == nullptr) return;
      for (const auto& worker : workers_) {
        if (worker.get_id() == std::this_thread::get_id()) {
          throw std::runtime_error("Transport cannot stop from an I/O worker!");
        }
      }
      if (stopping_) {
        if (stopping_thread_ == std::this_thread::get_id()) return;
        lifecycle_cv_.wait(lifecycle_lock,
                           [this]() { return !stopping_; });
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
    completion_mailbox_.close();
    if (listener != INVALID_SOCKET) closesocket(listener);
    for (const auto& ctx : contexts) ctx->abort();
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
  std::size_t setup_listener(uint16_t port_num) {
    std::size_t workers =
        std::max<std::size_t>(1, std::thread::hardware_concurrency());
    HANDLE ioh = CreateIoCompletionPort(
        INVALID_HANDLE_VALUE, 0, 0, static_cast<DWORD>(workers));
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
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_num);
    if (bind(sock, (const sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
      // ((error)) -> Could not bind socket!
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error("Could not bind socket!");
    }
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
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
    completion_mailbox_.open(ioh);
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
        bool stopping = false;
        while (!stopping) {
          ULONG_PTR key = NULL;
          LPOVERLAPPED lpo = NULL;
          DWORD bytes = 0;  // bytes transfered..
          DWORD tout = INFINITE;
          BOOL st = GetQueuedCompletionStatus(io_h_, &bytes, &key, &lpo, tout);
          if (st == TRUE && key == detail::kResponseCompletionKey) {
            std::unique_ptr<detail::iocp_response_completion> completion{
                reinterpret_cast<detail::iocp_response_completion*>(lpo)};
            handle_response_completion(std::move(completion->completion));
            continue;
          }
          overlapped_base* ovb = reinterpret_cast<overlapped_base*>(lpo);
          if (st == TRUE && ovb == nullptr) {
            stopping = true;
          } else if (st == TRUE) {
            if (ovb->get_type() == io_type::kAccept) {
              auto ova = (overlapped_accept*)ovb;
              handle_accept(ova);
            } else if (ovb->get_type() == io_type::kReceive) {
              auto ovr = (overlapped_receive<RQty, RSty, DEty>*)ovb;
              handle_receive(ovr->ctx, bytes);
            } else if (ovb->get_type() == io_type::kSend) {
              auto ovs = (overlapped_send<RQty, RSty, DEty>*)ovb;
              handle_send(ovs->ctx, bytes);
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
  bool register_context(
      const std::shared_ptr<context<RQty, RSty, DEty>>& ctx) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    if (stopping_) return false;
    try {
      return contexts_.emplace(ctx->get_connection_key(), ctx).second;
    } catch (...) {
      return false;
    }
  }
  // +=========================================================================+
  // | [>] retire_context                                          ( private ) |
  // +=========================================================================+
  void retire_context(uint64_t connection_key) {
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      contexts_.erase(connection_key);
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
    int result = setsockopt(ova->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                            reinterpret_cast<const char*>(&listener),
                            sizeof(listener));
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
    std::shared_ptr<context<RQty, RSty, DEty>> ctx;
    try {
      uint64_t connection_key = connection_identity_.acquire();
      ctx = std::make_shared<context<RQty, RSty, DEty>>(
          connection_key, ova->socket, on_disconnection_,
          [this](uint64_t retired_connection_key) {
            retire_context(retired_connection_key);
          });
    } catch (...) {
      closesocket(ova->socket);
      finish_accept();
      return;
    }
    ULONG_PTR key = reinterpret_cast<ULONG_PTR>(ctx.get());
    if (!CreateIoCompletionPort((HANDLE)ova->socket, io_h_, key, 0)) {
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
    } catch (const std::exception&) {
      ctx->close();
      finish_accept();
      return;
    } catch (...) {
      ctx->close();
      finish_accept();
      return;
    }
    ctx->connected();
    // Let's arm next receive operation!
    if (!ctx->arm_next_receive_operation()) {
      finish_accept();
      return;
    }
    finish_accept();
  }
  // +=========================================================================+
  // | [>] enqueue_error_response                                  ( private ) |
  // +=========================================================================+
  void enqueue_error_response(std::shared_ptr<context<RQty, RSty, DEty>> ctx,
                               int reason_code, std::string_view reason) {
    try {
      RSty response;
      on_bad_request_(reason_code, reason, response);
      auto serialized = response.serialize();
      if (!serialized ||
          !ctx->enqueue_error_response(std::move(serialized))) {
        ctx->fail_response();
      }
    } catch (...) {
      ctx->fail_response();
    }
  }
  // +=========================================================================+
  // | [>] handle_receive                                          ( private ) |
  // +=========================================================================+
  void handle_receive(std::shared_ptr<context<RQty, RSty, DEty>> ctx,
                      DWORD bytes_received) {
    if (!ctx->receive_completed()) return;
    try {
      if (!bytes_received) {
        ctx->close();
        return;
      }
      std::size_t bytes_accumulated = 0;
      bool keep_decoding_requests = true;
      do {
        // Let's accumulate the received bytes into the decoder!
        bytes_accumulated = ctx->accumulate(bytes_received);
        if (bytes_accumulated == 0 || bytes_accumulated > bytes_received) {
          ctx->close();
          return;
        }
        // Let's try to deserialize some requests!
        do {
          using namespace protocol;
          deserialization_result<RQty> result = ctx->deserialize();
          if (result.code == deserialization_status::kMoreBytesNeeded) {
            // The protocol may need some bytes on the wire before it can go
            // on (their meaning is opaque here); they are written ahead of any
            // later response.
            if (!result.interim.empty()) {
              auto interim = std::make_unique<protocol::serialization_result>();
              interim->prefix.assign(result.interim);
              if (!ctx->enqueue_response(std::move(interim))) {
                ctx->fail_response();
                return;
              }
            }
            break;
          } else if (result.code == deserialization_status::kInvalidSource) {
            enqueue_error_response(ctx, result.reason,
                                   "Invalid request content!");
            return;
          } else if (result.code == deserialization_status::kSucceeded) {
            if (result.request == nullptr) {
              enqueue_error_response(ctx, 0, "Decoder error!");
              return;
            }
            bool close_channel =
                result.channel == protocol::channel_intent::kClose;
            if (close_channel) {
              keep_decoding_requests = false;
            }
            try {
              // Let's call user handler!
              RSty response;
              detail::deferred_response_context<
                  context<RQty, RSty, DEty>,
                  detail::iocp_completion_mailbox> dispatch{
                      *ctx, completion_mailbox_};
              on_request_(result.request, response, dispatch);
              if (!dispatch.deferred()) {
                auto serialized = response.serialize();
                if (!serialized ||
                    !ctx->enqueue_response(std::move(serialized))) {
                  ctx->fail_response();
                  return;
                }
              }
              if (close_channel) ctx->close();
            } catch (const std::exception& ex) {
              // Reuses the same bad-request channel as decoder rejections;
              // the reason code below mirrors
              // protocol::http::v11::rejection_reason::kHandlerError (7),
              // kept as a raw value here so the transport stays http-agnostic.
              enqueue_error_response(ctx, 7, ex.what());
              return;
            } catch (...) {
              enqueue_error_response(ctx, 7, "Request handler error!");
              return;
            }
          }
        } while (keep_decoding_requests);
        bytes_received -= static_cast<DWORD>(bytes_accumulated);
      } while (keep_decoding_requests && bytes_received > 0);
      ctx->arm_next_send_operation();
      ctx->arm_next_receive_operation();
    } catch (...) {
      ctx->abort();
    }
  }
  // +=========================================================================+
  // | [>] handle_send                                             ( private ) |
  // +=========================================================================+
  void handle_send(std::shared_ptr<context<RQty, RSty, DEty>> ctx,
                    DWORD bytes_sent) {
    try {
      ctx->check_sending_buffer_and_arm(bytes_sent);
    } catch (...) {
      ctx->abort();
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
        reinterpret_cast<overlapped_receive<RQty, RSty, DEty>*>(ovb)
            ->ctx->receive_failed();
        break;
      case io_type::kSend:
        reinterpret_cast<overlapped_send<RQty, RSty, DEty>*>(ovb)
            ->ctx->send_failed();
        break;
    }
  }
  // +=========================================================================+
  // | [>] handle_response_completion                             ( private ) |
  // +=========================================================================+
  void handle_response_completion(detail::response_completion completion) {
    std::shared_ptr<context<RQty, RSty, DEty>> ctx;
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      auto item = contexts_.find(completion.connection_key);
      if (item == contexts_.end()) return;
      ctx = item->second;
    }
    if (!completion.response) {
      ctx->fail_response();
      return;
    }
    ctx->complete_response(completion.position,
                           std::move(completion.response));
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
        delete reinterpret_cast<overlapped_receive<RQty, RSty, DEty>*>(ovb);
        break;
      case io_type::kSend:
        delete reinterpret_cast<overlapped_send<RQty, RSty, DEty>*>(ovb);
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
  network::detail::environment environment_;
  HANDLE io_h_ = nullptr;
  SOCKET accept_socket_ = INVALID_SOCKET;
  LPFN_ACCEPTEX accept_ex_ = nullptr;
  std::mutex lifecycle_mutex_;
  std::condition_variable lifecycle_cv_;
  std::size_t accept_depth_ = 0;
  std::size_t pending_accepts_ = 0;
  bool starting_ = false;
  bool stopping_ = false;
  std::thread::id stopping_thread_{};
  std::vector<std::jthread> workers_;
  std::unordered_map<uint64_t, std::shared_ptr<context<RQty, RSty, DEty>>>
      contexts_;
  types::on_request_dispatch_delegate<
      RQty, RSty,
      detail::deferred_response_context<
          context<RQty, RSty, DEty>,
          detail::iocp_completion_mailbox>> on_request_;
  types::on_bad_request_delegate<RSty> on_bad_request_;
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
  detail::connection_identity connection_identity_;
  detail::iocp_completion_mailbox completion_mailbox_;
};
}  // namespace martianlabs::doba::transport::server

#endif
