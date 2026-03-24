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

#include <functional>
#include <unordered_set>

#include "common/rob.h"
#include "common/deserialize_result.h"

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
template <typename RQty, typename RSty,
          template <typename, typename, std::size_t> class DEty,
          std::size_t BFsz>
class tcpip {
  // ---------------------------------------------------------------------------
  // TYPEs                                                           ( private )
  //
  struct context {
    context(SOCKET socket) : soc{socket} {}
    context(const context&) = delete;
    context(context&&) noexcept = delete;
    context& operator=(const context&) = delete;
    context& operator=(context&&) noexcept = delete;
    std::atomic<bool> closing{false};
    std::atomic<bool> snd_in_flight{false};
    std::atomic<bool> rcv_in_flight{false};
    DEty<RQty, RSty, BFsz> decoder;
    std::queue<std::shared_ptr<common::rob>> robs_queue;
    std::mutex robs_queue_mutex;
    SOCKET soc;
  };
  enum class io_type : uint8_t { kAccept, kEnqueue, kSend, kReceive };
  struct overlapped_base : OVERLAPPED {
    explicit overlapped_base(io_type in_type, std::shared_ptr<context> in_ctx)
        : OVERLAPPED{}, type{in_type}, ctx{in_ctx} {}
    const io_type type;
    std::shared_ptr<context> ctx;
  };
  struct overlapped_accept : overlapped_base {
    explicit overlapped_accept(std::shared_ptr<context> in_ctx)
        : overlapped_base(io_type::kAccept, in_ctx) {}
  };
  struct overlapped_enqueue : overlapped_base {
    explicit overlapped_enqueue(
        std::shared_ptr<context> in_ctx,
        std::queue<std::shared_ptr<common::rob>> in_robs)
        : overlapped_base(io_type::kEnqueue, in_ctx), robs{in_robs} {}
    std::queue<std::shared_ptr<common::rob>> robs;
  };
  struct overlapped_receive : overlapped_base {
    explicit overlapped_receive(std::shared_ptr<context> in_ctx)
        : overlapped_base(io_type::kReceive, in_ctx) {}
    const std::size_t buf_sze = BFsz;
    CHAR buf[BFsz]{};
    WSABUF wsa{};
  };
  struct overlapped_send : overlapped_base {
    explicit overlapped_send(std::shared_ptr<context> in_ctx)
        : overlapped_base(io_type::kSend, in_ctx) {}
    const std::size_t buf_sze = BFsz;
    std::size_t buf_off = 0;
    CHAR buf[BFsz]{};
    WSABUF wsa{};
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
    if (io_h_ != INVALID_HANDLE_VALUE) {
      throw std::runtime_error("TCP/IP transport already started!");
    }
    // Let's setup all the required resources..
    setupWorkers(setupListener(port));
    // Let's start incoming connections loop!
    while (keep_running_.load(std::memory_order_relaxed)) {
      SOCKET soc = WSAAccept(accept_socket_, NULL, NULL, NULL, NULL);
      if (soc == INVALID_SOCKET) {
        continue;
      }
      // Set the socket i/o mode: In this case FIONBIO enables or disables the
      // blocking mode for the socket based on the numerical value of iMode.
      // iMode = 0, blocking mode; iMode != 0, non-blocking mode.
      ULONG i_mode_flag = 1;
      int ioctl_socket_res = ioctlsocket(soc, FIONBIO, &i_mode_flag);
      if (ioctl_socket_res != NO_ERROR) {
        closesocket(soc);
        continue;
      }
      // TCP_NODELAY is a socket option in TCP that disables Nagle's
      // algorithm. Nagle's algorithm is a mechanism that delays sending small
      // packets to improve network efficiency by combining them into larger
      // packets. By disabling this algorithm, TCP_NODELAY allows for
      // immediate sending of packets, which can reduce latency but may also
      // lead to more network overhead
      int ndf = 1;
      if (setsockopt(soc, IPPROTO_TCP, TCP_NODELAY, (char*)&ndf, sizeof(ndf))) {
        closesocket(soc);
        continue;
      }
      // Let's associate the accept socket with the i/o port!
      std::shared_ptr<context> ctx = std::make_shared<context>(soc);
      ULONG_PTR key = reinterpret_cast<ULONG_PTR>(ctx.get());
      HANDLE handle = CreateIoCompletionPort((HANDLE)soc, io_h_, key, 0);
      if (!handle) {
        // ((error)) -> Trying to associate socket to the i/o port!
        closesocket(soc);
        continue;
      }
      // Let's notify waiting thread for the new connection!
      overlapped_accept* ova = new overlapped_accept(ctx);
      if (!PostQueuedCompletionStatus(io_h_, 0, key, ova)) {
        // ((error)) -> Trying to notify waiting thread!
        closesocket(soc);
        continue;
      }
    }
  }
  void stop() {
    keep_running_.store(false, std::memory_order_relaxed);
    for (auto& worker : workers_) {
      if (worker.joinable()) {
        worker.join();
      }
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
  std::size_t setupListener(const char port[]) {
    // Let's use our help class to create a cpu pinning plan!
    auto workers = std::thread::hardware_concurrency() / 2;
    // Let's create our main i/o completion port!
    auto io_completion_port_handle =
        CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, workers);
    if (!io_completion_port_handle) {
      // ((error)) -> while setting up the i/o completion port!
      throw std::runtime_error("I/O Completion Port could not be created!");
    }
    // Let's setup our main listening socket (server)!
    SOCKET sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                             WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET) {
      // ((error)) -> Could not create socket!
      CloseHandle(io_completion_port_handle);
      throw std::runtime_error("Socket could not be created!");
    }
    // Set the socket i/o mode: In this case FIONBIO enables or disables the
    // blocking mode for the socket based on the numerical value of iMode.
    // iMode = 0, blocking mode; iMode != 0, non-blocking mode.
    ULONG i_mode = 0;
    if (ioctlsocket(sock, FIONBIO, &i_mode) != NO_ERROR) {
      // ((error)) -> Could not change blocking mode on socket!
      CloseHandle(io_completion_port_handle);
      closesocket(sock);
      throw std::runtime_error("Socket Non-Blocking mode could not be set!");
    }
    int port_num = std::stoi(port);
    if (port_num < 1 || port_num > 65535) {
      // ((error)) -> Could not set socket opt reuse-address!
      CloseHandle(io_completion_port_handle);
      closesocket(sock);
      throw std::runtime_error("Invalid port (range from 1 to 65535)!");
    }
    sockaddr_in addr = {0};
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_num);
    if (bind(sock, (const sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
      // ((error)) -> Could not bind socket!
      CloseHandle(io_completion_port_handle);
      closesocket(sock);
      throw std::runtime_error("Could not bind socket!");
    }
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
      // ((error)) -> Could not listen socket!
      CloseHandle(io_completion_port_handle);
      closesocket(sock);
      throw std::runtime_error("Could not listen on socket!");
    }
    accept_socket_ = sock;
    io_h_ = io_completion_port_handle;
    return workers;
  }
  void setupWorkers(std::size_t number_of_workers) {
    for (int i = 0; i < number_of_workers; i++) {
      workers_.emplace_back(std::thread([this]() {
        while (true) {
          ULONG_PTR key = NULL;
          LPOVERLAPPED lpo = NULL;
          DWORD bytes = 0;
          DWORD tout = INFINITE;
          BOOL sta = GetQueuedCompletionStatus(io_h_, &bytes, &key, &lpo, tout);
          overlapped_base* ovb = reinterpret_cast<overlapped_base*>(lpo);
          switch (sta) {
            case true: {
              switch (ovb->type) {
                case io_type::kAccept:
                  handleAccept(ovb);
                  break;
                case io_type::kReceive:
                  handleReceive(ovb, bytes);
                  break;
                case io_type::kEnqueue:
                  handleEnqueue(ovb);
                  break;
                case io_type::kSend:
                  handleSend(ovb, bytes);
                  break;
              }
              break;
            }
            case false: {
              markContextForClosing(ovb->ctx);
              switch (ovb->type) {
                case io_type::kAccept:
                  delete reinterpret_cast<overlapped_accept*>(ovb);
                  break;
                case io_type::kEnqueue:
                  delete reinterpret_cast<overlapped_enqueue*>(ovb);
                  break;
                case io_type::kReceive:
                  ovb->ctx->rcv_in_flight = false;
                  delete reinterpret_cast<overlapped_receive*>(ovb);
                  break;
                case io_type::kSend:
                  ovb->ctx->snd_in_flight = false;
                  delete reinterpret_cast<overlapped_send*>(ovb);
                  break;
              }
              break;
            }
          }
        }
      }));
    }
  }
  inline void handleAccept(overlapped_base* ovb) {
    overlapped_accept* ova = reinterpret_cast<overlapped_accept*>(ovb);
    // Let's call user's callback to notify for new connection!
    if (on_client_connected_) {
      try {
        on_client_connected_();
      } catch (std::exception ex) {
        // [to-do] -> this is a critical error!
      } catch (...) {
        // [to-do] -> this is a critical error!
      }
    }
    receive(ova->ctx);
    delete ova;
  }
  inline void handleReceive(overlapped_base* ovb, DWORD bytes) {
    overlapped_receive* ovr = reinterpret_cast<overlapped_receive*>(ovb);
    // If the associated context is in 'closing' status then the operation is
    // discarded and no actions are performed!
    if (ovr->ctx->closing) {
      ovr->ctx->rcv_in_flight = false;
      delete ovr;
      return;
    }
    // Any of the following situations will trigger a disconnection!
    if (!bytes || !ovr->ctx->decoder.add(ovr->buf, bytes)) {
      ovr->ctx->rcv_in_flight = false;
      markContextForClosing(ovr->ctx);
      delete ovr;
      return;
    }
    bool keep_decoding = true;
    while (keep_decoding) {
      auto [result, req] = ovr->ctx->decoder.deserialize();
      switch (result) {
        case common::deserialize_result::kSucceeded:
          if (on_request_) {
            std::shared_ptr<RSty> res = std::make_shared<RSty>();
            try {
              on_request_(
                  req, res,
                  [ctx = ovr->ctx, iocp = io_h_](std::shared_ptr<RSty> res) {
                    auto* ove = new overlapped_enqueue(ctx, res->serialize());
                    if (!PostQueuedCompletionStatus(iocp, 0, 0xFF, ove)) {
                      // [to-do] -> this is a critical error!
                      delete ove;
                    }
                  });
            } catch (std::exception ex) {
              // [to-do] -> this is a critical error!
              ovr->ctx->rcv_in_flight = false;
              markContextForClosing(ovr->ctx);
              keep_decoding = false;
              delete ovr;
              return;
            } catch (...) {
              // [to-do] -> this is a critical error!
              ovr->ctx->rcv_in_flight = false;
              markContextForClosing(ovr->ctx);
              keep_decoding = false;
              delete ovr;
              return;
            }
          }
          break;
        case common::deserialize_result::kMoreBytesNeeded:
          // no request returned here! there is no need to delete it..
          keep_decoding = false;
          break;
        case common::deserialize_result::kInvalidSource:
          // no request returned here! there is no need to delete it..
          // invalid source data will automatically trigger a disconnection.
          ovr->ctx->rcv_in_flight = false;
          markContextForClosing(ovr->ctx);
          keep_decoding = false;
          delete ovr;
          return;
      }
    }
    ovr->ctx->rcv_in_flight = false;
    receive(ovr->ctx);
    delete ovr;
  }
  inline void handleEnqueue(overlapped_base* ovb) {
    overlapped_enqueue* ove = reinterpret_cast<overlapped_enqueue*>(ovb);
    if (ove->ctx->closing) {
      delete ove;
      return;
    }
    moveRobs(ove);
    bool expected = false;
    if (ove->ctx->snd_in_flight.compare_exchange_strong(expected, true)) {
      send(ove->ctx);
    }
    delete ove;
  }
  inline void handleSend(overlapped_base* ovb, DWORD bytes) {
    overlapped_send* ovs = reinterpret_cast<overlapped_send*>(ovb);
    if (ovs->ctx->closing) {
      ovs->ctx->snd_in_flight = false;
      delete ovs;
      return;
    }
    send(ovs->ctx);
    ovs->ctx->snd_in_flight = false;
    delete ovs;
  }
  inline void moveRobs(overlapped_enqueue* ove) {
    std::unique_lock<std::mutex> robs_queue_lock(ove->ctx->robs_queue_mutex);
    while (!ove->robs.empty()) {
      ove->ctx->robs_queue.emplace(ove->robs.front());
      ove->robs.pop();
    }
  }
  inline bool drainRobs(overlapped_send* ovs) {
    std::unique_lock<std::mutex> robs_queue_lock(ovs->ctx->robs_queue_mutex);
    ovs->wsa.buf = ovs->buf;
    std::size_t off = 0;
    while (!ovs->ctx->robs_queue.empty()) {
      auto bytes_read_from_current_rob = ovs->ctx->robs_queue.front()->read(
          &ovs->buf[off], ovs->buf_sze - off);
      if (!bytes_read_from_current_rob) {
        ovs->ctx->robs_queue.pop();
        continue;
      }
      off += bytes_read_from_current_rob;
      if (off == ovs->buf_sze) {
        break;
      }
    }
    ovs->buf_off = off;
    ovs->wsa.len = ovs->buf_off;
    return off > 0;
  }
  inline void markContextForClosing(std::shared_ptr<context> ctx) {
    bool expected = false;
    if (ctx->closing.compare_exchange_strong(expected, true)) {
      closesocket(ctx->soc);
      ctx->soc = INVALID_SOCKET;
      if (on_client_disconnected_) {
        try {
          on_client_disconnected_();
        } catch (std::exception ex) {
          // [to-do] -> this is a critical error!
        } catch (...) {
          // [to-do] -> this is a critical error!
        }
      }
    }
  }
  inline void receive(std::shared_ptr<context> ctx) {
    if (!ctx || ctx->closing) return;
    bool expected = false;
    if (ctx->rcv_in_flight.compare_exchange_strong(expected, true)) {
      DWORD f = 0, r = 0;
      overlapped_receive* ovr = new overlapped_receive(ctx);
      ovr->wsa.buf = ovr->buf;
      ovr->wsa.len = ovr->buf_sze;
      int s = WSARecv(ctx->soc, &ovr->wsa, 1, &r, &f, ovr, 0);
      if (s == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        ctx->rcv_in_flight = false;
        delete ovr;
      }
    }
  }
  inline void send(std::shared_ptr<context> ctx) {
    if (!ctx || ctx->closing) return;
    bool expected = false;
    overlapped_send* ovs = new overlapped_send(ctx);
    if (!drainRobs(ovs)) {
      ovs->ctx->snd_in_flight = false;
      delete ovs;
      return;
    }
    DWORD bytes = 0;
    DWORD flags = 0;
    int s = WSASend(ctx->soc, &ovs->wsa, 1, &bytes, flags, ovs, 0);
    if (s == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      ctx->snd_in_flight = false;
      markContextForClosing(ctx);
      delete ovs;
    }
  }
  // ---------------------------------------------------------------------------
  // ATTRIBUTEs                                                      ( private )
  //
  std::atomic<bool> keep_running_ = true;
  HANDLE io_h_ = INVALID_HANDLE_VALUE;
  SOCKET accept_socket_ = INVALID_SOCKET;
  std::vector<std::thread> workers_;
  on_request_fn on_request_;
  on_client_connected_fn on_client_connected_;
  on_client_disconnected_fn on_client_disconnected_;
};
}  // namespace martianlabs::doba::transport::server

#endif
