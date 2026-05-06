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
// =============================================================================
// tcpip [windows]                                                     ( class )
// -----------------------------------------------------------------------------
// This specification holds for the WindowsTM server transport implementation.
// -----------------------------------------------------------------------------
// Template parameters:
//    RQty - request being used.
//    RSty - response being used.
// =============================================================================
template <typename RQty, typename RSty, std::size_t BFsz>
class tcpip {
  // ---------------------------------------------------------------------------
  // CONSTANTs                                                       ( private )
  //
  static constexpr DWORD kAABs = static_cast<DWORD>(
      sizeof(sockaddr_storage) + 16);  // Accept-Address-Bytes
  // ---------------------------------------------------------------------------
  // TYPEs                                                           ( private )
  //
  struct overlapped_receive;
  struct overlapped_send;
  struct context {
    context(SOCKET in_socket) : socket{in_socket} { contexts_in_use_++; }
    context(const context&) = delete;
    context(context&&) noexcept = delete;
    context& operator=(const context&) = delete;
    context& operator=(context&&) noexcept = delete;
    ~context() { contexts_in_use_--; }
    static bool empty() { return contexts_in_use_ == 0; }
    static inline std::atomic<std::size_t> contexts_in_use_;
    std::atomic<bool> closing{false};
    bool sending{false};
    std::queue<std::shared_ptr<RSty>> responses;
    std::mutex responses_mutex;
    overlapped_receive* ovr{nullptr};
    overlapped_send* ovs{nullptr};
    WSABUF wsa_rcv{};
    WSABUF wsa_snd{};
    CHAR rcv_buf[BFsz]{};
    CHAR* snd_buf{nullptr};
    ULONG rcv_len{0};
    ULONG snd_len{0};
    SOCKET socket;
  };
  enum class io_type : uint8_t { kAccept, kSend, kReceive, kStop };
  struct overlapped_base : OVERLAPPED {
    overlapped_base(io_type in_type, std::shared_ptr<context> in_ctx = nullptr)
        : OVERLAPPED{}, type{in_type}, ctx{in_ctx} {}
    const io_type type;
    std::shared_ptr<context> ctx;
  };
  struct overlapped_accept : overlapped_base {
    overlapped_accept(SOCKET in_socket)
        : overlapped_base(io_type::kAccept), socket{in_socket} {}
    SOCKET socket;
    CHAR addresses[(kAABs * 2)]{};
  };
  struct overlapped_receive : overlapped_base {
    overlapped_receive(std::shared_ptr<context> in_ctx)
        : overlapped_base(io_type::kReceive, in_ctx) {}
  };
  struct overlapped_send : overlapped_base {
    overlapped_send(std::shared_ptr<context> in_ctx)
        : overlapped_base(io_type::kSend, in_ctx) {}
  };
  struct overlapped_stop : overlapped_base {
    overlapped_stop() : overlapped_base(io_type::kStop) {}
  };

 public:
  // ---------------------------------------------------------------------------
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  tcpip() = default;
  tcpip(const tcpip&) = delete;
  tcpip(tcpip&&) noexcept = delete;
  ~tcpip() { stop(); }
  // ---------------------------------------------------------------------------
  // OPERATORs                                                        ( public )
  //
  tcpip& operator=(const tcpip&) = delete;
  tcpip& operator=(tcpip&&) noexcept = delete;
  // ---------------------------------------------------------------------------
  // USINGs                                                           ( public )
  //
  using on_send_fn = std::function<void(std::shared_ptr<RSty>)>;
  using on_request_fn = std::function<void(std::shared_ptr<const RQty>,
                                           std::shared_ptr<RSty>, on_send_fn)>;
  using on_client_connected_fn = std::function<void()>;
  using on_client_disconnected_fn = std::function<void()>;
  // ---------------------------------------------------------------------------
  // METHODs                                                          ( public )
  //
  void start(const char port[]) {
    if (io_h_ != nullptr) {
      throw std::runtime_error("TCP/IP transport already started!");
    }
    // Let's setup all the required resources..
    auto workers = setupListener(port);
    setupWorkers(workers);
    setupAcceptPipeline(std::max<std::size_t>(2, workers * 2));
  }
  void stop() {
    if (io_h_ == nullptr) return;
    if (accept_socket_ != INVALID_SOCKET) {
      closesocket(accept_socket_);
      accept_socket_ = INVALID_SOCKET;
    }
    if (io_h_ != nullptr) {
      for (std::size_t i = 0; i < workers_.size(); i++) {
        PostQueuedCompletionStatus(io_h_, 0, 0, new overlapped_stop());
      }
    }
    for (auto& worker : workers_) {
      if (worker.joinable()) {
        worker.join();
      }
    }
    workers_.clear();
    accept_ex_ = nullptr;
    accept_depth_ = 0;
    if (io_h_ != nullptr) {
      CloseHandle(io_h_);
      io_h_ = nullptr;
    }
  }
  template <typename Fn>
  void setOnRequest(Fn&& fn) {
    on_request_ = std::forward<Fn>(fn);
  }
  template <typename Fn>
  void setOnConnection(Fn&& fn) {
    on_client_connected_ = std::forward<Fn>(fn);
  }
  template <typename Fn>
  void setOnDisconnection(Fn&& fn) {
    on_client_disconnected_ = std::forward<Fn>(fn);
  }

