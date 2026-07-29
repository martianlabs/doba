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
      : socket{in_socket}, on_disconnection_{on_disconnection} {}
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
    return decoder.accumulate(ovr_wsa.buf, bytes_received);
  }
  // +=========================================================================+
  // | [>] deserialize                                              ( public ) |
  // +=========================================================================+
  protocol::deserialization_result<RQty> deserialize() {
    return decoder.deserialize();
  }
  // +=========================================================================+
  // | [>] get_next_response_id                                     ( public ) |
  // +=========================================================================+
  uint64_t get_next_response_id() { return next_response_id++; }
  // +=========================================================================+
  // | [>] add_response_to_queue                                    ( public ) |
  // +=========================================================================+
  void add_response_to_queue(
      std::unique_ptr<protocol::serialization_result> response,
      uint64_t response_id, bool close_this_context_after_sending = false) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex);
    if (!response || closing || close_after_sending) {
      // If the response is null, the context is closing, or we already need to
      // close after sending, we should not add the response to the queue.
      return;
    }
    response_data rdata{response_id, std::move(response)};
    // We need to keep the responses in order!
    auto itr = responses.begin();
    while (itr != responses.end()) {
      if (itr->id > rdata.id) {
        // Insert the new response before the current one to maintain order.
        responses.insert(itr, std::move(rdata));
        return;
      }
      itr++;
    }
    // If we reach this point, it means the new response has the highest ID and
    // should be added at the end.
    responses.emplace_back(std::move(rdata));
    close_after_sending = close_this_context_after_sending;
  }
  // +=========================================================================+
  // | [>] send_error_and_mark_for_closing                          ( public ) |
  // +=========================================================================+
  void send_error_and_mark_for_closing(std::shared_ptr<RSty> error_response) {
    if (!error_response) {
      // If the response is null, we should mark the context for closing without
      // sending any error response.
      mark_context_for_closing();
      return;
    }
    uint64_t id = get_next_response_id();
    add_response_to_queue(std::move(error_response->serialize()), id, true);
    arm_next_send_operation();
  }
  // +=========================================================================+
  // | [>] mark_context_for_closing                                 ( public ) |
  // +=========================================================================+
  void mark_context_for_closing() {
    // Let's prepare this context for closing!
    bool expected = false;
    if (closing.compare_exchange_strong(expected, true)) {
      if (socket != INVALID_SOCKET) {
        closesocket(socket);
      }
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
  // | [>] check_sending_buffer                                     ( public ) |
  // +=========================================================================+
  bool check_sending_buffer(std::size_t bytes_sent) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex);
    sending_buffer.erase(0, bytes_sent);
    sending = false;
    if (close_after_sending && sending_buffer.empty() && responses.empty()) {
      // If we need to close the channel after sending and there is nothing
      // left to send, we should mark the context for closing.
      mark_context_for_closing();
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] arm_next_receive_operation                               ( public ) |
  // +=========================================================================+
  void arm_next_receive_operation() {
    if (!receive()) mark_context_for_closing();
  }
  // +=========================================================================+
  // | [>] arm_next_send_operation                                  ( public ) |
  // +=========================================================================+
  void arm_next_send_operation() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex);
    if (sending) return;
    auto itr = responses.begin();
    while (itr != responses.end()) {
      if (itr->id != expected_response_id) {
        /*
        pepe
        */

        /*
        necesitamos añadir el soporte para cuando una response no esta
        en orden, es decir, cuando el id de la response no es el esperado. Esto
        puede ocurrir si el usuario envia responses fuera de orden. En este
        caso, debemos decidir si queremos esperar a que llegue la response con
        el id esperado o si queremos cerrar la conexion. Por ahora, vamos a
        imprimir un mensaje de error y cerrar la conexion.
        */

        /*
        pepe fin
        */

        break;
      }
      sending_buffer.append(itr->response->prefix);
      expected_response_id++;
      itr = responses.erase(itr);
    }
    if (sending_buffer.empty()) {
      // Let's return if there is nothing to send, we will wait for the next
      // response to be added to the queue.
      return;
    }
    sending = send();
    if (!sending) {
      // Let's mark the context for closing if we failed to send the data.
      mark_context_for_closing();
    }
  }
  // +=========================================================================+
  // | [>] receive                                                  ( public ) |
  // +=========================================================================+
  bool receive() {
    DWORD f = 0, r = 0;
    overlapped_receive<RQty, RSty, DEty>* ovr =
        new overlapped_receive<RQty, RSty, DEty>(this->shared_from_this());
    ovr_wsa.buf = ovr_buf;
    ovr_wsa.len = kReceiveBufferSz;
    int res = WSARecv(socket, &ovr_wsa, 1, &r, &f, ovr, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      delete ovr;
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] send                                                    ( private ) |
  // +=========================================================================+
  bool send() {
    DWORD f = 0, snt = 0;
    overlapped_send<RQty, RSty, DEty>* ovs =
        new overlapped_send<RQty, RSty, DEty>(this->shared_from_this());
    ovs_wsa.buf = sending_buffer.data();
    ovs_wsa.len = sending_buffer.size();
    int res = WSASend(socket, &ovs_wsa, 1, &snt, f, ovs, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      delete ovs;
      return false;
    }
    return true;
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  // [common] section!
  types::on_client_disconnected_delegate on_disconnection_;
  std::atomic<bool> closing{false};
  SOCKET socket{INVALID_SOCKET};
  // [decoder] section!
  DEty<RQty, RSty> decoder{};
  // [overlapped-receive] section!
  CHAR ovr_buf[kReceiveBufferSz]{0};
  WSABUF ovr_wsa{0};
  // [overlapped-send] section!
  WSABUF ovs_wsa{0};
  // [responses] section!
  std::vector<response_data> responses;
  std::atomic<uint64_t> expected_response_id{0};
  std::atomic<uint64_t> next_response_id{0};
  bool close_after_sending{false};
  std::string sending_buffer;
  std::mutex sending_mutex;
  bool sending{false};
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
    if (io_h_ != nullptr) {
      // If the i/o completion port is valid then we are already started!
      return;
    }
    setup_accept_pipeline(setup_workers(setup_listener(port)));
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    if (io_h_ == nullptr) {
      // If the i/o completion port is not valid then we are already stopped!
      return;
    }
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
            switch (ovb->get_type()) {
              case io_type::kAccept:
                handle_accept(ovb);
                break;
              case io_type::kReceive:
                handle_receive(ovb, bytes);
                break;
              case io_type::kSend:
                handle_send(ovb, bytes);
                break;
              case io_type::kStop:
                handle_stop(stopping);
                break;
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
  void handle_accept(overlapped_base* ovb) {
    overlapped_accept* ova = reinterpret_cast<overlapped_accept*>(ovb);
    if (!post_accept()) {
      // ((error)) -> Could not post next AcceptEx operation!
      return;
    }
    int result = setsockopt(ova->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                            reinterpret_cast<const char*>(&accept_socket_),
                            sizeof(accept_socket_));
    if (result == SOCKET_ERROR) {
      // ((error)) -> Could not update accept context!
      closesocket(ova->socket);
      return;
    }
    ULONG i_mode_flag = 1;
    result = ioctlsocket(ova->socket, FIONBIO, &i_mode_flag);
    if (result != NO_ERROR) {
      // ((error)) -> Could not set non-blocking mode on accepted socket!
      closesocket(ova->socket);
      return;
    }
    int ndf = 1;
    result = setsockopt(ova->socket, IPPROTO_TCP, TCP_NODELAY,
                        reinterpret_cast<const char*>(&ndf), sizeof(ndf));
    if (result == SOCKET_ERROR) {
      // ((error)) -> Could not set TCP_NODELAY on accepted socket!
      closesocket(ova->socket);
      return;
    }
    std::shared_ptr<context<RQty, RSty, DEty>> ctx =
        std::make_shared<context<RQty, RSty, DEty>>(ova->socket,
                                                    on_disconnection_);
    ULONG_PTR key = reinterpret_cast<ULONG_PTR>(ctx.get());
    if (!CreateIoCompletionPort((HANDLE)ova->socket, io_h_, key, 0)) {
      // ((error)) -> Could not associate accepted socket to IOCP!
      closesocket(ova->socket);
      return;
    }
    // Let's arm next receive operation!
    ctx->arm_next_receive_operation();
    // Let's call user's callback to notify for new connection!
    try {
      on_connection_();
    } catch (const std::exception& ex) {
      ctx->mark_context_for_closing();
      return;
    } catch (...) {
      ctx->mark_context_for_closing();
      return;
    }
  }
  // +=========================================================================+
  // | [>] handle_receive                                          ( private ) |
  // +=========================================================================+
  void handle_receive(overlapped_base* ovb, DWORD bytes_received) {
    overlapped_receive<RQty, RSty, DEty>* ovr =
        reinterpret_cast<overlapped_receive<RQty, RSty, DEty>*>(ovb);
    std::size_t bytes_added = 0;
    bool close_channel = false;
    do {
      if (!bytes_received) {
        // In this case we'll mark this context as 'closing'..
        close_channel = true;
        break;
      }
      // -----------------------------------------------------------------------
      // Let's try to accumulate the maximum number of received bytes!
      // -----------------------------------------------------------------------
      bytes_added += ovr->ctx->accumulate(bytes_received);
      // -----------------------------------------------------------------------
      // Let's try to deserialize some requests!
      // -----------------------------------------------------------------------
      protocol::deserialization_result<RQty> result;
      do {
        // Let's try to deserialize a request!
        result = ovr->ctx->deserialize();
        // If the decoder needs more bytes, let's arm another receive operation!
        if (result.code == protocol::deserialization_status::kMoreBytesNeeded) {
          break;
        }
        std::shared_ptr<RSty> res = std::make_shared<RSty>();
        // If the decoder failed to deserialize the request, then error!
        if (result.code == protocol::deserialization_status::kInvalidSource) {
          try {
            on_bad_request_("Invalid source deserialization content!", res);
            ovr->ctx->send_error_and_mark_for_closing(res);
          } catch (const std::exception&) {
            ovr->ctx->mark_context_for_closing();
          } catch (...) {
            ovr->ctx->mark_context_for_closing();
          }
          return;
        }
        // In case of 'succeeded' operation, returned request cannot be NULL!
        if (result.request == nullptr) {
          // In this case we'll mark this context as 'closing'..
          try {
            on_bad_request_("Inconsistent deserialization status!", res);
            ovr->ctx->send_error_and_mark_for_closing(res);
          } catch (const std::exception&) {
            ovr->ctx->mark_context_for_closing();
          } catch (...) {
            ovr->ctx->mark_context_for_closing();
          }
          return;
        }
        // Happy path: allocate response only when deserialization succeeded.
        try {
          // Let's call user handler!
          std::thread::id tid = std::this_thread::get_id();
          uint64_t id = ovr->ctx->get_next_response_id();
          on_request_(result.request, res, [ctx = ovr->ctx, id, tid](auto res) {
            ctx->add_response_to_queue(std::move(res->serialize()), id);
            if (std::this_thread::get_id() != tid) {
              // Let's arm next send operation only if we are not in the same
              // thread as the worker (delayed operation)!
              ctx->arm_next_send_operation();
            }
          });
        } catch (const std::exception& ex) {
          ovr->ctx->mark_context_for_closing();
          return;
        } catch (...) {
          ovr->ctx->mark_context_for_closing();
          return;
        }
      } while (result.code == protocol::deserialization_status::kSucceeded);
    } while (bytes_added < bytes_received);
    // Let's check if we need to close the channel!
    if (close_channel) {
      ovr->ctx->mark_context_for_closing();
      return;
    }
    // Let's arm next receive op (for any pending request to be received)!
    ovr->ctx->arm_next_receive_operation();
    // Let's arm next send op (for any pending response to be sent)!
    ovr->ctx->arm_next_send_operation();
  }
  // +=========================================================================+
  // | [>] handle_send                                             ( private ) |
  // +=========================================================================+
  void handle_send(overlapped_base* ovb, DWORD bytes_sent) {
    overlapped_send<RQty, RSty, DEty>* ovs =
        reinterpret_cast<overlapped_send<RQty, RSty, DEty>*>(ovb);
    if (!ovs->ctx->check_sending_buffer(bytes_sent)) return;
    ovs->ctx->arm_next_send_operation();
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
            ->ctx->mark_context_for_closing();
        break;
      case io_type::kSend:
        // Let's just mark the context for closing!
        reinterpret_cast<overlapped_send<RQty, RSty, DEty>*>(ovb)
            ->ctx->mark_context_for_closing();
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
    if (soc == INVALID_SOCKET) {
      // ((error)) -> Could not create socket for AcceptEx!
      return false;
    }
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
