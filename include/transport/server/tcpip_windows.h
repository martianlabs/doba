﻿//      _       _           
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

namespace martianlabs::doba::transport::server {
// =============================================================================
// tcpip [windows]                                                     ( class )
// -----------------------------------------------------------------------------
// This specification holds for the WindowsTM server transport implementation.
// -----------------------------------------------------------------------------
// Template parameters:
//    RQty - request being used.
//    RSty - request being used.
// =============================================================================
template <typename RQty, typename RSty>
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
  using on_req_ok_fn = std::function<process_result(const RQty&, RSty&)>;
  using on_req_err_fn = std::function<void(RSty&)>;
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
        int tcp_no_delay_flag = 1;
        if (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY,
                       (char*)&tcp_no_delay_flag, sizeof(tcp_no_delay_flag)))
          continue;
        // let's associate the accept socket with the i/o port!
        context* per_socket_context = new context(socket, buffer_size_);
        if (!CreateIoCompletionPort((HANDLE)socket, io_h_,
                                    (ULONG_PTR)per_socket_context, 0)) {
          // ((error)) -> trying to associate socket to the i/o port!
          delete per_socket_context;
          continue;
        }
        // let's notify waiting thread for the new connection!
        if (!PostQueuedCompletionStatus(io_h_, 0, (ULONG_PTR)per_socket_context,
                                        NULL)) {
          // ((error)) -> trying to notify waiting thread!
          delete per_socket_context;
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
  void set_port(const std::string_view& port) { port_.assign(port); }
  void set_number_of_workers(uint8_t workers) { workers_ = workers; }
  void set_buffer_size(uint32_t buffer_size) { buffer_size_ = buffer_size; }
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
  static constexpr uint8_t kDefaultNumberOfWorkers = 4;
  static constexpr const char kDefaultPortNumber[] = "80";
  static constexpr uint32_t kDefaultBufferSize = 1024;
  // ___________________________________________________________________________
  // TYPEs                                                           ( private )
  //
  struct context {
    context(SOCKET socket, ULONG buffer_size) {
      ZeroMemory(&ovl, sizeof(WSAOVERLAPPED));
      soc = socket;
      buf = new CHAR[buffer_size];
      wsa.buf = buf;
      wsa.len = buffer_size;
      receiving = true;
      close_after_response = false;
    }
    WSAOVERLAPPED ovl;
    SOCKET soc;
    CHAR FAR* buf;
    WSABUF wsa;
    bool receiving;
    bool close_after_response;
    std::mutex mutex;
    RQty req;
    RSty res;
    std::shared_ptr<std::istream> stream;
    uint32_t ref_count{1};
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
    sockaddr_in address = {0};
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = PF_INET;
    address.sin_port = htons(atoi(port_.c_str()));
    if (bind(sock, (const sockaddr*)&address, sizeof(address)) ==
        SOCKET_ERROR) {
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
        std::queue<context*> removal_queue;
        while (true) {
          ULONG_PTR key = NULL;
          LPOVERLAPPED ovl = NULL;
          DWORD sz = 0;
          // here we act like a garbage collector and delete any non needed
          // context!
          while (!removal_queue.empty()) {
            closesocket(removal_queue.front()->soc);
            delete[] removal_queue.front()->buf;
            delete removal_queue.front();
            removal_queue.pop();
          }
          if (!GetQueuedCompletionStatus(io_h_, &sz, &key, &ovl, INFINITE)) {
            if (!ovl && WSAGetLastError() == ERROR_INVALID_HANDLE) break;
            if (ovl && key) {
              // abrupt connection close!
              std::lock_guard<std::mutex> lock(((context*)key)->mutex);
              if (!((context*)key)->ref_count)
                removal_queue.push((context*)key);
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
                if (on_rcv_.has_value()) on_rcv_.value()(ctx->soc, sz);
                switch (ctx->req.deserialize(ctx->wsa.buf, sz)) {
                  case process_result::kCompleted:
                    if (on_rok_.has_value()) {
                      switch (on_rok_.value()(ctx->req, ctx->res)) {
                        case process_result::kCompleted:
                          sending(ctx, ctx->res.serialize(), false);
                          break;
                        case process_result::kCompletedAndClose:
                          sending(ctx, ctx->res.serialize(), true);
                          break;
                        case process_result::kNeedMoreBytes:
                          break;
                        case process_result::kError:
                          if (on_rer_.has_value()) on_rer_.value()(ctx->res);
                          sending(ctx, ctx->res.serialize(), true);
                          break;
                      }
                    }
                    break;
                  case process_result::kCompletedAndClose:
                    ctx->close_after_response = true;
                    break;
                  case process_result::kNeedMoreBytes:
                    break;
                  case process_result::kError:
                    if (on_rer_.has_value()) on_rer_.value()(ctx->res);
                    sending(ctx, ctx->res.serialize(), true);
                    break;
                }
              } else {
                if (on_snd_.has_value()) on_snd_.value()(ctx->soc, sz);
              }
            } else {
              if (!ctx->ref_count) {
                removal_queue.push(ctx);
                continue;
              }
            }
          } else if (!sz) {
            if (on_cnn_.has_value()) on_cnn_.value()(ctx->soc);
            receiving(ctx);
          }
          if (!perform(ctx) && !ctx->ref_count) removal_queue.push(ctx);
        }
      }));
    }
  }
  void receiving(context* ctx) {
    ctx->req.reset();
    ctx->res.reset();
    ctx->receiving = true;
    ctx->wsa.len = buffer_size_;
  }
  void sending(context* ctx, std::shared_ptr<std::istream> stream, bool car) {
    ctx->stream = stream;
    ctx->receiving = false;
    ctx->close_after_response = car;
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
    auto n = ctx->stream->read(ctx->buf, buffer_size_).gcount();
    if (ctx->stream->bad() || (ctx->stream->fail() && !ctx->stream->eof()))
      return false;
    if (!n) {
      if (ctx->close_after_response) return false;
      receiving(ctx);
      return receive(ctx);
    }
    ctx->wsa.len = (ULONG)n;
    DWORD f = 0, s = 0;
    int res = WSASend(ctx->soc, &ctx->wsa, 1, &s, f, &ctx->ovl, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
      return false;
    ctx->ref_count++;
    return true;
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  bool keep_running_ = true;
  std::string port_ = kDefaultPortNumber;
  uint8_t workers_ = kDefaultNumberOfWorkers;
  uint32_t buffer_size_ = kDefaultBufferSize;
  HANDLE io_h_ = INVALID_HANDLE_VALUE;
  SOCKET accept_socket_ = INVALID_SOCKET;
  std::queue<std::unique_ptr<std::thread>> threads_;
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