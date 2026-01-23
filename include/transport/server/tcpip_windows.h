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
  // TYPEs                                                           ( private )
  //
  enum class io_type : uint8_t { kEnqueue, kSend, kReceive };
  struct overlapped_base : public OVERLAPPED {
    overlapped_base(io_type in_type) : OVERLAPPED{}, type{in_type} {}
    const io_type type;
  };
  struct overlapped_receive : public overlapped_base {
    overlapped_receive() : overlapped_base(io_type::kReceive) {};
    std::size_t buf_len = 0;
    std::size_t buf_sze = BFsz;
    CHAR buf[BFsz]{};
    WSABUF wsa{};
  };
  struct overlapped_send : public overlapped_base {
    overlapped_send() : overlapped_base(io_type::kSend) {}
    std::size_t buf_len = 0;
    std::size_t buf_sze = BFsz;
    CHAR buf[BFsz]{};
    WSABUF wsa{};
  };
  struct overlapped_enqueue : public overlapped_base {
    overlapped_enqueue(const std::queue<common::rob*>& robs_queue)
        : overlapped_base(io_type::kEnqueue), robs{robs_queue} {}
    std::queue<common::rob*> robs;
  };
  struct context {
    context(SOCKET socket) : soc(socket) {}
    context(const context&) = delete;
    context(context&&) noexcept = delete;
    ~context() {
      closesocket(soc);
      while (!robs_queue.empty()) {
        delete robs_queue.front();
        robs_queue.pop();
      }
    }
    context& operator=(const context&) = delete;
    context& operator=(context&&) noexcept = delete;
    std::atomic<long> io_completions_pending{0};
    std::atomic<bool> send_in_flight{false};
    std::atomic<bool> closing{false};
    SOCKET soc = INVALID_SOCKET;
    std::queue<common::rob*> robs_queue;
    std::mutex robs_queue_mutex;
    overlapped_receive ovr;
    overlapped_send ovs;
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
  using on_send_fn = std::function<void(RSty*)>;
  using on_request_fn = std::function<void(const RQty*, RSty*, on_send_fn)>;
  using on_client_connected_fn = std::function<void()>;
  using on_client_disconnected_fn = std::function<void()>;
  // ---------------------------------------------------------------------------
  // METHODs                                                          ( public )
  //
  result start(const char port[]) {
    if (io_h_ != INVALID_HANDLE_VALUE) {
      return result::kAlreadyInitialized;
    }
    // let's setup all the required resources..
    if (!setup_listener(port)) {
      return result::kCouldNotSetupPlaformResources;
    }
    if (!setup_workers()) {
      return result::kCouldNotSetupPlaformResources;
    }
    // let's start incoming connections loop!
    manager_ = std::make_unique<std::thread>([this]() {
      while (keep_running_.load(std::memory_order_relaxed)) {
        SOCKET socket = WSAAccept(accept_socket_, NULL, NULL, NULL, NULL);
        if (socket == INVALID_SOCKET) {
          continue;
        }
        // set the socket i/o mode: In this case FIONBIO enables or disables the
        // blocking mode for the socket based on the numerical value of iMode.
        // iMode = 0, blocking mode; iMode != 0, non-blocking mode.
        ULONG i_mode_flag = 1;
        int ioctl_socket_res = ioctlsocket(socket, FIONBIO, &i_mode_flag);
        if (ioctl_socket_res != NO_ERROR) {
          closesocket(socket);
          continue;
        }
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
        context* ctx = new context(socket);
        HANDLE handle = CreateIoCompletionPort(
            (HANDLE)socket, io_h_, reinterpret_cast<ULONG_PTR>(ctx), 0);
        if (!handle) {
          // ((error)) -> trying to associate socket to the i/o port!
          closesocket(socket);
          delete ctx;
          continue;
        }
        // let's notify waiting thread for the new connection!
        if (!PostQueuedCompletionStatus(
                io_h_, 0, reinterpret_cast<ULONG_PTR>(ctx), NULL)) {
          // ((error)) -> trying to notify waiting thread!
          closesocket(socket);
          delete ctx;
          continue;
        }
      }
    });
    return result::kSucceeded;
  }
  result stop() {
    keep_running_.store(false, std::memory_order_relaxed);
    if (accept_socket_ != INVALID_SOCKET) {
      if (closesocket(accept_socket_) == SOCKET_ERROR) {
        return result::kCouldNotCleanupPlaformResources;
      }
      accept_socket_ = INVALID_SOCKET;
    }
    if (manager_ && manager_->joinable()) {
      manager_->join();
    }
    if (io_h_ != INVALID_HANDLE_VALUE) {
      if (!CloseHandle(io_h_)) {
        return result::kCouldNotCleanupPlaformResources;
      }
      io_h_ = INVALID_HANDLE_VALUE;
    }
    while (!threads_.empty()) {
      if (threads_.front().joinable()) {
        threads_.front().join();
      }
      threads_.pop();
    }
    keep_running_.store(true, std::memory_order_relaxed);
    return result::kSucceeded;
  }
  template <typename Fn>
  void set_on_request(Fn&& fn) {
    on_request_ = std::forward<Fn>(fn);
  }
  template <typename Fn>
  void set_on_connection(Fn&& fn) {
    on_client_connected_ = std::forward<Fn>(fn);
  }
  template <typename Fn>
  void set_on_disconnection(Fn&& fn) {
    on_client_disconnected_ = std::forward<Fn>(fn);
  }

 private:
  // ---------------------------------------------------------------------------
  // METHODs                                                         ( private )
  //
  bool setup_listener(const char port[]) {
    // let's use our help class to create a cpu pinning plan!
    auto workers = std::thread::hardware_concurrency() / 2;
    // let's create our main i/o completion port!
    auto io_completion_port_handle =
        CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, workers);
    if (!io_completion_port_handle) {
      // ((error)) -> while setting up the i/o completion port!
      return false;
    }
    // let's setup our main listening socket (server)!
    SOCKET sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0,
                             WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET) {
      // ((error)) -> could not create socket!
      CloseHandle(io_completion_port_handle);
      return false;
    }
    // set the socket i/o mode: In this case FIONBIO enables or disables the
    // blocking mode for the socket based on the numerical value of iMode.
    // iMode = 0, blocking mode; iMode != 0, non-blocking mode.
    ULONG i_mode = 0;
    if (ioctlsocket(sock, FIONBIO, &i_mode) != NO_ERROR) {
      // ((error)) -> could not change blocking mode on socket!
      CloseHandle(io_completion_port_handle);
      closesocket(sock);
      return false;
    }
    int port_num = std::stoi(port);
    if (port_num < 1 || port_num > 65535) {
      // ((error)) -> could not set socket opt reuse-address!
      CloseHandle(io_completion_port_handle);
      closesocket(sock);
      return false;
    }
    sockaddr_in addr = {0};
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_num);
    if (bind(sock, (const sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
      // ((error)) -> could not bind socket!
      CloseHandle(io_completion_port_handle);
      closesocket(sock);
      return false;
    }
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
      // ((error)) -> could not listen socket!
      CloseHandle(io_completion_port_handle);
      closesocket(sock);
      return false;
    }
    accept_socket_ = sock;
    number_of_workers_ = workers;
    io_h_ = io_completion_port_handle;
    return true;
  }
  bool setup_workers() {
    for (int i = 0; i < number_of_workers_; i++) {
      threads_.emplace(std::thread([this]() {
        while (true) {
          ULONG_PTR key = NULL;
          LPOVERLAPPED ovl = NULL;
          DWORD bytes = 0;
          DWORD tout = INFINITE;
          bool close_context = false;
          BOOL ios = GetQueuedCompletionStatus(io_h_, &bytes, &key, &ovl, tout);
          context* c = reinterpret_cast<context*>(key);
          if (ios) {
            if (ovl) {
              if (!c->closing.load(std::memory_order_acquire)) {
                switch (reinterpret_cast<overlapped_base*>(ovl)->type) {
                  case io_type::kEnqueue:
                    if (!handle_completion_enqueue(c, ovl)) {
                      bool expected = false;
                      if (c->closing.compare_exchange_strong(expected, true)) {
                        closesocket(c->soc);
                        c->soc = INVALID_SOCKET;
                      }
                    }
                    break;
                  case io_type::kReceive:
                    if (!bytes || !handle_completion_receive(c, bytes)) {
                      bool expected = false;
                      if (c->closing.compare_exchange_strong(expected, true)) {
                        closesocket(c->soc);
                        c->soc = INVALID_SOCKET;
                      }
                    }
                    break;
                  case io_type::kSend:
                    if (!handle_completion_send(c, bytes)) {
                      bool expected = false;
                      if (c->closing.compare_exchange_strong(expected, true)) {
                        closesocket(c->soc);
                        c->soc = INVALID_SOCKET;
                      }
                    }
                    break;
                }
              }
              c->io_completions_pending--;
            } else {
              if (!bytes) {
                // this means a [new] connection!
                if (on_client_connected_) {
                  on_client_connected_();
                }
                if (!receive(c)) {
                  bool expected = false;
                  if (c->closing.compare_exchange_strong(expected, true)) {
                    closesocket(c->soc);
                    c->soc = INVALID_SOCKET;
                  }
                }
              } else {
                bool expected = false;
                if (c->closing.compare_exchange_strong(expected, true)) {
                  closesocket(c->soc);
                  c->soc = INVALID_SOCKET;
                }
              }
            }
          } else {
            if (GetLastError() == ERROR_INVALID_HANDLE) {
              // this means [shoutdown] operations!
              if (!key && !ovl) {
                break;
              }
            } else {
              if (key) {
                bool expected = false;
                if (c->closing.compare_exchange_strong(expected, true)) {
                  closesocket(c->soc);
                  c->soc = INVALID_SOCKET;
                }
              }
              if (ovl) {
                c->io_completions_pending--;
                switch (reinterpret_cast<overlapped_base*>(ovl)->type) {
                  case io_type::kEnqueue:
                    delete reinterpret_cast<overlapped_enqueue*>(ovl);
                    break;
                  case io_type::kReceive:
                  case io_type::kSend:
                    break;
                }
              }
            }
          }
          if (c && c->closing && !c->io_completions_pending) {
            /*
            pepe
            */
            /*
            pepe fin
            */
          }
        }
      }));
    }
    return true;
  }
  inline bool handle_completion_enqueue(context* c, LPOVERLAPPED ovl) {
    overlapped_enqueue* ove = reinterpret_cast<overlapped_enqueue*>(ovl);
    {
      std::lock_guard<std::mutex> responses_lock(c->robs_queue_mutex);
      while (!ove->robs.empty()) {
        c->robs_queue.push(ove->robs.front());
        ove->robs.pop();
      }
    }
    delete ove;
    bool expected = false;
    if (c->send_in_flight.compare_exchange_strong(expected, true)) {
      if (drain_robs_queue(c)) {
        return send(c);
      }
    }
    return true;
  }
  inline bool handle_completion_receive(context* c, DWORD bytes) {
    if ((c->ovr.buf_len + bytes) > c->ovr.buf_sze) {
      // buffer is not big enough to hold for the current request!
      return false;
    }
    c->ovr.buf_len += bytes;
    std::size_t off = 0;
    while (off < c->ovr.buf_len) {
      std::size_t used = 0;
      RQty* req = RQty::from(&c->ovr.buf[off], c->ovr.buf_len - off, used);
      if (!used) {
        if (req) {
          // this is inconsistent (0 used bytes should mean a null req)!
          // returning false here will trigger an automatic disconnection!
          delete req;
          return false;
        } else {
          // buffer does not contain enough data to build a single request!
          break;
        }
      }
      if (off + used > c->ovr.buf_len) {
        // returned number of used bytes is not coherent!
        // returning false here will trigger an automatic disconnection!
        delete req;
        return false;
      }
      if (!req) {
        // there were errors while decoding incoming data!
        // returning false here will trigger an automatic disconnection!
        return false;
      }
      off += used;
      if (on_request_) {
        RSty* res = new RSty();
        on_request_(req, res, [this, c](RSty* res) {
          c->io_completions_pending++;
          if (!PostQueuedCompletionStatus(
                  io_h_, 0, reinterpret_cast<ULONG_PTR>(c),
                  new overlapped_enqueue(res->serialize()))) {
            c->io_completions_pending--;
            // [to-do] -> this is a critical error! we should inform about it!
          }
          delete res;
        });
      }
      delete req;
    }
    // let's adjust the reception buffer now!
    if (c->ovr.buf_len > off) {
      std::memmove(c->ovr.buf, &c->ovr.buf[off], c->ovr.buf_len - off);
    }
    c->ovr.buf_len -= off;
    return receive(c);
  }
  inline bool handle_completion_send(context* c, DWORD bytes) {
    if (bytes > c->ovs.buf_len) {
      // this is inconsistent because we could not send more bytes than
      // specified! returning false here will trigger an automatic
      // disconnection!
      return false;
    }
    c->ovs.buf_len -= bytes;
    if (!drain_robs_queue(c)) {
      c->send_in_flight.store(false, std::memory_order_release);
      return true;
    }
    return send(c);
  }
  inline bool drain_robs_queue(context* c) {
    std::lock_guard<std::mutex> robs_queue_lock(c->robs_queue_mutex);
    std::size_t off = 0;
    while (!c->robs_queue.empty() && off < c->ovs.buf_sze) {
      if (auto bytes_read_from_current_rob = c->robs_queue.front()->read(
              &c->ovs.buf[off], c->ovs.buf_sze - off)) {
        off += bytes_read_from_current_rob;
      } else {
        delete c->robs_queue.front();
        c->robs_queue.pop();
      }
    }
    c->ovs.buf_len = off;
    return off > 0;
  }
  inline bool receive(context* c) {
    DWORD f = 0, r = 0;
    std::memset(static_cast<OVERLAPPED*>(&c->ovr), 0, sizeof(OVERLAPPED));
    c->ovr.wsa.buf = &c->ovr.buf[c->ovr.buf_len];
    c->ovr.wsa.len = c->ovr.buf_sze - c->ovr.buf_len;
    c->io_completions_pending++;
    int s = WSARecv(c->soc, &c->ovr.wsa, 1, &r, &f, &c->ovr, 0);
    if (s == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      c->io_completions_pending--;
      return false;
    }
    return true;
  }
  inline bool send(context* c) {
    if (c->ovs.buf_len) {
      DWORD flags = 0, bytes = 0;
      std::memset(static_cast<OVERLAPPED*>(&c->ovs), 0, sizeof(OVERLAPPED));
      c->ovs.wsa.buf = c->ovs.buf;
      c->ovs.wsa.len = c->ovs.buf_len;
      c->io_completions_pending++;
      int s = WSASend(c->soc, &c->ovs.wsa, 1, &bytes, flags, &c->ovs, 0);
      if (s == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        c->send_in_flight.store(false, std::memory_order_release);
        c->io_completions_pending--;
        return false;
      }
    }
    return true;
  }
  // ---------------------------------------------------------------------------
  // ATTRIBUTEs                                                      ( private )
  //
  DWORD number_of_workers_ = 0;
  std::atomic<bool> keep_running_ = true;
  HANDLE io_h_ = INVALID_HANDLE_VALUE;
  SOCKET accept_socket_ = INVALID_SOCKET;
  std::queue<std::thread> threads_;
  std::unique_ptr<std::thread> manager_;
  on_request_fn on_request_;
  on_client_connected_fn on_client_connected_;
  on_client_disconnected_fn on_client_disconnected_;
};
}  // namespace martianlabs::doba::transport::server

#endif
