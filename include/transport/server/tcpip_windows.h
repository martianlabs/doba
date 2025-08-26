//      _       _           
//   __| | ___ | |__   __ _ 
//  / _` |/ _ \| '_ \ / _` |
// | (_| | (_) | |_) | (_| |
//  \__,_|\___/|_.__/ \__,_|
// 
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef martianlabs_doba_transport_server_tcpipwindows_h
#define martianlabs_doba_transport_server_tcpipwindows_h

#include <optional>
#include <functional>

#include "reference_buffer.h"

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
template <typename RQty, typename RSty,
          template <typename, typename> typename DEty>
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
  using on_connection_fn = std::function<void(socket_type)>;
  using on_disconnection_fn = std::function<void(socket_type)>;
  using on_bytes_received_fn = std::function<void(socket_type, unsigned long)>;
  using on_bytes_sent_fn = std::function<void(socket_type, unsigned long)>;
  using on_req_ok_fn = std::function<std::shared_ptr<RSty>(
      const RQty&, std::function<std::shared_ptr<RSty>()>)>;
  using on_req_err_fn = std::function<std::shared_ptr<RSty>(
      std::function<std::shared_ptr<RSty>()>)>;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  result start() {
    if (io_h_ != INVALID_HANDLE_VALUE) return result::kAlreadyInitialized;
    // let's setup all the required resources..
    if (!setupWinsock()) return result::kCouldNotSetupPlaformResources;
    if (!setupListener()) return result::kCouldNotSetupPlaformResources;
    setupWorkers();
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
          continue;
        }
        // let's associate the accept socket with the i/o port!
        auto ctx = pop_context(socket);
        if (!CreateIoCompletionPort((HANDLE)socket, io_h_, (ULONG_PTR)ctx, 0)) {
          // ((error)) -> trying to associate socket to the i/o port!
          continue;
        }
        // let's notify waiting thread for the new connection!
        if (!PostQueuedCompletionStatus(io_h_, 0, (ULONG_PTR)ctx, NULL)) {
          // ((error)) -> trying to notify waiting thread!
          continue;
        }
      }
    });
    return result::kSucceeded;
  }
  result stop() {
    if (io_h_ != INVALID_HANDLE_VALUE) {
      if (!CloseHandle(io_h_)) return result::kCouldNotCleanupPlaformResources;
      io_h_ = INVALID_HANDLE_VALUE;
    }
    if (accept_socket_ != INVALID_SOCKET) {
      if (closesocket(accept_socket_) == SOCKET_ERROR)
        return result::kCouldNotCleanupPlaformResources;
      accept_socket_ = INVALID_SOCKET;
    }
    keep_running_ = false;
    if (manager_->joinable()) manager_->join();
    while (!threads_.empty()) {
      if (threads_.front()->joinable()) threads_.front()->join();
      threads_.pop();
    }
    keep_running_ = true;
    return result::kSucceeded;
  }
  void set_port(std::string_view port) { port_.assign(port); }
  void set_number_of_workers(uint8_t workers) { workers_ = workers; }
  void set_on_connection(const on_connection_fn& fn) { on_cnn_ = fn; }
  void set_on_disconnection(const on_disconnection_fn& fn) { on_dis_ = fn; }
  void set_on_bytes_received(const on_bytes_received_fn& fn) { on_rcv_ = fn; }
  void set_on_bytes_sent(const on_bytes_sent_fn& fn) { on_snd_ = fn; }
  void set_on_request_ok(const on_req_ok_fn& fn) { on_rok_ = fn; }
  void set_on_request_error(const on_req_err_fn& fn) { on_rer_ = fn; }

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr uint8_t kDefaultNumberOfWorkers = 2;
  static constexpr const char kDefaultPortNumber[] = "80";
  static constexpr std::size_t kDefaultBufferSize = 1024;
  // ___________________________________________________________________________
  // TYPEs                                                           ( private )
  //
  struct context {
    context() {
      ZeroMemory(buf, kDefaultBufferSize);
      ZeroMemory(&ovl, sizeof(WSAOVERLAPPED));
      soc = INVALID_SOCKET;
      wsa.buf = buf;
      wsa.len = kDefaultBufferSize;
      receiving = true;
      close_after_response = false;
      decoder.reset();
      responses = {};
      reference.reset();
      ref_count = 1;
    }
    void reset() {
      closesocket(soc);
      soc = INVALID_SOCKET;
      wsa.buf = buf;
      wsa.len = kDefaultBufferSize;
      receiving = true;
      close_after_response = false;
      decoder.reset();
      responses = {};
      reference.reset();
      ref_count = 1;
    }
    WSAOVERLAPPED ovl;
    SOCKET soc;
    CHAR FAR buf[kDefaultBufferSize];
    WSABUF wsa;
    bool receiving;
    bool close_after_response;
    std::mutex mutex;
    DEty<RQty, RSty> decoder;
    std::queue<std::shared_ptr<RSty>> responses;
    std::shared_ptr<reference_buffer> reference;
    std::size_t ref_count;
  };
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  bool setupWinsock() {
    struct WsaInitializer {
      bool initialized = false;
      WsaInitializer() { initialized = !WSAStartup(MAKEWORD(2, 2), &wsa_data); }
      ~WsaInitializer() { WSACleanup(); }
      WSADATA wsa_data;
    };
    static WsaInitializer wsa_initializer;
    return wsa_initializer.initialized;
  }
  bool setupListener() {
    // let's create our main i/o completion port!
    HANDLE handle =
        CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, workers_);
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
    sockaddr_in addr = {0};
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(atoi(port_.c_str()));
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
    io_h_ = handle;
    accept_socket_ = sock;
    return true;
  }
  void setupWorkers() {
    for (int i = 0; i < workers_; i++) {
      threads_.push(std::make_unique<std::thread>([this]() {
        while (true) {
          ULONG_PTR key = NULL;
          LPOVERLAPPED ovl = NULL;
          DWORD sz = 0;
          if (!GetQueuedCompletionStatus(io_h_, &sz, &key, &ovl, INFINITE)) {
            if (!ovl) {
              if (WSAGetLastError() == ERROR_INVALID_HANDLE) {
                break;
              }
            } else if (key) {
              push_context((context*)key);
            }
            continue;
          }
          if (!key) continue;
          context* ctx = (context*)key;
          std::lock_guard<std::mutex> lock(ctx->mutex);
          ctx->ref_count--;
          // let's check if there's a completed operation..
          if (ovl) {
            if (sz) {
              if (ctx->receiving) {
                if (ctx->decoder.add(ctx->wsa.buf, sz)) {
                  while (auto req = ctx->decoder.process(
                             [this]() -> auto { return pop_request(); })) {
                    if (on_rok_.has_value()) {
                      enqueue_for_sending(
                          ctx, on_rok_.value()(*req, [this]() -> auto {
                            return pop_response();
                          }));
                    }
                    push_request(req);
                  }
                  sending(ctx, false);
                } else {
                  push_context(ctx);
                  continue;
                }
              } else if (on_snd_.has_value()) {
                on_snd_.value()(ctx->soc, sz);
              }
            } else {
              if (!ctx->ref_count) {
                push_context(ctx);
                continue;
              }
            }
          } else if (!sz) {
            if (on_cnn_.has_value()) {
              on_cnn_.value()(ctx->soc);
            }
            receiving(ctx);
          }
          if (!perform(ctx) && !ctx->ref_count) {
            push_context(ctx);
          }
        }
      }));
    }
  }
  void receiving(context* ctx) {
    ctx->reference = nullptr;
    ctx->receiving = true;
    ctx->wsa.len = kDefaultBufferSize;
  }
  void enqueue_for_sending(context* ctx, auto response) {
    if (response) ctx->responses.push(response);
  }
  void sending(context* ctx, bool car) {
    if (!ctx->responses.empty()) {
      ctx->receiving = false;
      ctx->close_after_response = car;
    }
  }
  bool perform(context* ctx) {
    return ctx->receiving ? receive(ctx) : send(ctx);
  }
  bool receive(context* ctx) {
    DWORD f = 0, r = 0;
    int res = WSARecv(ctx->soc, &ctx->wsa, 1, &r, &f, &ctx->ovl, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
      return false;
    ctx->ref_count++;
    return true;
  }
  bool send(context* ctx) {
    if (!ctx->reference) {
      if (!ctx->responses.empty()) {
        ctx->reference = ctx->responses.front()->serialize();
      }
    }
    if (!ctx->reference) {
      receiving(ctx);
      return receive(ctx);
    }
    auto result = ctx->reference->read(ctx->buf, kDefaultBufferSize);
    if (!result.has_value() || !result.value()) {
      push_response(ctx->responses.front());
      ctx->responses.pop();
      ctx->reference = nullptr;
      return send(ctx);
    }
    ctx->wsa.len = (ULONG)result.value();
    DWORD f = 0, s = 0;
    int res = WSASend(ctx->soc, &ctx->wsa, 1, &s, f, &ctx->ovl, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
      return false;
    ctx->ref_count++;
    return true;
  }
  void push_context(context* ctx) {
    std::lock_guard<std::mutex> lock(contexts_queue_mutex_);
    ctx->reset();
    contexts_queue_.push(ctx);
  }
  context* pop_context(SOCKET soc) {
    std::lock_guard<std::mutex> lock(contexts_queue_mutex_);
    if (contexts_queue_.empty()) contexts_queue_.push(new context());
    context* ctx = contexts_queue_.front();
    ctx->soc = soc;
    contexts_queue_.pop();
    return ctx;
  }
  void push_request(std::shared_ptr<RQty> req) {
    std::lock_guard<std::mutex> lock(reqs_queue_mutex_);
    req->reset();
    reqs_queue_.push(req);
  }
  std::shared_ptr<RQty> pop_request() {
    std::lock_guard<std::mutex> lock(reqs_queue_mutex_);
    if (reqs_queue_.empty()) reqs_queue_.push(std::make_shared<RQty>());
    std::shared_ptr<RQty> req = reqs_queue_.front();
    reqs_queue_.pop();
    return req;
  }
  void push_response(std::shared_ptr<RSty> res) {
    std::lock_guard<std::mutex> lock(ress_queue_mutex_);
    res->reset();
    ress_queue_.push(res);
  }
  std::shared_ptr<RSty> pop_response() {
    std::lock_guard<std::mutex> lock(ress_queue_mutex_);
    if (ress_queue_.empty()) ress_queue_.push(std::make_shared<RSty>());
    std::shared_ptr<RSty> res = ress_queue_.front();
    ress_queue_.pop();
    return res;
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  bool keep_running_ = true;
  std::string port_ = kDefaultPortNumber;
  uint8_t workers_ = kDefaultNumberOfWorkers;
  HANDLE io_h_ = INVALID_HANDLE_VALUE;
  SOCKET accept_socket_ = INVALID_SOCKET;
  std::queue<std::unique_ptr<std::thread>> threads_;
  std::queue<context*> contexts_queue_;
  std::mutex contexts_queue_mutex_;
  std::queue<std::shared_ptr<RQty>> reqs_queue_;
  std::mutex reqs_queue_mutex_;
  std::queue<std::shared_ptr<RSty>> ress_queue_;
  std::mutex ress_queue_mutex_;
  std::unique_ptr<std::thread> manager_;
  std::optional<on_connection_fn> on_cnn_;
  std::optional<on_disconnection_fn> on_dis_;
  std::optional<on_bytes_received_fn> on_rcv_;
  std::optional<on_bytes_sent_fn> on_snd_;
  std::optional<on_req_ok_fn> on_rok_;
  std::optional<on_req_err_fn> on_rer_;
};
}  // namespace martianlabs::doba::transport::server

#endif