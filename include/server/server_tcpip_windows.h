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
//    PRty - server processor type being used.
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
    if (io_port_ != INVALID_HANDLE_VALUE) {
      // ((Error)) -> server already initialized!
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
          // ((Error)) -> trying to accept a new connection: shutting down?
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
          ovl->socket = s;
          ovl->req_buf = (CHAR*)malloc(kRecvBufSz);
          ovl->res_buf = (CHAR*)malloc(kSendBufSz);
          switch_to_receive_mode(ovl);
          ULONG_PTR ptr_ovl = (ULONG_PTR)ovl;
          if (CreateIoCompletionPort((HANDLE)s, io_port_, ptr_ovl, 0)) {
            // let's notify waiting thread for the new connection!
            if (!PostQueuedCompletionStatus(io_port_, 0, ptr_ovl, NULL)) {
              // ((Error)) -> trying to notify waiting thread!
              // ((To-Do)) -> raise an exception?
            }
          } else {
            // ((Error)) -> trying to associate socket to the i/o port!
            // ((To-Do)) -> raise an exception?
          }
        } else {
          // ((Error)) -> could not change blocking mode on socket!
          // ((To-Do)) -> raise an exception?
        }
      }
      if (io_port_ != INVALID_HANDLE_VALUE) {
        for (auto& worker : workers_) {
          if (worker->joinable()) {
            worker->join();
          }
        }
        io_port_ = INVALID_HANDLE_VALUE;
        accept_socket_ = INVALID_SOCKET;
      }
    });
  }
  void stop() {
    if (io_port_ != INVALID_HANDLE_VALUE) {
      if (CloseHandle(io_port_) == FALSE) {
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
    WSAOVERLAPPED overlapped;
    WSABUF wsaBuffer;
    SOCKET socket;
    CHAR* req_buf;
    CHAR* res_buf;
    ULONG req_len;
    BOOL reading;
    std::mutex lock;
  };
  // ___________________________________________________________________________
  // METHODs                                                         ( private )
  //
  void setupWinsock() {
    struct WsaInitializer_ {
      WsaInitializer_() {
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
          // ((Error)) -> winsock could not be initialized!
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
      // ((Error)) -> while setting up the i/o completion port!
      throw std::runtime_error("could not setup i/o completion port!");
    }
    // let's setup our main listening socket (server)!
    SOCKET sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0,
                             WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET) {
      // ((Error)) -> could not create socket!
      CloseHandle(io);
      throw std::runtime_error("could not create listening socket!");
    }
    // let's associate the listening port to the i/o completion port!
    if (CreateIoCompletionPort((HANDLE)sock, io, 0UL, 0) == nullptr) {
      // ((Error)) -> while setting up the i/o completion port!
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
      // ((Error)) -> could not change blocking mode on socket!
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
      // ((Error)) -> could not bind socket!
      CloseHandle(io);
      closesocket(sock);
      throw std::runtime_error("could not bind to listening socket!");
    }
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
      // ((Error)) -> could not listen socket!
      CloseHandle(io);
      closesocket(sock);
      throw std::runtime_error("could not bind to listening socket!");
    }
    accept_socket_ = sock;
    io_port_ = io;
  }
  void setupWorkers(const uint8_t& number_of_workers) {
    for (int i = 0; i < number_of_workers; i++) {
      workers_.push_back(std::make_shared<std::thread>([this]() {
        ULONG_PTR key = NULL;
        LPOVERLAPPED ovl = NULL;
        DWORD sz = 0;
        while (true) {
          if (GetQueuedCompletionStatus(io_port_, &sz, &key, &ovl, INFINITE)) {
            if (!key) {
              continue;
            }
            Context* context = (Context*)key;
            std::shared_ptr<Context> context_shr = get_context(context->socket);
            std::lock_guard<std::mutex> guard(context->lock);
            switch (context->reading) {
              case true:
                receive(context);
                try {
                  try_to_process_request(context, sz);
                } catch (const std::exception&) {
                  // connection closed! let's free the associated resources!
                  continue;
                }
                break;
              case false:
                send(context);
                break;
            }
            if (ovl) {
              if (!sz) {
                // connection closed! let's free the associated resources!
                unregister_context(context);
              }
            } else if (!sz) {
              // connection created! let's inform user!
              register_context(context);
            }
          } else {
            if (!ovl) {
              if (GetLastError() == ERROR_INVALID_HANDLE) {
                break;
              }
            }
          }
        }
      }));
    }
  }
  void switch_to_receive_mode(Context* ctx) {
    ctx->req_len = 0;
    ctx->wsaBuffer.buf = ctx->req_buf;
    ctx->wsaBuffer.len = kRecvBufSz;
    ctx->reading = true;
  }
  void switch_to_send_mode(Context* ctx) {
    ctx->wsaBuffer.buf = ctx->res_buf;
    ctx->wsaBuffer.len = 0;
    ctx->reading = false;
  }
  void try_to_process_request(Context* ctx, DWORD bytes_received) {
    ctx->wsaBuffer.buf += bytes_received;
    ctx->wsaBuffer.len -= bytes_received;
    ctx->req_len += bytes_received;
    std::size_t consumed = 0;
    auto req = PRty::req::deserialize(ctx->req_buf, ctx->req_len, consumed);
    if (consumed) {
      if (memmove(ctx->req_buf, &ctx->req_buf[consumed],
                  ctx->req_len - consumed)) {
        ctx->req_len -= (ULONG)consumed;
        ctx->wsaBuffer.buf = ctx->req_buf;
        ctx->wsaBuffer.len += (ULONG)consumed;
      } else {
        // ((Error)) -> out of memory!
        throw std::runtime_error("out of memory!");
      }
    }
    if (req.has_value()) {
      auto res = PRty::process(req.value());
      if (res.has_value()) {
        /*
        pepe
        */

        /*
        PRty::res::serialize(res.value(), ctx->res);
        */

        /*
        pepe fin
        */

        switch_to_send_mode(ctx);
        send(ctx);
      }
    } else if (ctx->req_len == kRecvBufSz) {
      switch_to_receive_mode(ctx);
    }
  }
  bool receive(Context* ctx) {
    DWORD recv_flags = 0;
    int res = WSARecv(ctx->socket, &ctx->wsaBuffer, 0x1, NULL, &recv_flags,
                      &ctx->overlapped, nullptr);
    return !(res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING);
  }
  void send(Context* ctx) {
    /*
    pepe
    */

    int res = ::send(ctx->socket, "HTTP/1.1 200 OK\nContent-Length: 0\n\n",
                     (int)strlen("HTTP/1.1 200 OK\nContent-Length: 0\n\n"),
                     0) != SOCKET_ERROR;
    switch_to_receive_mode(ctx);
    receive(ctx);

    /*
    pepe fin
    */
  }
  void register_context(Context* context) {
    std::lock_guard<std::mutex> guard(map_mutex_);
    map_.insert(std::make_pair(
        context->socket,
        std::shared_ptr<Context>(
            context, [this](Context* ctx) { delete_context(ctx); })));
  }
  void unregister_context(Context* context) {
    std::lock_guard<std::mutex> guard(map_mutex_);
    map_.erase(context->socket);
  }
  std::shared_ptr<Context> get_context(SOCKET s) {
    std::lock_guard<std::mutex> guard(map_mutex_);
    std::shared_ptr<Context> result;
    auto itr = map_.find(s);
    if (itr != map_.end()) {
      result = itr->second;
    }
    return result;
  }
  void delete_context(Context* context) {
    std::lock_guard<std::mutex> guard(garbage_mutex_);
    closesocket(context->socket);
    free(context->req_buf);
    free(context->res_buf);
    garbage_.push(context);
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  bool keep_running_ = true;
  HANDLE io_port_ = INVALID_HANDLE_VALUE;
  SOCKET accept_socket_ = INVALID_SOCKET;
  std::vector<std::shared_ptr<std::thread>> workers_;
  std::shared_ptr<std::thread> manager_;
  PRty protocol_;
  std::unordered_map<SOCKET, std::shared_ptr<Context>> map_;
  std::mutex map_mutex_;
  std::queue<Context*> garbage_;
  std::mutex garbage_mutex_;
};
}  // namespace martianlabs::doba::server

#endif