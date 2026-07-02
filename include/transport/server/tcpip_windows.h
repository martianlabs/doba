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
//        --- martianLabs Anti-AI Usage and Model-Training Addendum ---
//
// TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
//
// Copyright 2025 martianLabs
//
// Except as otherwise stated in this Addendum, this software is licensed
// under the Apache License, Version 2.0 (the "License"); you may not use
// this file except in compliance with the License.
//
// The following additional terms are hereby added to the Apache License for
// the purpose of restricting the use of this software by Artificial
// Intelligence systems, machine learning models, data-scraping bots, and
// automated systems.
//
// 1.  MACHINE LEARNING AND AI RESTRICTIONS
//     1.1. No entity, organization, or individual may use this software,
//          its source code, object code, or any derivative work for the
//          purpose of training, fine-tuning, evaluating, or improving any
//          machine learning model, artificial intelligence system, large
//          language model, or similar automated system.
//     1.2. No automated system may copy, parse, analyze, index, or
//          otherwise process this software for any AI-related purpose.
//     1.3. Use of this software as input, prompt material, reference
//          material, or evaluation data for AI systems is expressly
//          prohibited.
//
// 2.  SCRAPING AND AUTOMATED ACCESS RESTRICTIONS
//     2.1. No automated crawler, training pipeline, or data-extraction
//          system may collect, store, or incorporate any portion of this
//          software in any dataset used for machine learning or AI
//          training.
//     2.2. Any automated access must comply with this License and with
//          applicable copyright law.
//
// 3.  PROHIBITION ON DERIVATIVE DATASETS
//     3.1. You may not create datasets, corpora, embeddings, vector
//          stores, or similar derivative data intended for use by
//          automated systems, AI models, or machine learning algorithms.
//
// 4.  NO WAIVER OF RIGHTS
//     4.1. These restrictions apply in addition to, and do not limit,
//          the rights and protections provided to the copyright holder
//          under the Apache License Version 2.0 and applicable law.
//
// 5.  ACCEPTANCE
//     5.1. Any use of this software constitutes acceptance of both the
//          Apache License Version 2.0 and this Anti-AI Addendum.
//
// You may obtain a copy of the Apache License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.  See the License for the specific language governing
// permissions and limitations under the Apache License Version 2.0.

#ifndef martianlabs_doba_transport_server_tcpip_windows_h
#define martianlabs_doba_transport_server_tcpip_windows_h

#include <mswsock.h>

#include <functional>
#include <unordered_set>

#include "protocol/deserialization.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcpip [windowsTM]                                           ( class ) |
// +---------------------------------------------------------------------------+
// | This specification holds for the WindowsTM server transport.              |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used.                                              |
// |   RSty - response being used.                                             |
// |   BFsz - buffer size for I/O operations.                                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, std::size_t BFsz>
class tcpip {
  // +=========================================================================+
  // | [>] CONSTANTS                                               ( private ) |
  // +=========================================================================+
  static constexpr DWORD kAcceptAddressBytes =
      static_cast<DWORD>(sizeof(sockaddr_storage) + 16);
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  struct context {
    // [constructors/destructors]
    context(SOCKET in_socket) : socket{in_socket} {}
    context(const context&) = delete;
    context(context&&) noexcept = delete;
    // [operators]
    context& operator=(const context&) = delete;
    context& operator=(context&&) noexcept = delete;
    ~context() = default;
    // [push_pending_response]
    INLINE void push_pending_response(std::unique_ptr<std::string> buffer) {
      if (!buffer || buffer->empty()) return;
      responses.push(std::move(buffer));
    }
    // [pop_pending_response]
    INLINE std::unique_ptr<std::string> pop_pending_response() {
      if (responses.empty()) return nullptr;
      std::unique_ptr<std::string> response = std::move(responses.front());
      responses.pop();
      return response;
    }
    // [general] attributes
    SOCKET socket{INVALID_SOCKET};
    std::atomic<bool> closing{false};
    // [responses] attributes
    std::unique_ptr<std::string> response_in_flight{nullptr};
    std::queue<std::unique_ptr<std::string>> responses;
    // [sending] attributes
    bool close_after_sending{false};
    std::mutex sending_mutex;
    bool sending{false};
  };
  enum class io_type : uint8_t { kAccept, kSend, kReceive, kStop };
  struct overlapped_base : OVERLAPPED {
    overlapped_base(io_type in_type, std::shared_ptr<context> in_ctx = nullptr)
        : OVERLAPPED{}, type{in_type}, ctx{in_ctx} {}
    INLINE io_type get_type() const { return type; }
    INLINE std::shared_ptr<context> get_context() const { return ctx; }

