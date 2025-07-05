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

#ifndef martianlabs_doba_server_transport_tcpipwindows_h
#define martianlabs_doba_server_transport_tcpipwindows_h

namespace martianlabs::doba::server::transport {
// =============================================================================
// tcpip [windows]                                                     ( class )
// -----------------------------------------------------------------------------
// This specification holds for the WindowsTM server transport implementation.
// -----------------------------------------------------------------------------
// Template parameters:
//    PRty - protocol (request/reponse) being used.
// =============================================================================
template <typename PRty>
class tcpip {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  tcpip() = default;
  tcpip(const tcpip&) = delete;
  tcpip(tcpip&&) noexcept = delete;
  virtual ~tcpip() { stop(); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  tcpip& operator=(const tcpip&) = delete;
  tcpip& operator=(tcpip&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  server::transport::result start(const std::string& port, const uint8_t& number_of_workers) {
    if (io_handle_ != INVALID_HANDLE_VALUE) {
      // ((error)) -> server already initialized!
      return;
    }
    // let's setup all the required resources..
    if (!setupWinsock()) {
      // ((error)) -> could not initialize winsock resources!
      return;
    }
    if (!setupListener(port, number_of_workers)) {
      // ((error)) -> could not initialize socket resources!
      return;
    }
    setupWorkers(number_of_workers);
    // let's start incoming connections loop!
    manager_ = std::make_unique<std::thread>([this]() {
      while (keep_running_) {
        SOCKET socket = WSAAccept(accept_socket_, NULL, NULL, NULL, NULL);
        if (socket == INVALID_SOCKET) {
          // ((error)) -> could not accept incoming connection!
          break;
        }
        // set the socket i/o mode: In this case FIONBIO enables or disables the
        // blocking mode for the socket based on the numerical value of iMode.
        // iMode = 0, blocking mode; iMode != 0, non-blocking mode.
        ULONG i_mode_flag = 1;
        int ioctl_socket_res = ioctlsocket(socket, FIONBIO, &i_mode_flag);
        if (ioctl_socket_res != NO_ERROR) {
          // ((error)) -> trying to set 'blocking' mode!
          continue;
        }
        int tcp_no_delay_flag = 1;
        if (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY,
                       (char*)&tcp_no_delay_flag, sizeof(tcp_no_delay_flag))) {
          // ((error)) -> trying to disable nagle's algorithm!
          continue;
        }
        // let's associate the accept socket with the i/o port!
        context* per_socket_context = new context(socket);
        if (!CreateIoCompletionPort((HANDLE)socket, io_handle_,
                                    (ULONG_PTR)per_socket_context, 0)) {
          // ((error)) -> trying to associate socket to the i/o port!
          delete per_socket_context;
          continue;
        }
        // let's notify waiting thread for the new connection!
        if (!PostQueuedCompletionStatus(io_handle_, 0,
                                        (ULONG_PTR)per_socket_context, NULL)) {
          // ((error)) -> trying to notify waiting thread!
          delete per_socket_context;
          continue;
        }
      }
    });
  }
  bool stop() {
    if (io_handle_ != INVALID_HANDLE_VALUE) {
      if (!CloseHandle(io_handle_)) {
        return false;
      }
      io_handle_ = INVALID_HANDLE_VALUE;
    }
    if (accept_socket_ != INVALID_SOCKET) {
      if (closesocket(accept_socket_) == SOCKET_ERROR) {
        return false;
      }
      accept_socket_ = INVALID_SOCKET;
    }
    keep_running_ = false;
    if (manager_->joinable()) manager_->join();
    while (!workers_.empty()) {
      if (workers_.front()->joinable()) workers_.front()->join();
      workers_.pop();
    }
    keep_running_ = true;
    return true;
  }

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr std::size_t kBufferSize = 2048;
  // ___________________________________________________________________________
  // TYPEs                                                           ( private )
  //
  struct context {
    context(SOCKET socket) {
      ZeroMemory(&ovl, sizeof(WSAOVERLAPPED));
      soc = socket;
      buf = new CHAR[kBufferSize];
      wsa.buf = buf;
      wsa.len = kBufferSize;
      receiving = true;
      close_after_response = false;
    }
    WSAOVERLAPPED ovl;
    SOCKET soc;
    CHAR FAR* buf;
    WSABUF wsa;
    bool receiving;
    bool close_after_response;
    PRty protocol;
    std::mutex mutex;
    typename PRty::req req;
    typename PRty::res res;
    std::shared_ptr<std::istream> stream;
    uint16_t ref_count{1};
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
  bool setupListener(const std::string& port, const uint8_t& workers) {
    // let's create our main i/o completion port!
    HANDLE handle =
        CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, workers);
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
    address.sin_port = htons(atoi(port.c_str()));
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
    io_handle_ = handle;
    accept_socket_ = sock;
    return true;
  }
  void setupWorkers(const uint8_t& number_of_workers) {
    for (int i = 0; i < number_of_workers; i++) {
      workers_.push(std::make_unique<std::thread>([this]() {
        while (true) {
          ULONG_PTR key = NULL;
          LPOVERLAPPED ovl = NULL;
          DWORD sz = 0;
          if (GetQueuedCompletionStatus(io_handle_, &sz, &key, &ovl,
                                        INFINITE)) {
            if (!key) continue;
            bool free_ctx = false;
            context* ctx = (context*)key;
            {
              std::lock_guard<std::mutex> lock(ctx->mutex);
              ctx->ref_count--;
              // let's check if there's a completed operation..
              if (ovl) {
                if (sz) {
                  if (ctx->receiving) {
                    switch (ctx->req.deserialize(ctx->wsa.buf, sz)) {
                      case protocol::result::kCompleted:
                        switch (ctx->protocol.process(ctx->req, ctx->res)) {
                          case protocol::result::kCompleted:
                            sending(ctx, ctx->res.serialize(), false);
                            break;
                          case protocol::result::kCompletedAndClose:
                            sending(ctx, ctx->res.serialize(), true);
                            break;
                          case protocol::result::kError:
                            break;
                        }
                        break;
                      case protocol::result::kCompletedAndClose:
                        ctx->close_after_response = true;
                        break;
                      case protocol::result::kMoreBytesNeeded:
                        break;
                      case protocol::result::kError:
                        break;
                    }
                  }
                } else {
                  free_ctx = !ctx->ref_count;
                }
              } else if (!sz) {
                receiving(ctx);
              }
              free_ctx = free_ctx || (!perform(ctx) && !ctx->ref_count);
            }
            if (free_ctx) delete_context(ctx);
          } else if (ovl && key) {
            // abrupt connection close!
            delete_context(((context*)key));
            continue;
          } else if (!ovl && GetLastError() == ERROR_INVALID_HANDLE) {
            // io port closed! let's exit from loop!
            break;
          }
        }
      }));
    }
  }
  void receiving(context* ctx) {
    ctx->req.reset();
    ctx->res.reset();
    ctx->receiving = true;
    ctx->wsa.len = kBufferSize;
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
    if (res == SOCKET_ERROR) {
      if (WSAGetLastError() != WSA_IO_PENDING) {
        return false;
      }
    }
    ctx->ref_count++;
    return true;
  }
  bool send(context* ctx) {
    auto n = ctx->stream->read(ctx->buf, kBufferSize).gcount();
    if (ctx->stream->bad() || (ctx->stream->fail() && !ctx->stream->eof())) {
      return false;
    }
    if (!n) {
      if (ctx->close_after_response) {
        return false;
      }
      receiving(ctx);
      return receive(ctx);
    }
    ctx->wsa.len = (ULONG)n;
    DWORD f = 0, s = 0;
    int res = WSASend(ctx->soc, &ctx->wsa, 1, &s, f, &ctx->ovl, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      return false;
    }
    ctx->ref_count++;
    return true;
  }
  void delete_context(context* ctx) {
    closesocket(ctx->soc);
    delete[] ctx->buf;
    delete ctx;
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  bool keep_running_ = true;
  HANDLE io_handle_ = INVALID_HANDLE_VALUE;
  SOCKET accept_socket_ = INVALID_SOCKET;
  std::queue<std::unique_ptr<std::thread>> workers_;
  std::unique_ptr<std::thread> manager_;
};
}  // namespace martianlabs::doba::server::transport

#endif