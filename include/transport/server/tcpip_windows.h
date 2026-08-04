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
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <span>

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
enum class io_type : uint8_t { kAccept, kSend, kReceive, kStop };
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
  uint64_t id{0};
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
  std::unique_ptr<protocol::serialization_result> response;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] overlapped_stop                                            ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct overlapped_stop : overlapped_base {
  overlapped_stop() : overlapped_base(io_type::kStop) {}
};
// /////////////////////////////////////////////////////////////////////////////
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
          types::on_client_disconnected_delegate on_disconnection)
      : socket_{in_socket}, on_disconnection_{on_disconnection} {}
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
    std::lock_guard<std::mutex> lock(mutex_);
    return decoder_.accumulate(ovr_wsa_.buf, bytes_received);
  }
  // +=========================================================================+
  // | [>] deserialize                                              ( public ) |
  // +=========================================================================+
  protocol::deserialization_result<RQty> deserialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    return decoder_.deserialize();
  }
  // +=========================================================================+
  // | [>] get_next_response_id                                     ( public ) |
  // +=========================================================================+
  uint64_t get_next_response_id() {
    std::lock_guard<std::mutex> lock(mutex_);
    return get_next_rid_();
  }
  // +=========================================================================+
  // | [>] enqueue_response                                         ( public ) |
  // +=========================================================================+
  void enqueue_response(
      std::unique_ptr<protocol::serialization_result> response,
      uint64_t response_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    enqueue_response_(std::move(response), response_id);
  }
  // +=========================================================================+
  // | [>] enqueue_error_response                                   ( public ) |
  // +=========================================================================+
  void enqueue_error_response(std::shared_ptr<RSty> res) {
    std::lock_guard<std::mutex> lock(mutex_);
    enqueue_error_response_(std::move(res));
  }
  // +=========================================================================+
  // | [>] check_sending_buffer_and_arm                             ( public ) |
  // +=========================================================================+
  void check_sending_buffer_and_arm(std::size_t bytes_sent) {
    std::lock_guard<std::mutex> lock(mutex_);
    ovs_buf_.erase(0, bytes_sent);
    sending_ = false;
    arm_next_send_operation_();
  }
  // +=========================================================================+
  // | [>] arm_next_receive_operation                               ( public ) |
  // +=========================================================================+
  void arm_next_receive_operation() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (closing_) return;
    if (!receive_()) closing_ = true;
  }
  // +=========================================================================+
  // | [>] arm_next_send_operation                                  ( public ) |
  // +=========================================================================+
  void arm_next_send_operation() {
    std::lock_guard<std::mutex> lock(mutex_);
    arm_next_send_operation_();
  }
  // +=========================================================================+
  // | [>] set_closing_rid                                          ( public ) |
  // +=========================================================================+
  void set_closing_rid(uint64_t rid) {
    std::lock_guard<std::mutex> lock(mutex_);
    set_closing_rid_(rid);
  }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +=========================================================================+
  void close() {
    std::lock_guard<std::mutex> lock(mutex_);
    closing_ = true;
    arm_next_send_operation_();
  }

 private:
  // +=========================================================================+
  // | [>] enqueue_response_                                       ( private ) |
  // +=========================================================================+
  void enqueue_response_(
      std::unique_ptr<protocol::serialization_result> response,
      uint64_t response_id) {
    if (closing_) {
      if (!closing_rid_) {
        return;
      } else if (expected_response_id_ > response_id) {
        return;
      }
    }
    if (!response) return;
    // We need to keep the responses in order (and avoid duplicates)!
    response_data rdata{response_id, std::move(response)};
    auto itr = responses_.begin();
    while (itr != responses_.end()) {
      if (itr->id == rdata.id) return;
      if (itr->id > rdata.id) {
        responses_.insert(itr, std::move(rdata));
        return;
      }
      itr++;
    }
    responses_.emplace_back(std::move(rdata));
  }
  // +=========================================================================+
  // | [>] get_next_rid_                                           ( private ) |
  // +=========================================================================+
  uint64_t get_next_rid_() { return next_response_id_++; }
  // +=========================================================================+
  // | [>] set_closing_rid_                                        ( private ) |
  // +=========================================================================+
  void set_closing_rid_(uint64_t rid) { closing_rid_ = rid; }
  // +=========================================================================+
  // | [>] arm_next_send_operation_                                ( private ) |
  // +=========================================================================+
  void arm_next_send_operation_() {
    if (closing_) {
      cleanup_resources_();
      if (socket_ == INVALID_SOCKET) return;
    }
    if (sending_) return;
    auto itr = responses_.begin();
    while (itr != responses_.end()) {
      if (itr->id != expected_response_id_) break;
      if (ovs_buf_.size() >= kSendBufferMaxSz) break;
      if (!itr->prefix_written) {
        ovs_buf_.append(itr->response->prefix);
        itr->prefix_written = true;
        continue;
      }
      auto& source = itr->response->source;
      if (source.has_value() && !source->eof()) {
        // Let's pour, at most, the remaining outgoing buffer capacity!
        std::byte chunk[kSendChunkSz];
        std::size_t room = kSendBufferMaxSz - ovs_buf_.size();
        if (room > kSendChunkSz) room = kSendChunkSz;
        std::size_t read = source->read(std::span<std::byte>(chunk, room));
        if (source->failed()) {
          closing_ = true;
          return;
        }
        if (!read) {
          // Sources are synchronous readers, so a zero-byte read means there
          // is nothing else to pour: let's retire this response right below!
          source.reset();
          continue;
        }
        ovs_buf_.append(reinterpret_cast<const char*>(chunk), read);
        continue;
      }
      expected_response_id_++;
      itr = responses_.erase(itr);
    }
    if (ovs_buf_.empty()) {
      cleanup_resources_();
      return;
    }
    if (!send_()) {
      closing_ = true;
      return;
    }
    sending_ = true;
  }
  // +=========================================================================+
  // | [>] enqueue_error_response_                                 ( private ) |
  // +=========================================================================+
  void enqueue_error_response_(std::shared_ptr<RSty> response) {
    if (!response || closing_) return;
    set_closing_rid_(get_next_rid_());
    enqueue_response_(std::move(response->serialize()), *closing_rid_);
    arm_next_send_operation_();
    closing_ = true;
  }
  // +=========================================================================+
  // | [>] receive_                                                ( private ) |
  // +=========================================================================+
  bool receive_() {
    DWORD f = 0, r = 0;
    overlapped_receive<RQty, RSty, DEty>* ovr =
        new overlapped_receive<RQty, RSty, DEty>(this->shared_from_this());
    ovr_wsa_.buf = ovr_buf_;
    ovr_wsa_.len = kReceiveBufferSz;
    int res = WSARecv(socket_, &ovr_wsa_, 1, &r, &f, ovr, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      delete ovr;
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] send_                                                   ( private ) |
  // +=========================================================================+
  bool send_() {
    DWORD f = 0, snt = 0;
    overlapped_send<RQty, RSty, DEty>* ovs =
        new overlapped_send<RQty, RSty, DEty>(this->shared_from_this());
    ovs_wsa_.buf = ovs_buf_.data();
    ovs_wsa_.len = ovs_buf_.size();
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
    if (sending_ || !ovs_buf_.empty() || !responses_.empty()) return;
    if (closing_rid_ && expected_response_id_ <= *closing_rid_) return;
    if (socket_ != INVALID_SOCKET) {
      closesocket(socket_);
      socket_ = INVALID_SOCKET;
      try {
        // Let's call user's callback to notify for disconnection!
        on_disconnection_();
      } catch (const std::exception& ex) {
        // [to-do] -> add support for this!
      } catch (...) {
        // [to-do] -> add support for this!
      }
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  // [common] section!
  types::on_client_disconnected_delegate on_disconnection_;
  std::optional<uint64_t> closing_rid_;
  SOCKET socket_{INVALID_SOCKET};
  mutable std::mutex mutex_;
  bool closing_{false};
  bool sending_{false};
  // [decoder] section!
  DEty<RQty, RSty> decoder_{};
  // [overlapped-receive] section!
  CHAR ovr_buf_[kReceiveBufferSz]{0};
  WSABUF ovr_wsa_{0};
  // [overlapped-send] section!
  std::string ovs_buf_;
  WSABUF ovs_wsa_{0};
  // [responses] section!
  std::vector<response_data> responses_;
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
    if (io_h_ != nullptr) return;
    setup_accept_pipeline(setup_workers(setup_listener(port)));
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    if (io_h_ == nullptr) return;
    // Let's close the listening socket and post stop messages to all workers!
    closesocket(accept_socket_);
    accept_socket_ = INVALID_SOCKET;
    for (std::size_t i = 0; i < workers_.size(); i++) {
      overlapped_stop* ovp = new overlapped_stop();
      if (!PostQueuedCompletionStatus(io_h_, 0, 0, ovp)) {
        // PostQueuedCompletionStatus() only fails when 'io_h_' is no longer a
        // valid completion port handle, so retrying it here would not help.
        // Free the un-posted overlapped_stop() to avoid leaking it (it would
        // otherwise never reach handle_overlapped()). The affected worker's
        // own in-flight GetQueuedCompletionStatus() call will then fail too
        // (lpo == NULL), which the worker loop already treats as
        // 'stopping = true' (see setup_workers()), so it still exits cleanly
        // instead of leaving workers_.clear() below blocked on join() forever.
        delete ovp;
      }
    }
    workers_.clear();
    accept_ex_ = nullptr;
    accept_depth_ = 0;
    if (!CloseHandle(io_h_)) {
      // CloseHandle() only fails here if 'io_h_' were already invalid, which
      // should not happen given the guard at the top of this method; there is
      // no meaningful recovery action available. 'io_h_' is reset to nullptr
      // right below regardless, so this object's internal state remains
      // consistent (considered stopped) even if the OS handle was not
      // released.
    }
    io_h_ = nullptr;
  }
  // +=========================================================================+
  // | [>] set_on_request                                           ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_request(FNty&& fn) {
    on_request_ = std::forward<FNty>(fn);
  }
  // +=========================================================================+
  // | [>] set_on_bad_request                                       ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_bad_request(FNty&& fn) {
    on_bad_request_ = std::forward<FNty>(fn);
  }
  // +=========================================================================+
  // | [>] set_on_connection                                        ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_connection(FNty&& fn) {
    on_connection_ = std::forward<FNty>(fn);
  }
  // +=========================================================================+
  // | [>] set_on_disconnection                                     ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_disconnection(FNty&& fn) {
    on_disconnection_ = std::forward<FNty>(fn);
  }

 private:
  // +=========================================================================+
  // | [>] setup_listener                                          ( private ) |
  // +=========================================================================+
  std::size_t setup_listener(const char port[]) {
    std::size_t workers =
        std::max<std::size_t>(1, std::thread::hardware_concurrency());
    HANDLE ioh = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, workers);
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
    int port_num = std::stoi(port);
    if (port_num < 1 || port_num > 65535) {
      // ((error)) -> Invalid port number!
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error("Invalid port (range from 1 to 65535)!");
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
    if (WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &acceptex_guid,
                 sizeof(acceptex_guid), &accept_ex_, sizeof(accept_ex_), &bytes,
                 nullptr, nullptr) == SOCKET_ERROR ||
        accept_ex_ == nullptr) {
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
    return workers;
  }
  // +=========================================================================+
  // | [>] setup_accept_pipeline                                   ( private ) |
  // +=========================================================================+
  void setup_accept_pipeline(std::size_t workers) {
    std::size_t accept_depth = std::max<std::size_t>(2, workers);
    accept_depth_ = accept_depth;
    for (std::size_t i = 0; i < accept_depth_; i++) {
      if (!post_accept()) {
        // ((error)) -> Could not arm AcceptEx pipeline!
        throw std::runtime_error("AcceptEx pipeline could not be armed!");
      }
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
          DWORD err = st ? ERROR_SUCCESS : GetLastError();
          overlapped_base* ovb = reinterpret_cast<overlapped_base*>(lpo);
          if (st == TRUE) {
            if (ovb->get_type() == io_type::kAccept) {
              auto ova = (overlapped_accept*)ovb;
              handle_accept(ova);
            } else if (ovb->get_type() == io_type::kReceive) {
              auto ovr = (overlapped_receive<RQty, RSty, DEty>*)ovb;
              handle_receive(ovr->ctx, bytes);
            } else if (ovb->get_type() == io_type::kSend) {
              auto ovs = (overlapped_send<RQty, RSty, DEty>*)ovb;
              handle_send(ovs->ctx, bytes);
            } else if (ovb->get_type() == io_type::kStop) {
              handle_stop(stopping);
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
  // | [>] handle_accept                                           ( private ) |
  // +=========================================================================+
  void handle_accept(overlapped_accept* ova) {
    if (!post_accept()) return;
    int result = setsockopt(ova->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                            reinterpret_cast<const char*>(&accept_socket_),
                            sizeof(accept_socket_));
    if (result == SOCKET_ERROR) {
      closesocket(ova->socket);
      return;
    }
    ULONG i_mode_flag = 1;
    result = ioctlsocket(ova->socket, FIONBIO, &i_mode_flag);
    if (result != NO_ERROR) {
      closesocket(ova->socket);
      return;
    }
    int ndf = 1;
    result = setsockopt(ova->socket, IPPROTO_TCP, TCP_NODELAY,
                        reinterpret_cast<const char*>(&ndf), sizeof(ndf));
    if (result == SOCKET_ERROR) {
      closesocket(ova->socket);
      return;
    }
    std::shared_ptr<context<RQty, RSty, DEty>> ctx =
        std::make_shared<context<RQty, RSty, DEty>>(ova->socket,
                                                    on_disconnection_);
    ULONG_PTR key = reinterpret_cast<ULONG_PTR>(ctx.get());
    if (!CreateIoCompletionPort((HANDLE)ova->socket, io_h_, key, 0)) {
      closesocket(ova->socket);
      ova->socket = INVALID_SOCKET;
      return;
    }
    // Let's arm next receive operation!
    ctx->arm_next_receive_operation();
    // Let's call user's callback to notify for new connection!
    try {
      on_connection_();
    } catch (const std::exception& ex) {
      ctx->close();
      return;
    } catch (...) {
      ctx->close();
      return;
    }
  }
  // +=========================================================================+
  // | [>] handle_receive                                          ( private ) |
  // +=========================================================================+
  void handle_receive(std::shared_ptr<context<RQty, RSty, DEty>> ctx,
                      DWORD bytes_received) {
    if (!bytes_received) {
      ctx->close();
      return;
    }
    DWORD bytes_accumulated = 0;
    bool keep_decoding_requests = true;
    protocol::deserialization_result<RQty> result;
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
        std::shared_ptr<RSty> response = std::make_shared<RSty>();
        deserialization_result<RQty> result = ctx->deserialize();
        if (result.code == deserialization_status::kMoreBytesNeeded) {
          break;
        } else if (result.code == deserialization_status::kInvalidSource) {
          try {
            on_bad_request_("Invalid request content!", response);
            ctx->enqueue_error_response(response);
          } catch (const std::exception&) {
            ctx->close();
          } catch (...) {
            ctx->close();
          }
          return;
        } else if (result.code == deserialization_status::kSucceeded) {
          if (result.request == nullptr) {
            try {
              on_bad_request_("Decoder error!", response);
              ctx->enqueue_error_response(response);
            } catch (const std::exception&) {
              ctx->close();
            } catch (...) {
              ctx->close();
            }
            return;
          }
          uint64_t this_response_id = ctx->get_next_response_id();
          if (result.channel == protocol::channel_intent::kClose) {
            ctx->set_closing_rid(this_response_id);
            keep_decoding_requests = false;
            ctx->close();
          }
          try {
            // Let's call user handler!
            on_request_(result.request, response,
                        [context = ctx, this_response_id,
                         this_thread_id](std::shared_ptr<RSty> response) {
                          if (!response) return;
                          context->enqueue_response(
                              std::move(response->serialize()),
                              this_response_id);
                          // Let's arm next send operation only if we are not in
                          // the same thread as the worker (delayed operation)!
                          if (std::this_thread::get_id() != this_thread_id) {
                            context->arm_next_send_operation();
                          }
                        });
          } catch (const std::exception& ex) {
            ctx->close();
            return;
          } catch (...) {
            ctx->close();
            return;
          }
        }
      } while (keep_decoding_requests);
      bytes_received -= bytes_accumulated;
    } while (keep_decoding_requests && bytes_received > 0);
    ctx->arm_next_receive_operation();
    ctx->arm_next_send_operation();
  }
  // +=========================================================================+
  // | [>] handle_send                                             ( private ) |
  // +=========================================================================+
  void handle_send(std::shared_ptr<context<RQty, RSty, DEty>> ctx,
                   DWORD bytes_sent) {
    ctx->check_sending_buffer_and_arm(bytes_sent);
  }
  // +=========================================================================+
  // | [>] handle_stop                                             ( private ) |
  // +=========================================================================+
  void handle_stop(bool& stopping) { stopping = true; }
  // +=========================================================================+
  // | [>] handle_error                                            ( private ) |
  // +=========================================================================+
  void handle_error(overlapped_base* ovb) {
    switch (ovb->get_type()) {
      case io_type::kAccept:
        // Let's just ignore it and continue accepting new connections!
        break;
      case io_type::kReceive:
        // Let's just mark the context for closing!
        reinterpret_cast<overlapped_receive<RQty, RSty, DEty>*>(ovb)
            ->ctx->close();
        break;
      case io_type::kSend:
        // Let's just mark the context for closing!
        reinterpret_cast<overlapped_send<RQty, RSty, DEty>*>(ovb)->ctx->close();
        break;
      case io_type::kStop:
        // Let's just ignore it and continue stopping the worker!
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
      case io_type::kStop:
        delete reinterpret_cast<overlapped_stop*>(ovb);
        break;
    }
  }
  // +=========================================================================+
  // | [>] post_accept                                             ( private ) |
  // +=========================================================================+
  bool post_accept() {
    SOCKET soc = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                            WSA_FLAG_OVERLAPPED);
    if (soc == INVALID_SOCKET) return false;
    overlapped_accept* ova = new overlapped_accept(soc);
    DWORD received = 0;
    BOOL accepted_connection =
        accept_ex_(accept_socket_, ova->socket, ova->addresses, 0,
                   kAcceptAddressBytes, kAcceptAddressBytes, &received, ova);
    if (accepted_connection == FALSE && WSAGetLastError() != ERROR_IO_PENDING) {
      // ((error)) -> Could not post AcceptEx operation!
      delete ova;
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +=========================================================================+
  HANDLE io_h_ = nullptr;
  SOCKET accept_socket_ = INVALID_SOCKET;
  LPFN_ACCEPTEX accept_ex_ = nullptr;
  std::size_t accept_depth_ = 0;
  std::vector<std::jthread> workers_;
  types::on_request_delegate<RQty, RSty> on_request_;
  types::on_bad_request_delegate<RSty> on_bad_request_;
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
};
}  // namespace martianlabs::doba::transport::server

#endif