   private:
    const io_type type;
    std::shared_ptr<context> ctx;
  };
  struct overlapped_accept : overlapped_base {
    overlapped_accept(SOCKET in_socket)
        : overlapped_base(io_type::kAccept), socket{in_socket} {}
    SOCKET socket{INVALID_SOCKET};
    CHAR addresses[(kAcceptAddressBytes * 2)]{0};
  };
  struct overlapped_receive : overlapped_base {
    overlapped_receive(std::shared_ptr<context> in_ctx)
        : overlapped_base(io_type::kReceive, in_ctx) {}
    WSABUF wsa;
    CHAR buffer[BFsz];
  };
  struct overlapped_send : overlapped_base {
    overlapped_send(std::shared_ptr<context> in_ctx)
        : overlapped_base(io_type::kSend, in_ctx) {}
    WSABUF wsa;
    std::unique_ptr<std::string> buffer;
  };
  struct overlapped_stop : overlapped_base {
    overlapped_stop() : overlapped_base(io_type::kStop) {}
  };

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
  // | [>] PROPERTIEs                                               ( public ) |
  // +=========================================================================+
  types::on_request_delegate<RQty, RSty> on_request;
  types::on_bad_request_delegate<RSty> on_bad_request;
  types::on_client_connected_delegate on_connection;
  types::on_client_disconnected_delegate on_disconnection;
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +=========================================================================+
  void start(const char port[]) {
    // If the i/o completion port is valid then we are already started!
    if (io_h_ != nullptr) return;
    // Let's setup all the required resources..
    auto workers = setup_listener(port);
    setup_workers(workers);
    setup_accept_pipeline(std::max<std::size_t>(2, workers));
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    // If the i/o completion port is not valid then we are already stopped!
    if (io_h_ == nullptr) return;
    // Let's close the listening socket and post stop messages to all workers!
    closesocket(accept_socket_);
    accept_socket_ = INVALID_SOCKET;
    for (std::size_t i = 0; i < workers_.size(); i++) {
      if (!PostQueuedCompletionStatus(io_h_, 0, 0, new overlapped_stop())) {
        /*
        pepe
        */

        /*
        pepe fin
        */
      }
    }
    workers_.clear();
    accept_ex_ = nullptr;
    accept_depth_ = 0;
    if (!CloseHandle(io_h_)) {
      /*
      pepe
      */

      /*
      pepe fin
      */
    }
    io_h_ = nullptr;
  }

