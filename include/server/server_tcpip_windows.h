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

#ifndef martianlabs_doba_server_serverwindows_h
#define martianlabs_doba_server_serverwindows_h

#include <mutex>
#include <unordered_map>

#include "buffex.h"

namespace martianlabs::doba::server {
// =============================================================================
// server_tcpip_windows                                            ( interface )
// -----------------------------------------------------------------------------
// This specification holds for the WindowsTM server implementation.
// -----------------------------------------------------------------------------
// Template parameters:
//    PRty - processor type being used.
// =============================================================================
template <typename PRty>
class server_tcpip {
 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  server_tcpip() = default;
  server_tcpip(const server_tcpip&) = delete;
  server_tcpip(server_tcpip&&) noexcept = delete;
  virtual ~server_tcpip() { stop(); }
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  server_tcpip& operator=(const server_tcpip&) = delete;
  server_tcpip& operator=(server_tcpip&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  void start(const std::string& port, const uint8_t& number_of_workers) {
    if (io_ != INVALID_HANDLE_VALUE) {
      // ((error)) -> server already initialized!
      throw std::logic_error("Already initialized server!");
    }
    // let's setup all the required resources..
    setupWinsock();
    setupListener(port, number_of_workers);
    setupWorkers(number_of_workers);
    // let's start incoming connections loop!
    manager_ = std::make_shared<std::thread>([this]() {
      while (keep_running_) {
        SOCKET s = WSAAccept(accept_socket_, NULL, NULL, NULL, NULL);
        if (s == INVALID_SOCKET) {
          // ((error)) -> trying to accept a new connection: shutting down?
          break;
        }
        // set the socket i/o mode: In this case FIONBIO enables or disables the
        // blocking mode for the socket based on the numerical value of iMode.
        // iMode = 0, blocking mode; iMode != 0, non-blocking mode.
        ULONG i_mode = 1;
        auto ioctl_socket_res = ioctlsocket(s, FIONBIO, &i_mode);
        if (ioctl_socket_res == NO_ERROR) {
          // let's associate the accept socket with the i/o port!
          Context* ovl = new Context();
          ZeroMemory(ovl, sizeof(Context));
          ovl->rbuf = new CHAR[kRecvBufSz];
          ovl->soc = s;
          switch_to_receive_mode(ovl);
          if (CreateIoCompletionPort((HANDLE)s, io_, (ULONG_PTR)ovl, 0)) {
            // let's notify waiting thread for the new connection!
            if (!PostQueuedCompletionStatus(io_, 0, (ULONG_PTR)ovl, NULL)) {
              // ((error)) -> trying to notify waiting thread!
              // ((to-do)) -> raise an exception?
            }
          } else {
            // ((error)) -> trying to associate socket to the i/o port!
            // ((to-do)) -> raise an exception?
          }
        } else {
          // ((error)) -> could not change blocking mode on socket!
          // ((to-do)) -> raise an exception?
        }
      }
      if (io_ != INVALID_HANDLE_VALUE) {
        for (auto& worker : workers_) {
          if (worker->joinable()) {
            worker->join();
          }
        }
        io_ = INVALID_HANDLE_VALUE;
        accept_socket_ = INVALID_SOCKET;
      }
    });
  }
  void stop() {
    if (io_ != INVALID_HANDLE_VALUE) {
      if (CloseHandle(io_) == FALSE) {
        throw std::logic_error("There were errors while closing i/o port!");
      }
    }
    if (accept_socket_ != INVALID_SOCKET) {
      if (closesocket(accept_socket_) == SOCKET_ERROR) {
        throw std::logic_error("Could not close accept socket!");
      }
    }
    keep_running_ = false;
    if (manager_->joinable()) {
      manager_->join();
    }
  }

 private:
  // ___________________________________________________________________________
  // CONSTANTs                                                       ( private )
  //
  static constexpr std::size_t kRecvBufSz = 1024;
  static constexpr std::size_t kSendBufSz = 1024;
  // ___________________________________________________________________________
  // TYPEs                                                           ( private )
  //
  struct Context {
    WSAOVERLAPPED ovl;
    WSABUF wsa;
    SOCKET soc;
    std::mutex lock;
    CHAR FAR* rbuf;
    bool receiving;
    PRty processor;
  };
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  void setupWinsock() {
    struct WsaInitializer_ {
      WsaInitializer_() {
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
          // ((error)) -> winsock could not be initialized!
          throw std::runtime_error("could not initialize winsock!");
        }
      }
      ~WsaInitializer_() { WSACleanup(); }
      WSADATA wsa_data;
    };
    static auto wsa_initializer_ptr_ = std::make_shared<WsaInitializer_>();
  }
  void setupListener(const std::string& port, const uint8_t& workers) {
    // let's create our main i/o completion port!
    auto io = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, workers);
    if (!io) {
      // ((error)) -> while setting up the i/o completion port!
      throw std::runtime_error("could not setup i/o completion port!");
    }
    // let's setup our main listening socket (server)!
    SOCKET sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0,
                             WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET) {
      // ((error)) -> could not create socket!
      CloseHandle(io);
      throw std::runtime_error("could not create listening socket!");
    }
    // let's associate the listening port to the i/o completion port!
    if (CreateIoCompletionPort((HANDLE)sock, io, 0UL, 0) == nullptr) {
      // ((error)) -> while setting up the i/o completion port!
      CloseHandle(io);
      closesocket(sock);
      throw std::runtime_error("could not setup i/o completion port!");
    }
    // set the socket i/o mode: In this case FIONBIO enables or disables the
    // blocking mode for the socket based on the numerical value of iMode.
    // iMode = 0, blocking mode; iMode != 0, non-blocking mode.
    ULONG i_mode = 0;
    auto ioctl_socket_res = ioctlsocket(sock, FIONBIO, &i_mode);
    if (ioctl_socket_res != NO_ERROR) {
      // ((error)) -> could not change blocking mode on socket!
      CloseHandle(io);
      closesocket(sock);
      throw std::runtime_error("could not set listening socket i/o mode!");
    }
    sockaddr_in address = {0};
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = PF_INET;
    address.sin_port = htons(atoi(port.c_str()));
    auto bind_res = bind(sock, (const sockaddr*)&address, sizeof(address));
    if (bind_res == SOCKET_ERROR) {
      // ((error)) -> could not bind socket!
      CloseHandle(io);
      closesocket(sock);
      throw std::runtime_error("could not bind to listening socket!");
    }
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
      // ((error)) -> could not listen socket!
      CloseHandle(io);
      closesocket(sock);
      throw std::runtime_error("could not bind to listening socket!");
    }
    accept_socket_ = sock;
    io_ = io;
  }
  void setupWorkers(const uint8_t& number_of_workers) {
    for (int i = 0; i < number_of_workers; i++) {
      workers_.push_back(std::make_shared<std::thread>([this]() {
        ULONG_PTR key = NULL;
        LPOVERLAPPED ovl = NULL;
        DWORD sz = 0;
        while (true) {
          if (GetQueuedCompletionStatus(io_, &sz, &key, &ovl, INFINITE)) {
            if (!key) {
              continue;
            }
            Context* ctx = (Context*)key;
            std::lock_guard<std::mutex> guard(ctx->lock);
            // let's check if there's a completed operation..
            if (ovl) {
              if (sz) {
                if (ctx->receiving) {
                  decode(ctx, sz);
                }
              } else {
                unregister_context(ctx);
              }
            } else if (!sz) {
              register_context(ctx);
            }
            // let's start a new asynchronous operation..
            if (ctx->receiving) {
              DWORD f = 0, r = 0;
              int res = WSARecv(ctx->soc, &ctx->wsa, 1, &r, &f, &ctx->ovl, 0);
              if (!res) {
                decode(ctx, r);
              } else if (res == SOCKET_ERROR) {
                if (WSAGetLastError() != WSA_IO_PENDING) {
                  unregister_context(ctx);
                }
              }
            }
          } else if (!ovl && GetLastError() == ERROR_INVALID_HANDLE) {
            // io port closed! let's exit from loop!
            break;
          }
        }
      }));
    }
  }
  void decode(Context* ctx, DWORD length) {
    std::optional<typename PRty::req> request =
        ctx->processor.decode(ctx->wsa.buf, length);
  }
  void register_context(Context* context) {
    std::lock_guard<std::mutex> guard(map_mutex_);
    map_.insert(std::make_pair(
        context->soc, std::shared_ptr<Context>(context, [this](Context* ctx) {
          delete_context(ctx);
        })));
  }
  void unregister_context(Context* context) {
    std::lock_guard<std::mutex> guard(map_mutex_);
    map_.erase(context->soc);
  }
  void delete_context(Context* context) {
    closesocket(context->soc);
  }
  void switch_to_receive_mode(Context* ctx) {
    ctx->wsa.buf = ctx->rbuf;
    ctx->wsa.len = kRecvBufSz;
    ctx->receiving = true;
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  bool keep_running_ = true;
  HANDLE io_ = INVALID_HANDLE_VALUE;
  SOCKET accept_socket_ = INVALID_SOCKET;
  std::vector<std::shared_ptr<std::thread>> workers_;
  std::shared_ptr<std::thread> manager_;
  PRty protocol_;
  std::unordered_map<SOCKET, std::shared_ptr<Context>> map_;
  std::mutex map_mutex_;
};
}  // namespace martianlabs::doba::server

#endif