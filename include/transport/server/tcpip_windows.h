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

#include <future>
#include <optional>
#include <functional>

#include "common/virtual_buffer.h"
#include "common/thread_pool.h"

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
  using on_client_request_function_prototype =
      std::function<void(const RQty&, RSty&)>;
  using on_client_request_result_prototype =
      std::tuple<on_client_request_function_prototype, std::shared_ptr<RSty>,
                 common::execution_policy>;
  using on_client_connection_prototype = std::function<void(socket_type)>;
  using on_client_disconnection_prototype = std::function<void(socket_type)>;
  using on_bytes_received_prototype =
      std::function<void(socket_type, unsigned long)>;
  using on_bytes_sent_prototype =
      std::function<void(socket_type, unsigned long)>;
  using on_client_request_prototype =
      std::function<on_client_request_result_prototype(
          std::shared_ptr<const RQty>)>;
  using on_error_prototype = std::function<std::shared_ptr<RSty>()>;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  result start() {
    if (io_h_ != INVALID_HANDLE_VALUE) return result::kAlreadyInitialized;
    // let's setup all the required resources..
    if (!setupWinsock()) return result::kCouldNotSetupPlaformResources;
    if (!setupListener()) return result::kCouldNotSetupPlaformResources;
    if (!setupWorkers()) return result::kCouldNotSetupPlaformResources;
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
        context* ctx = pop_context();
        ctx->soc = socket;
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
    if (pool_) {
      pool_->stop();
    }
    if (io_h_ != INVALID_HANDLE_VALUE) {
      if (!CloseHandle(io_h_)) {
        return result::kCouldNotCleanupPlaformResources;
      }
      io_h_ = INVALID_HANDLE_VALUE;
    }
    if (accept_socket_ != INVALID_SOCKET) {
      if (closesocket(accept_socket_) == SOCKET_ERROR) {
        return result::kCouldNotCleanupPlaformResources;
      }
      accept_socket_ = INVALID_SOCKET;
    }
    keep_running_ = false;
    if (manager_->joinable()) manager_->join();
    while (!threads_.empty()) {
      if (threads_.front().joinable()) threads_.front().join();
      threads_.pop();
    }
    pool_.reset();
    keep_running_ = true;
    return result::kSucceeded;
  }
  void set_port(std::string_view port) { port_.assign(port); }
  void set_number_of_workers(uint8_t workers) { workers_ = workers; }
  void set_on_client_connection(const on_client_connection_prototype& fn) {
    on_cnn_ = fn;
  }
  void set_on_client_disconnection(
      const on_client_disconnection_prototype& fn) {
    on_dis_ = fn;
  }
  void set_on_bytes_received(const on_bytes_received_prototype& fn) {
    on_rcv_ = fn;
  }
  void set_on_bytes_sent(const on_bytes_sent_prototype& fn) { on_snd_ = fn; }
  void set_on_client_request(const on_client_request_prototype& fn) {
    on_req_ = fn;
  }
  void set_on_error(const on_error_prototype& fn) { on_err_ = fn; }

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
  enum class io_type { kSend, kReceive };
  struct overlapped {
    overlapped(io_type in_type) : type{in_type} {}
    WSAOVERLAPPED ovl;
    CHAR FAR buffer[kDefaultBufferSize];
    io_type type;
    WSABUF wsa;
  };
  struct context {
    context() {
      soc = INVALID_SOCKET;
      decoder = std::make_shared<DEty>();
    }
    SOCKET soc;
    std::mutex mutex;
    overlapped rx_overlapped{io_type::kReceive};
    std::shared_ptr<DEty> decoder;
    std::atomic<int> operations_in_course{1};
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
  bool setupWorkers() {
    if (!(pool_ = std::make_shared<common::thread_pool>(workers_))) {
      return false;
    }
    for (int i = 0; i < workers_; i++) {
      threads_.emplace(std::thread([this]() {
        while (true) {
          ULONG_PTR key = NULL;
          overlapped* overlapped_operation = NULL;
          DWORD bytes = 0;
          if (!GetQueuedCompletionStatus(io_h_, &bytes, &key,
                                         (LPOVERLAPPED*)&overlapped_operation,
                                         INFINITE)) {
            if (!overlapped_operation &&
                GetLastError() == ERROR_INVALID_HANDLE) {
              // this will cause worker termination!
              break;
            }
            if (overlapped_operation && key) {
              // this means a client-side close operation!
              context* ctx = (context*)key;
              ctx->operations_in_course--;
              if (!ctx->operations_in_course) {
                if (ctx->soc != INVALID_SOCKET) {
                  shutdown(ctx->soc, SD_BOTH);
                  closesocket(ctx->soc);
                  ctx->soc = INVALID_SOCKET;
                  ctx->operations_in_course = 1;
                  ctx->decoder->reset();
                  push_context(ctx);
                }
              }
              if (overlapped_operation->type == io_type::kSend) {
                delete overlapped_operation;
              }
            }
            continue;
          }
          if (!key) continue;
          context* ctx = (context*)key;
          std::lock_guard<std::mutex> lock(ctx->mutex);
          ctx->operations_in_course--;
          if (!overlapped_operation && !bytes) {
            // this means that a new connection was created..
            if (!receive(ctx, &ctx->rx_overlapped)) {
              if (!ctx->operations_in_course) {
                if (ctx->soc != INVALID_SOCKET) {
                  shutdown(ctx->soc, SD_BOTH);
                  closesocket(ctx->soc);
                  ctx->soc = INVALID_SOCKET;
                  ctx->operations_in_course = 1;
                  ctx->decoder->reset();
                  push_context(ctx);
                }
              }
            }
            on_cnn_(ctx->soc);
            continue;
          }
          switch (overlapped_operation->type) {
            case io_type::kReceive:
              if (bytes) {
                if (ctx->decoder->add(overlapped_operation->wsa.buf, bytes)) {
                  if (!process(ctx)) {
                    if (!ctx->operations_in_course) {
                      if (ctx->soc != INVALID_SOCKET) {
                        shutdown(ctx->soc, SD_BOTH);
                        closesocket(ctx->soc);
                        ctx->soc = INVALID_SOCKET;
                        ctx->operations_in_course = 1;
                        ctx->decoder->reset();
                        push_context(ctx);
                      }
                    }
                  }
                  if (!receive(ctx, &ctx->rx_overlapped)) {
                    if (!ctx->operations_in_course) {
                      if (ctx->soc != INVALID_SOCKET) {
                        shutdown(ctx->soc, SD_BOTH);
                        closesocket(ctx->soc);
                        ctx->soc = INVALID_SOCKET;
                        ctx->operations_in_course = 1;
                        ctx->decoder->reset();
                        push_context(ctx);
                      }
                    }
                  }
                }
              } else {
                if (!ctx->operations_in_course) {
                  if (ctx->soc != INVALID_SOCKET) {
                    shutdown(ctx->soc, SD_BOTH);
                    closesocket(ctx->soc);
                    ctx->soc = INVALID_SOCKET;
                    ctx->operations_in_course = 1;
                    ctx->decoder->reset();
                    push_context(ctx);
                  }
                }
              }
              break;
            case io_type::kSend:
              if (bytes) {
              }
              delete overlapped_operation;
              break;
          }
        }
      }));
    }
    return true;
  }
  bool receive(context* ctx, overlapped* ovl) {
    if (ctx->soc == INVALID_SOCKET) return false;
    DWORD f = 0, r = 0;
    ZeroMemory(&ovl->ovl, sizeof(WSAOVERLAPPED));
    ovl->wsa.buf = ovl->buffer;
    ovl->wsa.len = kDefaultBufferSize;
    int result = WSARecv(ctx->soc, &ovl->wsa, 1, &r, &f, (LPOVERLAPPED)ovl, 0);
    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      return false;
    }
    ctx->operations_in_course++;
    return true;
  }
  bool send(context* ctx, overlapped* ovl, CHAR* buffer, std::size_t length) {
    if (ctx->soc == INVALID_SOCKET) return false;
    DWORD f = 0, w = 0;
    ZeroMemory(&ovl->ovl, sizeof(WSAOVERLAPPED));
    ovl->wsa.buf = buffer;
    ovl->wsa.len = (ULONG)length;
    int result = WSASend(ctx->soc, &ovl->wsa, 1, &w, f, (LPOVERLAPPED)ovl, 0);
    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      return false;
    }
    ctx->operations_in_course++;
    return true;
  }
  bool process(context* ctx) {
    auto processor = [this](auto ctx, auto result, auto req, auto res) -> bool {
      std::get<on_client_request_function_prototype>(result)(*req, *res);
      auto serialized = res->serialize();
      while (true) {
        overlapped* ovl = new overlapped(io_type::kSend);
        auto bytes = serialized->read(ovl->buffer, kDefaultBufferSize);
        if (!bytes.has_value() || !bytes.value()) {
          delete ovl;
          break;
        }
        if (!send(ctx, ovl, ovl->buffer, bytes.value())) {
          delete ovl;
          return false;
        }
      }
      return true;
    };
    while (auto req = ctx->decoder->process()) {
      auto result = on_req_(req);
      if (auto res = std::get<std::shared_ptr<RSty>>(result); res) {
        switch (std::get<common::execution_policy>(result)) {
          case common::execution_policy::kSync:
            return processor(ctx, result, req, res);
            break;
          case common::execution_policy::kAsync:
            pool_->enqueue([this, processor, ctx, result, req, res]() {
              std::lock_guard<std::mutex> lock(ctx->mutex);
              if (!processor(ctx, result, req, res)) {
                if (ctx->soc != INVALID_SOCKET) {
                  shutdown(ctx->soc, SD_BOTH);
                  closesocket(ctx->soc);
                  ctx->soc = INVALID_SOCKET;
                  ctx->operations_in_course = 1;
                  ctx->decoder->reset();
                  push_context(ctx);
                }
              }
            });
            break;
        }
      }
    }
    return true;
  }
  void push_context(context* ctx) {
    std::lock_guard<std::mutex> lock(contexts_queue_mutex_);
    contexts_queue_.emplace(ctx);
  }
  context* pop_context() {
    std::lock_guard<std::mutex> lock(contexts_queue_mutex_);
    if (contexts_queue_.empty()) contexts_queue_.emplace(new context());
    context* ctx = contexts_queue_.front();
    contexts_queue_.pop();
    return ctx;
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  bool keep_running_ = true;
  std::string port_ = kDefaultPortNumber;
  uint8_t workers_ = kDefaultNumberOfWorkers;
  HANDLE io_h_ = INVALID_HANDLE_VALUE;
  SOCKET accept_socket_ = INVALID_SOCKET;
  std::shared_ptr<common::thread_pool> pool_;
  std::queue<std::thread> threads_;
  std::queue<context*> contexts_queue_;
  std::mutex contexts_queue_mutex_;
  std::unique_ptr<std::thread> manager_;
  on_client_connection_prototype on_cnn_;
  on_client_disconnection_prototype on_dis_;
  on_bytes_received_prototype on_rcv_;
  on_bytes_sent_prototype on_snd_;
  on_client_request_prototype on_req_;
  on_error_prototype on_err_;
};
}  // namespace martianlabs::doba::transport::server

#endif