 private:
  // +=========================================================================+
  // | [>] setup_listener                                          ( private ) |
  // +=========================================================================+
  std::size_t setup_listener(const char port[]) {
    // Let's use our help class to create a cpu pinning plan!
    std::size_t workers = std::thread::hardware_concurrency();
    // Let's create our main i/o completion port!
    HANDLE ioh = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, workers);
    if (ioh == NULL) {
      // ((error)) -> while setting up the i/o completion port!
      throw std::runtime_error("I/O Completion Port could not be created!");
    }
    // Let's setup our main listening socket (server)!
    SOCKET sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                             WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET) {
      // ((error)) -> Could not create socket!
      CloseHandle(ioh);
      throw std::runtime_error("Socket could not be created!");
    }
    int port_num = std::stoi(port);
    if (port_num < 1 || port_num > 65535) {
      // ((error)) -> Could not set socket opt reuse-address!
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
      // ((error)) -> Could not listen socket!
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
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error("AcceptEx entry point could not be loaded!");
    }
    if (!CreateIoCompletionPort((HANDLE)sock, ioh, 0, 0)) {
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
  void setup_accept_pipeline(std::size_t accepts_in_flight) {
    accept_depth_ = accepts_in_flight;
    for (std::size_t i = 0; i < accept_depth_; i++) {
      if (!post_accept()) {
        throw std::runtime_error("AcceptEx pipeline could not be armed!");
      }
    }
  }
  // +=========================================================================+
  // | [>] setup_workers                                           ( private ) |
  // +=========================================================================+
  void setup_workers(std::size_t number_of_workers) {
    for (std::size_t i = 0; i < number_of_workers; i++) {
      workers_.emplace_back(std::jthread([this]() {
        bool stopping = false;
        while (!stopping) {
          ULONG_PTR key = NULL;
          LPOVERLAPPED lpo = NULL;
          DWORD n = 0;  // bytes transfered..
          DWORD tout = INFINITE;
          BOOL st = GetQueuedCompletionStatus(io_h_, &n, &key, &lpo, tout);
          DWORD err = st ? ERROR_SUCCESS : GetLastError();
          overlapped_base* ovb = reinterpret_cast<overlapped_base*>(lpo);
          if (st == TRUE) {
            switch (ovb->get_type()) {
              case io_type::kAccept:
                handle_accept(reinterpret_cast<overlapped_accept*>(ovb));
                break;
              case io_type::kReceive:
                handle_receive(reinterpret_cast<overlapped_receive*>(ovb), n);
                break;
              case io_type::kSend:
                handle_send(reinterpret_cast<overlapped_send*>(ovb), n);
                break;
              case io_type::kStop:
                handle_stop(stopping);
                break;
            }
          } else {
            handle_error(ovb);
          }
          handle_overlapped(ovb);
        }
      }));
    }
  }
  // +=========================================================================+
  // | [>] handle_accept                                           ( private ) |
  // +=========================================================================+
  INLINE void handle_accept(overlapped_accept* ova) {
    if (!post_accept()) return;
    // Let's create a new context for this accepted socket!
    std::shared_ptr<context> ctx = std::make_shared<context>(ova->socket);
    // Let's set the accepted socket to be associated with the listening socket!
    int result = setsockopt(ova->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                            reinterpret_cast<const char*>(&accept_socket_),
                            sizeof(accept_socket_));
    if (result == SOCKET_ERROR) {
      closesocket(ova->socket);
      return;
    }
    // Let's set the accepted socket to be non-blocking!
    ULONG i_mode_flag = 1;
    result = ioctlsocket(ova->socket, FIONBIO, &i_mode_flag);
    if (result != NO_ERROR) {
      closesocket(ova->socket);
      return;
    }
    // Let's set the accepted socket to be no-delay!
    int ndf = 1;
    result = setsockopt(ova->socket, IPPROTO_TCP, TCP_NODELAY,
                        reinterpret_cast<const char*>(&ndf), sizeof(ndf));
    if (result == SOCKET_ERROR) {
      closesocket(ova->socket);
      return;
    }
    // Let's associate the accepted socket to the i/o completion port!
    ULONG_PTR key = reinterpret_cast<ULONG_PTR>(ctx.get());
    if (!CreateIoCompletionPort((HANDLE)ova->socket, io_h_, key, 0)) {
      closesocket(ova->socket);
      return;
    }
    // Let's arm next receive operation!
    arm_next_receive_operation(ctx);
    // Let's call user's callback to notify for new connection!
    try {
      on_connection();
    } catch (const std::exception& ex) {
      mark_context_for_closing(ctx);
      return;
    } catch (...) {
      mark_context_for_closing(ctx);
      return;
    }
  }
  // +=========================================================================+
  // | [>] handle_receive                                          ( private ) |
  // +=========================================================================+
  INLINE void handle_receive(overlapped_receive* ovr, DWORD bytes_received) {
    // If the associated context is in 'closing' status then the operation is
    // discarded and no actions are performed!
    if (ovr->get_context()->closing.load()) return;
    // Any of the following situations will trigger a disconnection!
    if (!bytes_received) {
      mark_context_for_closing(ovr->get_context());
      return;
    }
    // Now, we'll try to decode as many requests as possible..
    DWORD oin = 0;
    std::unique_ptr<std::string> out = std::make_unique<std::string>();
    while (oin < bytes_received) {
      std::shared_ptr<RSty> res = std::make_shared<RSty>();
      std::size_t space_left_in = bytes_received - oin;
      protocol::deserialization_result<RQty> result =
          RQty::deserialize(std::string_view(&ovr->buffer[oin], space_left_in));
      if (result.code == protocol::deserialization_status::kMoreBytesNeeded) {
        // In case of 'failed' operation, bytes can NOT be greater than zero!
        if (result.bytes_used > 0) {
          // In this case we'll mark this context as 'closing'..
          try {
            on_bad_request("Inconsistent deserialization status!", res);
            send_error_and_mark_context_for_closing(ovr->get_context(), res);
          } catch (const std::exception&) {
            mark_context_for_closing(ovr->get_context());
          } catch (...) {
            mark_context_for_closing(ovr->get_context());
          }
          return;
        }
        break;
      }
      if (result.code == protocol::deserialization_status::kInvalidSource) {
        // In this case we'll mark this context as 'closing'..
        try {
          on_bad_request("Invalid source deserialization content!", res);
          send_error_and_mark_context_for_closing(ovr->get_context(), res);
        } catch (const std::exception&) {
          mark_context_for_closing(ovr->get_context());
        } catch (...) {
          mark_context_for_closing(ovr->get_context());
        }
        return;
      }
      // In case of 'succeeded' operation, consumed bytes can NOT be zero!
      // Also, used bytes in deserialization must be within valid memory range!
      if (result.bytes_used == 0 || result.bytes_used > (space_left_in)) {
        // In this case we'll mark this context as 'closing'..
        try {
          on_bad_request("Inconsistent deserialization status!", res);
          send_error_and_mark_context_for_closing(ovr->get_context(), res);
        } catch (const std::exception&) {
          mark_context_for_closing(ovr->get_context());
        } catch (...) {
          mark_context_for_closing(ovr->get_context());
        }
        return;
      }
      // In case of 'succeeded' operation, returned request cannot be NULL!
      if (result.request == nullptr) {
        // In this case we'll mark this context as 'closing'..
        try {
          on_bad_request("Inconsistent deserialization status!", res);
          send_error_and_mark_context_for_closing(ovr->get_context(), res);
        } catch (const std::exception&) {
          mark_context_for_closing(ovr->get_context());
        } catch (...) {
          mark_context_for_closing(ovr->get_context());
        }
        return;
      }
      // Let's call user handler!
      try {
        std::shared_ptr<context> ctx = ovr->get_context();
        on_request(result.request, res, [this, ctx, &out](auto res) {
          // If the associated context is in 'closing' status then
          // the operation is discarded and no actions are performed!
          if (ctx->closing.load()) return;
          // Let's append this response to the outgoing buffer!
          out->append(res->serialize());
        });
      } catch (const std::exception& ex) {
        mark_context_for_closing(ovr->get_context());
        return;
      } catch (...) {
        mark_context_for_closing(ovr->get_context());
        return;
      }
      oin += result.bytes_used;
    }
    // Let's add batched responses buffer to pending queue!
    enqueue(ovr->get_context(), std::move(out));
    // Let's arm next receive operation!
    arm_next_receive_operation(ovr->get_context());
    // Let's arm send operation!
    arm_next_send_operation(ovr->get_context());
  }
  // +=========================================================================+
  // | [>] handle_send                                             ( private ) |
  // +=========================================================================+
  INLINE void handle_send(overlapped_send* ovs, DWORD bytes) {
    if (!arm_pending_send_operation(ovs, bytes)) {
      arm_next_send_operation(ovs->get_context());
    }
  }
  // +=========================================================================+
  // | [>] handle_stop                                             ( private ) |
  // +=========================================================================+
  INLINE void handle_stop(bool& stopping) { stopping = true; }
  // +=========================================================================+
  // | [>] handle_error                                            ( private ) |
  // +=========================================================================+
  INLINE void handle_error(overlapped_base* ovb) {
    switch (ovb->get_type()) {
      case io_type::kAccept:
        break;
      case io_type::kReceive:
      case io_type::kSend:
        mark_context_for_closing(ovb->get_context());
        break;
      case io_type::kStop:
        break;
    }
  }
  // +=========================================================================+
  // | [>] handle_overlapped                                       ( private ) |
  // +=========================================================================+
  INLINE void handle_overlapped(overlapped_base* ovb) {
    switch (ovb->get_type()) {
      case io_type::kAccept:
        delete reinterpret_cast<overlapped_accept*>(ovb);
        break;
      case io_type::kReceive:
        delete reinterpret_cast<overlapped_receive*>(ovb);
        break;
      case io_type::kSend:
        delete reinterpret_cast<overlapped_send*>(ovb);
        break;
      case io_type::kStop:
        delete reinterpret_cast<overlapped_stop*>(ovb);
        break;
    }
  }
  // +=========================================================================+
  // | [>] post_accept                                             ( private ) |
  // +=========================================================================+
  INLINE bool post_accept() {
    SOCKET soc = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                            WSA_FLAG_OVERLAPPED);
    if (soc == INVALID_SOCKET) return false;
    overlapped_accept* ova = new overlapped_accept(soc);
    DWORD received = 0;
    BOOL accepted =
        accept_ex_(accept_socket_, ova->socket, ova->addresses, 0,
                   kAcceptAddressBytes, kAcceptAddressBytes, &received, ova);
    if (accepted == FALSE && WSAGetLastError() != ERROR_IO_PENDING) {
      delete ova;
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] send_error_and_mark_context_for_closing                 ( private ) |
  // +=========================================================================+
  INLINE void send_error_and_mark_context_for_closing(
      std::shared_ptr<context> ctx, std::shared_ptr<RSty> response) {
    if (response) {
      std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
      std::unique_ptr<std::string> out = std::make_unique<std::string>();
      ctx->close_after_sending = true;
      out->append(response->serialize());
      enqueue(ctx, std::move(out));
    }
    arm_next_send_operation(ctx);
  }
  // +=========================================================================+
  // | [>] mark_context_for_closing                                ( private ) |
  // +=========================================================================+
  INLINE void mark_context_for_closing(std::shared_ptr<context> ctx) {
    // Let's prepare this context for closing!
    bool expected = false;
    if (ctx->closing.compare_exchange_strong(expected, true)) {
      closesocket(ctx->socket);
      ctx->socket = INVALID_SOCKET;
      // Let's call user's callback to notify for disconnection!
      try {
        on_disconnection();
      } catch (const std::exception& ex) {
        // [to-do] -> add support for this!
      } catch (...) {
        // [to-do] -> add support for this!
      }
    }
  }
  // +=========================================================================+
  // | [>] arm_pending_send_operation                              ( private ) |
  // +=========================================================================+
  INLINE bool arm_pending_send_operation(overlapped_send* ovs, DWORD bytes) {
    // If the associated context is in 'closing' status then the operation is
    // not dispatched and no actions are performed!
    if (ovs->get_context()->closing.load()) return false;
    std::lock_guard<std::mutex> sending_lock(ovs->get_context()->sending_mutex);
    ovs->buffer->erase(0, bytes);
    ovs->get_context()->sending = false;
    if (ovs->buffer->empty()) return false;
    // Let's arm next send operation!
    if (!send(ovs->get_context(), std::move(ovs->buffer))) {
      mark_context_for_closing(ovs->get_context());
      ovs->get_context()->sending = false;
      return false;
    }
    ovs->get_context()->sending = true;
    return true;
  }
  // +=========================================================================+
  // | [>] arm_next_send_operation                                 ( private ) |
  // +=========================================================================+
  INLINE void arm_next_send_operation(std::shared_ptr<context> ctx) {
    // If the associated context is in 'closing' status then the operation is
    // not dispatched and no actions are performed!
    if (ctx->closing.load()) return;
    // If the associated context is already sending then the operation is not
    // dispatched and no actions are performed!
    std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
    if (ctx->sending) return;
    std::unique_ptr<std::string> buffer = dequeue(ctx);
    if (!buffer) {
      // Let's check if we need to close this context after sending!
      if (ctx->close_after_sending) mark_context_for_closing(ctx);
      return;
    }
    // Let's arm next send operation!
    if (!send(ctx, std::move(buffer))) {
      mark_context_for_closing(ctx);
      ctx->sending = false;
      return;
    }
    ctx->sending = true;
  }
  // +=========================================================================+
  // | [>] arm_next_receive_operation                              ( private ) |
  // +=========================================================================+
  INLINE void arm_next_receive_operation(std::shared_ptr<context> ctx) {
    // If the associated context is in 'closing' status then the operation is
    if (ctx->closing.load()) return;
    // Let's arm next receive operation!
    if (!receive(ctx)) mark_context_for_closing(ctx);
  }
  // +=========================================================================+
  // | [>] enqueue                                                 ( private ) |
  // +=========================================================================+
  INLINE void enqueue(std::shared_ptr<context> ctx,
                      std::unique_ptr<std::string> bf) {
    ctx->push_pending_response(std::move(bf));
  }
  // +=========================================================================+
  // | [>] dequeue                                                 ( private ) |
  // +=========================================================================+
  INLINE std::unique_ptr<std::string> dequeue(std::shared_ptr<context> ctx) {
    return ctx->pop_pending_response();
  }
  // +=========================================================================+
  // | [>] receive                                                 ( private ) |
  // +=========================================================================+
  INLINE bool receive(std::shared_ptr<context> ctx) {
    DWORD f = 0, r = 0;
    overlapped_receive* ovr = new overlapped_receive(ctx);
    std::memset(&ovr->wsa, 0, sizeof(WSABUF));
    std::memset(static_cast<OVERLAPPED*>(ovr), 0, sizeof(OVERLAPPED));
    ovr->wsa.buf = ovr->buffer;
    ovr->wsa.len = BFsz;
    int res = WSARecv(ctx->socket, &ovr->wsa, 1, &r, &f, ovr, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] send                                s                    ( private )
  // |
  // +=========================================================================+
  INLINE bool send(std::shared_ptr<context> ctx,
                   std::unique_ptr<std::string> buffer) {
    DWORD f = 0, s = 0;
    overlapped_send* ovs = new overlapped_send(ctx);
    std::memset(&ovs->wsa, 0, sizeof(WSABUF));
    std::memset(static_cast<OVERLAPPED*>(ovs), 0, sizeof(OVERLAPPED));
    ovs->buffer = std::move(buffer);
    ovs->wsa.buf = ovs->buffer->data();
    ovs->wsa.len = ovs->buffer->size();
    int res = WSASend(ctx->socket, &ovs->wsa, 1, &s, f, ovs, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
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
};
}  // namespace martianlabs::doba::transport::server

#endif
