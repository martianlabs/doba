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

#include "common/rorb.h"

namespace martianlabs::doba::transport::server {
// =============================================================================
// tcpip [windows]                                                     ( class )
// -----------------------------------------------------------------------------
// This specification holds for the WindowsTM server transport implementation.
// -----------------------------------------------------------------------------
// Template parameters:
//    RQty - request being used.
//    RSty - response being used.
//    DEty - decoder being used.
// =============================================================================
template <typename RQty, typename RSty, typename DEty>
class tcpip {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  tcpip() = default;
  tcpip(const tcpip&) = delete;
  tcpip(tcpip&&) noexcept = delete;
  ~tcpip() { stop(); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  tcpip& operator=(const tcpip&) = delete;
  tcpip& operator=(tcpip&&) noexcept = delete;
  // ___________________________________________________________________________
  // USINGs                                                           ( public )
  //
  using on_request = std::function<void(
      std::shared_ptr<const RQty>, std::function<void(std::shared_ptr<RSty>)>)>;
  using on_connection = std::function<void(uint32_t)>;
  using on_disconnection = std::function<void(uint32_t)>;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  result start(const char port[], std::size_t number_of_workers,
               std::size_t io_buffer_size) {
    if (io_completion_port_handler_ != INVALID_HANDLE_VALUE) {
      return result::kAlreadyInitialized;
    }
    buffer_size_ = io_buffer_size;
    // let's setup all the required resources..
    if (!setup_winsock()) {
      return result::kCouldNotSetupPlaformResources;
    }
    if (!setup_listener(number_of_workers, port)) {
      return result::kCouldNotSetupPlaformResources;
    }
    if (!setup_workers(number_of_workers)) {
      return result::kCouldNotSetupPlaformResources;
    }
    // let's start incoming connections loop!
    manager_ = std::make_unique<std::thread>([this]() {
      while (keep_running_) {
        SOCKET socket = WSAAccept(accept_socket_, NULL, NULL, NULL, NULL);
        if (socket == INVALID_SOCKET) continue;
        // set the socket i/o mode: In this case FIONBIO enables or disables the
        // blocking mode for the socket based on the numerical value of iMode.
        // iMode = 0, blocking mode; iMode != 0, non-blocking mode.
        ULONG i_mode_flag = 1;
        int ioctl_socket_res = ioctlsocket(socket, FIONBIO, &i_mode_flag);
        if (ioctl_socket_res != NO_ERROR) continue;
        // TCP_NODELAY is a socket option in TCP that disables Nagle's
        // algorithm. Nagle's algorithm is a mechanism that delays sending small
        // packets to improve network efficiency by combining them into larger
        // packets. By disabling this algorithm, TCP_NODELAY allows for
        // immediate sending of packets, which can reduce latency but may also
        // lead to more network overhead
        int tcp_no_delay_flag = 1;
        if (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY,
                       (char*)&tcp_no_delay_flag, sizeof(tcp_no_delay_flag))) {
          closesocket(socket);
          continue;
        }
        // let's associate the accept socket with the i/o port!
        context* ctx = new context(socket, buffer_size_);
        if (!CreateIoCompletionPort((HANDLE)socket, io_completion_port_handler_,
                                    (ULONG_PTR)ctx, 0)) {
          // ((error)) -> trying to associate socket to the i/o port!
          delete ctx;
          continue;
        }
        // let's notify waiting thread for the new connection!
        if (!PostQueuedCompletionStatus(io_completion_port_handler_, 0,
                                        (ULONG_PTR)ctx, NULL)) {
          // ((error)) -> trying to notify waiting thread!
          delete ctx;
          continue;
        }
      }
    });
    return result::kSucceeded;
  }
  result stop() {
    if (io_completion_port_handler_ != INVALID_HANDLE_VALUE) {
      if (!CloseHandle(io_completion_port_handler_)) {
        return result::kCouldNotCleanupPlaformResources;
      }
      io_completion_port_handler_ = INVALID_HANDLE_VALUE;
    }
    if (accept_socket_ != INVALID_SOCKET) {
      if (closesocket(accept_socket_) == SOCKET_ERROR) {
        return result::kCouldNotCleanupPlaformResources;
      }
      accept_socket_ = INVALID_SOCKET;
    }
    keep_running_ = false;
    if (manager_ && manager_->joinable()) manager_->join();
    while (!threads_.empty()) {
      if (threads_.front().joinable()) threads_.front().join();
      threads_.pop();
    }
    keep_running_ = true;
    return result::kSucceeded;
  }
  template <typename Fn>
  void set_on_request(Fn&& fn) {
    on_request_ = std::forward<Fn>(fn);
  }
  template <typename Fn>
  void set_on_connection(Fn&& fn) {
    on_connection_ = std::forward<Fn>(fn);
  }
  template <typename Fn>
  void set_on_disconnection(Fn&& fn) {
    on_disconnection_ = std::forward<Fn>(fn);
  }

