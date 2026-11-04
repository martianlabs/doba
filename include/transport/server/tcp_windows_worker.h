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
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef martianlabs_doba_transport_server_tcp_windows_worker_h
#define martianlabs_doba_transport_server_tcp_windows_worker_h

#include <algorithm>
#include <atomic>
#include <charconv>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "network/environment.h"
#include "platform.h"
#include "transport/server/tcp_policies.h"
#include "transport/server/tcp_windows_context.h"
#include "transport/server/tcp_windows_overlapped_accept.h"

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] worker                                                     ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct manages a pool of worker threads that handle TCP connections  |
// | and data. It sets up a listener, accepts new connections, and creates     |
// | a context for each one. It also lets you start and stop the      |
// | workers and set callbacks for when connections are established or         |
// | disconnected.                                                             |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   ENty - engine type being used.                                          |
// |   FAty - engine factory type being used.                                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
struct worker {
 public:
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = tcp_policies;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit worker(policies_type configuration, FAty create_engine)
      : configuration_{configuration},
        create_engine_{std::move(create_engine)} {
    if (!configuration_.receive_buffer_capacity ||
        configuration_.receive_buffer_capacity >
            std::numeric_limits<ULONG>::max()) {
      throw std::runtime_error("Invalid receive buffer size!");
    }
    if (configuration_.worker_count > std::numeric_limits<DWORD>::max()) {
      throw std::runtime_error("Invalid worker count!");
    }
    if (!configuration_.max_buffered_send_bytes) {
      throw std::runtime_error("Invalid send buffer size!");
    }
    if (configuration_.listen_backlog < 0) {
      throw std::runtime_error("Invalid listen backlog!");
    }
  }
  worker(const worker&) = delete;
  worker(worker&&) noexcept = delete;
  ~worker() { stop(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  worker& operator=(const worker&) = delete;
  worker& operator=(worker&&) noexcept = delete;
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // Starts the listener and workers from the sequential owner.
  void start() {
    if (current_worker_ == this) {
      throw std::runtime_error("Transport cannot start from an I/O worker!");
    }
    {
      std::lock_guard<std::mutex> lock(lifecycle_mutex_);
      if (stopping_) {
        throw std::runtime_error(
            "Transport administration must be sequential!");
      }
      if (io_h_ != nullptr) return;
    }
    try {
      in_addr ip_address{};
      if (configuration_.ip.find('\0') != std::string::npos ||
          ::InetPtonA(AF_INET, configuration_.ip.c_str(), &ip_address) != 1) {
        throw std::runtime_error("Invalid IPv4 address!");
      }
      uint16_t port_num = parse_port(configuration_.port);
      std::size_t workers = setup_listener(ip_address, port_num);
      processing_timer_.start(io_h_);
      setup_workers(workers);
      setup_accept_pipeline(workers);
    } catch (...) {
      stop_();
      throw;
    }
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // Stops admission and drains socket I/O from the sequential owner.
  void stop() { stop_(); }
  // +=========================================================================+
  // | [>] set_on_connection                                        ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename FNty>
  void set_on_connection(FNty&& fn) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    ensure_stopped();
    on_connection_ = std::forward<FNty>(fn);
  }
  // +=========================================================================+
  // | [>] set_on_disconnection                                     ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename FNty>
  void set_on_disconnection(FNty&& fn) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    ensure_stopped();
    on_disconnection_ = std::forward<FNty>(fn);
  }

 private:
  // +=========================================================================+
  // | [>] stop_                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  // Drains real accept/context completions; nested stop is rejected.
  void stop_() {
    if (current_worker_ == this) {
      throw std::runtime_error("Transport cannot stop from an I/O worker!");
    }
    HANDLE ioh = nullptr;
    SOCKET listener = INVALID_SOCKET;
    std::vector<std::shared_ptr<context<ENty>>> contexts;
    {
      std::unique_lock<std::mutex> lifecycle_lock(lifecycle_mutex_);
      if (io_h_ == nullptr) return;
      if (stopping_) {
        throw std::runtime_error(
            "Transport administration must be sequential!");
      }
      contexts.reserve(contexts_.size() + pending_accepts_);
      stopping_ = true;
      ioh = io_h_;
      listener = accept_socket_;
      accept_socket_ = INVALID_SOCKET;
    }
    if (listener != INVALID_SOCKET) closesocket(listener);
    {
      std::unique_lock<std::mutex> lifecycle_lock(lifecycle_mutex_);
      lifecycle_cv_.wait(lifecycle_lock,
                         [this]() { return pending_accepts_ == 0; });
      for (const auto& ctx : contexts_) contexts.push_back(ctx);
    }
    processing_timer_.stop();
    for (const auto& ctx : contexts) {
      if (ctx->stop()) finish_context(ctx);
    }
    {
      std::unique_lock<std::mutex> lifecycle_lock(lifecycle_mutex_);
      lifecycle_cv_.wait(lifecycle_lock, [this]() {
        return pending_accepts_ == 0 && contexts_.empty();
      });
    }
    for (std::size_t i = 0; i < workers_.size(); i++) {
      if (!PostQueuedCompletionStatus(ioh, 0, 0, nullptr)) {
        CloseHandle(ioh);
        ioh = nullptr;
        break;
      }
    }
    workers_.clear();
    processing_timer_.reset();
    if (ioh != nullptr) CloseHandle(ioh);
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      io_h_ = nullptr;
      accept_ex_ = nullptr;
      accept_depth_ = 0;
      pending_accepts_ = 0;
      stopping_ = false;
    }
  }
  // +=========================================================================+
  // | [>] parse_port                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // | Converts a port number from a string to `uint16_t`.                    |
  // | If the value is invalid or out of range, it throws a runtime error.    |
  // +-------------------------------------------------------------------------+
  uint16_t parse_port(std::string_view port) const {
    if (port.empty()) {
      throw std::runtime_error("Invalid port (range from 1 to 65535)!");
    }
    unsigned int port_num = 0;
    const char* end = port.data() + port.size();
    auto result = std::from_chars(port.data(), end, port_num);
    if (result.ec != std::errc() || result.ptr != end || port_num < 1 ||
        port_num > 65535) {
      throw std::runtime_error("Invalid port (range from 1 to 65535)!");
    }
    return static_cast<uint16_t>(port_num);
  }
  // +=========================================================================+
  // | [>] ensure_stopped                                          ( private ) |
  // +-------------------------------------------------------------------------+
  // | Checks that the worker is stopped before callbacks can change.         |
  // | If it is, it throws a runtime error. This prevents changing the        |
  // | transport callbacks while the worker is active.                        |
  // +-------------------------------------------------------------------------+
  void ensure_stopped() const {
    if (stopping_ || io_h_ != nullptr) {
      throw std::runtime_error(
          "Transport callbacks cannot change while active!");
    }
  }
  // +=========================================================================+
  // | [>] setup_listener                                          ( private ) |
  // +-------------------------------------------------------------------------+
  // | Sets up the listener socket and I/O completion port.                   |
  // | It creates a socket, binds it to the given IP address and port,        |
  // | and starts listening. It also gets the `AcceptEx` function pointer     |
  // | and links the listener socket to the completion port.                  |
  // | If anything goes wrong, it throws a runtime error.                     |
  // | It returns the number of workers to start.                             |
  // +-------------------------------------------------------------------------+
  std::size_t setup_listener(in_addr ip, uint16_t port_num) {
    std::size_t workers = configuration_.worker_count;
    if (!workers) {
      workers = std::max<std::size_t>(1, std::thread::hardware_concurrency());
    }
    HANDLE ioh = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0,
                                        static_cast<DWORD>(workers));
    if (ioh == NULL) {
      // ((error)) -> Could not create I/O Completion Port!
      throw std::runtime_error("I/O Completion Port could not be created!");
    }
    SOCKET sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                             WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET) {
      // ((error)) -> Could not create socket!
      CloseHandle(ioh);
      throw std::runtime_error("Socket could not be created!");
    }
    sockaddr_in addr{};
    addr.sin_addr = ip;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_num);
    if (bind(sock, (const sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
      // ((error)) -> Could not bind socket!
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error("Could not bind socket!");
    }
    if (listen(sock, configuration_.listen_backlog
                         ? configuration_.listen_backlog
                         : SOMAXCONN) == SOCKET_ERROR) {
      // ((error)) -> Could not listen on socket!
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error("Could not listen on socket!");
    }
    GUID acceptex_guid = WSAID_ACCEPTEX;
    DWORD bytes = 0;
    LPFN_ACCEPTEX accept_ex = nullptr;
    if (WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &acceptex_guid,
                 sizeof(acceptex_guid), &accept_ex, sizeof(accept_ex), &bytes,
                 nullptr, nullptr) == SOCKET_ERROR ||
        accept_ex == nullptr) {
      // ((error)) -> Could not load AcceptEx entry point!
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error("AcceptEx entry point could not be loaded!");
    }
    if (!CreateIoCompletionPort((HANDLE)sock, ioh, 0, 0)) {
      // ((error)) -> Could not associate listener socket to IOCP!
      CloseHandle(ioh);
      closesocket(sock);
      throw std::runtime_error(
          "Listener socket could not be associated to IOCP!");
    }
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      accept_socket_ = sock;
      io_h_ = ioh;
      accept_ex_ = accept_ex;
    }
    return workers;
  }
  // +=========================================================================+
  // | [>] setup_accept_pipeline                                   ( private ) |
  // +-------------------------------------------------------------------------+
  // | Sets up the accept pipeline by working out how many accepts to keep    |
  // | pending and calling `replenish_accept_pipeline()`. If it can’t get the |
  // | pipeline ready, it throws a runtime error.                             |
  // +-------------------------------------------------------------------------+
  void setup_accept_pipeline(std::size_t workers) {
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      accept_depth_ = std::max<std::size_t>(2, workers);
    }
    if (!replenish_accept_pipeline()) {
      // ((error)) -> Could not arm AcceptEx pipeline!
      throw std::runtime_error("AcceptEx pipeline could not be armed!");
    }
  }
  // +=========================================================================+
  // | [>] setup_workers                                           ( private ) |
  // +-------------------------------------------------------------------------+
  // | Starts `number_of_workers` threads to handle I/O. Each thread runs a   |
  // | loop that waits for I/O completions and handles them as they come in.  |
  // | If anything goes wrong, it throws a runtime error.                     |
  // +-------------------------------------------------------------------------+
  std::size_t setup_workers(std::size_t number_of_workers) {
    for (std::size_t i = 0; i < number_of_workers; i++) {
      workers_.emplace_back([this]() {
        current_worker_ = this;
        bool stopping = false;
        while (!stopping) {
          ULONG_PTR key = NULL;
          LPOVERLAPPED lpo = NULL;
          DWORD bytes = 0;  // bytes transfered..
          BOOL st = GetQueuedCompletionStatus(io_h_, &bytes, &key, &lpo,
                                               INFINITE);
          overlapped_base* ovb = reinterpret_cast<overlapped_base*>(lpo);
          if (!ovb) {
            stopping = true;
            continue;
          }
          switch (ovb->get_type()) {
            case io_type::kAccept: {
              std::unique_ptr<overlapped_accept> op(
                  static_cast<overlapped_accept*>(ovb));
              if (st) handle_accept(op.get());
              finish_accept();
              break;
            }
            case io_type::kReceive: {
              std::unique_ptr<overlapped_receive<ENty>> op(
                  static_cast<overlapped_receive<ENty>*>(ovb));
              handle_receive(op->ctx, bytes, st != FALSE);
              break;
            }
            case io_type::kProcessing: {
              std::unique_ptr<overlapped_processing<ENty>> op(
                  static_cast<overlapped_processing<ENty>*>(ovb));
              handle_processing(op->ctx, 0);
              break;
            }
            case io_type::kProcessingTimer: {
              for (auto& [ctx, version] : processing_timer_.take_due()) {
                if (ctx) handle_processing(ctx, version);
              }
              break;
            }
            case io_type::kSend: {
              std::unique_ptr<overlapped_send<ENty>> op(
                  static_cast<overlapped_send<ENty>*>(ovb));
              handle_send(op.get(), bytes, st != FALSE);
              break;
            }
          }
        }
        current_worker_ = nullptr;
      });
    }
    return workers_.size();
  }
  // +=========================================================================+
  // | [>] get_accept_socket                                       ( private ) |
  // +-------------------------------------------------------------------------+
  // | Returns the accept socket while the worker is running.                 |
  // | If it’s stopping, it returns `INVALID_SOCKET`.                         |
  // | The returned socket may close after this call.                         |
  // +-------------------------------------------------------------------------+
  SOCKET get_accept_socket() {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    if (stopping_) return INVALID_SOCKET;
    return accept_socket_;
  }
  // +=========================================================================+
  // | [>] finish_accept                                           ( private ) |
  // +-------------------------------------------------------------------------+
  // | Decreases the number of pending accepts and checks whether the pipeline |
  // | needs topping up. If it does, it calls `replenish_accept_pipeline()`.  |
  // | This method is thread-safe and can be called from any thread.          |
  // +-------------------------------------------------------------------------+
  void finish_accept() {
    bool replenish = false;
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      if (pending_accepts_) pending_accepts_--;
      replenish = !stopping_ && accept_socket_ != INVALID_SOCKET;
    }
    lifecycle_cv_.notify_all();
    if (replenish) replenish_accept_pipeline();
  }
  // +=========================================================================+
  // | [>] register_context                                        ( private ) |
  // +-------------------------------------------------------------------------+
  // | Adds a context to the live set unless the worker is stopping.          |
  // | It tries to add the context and                                        |
  // | returns true if it succeeds. If an exception occurs, it returns false. |
  // | This method is thread-safe and can be called from any thread.          |
  // +-------------------------------------------------------------------------+
  bool register_context(const std::shared_ptr<context<ENty>>& ctx) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    if (stopping_) return false;
    try {
      return contexts_.insert(ctx).second;
    } catch (...) {
      return false;
    }
  }
  // +=========================================================================+
  // | [>] finish_context                                         ( private ) |
  // +-------------------------------------------------------------------------+
  // Erasure is the unique claim for notification, including a stop/completion
  // race. Keep the caller's shared owner through the callback and unlock first.
  void finish_context(const std::shared_ptr<context<ENty>>& ctx) {
    {
      std::lock_guard<std::mutex> lock(lifecycle_mutex_);
      if (!contexts_.erase(ctx)) return;
    }
    ctx->notify_disconnection();
    lifecycle_cv_.notify_all();
  }
  // +=========================================================================+
  // | [>] handle_accept                                           ( private ) |
  // +-------------------------------------------------------------------------+
  // | Checks a completed accept and the listener socket.                      |
  // | is valid, sets up the accepted socket, makes it non-blocking, and sets |
  // | `TCP_NODELAY`. Then it creates a context for the connection, links it  |
  // | to the I/O completion port, and calls the connection callback. If      |
  // | setup fails, the current owner closes the socket.     |
  // | Called on an I/O worker thread.                                        |
  // +-------------------------------------------------------------------------+
  void handle_accept(overlapped_accept* ova) {
    SOCKET listener = get_accept_socket();
    if (listener == INVALID_SOCKET) return;
    post_accept(true);
    const SOCKET accepted = ova->socket;
    if (setsockopt(accepted, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                   reinterpret_cast<const char*>(&listener),
                   sizeof(listener)) == SOCKET_ERROR) return;
    ULONG mode = 1;
    if (ioctlsocket(accepted, FIONBIO, &mode) != NO_ERROR) return;
    int no_delay = 1;
    if (setsockopt(accepted, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<const char*>(&no_delay),
                   sizeof(no_delay)) == SOCKET_ERROR) return;
    auto reservation = connection_reservation::acquire(budget_);
    if (!reservation) return;
    std::shared_ptr<context<ENty>> ctx;
    try {
      ctx = std::make_shared<context<ENty>>(
          accepted, configuration_.receive_buffer_capacity,
          configuration_.max_buffered_send_bytes, create_engine_, stopping_,
          on_connection_, on_disconnection_, std::move(*reservation),
          io_h_, &processing_timer_);
    } catch (...) { return; }
    // Before this transfer the accept operation owns the socket, even when
    // the engine constructor throws. Afterwards the context owns it.
    ova->socket = INVALID_SOCKET;
    if (!CreateIoCompletionPort((HANDLE)accepted, io_h_, 0, 0)) return;
    if (!register_context(ctx)) return;
    try {
      ctx->connected();
      ctx->notify_connection();
      if (ctx->start_io()) finish_context(ctx);
      return;
    } catch (...) {}
    if (ctx->abort()) finish_context(ctx);
  }
  // +=========================================================================+
  // | [>] handle_receive                                          ( private ) |
  // +-------------------------------------------------------------------------+
  // | Handles a finished receive operation. It checks that the receive       |
  // | succeeded, the connection is still open, and another receive can be    |
  // | started. If any of these checks fail, it stops the context.            |
  // | Called on an I/O worker thread.                                        |
  // +-------------------------------------------------------------------------+
  void handle_receive(const std::shared_ptr<context<ENty>>& ctx,
                      DWORD bytes_received, bool success) {
    bool finished;
    try {
      finished = success ? ctx->receive_completed(bytes_received)
                         : ctx->receive_failed();
    } catch (...) { finished = ctx->abort(); }
    if (finished) finish_context(ctx);
  }
  // +=========================================================================+
  // | [>] handle_send                                            ( private ) |
  // +-------------------------------------------------------------------------+
  void handle_send(overlapped_send<ENty>* op, DWORD bytes, bool success) {
    bool finished;
    try {
      finished = success ? op->ctx->send_completed(bytes, op->buffer.len)
                         : op->ctx->send_failed();
    } catch (...) { finished = op->ctx->abort(); }
    if (finished) finish_context(op->ctx);
  }
  // +=========================================================================+
  // | [>] handle_processing                                       ( private ) |
  // +-------------------------------------------------------------------------+
  void handle_processing(const std::shared_ptr<context<ENty>>& ctx,
                         std::uint64_t version) {
    bool finished;
    try { finished = ctx->processing_completed(version); }
    catch (...) { finished = ctx->abort(); }
    if (finished) finish_context(ctx);
  }
  // +=========================================================================+
  // | [>] replenish_accept_pipeline                              ( private ) |
  // +-------------------------------------------------------------------------+
  // | Posts new accept operations until the number waiting reaches the       |
  // | accept depth. If the worker is stopping, it returns                    |
  // | false. It returns true if it posts enough accepts.                     |
  // +-------------------------------------------------------------------------+
  bool replenish_accept_pipeline() {
    for (;;) {
      {
        std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
        if (stopping_ || accept_socket_ == INVALID_SOCKET) {
          return false;
        }
        if (pending_accepts_ >= accept_depth_) return true;
      }
      if (!post_accept(false)) return false;
    }
  }
  // +=========================================================================+
  // | [>] post_accept                                             ( private ) |
  // +-------------------------------------------------------------------------+
  // | Starts a new accept operation by creating a socket and an overlapped   |
  // | structure, then calling `AcceptEx`. If the worker is stopping, it      |
  // | closes the socket and returns false. If enough accepts are pending,    |
  // | it closes the unused socket and returns true.                         |
  // | If `AcceptEx` fails, it closes the socket and returns false.           |
  // | Otherwise, it returns true.                                            |
  // +-------------------------------------------------------------------------+
  bool post_accept(bool replacement) {
    SOCKET soc = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                            WSA_FLAG_OVERLAPPED);
    if (soc == INVALID_SOCKET) return false;
    std::unique_ptr<overlapped_accept> ova(
        new (std::nothrow) overlapped_accept(soc));
    if (!ova) {
      closesocket(soc);
      return false;
    }
    DWORD received = 0;
    std::unique_lock<std::mutex> lifecycle_lock(lifecycle_mutex_);
    if (stopping_ || accept_socket_ == INVALID_SOCKET) {
      lifecycle_lock.unlock();
      return false;
    }
    if (!replacement && pending_accepts_ >= accept_depth_) {
      lifecycle_lock.unlock();
      return true;
    }
    pending_accepts_++;
    BOOL accepted_connection =
        accept_ex_(accept_socket_, ova->socket, ova->addresses, 0,
                   kAcceptAddressBytes, kAcceptAddressBytes,
                   &received, ova.get());
    if (accepted_connection == FALSE && WSAGetLastError() != ERROR_IO_PENDING) {
      // ((error)) -> Could not post AcceptEx operation!
      pending_accepts_--;
      lifecycle_lock.unlock();
      lifecycle_cv_.notify_all();
      return false;
    }
    ova.release();
    return true;
  }
  // +=========================================================================+
  // | USINGs                                                      ( private ) |
  // +-------------------------------------------------------------------------+
  using context_set = std::unordered_set<std::shared_ptr<context<ENty>>>;
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  const policies_type configuration_;
  const std::shared_ptr<connection_budget> budget_{
      std::make_shared<connection_budget>(configuration_.max_connections)};
  const FAty create_engine_;
  network::detail::environment environment_;
  HANDLE io_h_ = nullptr;
  SOCKET accept_socket_ = INVALID_SOCKET;
  LPFN_ACCEPTEX accept_ex_ = nullptr;
  std::mutex lifecycle_mutex_;
  std::condition_variable lifecycle_cv_;
  std::size_t accept_depth_ = 0;
  std::size_t pending_accepts_ = 0;
  // Written by the administrative owner; read by IOCP and context locks.
  std::atomic<bool> stopping_{false};
  std::vector<std::jthread> workers_;
  inline static thread_local const worker* current_worker_{nullptr};
  context_set contexts_;
  // No native timer exists until a connection reports a future deadline.
  windows_processing_timer<ENty> processing_timer_;
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
