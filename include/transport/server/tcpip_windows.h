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
#include <deque>
#include <functional>
#include <mutex>
#include <new>
#include <span>
#include <unordered_map>

#include "network/environment.h"
#include "platform.h"
#include "protocol/deserialization.h"
#include "protocol/serialization.h"

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
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] response_data                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct response_data {
  std::unique_ptr<protocol::serialization_result> response;
  bool prefix_written{false};
};
// /////////////////////////////////////////////////////////////////////////////
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
  context(SOCKET in_socket,
          types::on_client_disconnected_delegate on_disconnection,
          std::function<void(context*)> on_retirement = {})
      : socket_{in_socket},
        on_disconnection_{on_disconnection},
        on_retirement_{on_retirement} {}
  context(const context&) = delete;
  context(context&&) noexcept = delete;
  ~context() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  context& operator=(const context&) = delete;
  context& operator=(context&&) noexcept = delete;
  // +=========================================================================+
  // | [>] accumulate                                               ( public ) |
  // +=========================================================================+
  std::size_t accumulate(std::size_t bytes_received) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    std::size_t bytes_accumulated =
        decoder_.accumulate(ovr_wsa_.buf + receive_offset_, bytes_received);
    receive_offset_ += bytes_accumulated;
    return bytes_accumulated;
  }
  // +=========================================================================+
  // | [>] deserialize                                              ( public ) |
  // +=========================================================================+
  protocol::deserialization_result<RQty> deserialize() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    return decoder_.deserialize();
  }
  // +=========================================================================+
  // | [>] get_next_response_id                                     ( public ) |
  // +=========================================================================+
  uint64_t get_next_response_id() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    return get_next_rid_();
  }
  // +=========================================================================+
  // | [>] enqueue_response                                         ( public ) |
  // +=========================================================================+
  void enqueue_response(
      std::unique_ptr<protocol::serialization_result> response,
      uint64_t response_id) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    enqueue_response_(std::move(response), response_id);
  }
  // +=========================================================================+
  // | [>] enqueue_error_response                                   ( public ) |
  // +=========================================================================+
  void enqueue_error_response(
      std::unique_ptr<protocol::serialization_result> response,
      uint64_t response_id) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    if (!enqueue_response_(std::move(response), response_id)) {
      fail_response_(response_id);
      return;
    }
    responses_.erase(responses_.begin() +
                         (response_id - expected_response_id_) + 1,
                     responses_.end());
    closing_ = true;
    arm_next_send_operation_();
  }
  // +=========================================================================+
  // | [>] fail_response                                            ( public ) |
  // +=========================================================================+
  void fail_response(uint64_t response_id) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    try {
      fail_response_(response_id);
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
  }
  // +=========================================================================+
  // | [>] arm_next_receive_operation                               ( public ) |
  // +=========================================================================+
  bool arm_next_receive_operation() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    if (closing_) return false;
    if (receive_()) return true;
    abort_();
    return false;
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
      std::unique_ptr<protocol::serialization_result> response,
      uint64_t response_id) {
    if (!response || response_id < expected_response_id_) return false;
    uint64_t offset = response_id - expected_response_id_;
    if (offset >= responses_.size()) return false;
    response_data& rdata = responses_[static_cast<std::size_t>(offset)];
    if (rdata.response) return false;
    rdata.response = std::move(response);
    return true;
  }
  // +=========================================================================+
  // | [>] get_next_rid_                                           ( private ) |
  // +=========================================================================+
  uint64_t get_next_rid_() {
    uint64_t response_id = next_response_id_++;
    if (!closing_) responses_.emplace_back();
    return response_id;
  }
  // +=========================================================================+
  // | [>] fail_response_                                          ( private ) |
  // +=========================================================================+
  void fail_response_(uint64_t response_id) {
    if (response_id < expected_response_id_) return;
    uint64_t offset = response_id - expected_response_id_;
    if (offset >= responses_.size()) return;
    auto itr = responses_.begin() + static_cast<std::size_t>(offset);
    if (itr->response) return;
    responses_.erase(itr, responses_.end());
    closing_ = true;
    arm_next_send_operation_();
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
      auto itr = responses_.begin();
      while (itr != responses_.end()) {
        if (!itr->response) break;
        if (sending_buffer_.size() >= kSendBufferMaxSz) break;
        if (!itr->prefix_written) {
          sending_buffer_.append(itr->response->prefix);
          itr->prefix_written = true;
          continue;
        }
        auto& source = itr->response->source;
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
        expected_response_id_++;
        itr = responses_.erase(itr);
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
        !responses_.empty()) {
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
    responses_.clear();
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
    if (on_retirement_) on_retirement_(this);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  // [common] section!
  types::on_client_disconnected_delegate on_disconnection_;
  std::function<void(context*)> on_retirement_;
  SOCKET socket_{INVALID_SOCKET};
  mutable std::mutex sending_mutex_;
  bool closing_{false};
  bool connected_{false};
  bool disconnected_{false};
  bool receiving_{false};
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
  std::deque<response_data> responses_;
  uint64_t expected_response_id_{0};
  uint64_t next_response_id_{0};
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
    on_request_ = std::forward<FNty>(fn);
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
      return contexts_.emplace(ctx.get(), ctx).second;
    } catch (...) {
      return false;
    }
  }
  // +=========================================================================+
  // | [>] retire_context                                          ( private ) |
  // +=========================================================================+
  void retire_context(context<RQty, RSty, DEty>* ctx) {
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
      ctx = std::make_shared<context<RQty, RSty, DEty>>(
          ova->socket, on_disconnection_,
          [this](context<RQty, RSty, DEty>* context) {
            retire_context(context);
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
                               uint64_t response_id, int reason_code,
                               std::string_view reason) {
    try {
      std::shared_ptr<RSty> response = std::make_shared<RSty>();
      on_bad_request_(reason_code, reason, response);
      ctx->enqueue_error_response(std::move(response->serialize()),
                                  response_id);
    } catch (...) {
      ctx->fail_response(response_id);
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
      std::thread::id this_thread_id = std::this_thread::get_id();
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
            // on (their meaning is opaque here); they take their own response
            // slot so they are written ahead of any later response.
            if (!result.interim.empty()) {
              auto interim = std::make_unique<protocol::serialization_result>();
              interim->prefix.assign(result.interim);
              ctx->enqueue_response(std::move(interim),
                                    ctx->get_next_response_id());
            }
            break;
          } else if (result.code == deserialization_status::kInvalidSource) {
            uint64_t response_id = ctx->get_next_response_id();
            enqueue_error_response(ctx, response_id, result.reason,
                                   "Invalid request content!");
            return;
          } else if (result.code == deserialization_status::kSucceeded) {
            if (result.request == nullptr) {
              uint64_t response_id = ctx->get_next_response_id();
              enqueue_error_response(ctx, response_id, 0, "Decoder error!");
              return;
            }
            std::shared_ptr<RSty> response = std::make_shared<RSty>();
            uint64_t this_response_id = ctx->get_next_response_id();
            if (result.channel == protocol::channel_intent::kClose) {
              keep_decoding_requests = false;
              ctx->close();
            }
            try {
              // Let's call user handler!
              on_request_(result.request, response,
                          [context = ctx, this_response_id,
                           this_thread_id](std::shared_ptr<RSty> response) {
                            if (!response) {
                              context->fail_response(this_response_id);
                              return;
                            }
                            try {
                              auto serialized = response->serialize();
                              if (!serialized) {
                                context->fail_response(this_response_id);
                                return;
                              }
                              context->enqueue_response(std::move(serialized),
                                                        this_response_id);
                              // Let's arm next send operation only from a
                              // different thread (delayed operation)!
                              if (std::this_thread::get_id() !=
                                  this_thread_id) {
                                context->arm_next_send_operation();
                              }
                            } catch (const std::exception&) {
                              context->fail_response(this_response_id);
                            } catch (...) {
                              context->fail_response(this_response_id);
                            }
                          });
            } catch (const std::exception& ex) {
              // Reuses the same bad-request channel as decoder rejections;
              // the reason code below mirrors
              // protocol::http::v11::rejection_reason::kHandlerError (7),
              // kept as a raw value here so the transport stays http-agnostic.
              enqueue_error_response(ctx, this_response_id, 7, ex.what());
              return;
            } catch (...) {
              enqueue_error_response(ctx, this_response_id, 7,
                                     "Request handler error!");
              return;
            }
          }
        } while (keep_decoding_requests);
        bytes_received -= static_cast<DWORD>(bytes_accumulated);
      } while (keep_decoding_requests && bytes_received > 0);
      ctx->arm_next_receive_operation();
      ctx->arm_next_send_operation();
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
  std::unordered_map<context<RQty, RSty, DEty>*,
                     std::shared_ptr<context<RQty, RSty, DEty>>>
      contexts_;
  types::on_request_delegate<RQty, RSty> on_request_;
  types::on_bad_request_delegate<RSty> on_bad_request_;
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
};
}  // namespace martianlabs::doba::transport::server

#endif