 private:
  // ___________________________________________________________________________
  // TYPEs                                                           ( private )
  //
  enum class io_type : uint8_t { kEnqueue, kSend, kReceive };
  struct overlapped_base : public OVERLAPPED {
    overlapped_base(io_type in_type) : OVERLAPPED{}, type{in_type}, wsa{} {}
    const io_type type;
    WSABUF wsa;
  };
  struct overlapped_receive : public overlapped_base {
    overlapped_receive(std::size_t in_buffer_size)
        : overlapped_base(io_type::kReceive) {
      buffer = new CHAR[in_buffer_size];
      buffer_size = in_buffer_size;
    }
    ~overlapped_receive() { delete[] buffer; }
    CHAR* buffer;
    DWORD buffer_size;
  };
  struct overlapped_send : public overlapped_base {
    overlapped_send(std::size_t in_buffer_size)
        : overlapped_base(io_type::kSend) {
      buffer = new CHAR[in_buffer_size];
      buffer_size = in_buffer_size;
    }
    ~overlapped_send() { delete[] buffer; }
    CHAR* buffer;
    DWORD buffer_size;
  };
  struct overlapped_enqueue : public overlapped_base {
    overlapped_enqueue() : overlapped_base(io_type::kEnqueue) {}
    std::queue<overlapped_send*> ovls;
  };
  struct context {
    context(SOCKET soc, std::size_t buffer_size) {
      socket = soc;
      ovr = new overlapped_receive(buffer_size);
    }
    context(const context&) = delete;
    context(context&&) noexcept = delete;
    ~context() { delete ovr; }
    context& operator=(const context&) = delete;
    context& operator=(context&&) noexcept = delete;
    void sending_queue_push(std::queue<overlapped_send*>& in) {
      while (!in.empty()) {
        sending_queue.emplace(in.front());
        in.pop();
      }
    }
    overlapped_send* sending_queue_front() const {
      return !sending_queue.empty() ? sending_queue.front() : nullptr;
    }
    void sending_queue_pop() { sending_queue.pop(); }
    overlapped_receive* ovr{nullptr};
    std::queue<overlapped_send*> sending_queue;
    std::mutex sending_queue_mutex;
    std::atomic<bool> sending{false};
    SOCKET socket;
    DEty decoder;
  };
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  bool setup_winsock() {
    struct WsaInitializer {
      bool initialized = false;
      WsaInitializer() { initialized = !WSAStartup(MAKEWORD(2, 2), &wsa_data); }
      ~WsaInitializer() { WSACleanup(); }
      WSADATA wsa_data;
    };
    static WsaInitializer wsa_initializer;
    return wsa_initializer.initialized;
  }
  bool setup_listener(std::size_t workers, const char port[]) {
    // let's create our main i/o completion port!
    HANDLE handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL,
                                           DWORD(workers));
    if (!handle) {
      // ((error)) -> while setting up the i/o completion port!
      return false;
    }
    // let's setup our main listening socket (server)!
    SOCKET sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0,
                             WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET) {
      // ((error)) -> could not create socket!
      CloseHandle(handle);
      return false;
    }
    // let's associate the listening port to the i/o completion port!
    if (CreateIoCompletionPort((HANDLE)sock, handle, 0UL, 0) == nullptr) {
      // ((error)) -> while setting up the i/o completion port!
      CloseHandle(handle);
      closesocket(sock);
      return false;
    }
    // set the socket i/o mode: In this case FIONBIO enables or disables the
    // blocking mode for the socket based on the numerical value of iMode.
    // iMode = 0, blocking mode; iMode != 0, non-blocking mode.
    ULONG i_mode = 0;
    if (ioctlsocket(sock, FIONBIO, &i_mode) != NO_ERROR) {
      // ((error)) -> could not change blocking mode on socket!
      CloseHandle(handle);
      closesocket(sock);
      return false;
    }
    // Permits multiple AF_INET or AF_INET6 sockets to be bound to an
    // identical socket address.  This option must be set on each
    // socket (including the first socket) prior to calling bind(2)
    // on the socket.  To prevent port hijacking, all of the
    // processes binding to the same address must have the same
    // effective UID.  This option can be employed with both TCP and
    // UDP sockets.
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    int port_num = std::stoi(port);
    if (port_num < 1 || port_num > 65535) {
      // ((error)) -> could not set socket opt reuse-address!
      CloseHandle(handle);
      closesocket(sock);
      return false;
    }
    sockaddr_in addr = {0};
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_num);
    if (bind(sock, (const sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
      // ((error)) -> could not bind socket!
      CloseHandle(handle);
      closesocket(sock);
      return false;
    }
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
      // ((error)) -> could not listen socket!
      CloseHandle(handle);
      closesocket(sock);
      return false;
    }
    io_completion_port_handler_ = handle;
    accept_socket_ = sock;
    return true;
  }
  bool setup_workers(std::size_t workers) {
    for (int i = 0; i < workers; i++) {
      threads_.emplace(std::thread([this]() {
        while (true) {
          ULONG_PTR key = NULL;
          overlapped_base* ovl = NULL;
          DWORD bytes = 0;
          if (!GetQueuedCompletionStatus(io_completion_port_handler_, &bytes,
                                         &key, (LPOVERLAPPED*)&ovl, INFINITE)) {
            if (!ovl && GetLastError() == ERROR_INVALID_HANDLE) {
              break;
            }
            if (ovl && key) {
              // this means a [closed] connection!
            }
            continue;
          }
          if (!key) {
            continue;
          }
          context* ctx = reinterpret_cast<context*>(key);
          if (!ovl) {
            if (!bytes) {
              // this means a [new] connection!
              if (!receive(ctx->socket, ctx->ovr)) {
                // this means a [closed] connection!
              }
            }
            continue;
          }
          switch (ovl->type) {
            case io_type::kEnqueue: {
              overlapped_enqueue* ove = static_cast<overlapped_enqueue*>(ovl);
              std::unique_lock<std::mutex> lock(ctx->sending_queue_mutex);
              ctx->sending_queue_push(ove->ovls);
              if (overlapped_send* ovs = ctx->sending_queue_front()) {
                if (!ctx->sending.exchange(true, std::memory_order_acq_rel)) {
                  if (send(ctx->socket, ovs)) {
                    ctx->sending_queue_pop();
                  } else {
                    // let's close this connection!
                  }
                }
                if (!PostQueuedCompletionStatus(io_completion_port_handler_, 0,
                                                (ULONG_PTR)key,
                                                new overlapped_enqueue())) {
                  // let's close this connection!
                }
              }
              delete ove;
              break;
            }
            case io_type::kSend: {
              overlapped_send* ovs = static_cast<overlapped_send*>(ovl);
              ctx->sending.store(false, std::memory_order_release);
              delete ovs;
              break;
            }
            case io_type::kReceive: {
              if (bytes) {
                if (process(ctx, ctx->ovr->wsa.buf, bytes)) {
                  if (!receive(ctx->socket, ctx->ovr)) {
                    // let's close this connection!
                  }
                } else {
                  // let's close this connection!
                }
              } else {
                // let's close this connection!
              }
              break;
            }
          }
        }
      }));
    }
    return true;
  }
  inline bool receive(SOCKET socket, overlapped_receive* ovr) {
    DWORD f = 0, r = 0;
    ovr->wsa.buf = ovr->buffer;
    ovr->wsa.len = ovr->buffer_size;
    int result = WSARecv(socket, &ovr->wsa, 1, &r, &f, (LPOVERLAPPED)ovr, 0);
    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      return false;
    }
    return true;
  }
  inline bool send(SOCKET socket, overlapped_send* ovl) {
    DWORD f = 0, w = 0;
    ovl->wsa.buf = static_cast<CHAR*>(ovl->buffer);
    ovl->wsa.len = static_cast<ULONG>(ovl->buffer_size);
    int result = WSASend(socket, &ovl->wsa, 1, &w, f, (LPOVERLAPPED)ovl, 0);
    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      return false;
    }
    return true;
  }
  inline bool process(context* ctx, const char* buffer, std::size_t size) {
    if (!ctx->decoder.add(buffer, size)) {
      return false;
    }
    while (std::shared_ptr<const RQty> req = ctx->decoder.process()) {
      on_request_(req, [this, ctx](std::shared_ptr<RSty> res) {
        std::queue<std::shared_ptr<common::rorb>> snd_queue = res->serialize();
        overlapped_enqueue* ove = new overlapped_enqueue();
        while (!snd_queue.empty()) {
          while (true) {
            overlapped_send* ovs = new overlapped_send(buffer_size_);
            auto bytes = snd_queue.front()->read(ovs->buffer, ovs->buffer_size);
            if (!bytes.has_value() || !bytes.value()) {
              delete ovs;
              break;
            }
            ovs->buffer_size = bytes.value();
            ove->ovls.emplace(ovs);
          }
          snd_queue.pop();
        }
        if (!PostQueuedCompletionStatus(io_completion_port_handler_, 0,
                                        (ULONG_PTR)ctx, ove)) {
          delete ove;
          // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
          // [to-do] -> add support for this!
          // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        }
      });
    }
    return true;
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  bool keep_running_ = true;
  std::size_t buffer_size_ = 0;
  HANDLE io_completion_port_handler_ = INVALID_HANDLE_VALUE;
  SOCKET accept_socket_ = INVALID_SOCKET;
  std::queue<std::thread> threads_;
  std::unique_ptr<std::thread> manager_;
  on_request on_request_;
  on_connection on_connection_;
  on_disconnection on_disconnection_;
};
}  // namespace martianlabs::doba::transport::server

#endif