 private:
  // ---------------------------------------------------------------------------
  // METHODs                                                         ( private )
  //
  bool postAccept() {
    SOCKET soc = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                            WSA_FLAG_OVERLAPPED);
    if (soc == INVALID_SOCKET) return false;
    overlapped_accept* ova = new overlapped_accept(soc);
    DWORD received = 0;
    BOOL accepted = accept_ex_(accept_socket_, ova->socket, ova->addresses, 0,
                               kAABs, kAABs, &received, ova);
    if (accepted == FALSE && WSAGetLastError() != ERROR_IO_PENDING) {
      closesocket(ova->socket);
      delete ova;
      return false;
    }
    return true;
  }
  std::size_t setupListener(const char port[]) {
    // Let's use our help class to create a cpu pinning plan!
    auto hc = std::thread::hardware_concurrency();
    auto workers = std::max(1u, hc ? hc / 2 : 1u);
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
  void setupAcceptPipeline(std::size_t accepts_in_flight) {
    accept_depth_ = accepts_in_flight;
    for (std::size_t i = 0; i < accept_depth_; i++) {
      if (!postAccept()) {
        throw std::runtime_error("AcceptEx pipeline could not be armed!");
      }
    }
  }
  void setupWorkers(std::size_t number_of_workers) {
    for (std::size_t i = 0; i < number_of_workers; i++) {
      workers_.emplace_back(std::thread([this]() {
        bool stopping = false;
        while (true) {
          ULONG_PTR key = NULL;
          LPOVERLAPPED lpo = NULL;
          DWORD bytes = 0;
          DWORD tout = INFINITE;
          if (stopping && context::empty()) break;
          BOOL st = GetQueuedCompletionStatus(io_h_, &bytes, &key, &lpo, tout);
          DWORD err = st ? ERROR_SUCCESS : GetLastError();
          overlapped_base* ovb = reinterpret_cast<overlapped_base*>(lpo);
          if (st == TRUE) {
            if (ovb->type == io_type::kAccept) {
              auto* ova = reinterpret_cast<overlapped_accept*>(ovb);
              handleAccept(ova);
              delete ova;
            } else if (ovb->type == io_type::kReceive) {
              handleReceive(ovb->ctx, bytes);
            } else if (ovb->type == io_type::kSend) {
              handleSend(ovb->ctx, bytes);
            } else if (ovb->type == io_type::kStop) {
              auto* ovx = reinterpret_cast<overlapped_stop*>(ovb);
              stopping = true;
              delete ovx;
            }
            continue;
          }
          handleError(ovb);
        }
      }));
    }
  }
  void handleAccept(overlapped_accept* ova) {
    if (!postAccept()) return;
    if (ova->socket == INVALID_SOCKET) return;
    // Let's create a new context for this accepted socket!
    std::shared_ptr<context> ctx = std::make_shared<context>(ova->socket);
    ctx->ovr = new overlapped_receive(ctx);
    ctx->ovs = new overlapped_send(ctx);
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
    if (!receive(ctx)) {
      closesocket(ova->socket);
      return;
    }
    // Let's call user's callback to notify for new connection!
    if (on_client_connected_) {
      try {
        on_client_connected_();
      } catch (const std::exception& ex) {
        markContextForClosing(ctx);
      } catch (...) {
        markContextForClosing(ctx);
      }
    }
  }
  void handleReceive(std::shared_ptr<context> ctx, DWORD bytes) {
    // If the associated context is in 'closing' status then the operation is
    // discarded and no actions are performed!
    if (!ctx || ctx->closing.load()) return;
    // Any of the following situations will trigger a disconnection!
    if (!on_request_ || !bytes || (ctx->rcv_len + bytes) >= BFsz) {
      markContextForClosing(ctx);
      return;
    }
    ctx->rcv_len += bytes;
    // Now we will perform the request decoding phase!
    while (true) {
      auto [code, req, bytes] = RQty::deserialize(ctx->rcv_buf, ctx->rcv_len);
      // Let's close this context if bytes consumed make no sense!
      if (bytes > ctx->rcv_len) {
        markContextForClosing(ctx);
        return;
      }
      if (code == protocol::deserialization_result::kSucceeded) {
        // In case of 'succeeded' operation, bytes can NOT be zero!
        if (bytes == 0) {
          markContextForClosing(ctx);
          return;
        }
        // Let's call user handler!
        std::shared_ptr<RSty> res = std::make_shared<RSty>();
        on_request_(req, res, [this, ctx](std::shared_ptr<RSty> res) {
          // If the associated context is in 'closing' status then the operation
          // is discarded and no actions are performed!
          if (!ctx || ctx->closing.load()) return;
          std::unique_lock<std::mutex> responses_lock(ctx->responses_mutex);
          // Let's enqueue incoming robs to the parent context queue!
          ctx->responses.push(res);
          // If a sending is in course, then return!
          if (ctx->sending) return;
          // Let's try to drain robs queue and send!
          tryToDrainRobsAndSend(ctx);
        });
        if (ctx->rcv_len > bytes) {
          std::memmove(ctx->rcv_buf, &ctx->rcv_buf[bytes],
                       ctx->rcv_len - bytes);
        }
        ctx->rcv_len -= bytes;
        continue;
      } else if (code == protocol::deserialization_result::kMoreBytesNeeded) {
        // In case of 'failed' operation, bytes can NOT be greater than zero!
        if (bytes < 0) {
          markContextForClosing(ctx);
          return;
        }
        break;
      } else {
        // For any other result code we'll mark this context as 'closing'..
        markContextForClosing(ctx);
        return;
      }
    }
    // Let's arm next receive operation!
    if (!receive(ctx)) markContextForClosing(ctx);
  }
  void handleSend(std::shared_ptr<context> ctx, DWORD bytes) {
    // If the associated context is in 'closing' status then the operation is
    // discarded and no actions are performed!
    if (!ctx || ctx->closing.load()) return;
    std::unique_lock<std::mutex> responses_lock(ctx->responses_mutex);
    // Let's check for abnormal situations!
    if (bytes > ctx->snd_len) {
      markContextForClosing(ctx);
      return;
    }
    ctx->snd_buf += bytes;
    ctx->snd_len -= bytes;
    // Now we need to check if there are bytes pending to be sent..
    if (ctx->snd_len) {
      ctx->sending = send(ctx);
      return;
    }
    // If we reached this point it means that we have a valid response in
    // queue and we completed the sending action!
    ctx->responses.pop();
    // Let's try to drain robs queue and send!
    tryToDrainRobsAndSend(ctx);
  }
  void handleError(overlapped_base* ovb) {
    if (ovb->ctx) markContextForClosing(ovb->ctx);
    if (ovb->type == io_type::kAccept) {
      delete reinterpret_cast<overlapped_accept*>(ovb);
    } else if (ovb->type == io_type::kStop) {
      delete reinterpret_cast<overlapped_stop*>(ovb);
    }
  }
  void markContextForClosing(std::shared_ptr<context> ctx) {
    bool expected = false;
    if (ctx->closing.compare_exchange_strong(expected, true)) {
      std::unique_lock<std::mutex> responses_lock(ctx->responses_mutex);
      ctx->responses = {};
      ctx->sending = false;
      ctx->ovr->ctx = nullptr;
      ctx->ovs->ctx = nullptr;
      closesocket(ctx->socket);
      ctx->socket = INVALID_SOCKET;
      // Let's call user's callback to notify for disconnection!
      if (on_client_disconnected_) {
        try {
          on_client_disconnected_();
        } catch (const std::exception& ex) {
          // [to-do] -> this is a critical error!
        } catch (...) {
          // [to-do] -> this is a critical error!
        }
      }
    }
  }
  void tryToDrainRobsAndSend(std::shared_ptr<context> ctx) {
    ctx->snd_buf = nullptr;
    ctx->snd_len = 0;
    ctx->wsa_snd.buf = nullptr;
    ctx->wsa_snd.len = 0;
    ctx->sending = false;
    while (!ctx->responses.empty()) {
      std::string_view serialized = ctx->responses.front()->serialize();
      if (!serialized.length()) {
        ctx->responses.pop();
        continue;
      }
      ctx->snd_buf = const_cast<CHAR*>(serialized.data());
      ctx->snd_len = serialized.length();
      break;
    }
    if (!ctx->snd_len) return;
    ctx->sending = send(ctx);
  }
  bool receive(std::shared_ptr<context> ctx) {
    DWORD f = 0, r = 0;
    ctx->wsa_rcv.buf = &ctx->rcv_buf[ctx->rcv_len];
    ctx->wsa_rcv.len = BFsz - ctx->rcv_len;
    int res = WSARecv(ctx->socket, &ctx->wsa_rcv, 1, &r, &f, ctx->ovr, 0);
    return !(res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING);
  }
  bool send(std::shared_ptr<context> ctx) {
    DWORD f = 0, s = 0;
    ctx->wsa_snd.buf = ctx->snd_buf;
    ctx->wsa_snd.len = ctx->snd_len;
    int res = WSASend(ctx->socket, &ctx->wsa_snd, 1, &s, f, ctx->ovs, 0);
    return !(res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING);
  }
  // ---------------------------------------------------------------------------
  // ATTRIBUTEs                                                      ( private )
  //
  HANDLE io_h_ = nullptr;
  SOCKET accept_socket_ = INVALID_SOCKET;
  LPFN_ACCEPTEX accept_ex_ = nullptr;
  std::size_t accept_depth_ = 0;
  std::vector<std::thread> workers_;
  on_request_fn on_request_;
  on_client_connected_fn on_client_connected_;
  on_client_disconnected_fn on_client_disconnected_;
};
}  // namespace martianlabs::doba::transport::server

#endif
