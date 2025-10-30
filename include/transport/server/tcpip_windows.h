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

#ifndef martianlabs_doba_transport_server_tcpip_windows_h
#define martianlabs_doba_transport_server_tcpip_windows_h

#include <future>
#include <optional>
#include <functional>

#include "common/rorb.h"
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
  using on_client_request_function = std::function<void(const RQty&, RSty&)>;
  using on_client_request_result =
      std::tuple<on_client_request_function, common::execution_policy>;
  using on_client_request = std::function<on_client_request_result(
      std::shared_ptr<const RQty>, std::shared_ptr<RSty>)>;
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
    if (pool_) pool_->stop();
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
    pool_.reset();
    keep_running_ = true;
    return result::kSucceeded;
  }
  template <typename Fn>
  void set_on_client_request(Fn&& fn) {
    on_client_request_ = std::forward<Fn>(fn);
  }

 private:
  // ___________________________________________________________________________
  // TYPEs                                                           ( private )
  //
  enum class send_result : uint8_t { kSuccess, kSending, kSocketError };
  enum class io_type : uint8_t { kPost, kSend, kReceive };
  struct overlapped {
    overlapped(std::size_t buffer_size = 0, io_type in_type = io_type::kPost) {
      type = in_type;
      buffer = buffer_size ? new char[buffer_size] : nullptr;
      buffer_length = 0;
    }
    overlapped(const overlapped&) = delete;
    overlapped(overlapped&&) noexcept = delete;
    ~overlapped() { delete[] buffer; }
    overlapped& operator=(const overlapped&) = delete;
    overlapped& operator=(overlapped&&) noexcept = delete;
    WSAOVERLAPPED inner_ovl;
    CHAR* buffer;
    DWORD buffer_length;
    io_type type;
    WSABUF wsa;
  };
  struct context {
    context(SOCKET s, std::size_t in_buffer_size) : soc{s} {
      rx_ovl = new overlapped(in_buffer_size, io_type::kReceive);
      tx_ovl = new overlapped(in_buffer_size, io_type::kSend);
    }
    context(const context&) = delete;
    context(context&&) noexcept = delete;
    ~context() {
      delete rx_ovl;
      delete tx_ovl;
    }
    context& operator=(const context&) = delete;
    context& operator=(context&&) noexcept = delete;
    inline void push_sending_packet(const std::pair<CHAR*, DWORD>& packet) {
      std::scoped_lock<std::mutex> lock(sending_queue_mutex);
      sending_queue.push(packet);
    }
    inline auto pop_sending_packet() {
      std::scoped_lock<std::mutex> lock(sending_queue_mutex);
      std::pair<CHAR*, DWORD> packet{nullptr, 0};
      if (!sending_queue.empty()) {
        packet = sending_queue.front();
        sending_queue.pop();
      }
      return packet;
    }
    std::queue<std::pair<CHAR*, DWORD>> sending_queue;
    std::mutex sending_queue_mutex;
    std::atomic<bool> active{true};
    std::atomic<bool> sending{false};
    std::atomic<int> pending_ops{0};
    overlapped* rx_ovl;
    overlapped* tx_ovl;
    DEty decoder;
    SOCKET soc;
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
    if (!(pool_ = std::make_shared<common::thread_pool>(workers))) {
      return false;
    }
    for (int i = 0; i < workers; i++) {
      threads_.emplace(std::thread([this]() {
        while (true) {
          context* ctx = NULL;
          overlapped* ovl = NULL;
          DWORD bytes = 0;
          if (!GetQueuedCompletionStatus(io_completion_port_handler_, &bytes,
                                         (PULONG_PTR)&ctx, (LPOVERLAPPED*)&ovl,
                                         INFINITE)) {
            if (!ovl && GetLastError() == ERROR_INVALID_HANDLE) {
              break;
            }
            if (ovl && ctx) {
              // this means a [closed] connection!
              // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
              // [to-do] -> we need to call user's disconnection event!
              // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
              if (ctx->active.exchange(false)) {
                shutdown(ctx->soc, SD_BOTH);
                closesocket(ctx->soc);
              }
              operation_completed(ctx);
            }
            continue;
          }
          if (!ctx) continue;
          if (!ovl) {
            if (!bytes) {
              // this means a [new] connection!
              // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
              // [to-do] -> we need to call user's connection event!
              // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
              if (!receive(ctx)) {
              }
            }
            continue;
          }
          switch (ovl->type) {
            case io_type::kPost:
              if (auto packet = ctx->pop_sending_packet(); packet.first) {
                switch (send(ctx, packet.first, packet.second)) {
                  case send_result::kSuccess:
                    delete[] packet.first;
                    break;
                  case send_result::kSending:
                    ctx->push_sending_packet(packet);
                    break;
                  case send_result::kSocketError:
                    delete[] packet.first;
                    break;
                }
              }
              delete ovl;
              break;
            case io_type::kSend:
              ctx->sending.store(false, std::memory_order_release);
              if (auto packet = ctx->pop_sending_packet(); packet.first) {
                switch (send(ctx, packet.first, packet.second)) {
                  case send_result::kSuccess:
                    delete[] packet.first;
                    break;
                  case send_result::kSending:
                    ctx->push_sending_packet(packet);
                    break;
                  case send_result::kSocketError:
                    delete[] packet.first;
                    break;
                }
              }
              break;
            case io_type::kReceive:
              if (bytes) {
                if (process(ctx, ovl->wsa.buf, bytes)) {
                  if (!receive(ctx)) {
                    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
                    // [to-do] -> we need to call user's disconnection event!
                    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    if (ctx->active.exchange(false)) {
                      shutdown(ctx->soc, SD_BOTH);
                      closesocket(ctx->soc);
                    }
                  }
                } else {
                  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
                  // [to-do] -> we need to call user's disconnection event!
                  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                  if (ctx->active.exchange(false)) {
                    shutdown(ctx->soc, SD_BOTH);
                    closesocket(ctx->soc);
                  }
                }
              } else {
                // this means a [closed] connection!
                // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
                // [to-do] -> we need to call user's disconnection event!
                // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                if (ctx->active.exchange(false)) {
                  shutdown(ctx->soc, SD_BOTH);
                  closesocket(ctx->soc);
                }
              }
              break;
          }
          operation_completed(ctx);
        }
      }));
    }
    return true;
  }
  inline bool receive(context* ctx) {
    std::memset(&ctx->rx_ovl->inner_ovl, 0, sizeof(WSAOVERLAPPED));
    std::memset(&ctx->rx_ovl->wsa, 0, sizeof(WSABUF));
    DWORD f = 0, r = 0;
    ctx->rx_ovl->wsa.buf = ctx->rx_ovl->buffer;
    ctx->rx_ovl->wsa.len = ULONG(buffer_size_);
    operation_started(ctx);
    int result = WSARecv(ctx->soc, &ctx->rx_ovl->wsa, 1, &r, &f,
                         (LPOVERLAPPED)ctx->rx_ovl, 0);
    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      operation_completed(ctx);
      return false;
    }
    return true;
  }
  inline send_result send(context* ctx, CHAR* buf, std::size_t len) {
    if (ctx->sending.exchange(true, std::memory_order_acq_rel)) {
      return send_result::kSending;
    }
    std::memset(&ctx->tx_ovl->inner_ovl, 0, sizeof(WSAOVERLAPPED));
    std::memset(&ctx->tx_ovl->wsa, 0, sizeof(WSABUF));
    DWORD f = 0, w = 0;
    ctx->tx_ovl->wsa.buf = static_cast<CHAR*>(buf);
    ctx->tx_ovl->wsa.len = static_cast<ULONG>(len);
    operation_started(ctx);
    int result = WSASend(ctx->soc, &ctx->tx_ovl->wsa, 1, &w, f,
                         (LPOVERLAPPED)ctx->tx_ovl, 0);
    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      operation_completed(ctx);
      return send_result::kSocketError;
    }
    return send_result::kSuccess;
  }
  inline bool process(context* ctx, const char* buffer, std::size_t size) {
    auto processor = [this](auto ctx, auto result, auto req, auto res) -> void {
      std::get<on_client_request_function>(result)(*req, *res);
      std::queue<std::shared_ptr<common::rorb>> snd_queue = res->serialize();
      while (!snd_queue.empty()) {
        while (true) {
          std::pair<CHAR*, DWORD> packet;
          packet.first = new CHAR[buffer_size_];
          packet.second = static_cast<DWORD>(buffer_size_);
          auto bytes = snd_queue.front()->read(packet.first, packet.second);
          if (!bytes.has_value() || !bytes.value()) {
            delete[] packet.first;
            break;
          }
          packet.second = static_cast<DWORD>(bytes.value());
          ctx->push_sending_packet(packet);
        }
        snd_queue.pop();
      }
      operation_started(ctx);
      if (!PostQueuedCompletionStatus(
              io_completion_port_handler_, 0, reinterpret_cast<ULONG_PTR>(ctx),
              reinterpret_cast<LPOVERLAPPED>(new overlapped()))) {
        operation_completed(ctx);
      }
    };
    if (!ctx->decoder.add(buffer, size)) {
      return false;
    }
    while (auto req = ctx->decoder.process()) {
      auto res = std::make_shared<RSty>();
      auto result = on_client_request_(req, res);
      switch (std::get<common::execution_policy>(result)) {
        case common::execution_policy::kSync:
          processor(ctx, result, req, res);
          break;
        case common::execution_policy::kAsync:
          pool_->enqueue([this, processor, ctx, result, req, res]() {
            processor(ctx, result, req, res);
          });
          break;
      }
    }
    return true;
  }
  inline void operation_started(context* ctx) {
    ctx->pending_ops.fetch_add(1, std::memory_order_acq_rel);
  }
  inline void operation_completed(context* ctx) {
    if (ctx->pending_ops.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      if (!ctx->active.load(std::memory_order_acquire)) {
        delete ctx;
      }
    }
  }
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  bool keep_running_ = true;
  std::size_t buffer_size_ = 0;
  HANDLE io_completion_port_handler_ = INVALID_HANDLE_VALUE;
  SOCKET accept_socket_ = INVALID_SOCKET;
  std::shared_ptr<common::thread_pool> pool_;
  std::queue<std::thread> threads_;
  std::unique_ptr<std::thread> manager_;
  on_client_request on_client_request_;
};
}  // namespace martianlabs::doba::transport::server

#endif
