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
#include <cstring>
#include <exception>
#include <limits>
#include <memory>
#include <mutex>
#include <new>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "network/environment.h"
#include "platform.h"
#include "transport/server/tcp_windows_context.h"
#include "transport/server/tcp_windows_overlapped_accept.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] worker [windowsTM]                                         ( struct ) |
// +---------------------------------------------------------------------------+
// | This specification holds for the Windows server worker.                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty, typename CNty,
          typename PTy>
struct worker {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = PTy;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit worker(
      policies_type configuration, FAty create_engine,
      typename CNty::shared_state shared_state)
      : configuration_{configuration},
        create_engine_{std::move(create_engine)},
        shared_state_{std::move(shared_state)} {
    if (!configuration_.recv_buffer_size ||
        configuration_.recv_buffer_size >
            std::numeric_limits<ULONG>::max()) {
      throw std::runtime_error("Invalid receive buffer size!");
    }
    if (configuration_.worker_count > std::numeric_limits<DWORD>::max()) {
      throw std::runtime_error("Invalid worker count!");
    }
    if (!configuration_.max_send_buffer_size) {
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
  void start() {
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      if (starting_ || stopping_ || io_h_ != nullptr) return;
      starting_ = true;
    }
    try {
      in_addr ip_address{};
      if (::InetPtonA(AF_INET, configuration_.ip.c_str(), &ip_address) != 1) {
        throw std::runtime_error("Invalid IPv4 address!");
      }
      uint16_t port_num = parse_port(configuration_.port.c_str());
      std::size_t workers = setup_listener(ip_address, port_num);
      setup_workers(workers);
      setup_accept_pipeline(workers);
    } catch (...) {
      stop_(true);
      throw;
    }
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      starting_ = false;
    }
    lifecycle_cv_.notify_all();
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void stop() { stop_(false); }
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
  void stop_(bool starting_failure) {
    if (current_worker_ == this) {
      throw std::runtime_error(
          "Transport cannot stop from an I/O worker!");
    }
    HANDLE ioh = nullptr;
    SOCKET listener = INVALID_SOCKET;
    std::vector<std::shared_ptr<context<ENty, CNty>>> contexts;
    {
      std::unique_lock<std::mutex> lifecycle_lock(lifecycle_mutex_);
      if (starting_failure) {
        starting_ = false;
        lifecycle_cv_.notify_all();
      } else {
        lifecycle_cv_.wait(lifecycle_lock, [this]() { return !starting_; });
      }
      if (io_h_ == nullptr) return;
      if (stopping_) {
        if (stopping_thread_ == std::this_thread::get_id()) return;
        lifecycle_cv_.wait(lifecycle_lock, [this]() { return !stopping_; });
        return;
      }
      stopping_ = true;
      stopping_thread_ = std::this_thread::get_id();
      ioh = io_h_;
      listener = accept_socket_;
      try {
        contexts.reserve(contexts_.size());
      } catch (...) {
        stopping_ = false;
        stopping_thread_ = {};
        lifecycle_cv_.notify_all();
        throw;
      }
      for (const auto& item : contexts_) contexts.emplace_back(item.second);
      accept_socket_ = INVALID_SOCKET;
    }
    if (listener != INVALID_SOCKET) closesocket(listener);
    for (const auto& ctx : contexts) ctx->stop();
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
    if (ioh != nullptr) CloseHandle(ioh);
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      io_h_ = nullptr;
      accept_ex_ = nullptr;
      accept_depth_ = 0;
      pending_accepts_ = 0;
      stopping_ = false;
      stopping_thread_ = {};
    }
    lifecycle_cv_.notify_all();
  }
  // +=========================================================================+
  // | [>] parse_port                                              ( private ) |
  // +-------------------------------------------------------------------------+
  uint16_t parse_port(const char port[]) const {
    if (!port || !*port) {
      throw std::runtime_error("Invalid port (range from 1 to 65535)!");
    }
    unsigned int port_num = 0;
    const char* end = port + std::strlen(port);
    auto result = std::from_chars(port, end, port_num);
    if (result.ec != std::errc() || result.ptr != end || port_num < 1 ||
        port_num > 65535) {
      throw std::runtime_error("Invalid port (range from 1 to 65535)!");
    }
    return static_cast<uint16_t>(port_num);
  }
  // +=========================================================================+
  // | [>] ensure_stopped                                          ( private ) |
  // +-------------------------------------------------------------------------+
  void ensure_stopped() const {
    if (starting_ || stopping_ || io_h_ != nullptr) {
      throw std::runtime_error(
          "Transport callbacks cannot change while active!");
    }
  }
  // +=========================================================================+
  // | [>] setup_listener                                          ( private ) |
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
    sockaddr_in addr = {0};
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
    accept_socket_ = sock;
    io_h_ = ioh;
    accept_ex_ = accept_ex;
    return workers;
  }
  // +=========================================================================+
  // | [>] setup_accept_pipeline                                   ( private ) |
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
  std::size_t setup_workers(std::size_t number_of_workers) {
    for (std::size_t i = 0; i < number_of_workers; i++) {
      workers_.emplace_back(std::jthread([this]() {
        current_worker_ = this;
        bool stopping = false;
        while (!stopping) {
          ULONG_PTR key = NULL;
          LPOVERLAPPED lpo = NULL;
          DWORD bytes = 0;  // bytes transfered..
          DWORD tout = INFINITE;
          BOOL st = GetQueuedCompletionStatus(io_h_, &bytes, &key, &lpo, tout);
          overlapped_base* ovb = reinterpret_cast<overlapped_base*>(lpo);
          if (st == TRUE && ovb == nullptr) {
            stopping = true;
          } else if (st == TRUE) {
            if (ovb->get_type() == io_type::kAccept) {
              auto ova = (overlapped_accept*)ovb;
              handle_accept(ova);
            } else if (ovb->get_type() == io_type::kReceive) {
              auto ovr = (overlapped_receive<ENty, CNty>*)ovb;
              handle_receive(ovr->ctx, bytes);
            } else if (ovb->get_type() == io_type::kSend ||
                       ovb->get_type() == io_type::kOutput) {
              auto ovs = (overlapped_send<ENty, CNty>*)ovb;
              handle_send(ovs, bytes);
            }
          } else if (ovb != nullptr) {
            // If the overlapped operation failed, we need to handle the error
            // based on the type of operation.
            handle_error(ovb);
          } else {
            // If lpo is NULL, it indicates that the completion port is being
            // closed, so we should stop the worker thread.
            stopping = true;
          }
          if (ovb != nullptr) {
            // After handling the overlapped operation, we need to clean up the
            // overlapped structure if it was dynamically allocated.
            handle_overlapped(ovb);
          }
        }
        current_worker_ = nullptr;
      }));
    }
    return workers_.size();
  }
  // +=========================================================================+
  // | [>] get_accept_socket                                       ( private ) |
  // +-------------------------------------------------------------------------+
  SOCKET get_accept_socket() {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    if (stopping_) return INVALID_SOCKET;
    return accept_socket_;
  }
  // +=========================================================================+
  // | [>] finish_accept                                           ( private ) |
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
  bool register_context(const std::shared_ptr<context<ENty, CNty>>& ctx) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    if (stopping_) return false;
    try {
      return contexts_.emplace(ctx.get(), ctx).second;
    } catch (...) {
      return false;
    }
  }
  // +=========================================================================+
  // | [>] retire_context                                          ( private ) |
  // +-------------------------------------------------------------------------+
  void retire_context(context<ENty, CNty>* ctx) {
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      contexts_.erase(ctx);
    }
    lifecycle_cv_.notify_all();
    replenish_accept_pipeline();
  }
  // +=========================================================================+
  // | [>] handle_accept                                           ( private ) |
  // +-------------------------------------------------------------------------+
  void handle_accept(overlapped_accept* ova) {
    SOCKET listener = get_accept_socket();
    if (listener == INVALID_SOCKET) {
      closesocket(ova->socket);
      finish_accept();
      return;
    }
    post_accept(true);
    int result =
        setsockopt(ova->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                   reinterpret_cast<const char*>(&listener), sizeof(listener));
    if (result == SOCKET_ERROR) {
      closesocket(ova->socket);
      finish_accept();
      return;
    }
    ULONG i_mode_flag = 1;
    result = ioctlsocket(ova->socket, FIONBIO, &i_mode_flag);
    if (result != NO_ERROR) {
      closesocket(ova->socket);
      finish_accept();
      return;
    }
    int ndf = 1;
    result = setsockopt(ova->socket, IPPROTO_TCP, TCP_NODELAY,
                        reinterpret_cast<const char*>(&ndf), sizeof(ndf));
    if (result == SOCKET_ERROR) {
      closesocket(ova->socket);
      finish_accept();
      return;
    }
    std::shared_ptr<context<ENty, CNty>> ctx;
    try {
      std::size_t encrypted_receive_buffer_size = 0;
      std::size_t network_bio_buffer_size = 0;
      if constexpr (requires(const PTy& value) {
                      value.encrypted_receive_buffer_size;
                      value.network_bio_buffer_size;
                    }) {
        encrypted_receive_buffer_size =
            configuration_.encrypted_receive_buffer_size;
        network_bio_buffer_size = configuration_.network_bio_buffer_size;
      }
      ctx = std::make_shared<context<ENty, CNty>>(
          ova->socket, configuration_.recv_buffer_size,
          configuration_.max_send_buffer_size, create_engine_, stopping_,
          on_connection_, on_disconnection_,
          [this](context<ENty, CNty>* context) { retire_context(context); },
          shared_state_, encrypted_receive_buffer_size,
          network_bio_buffer_size);
    } catch (...) {
      closesocket(ova->socket);
      finish_accept();
      return;
    }
    if (!CreateIoCompletionPort((HANDLE)ova->socket, io_h_, 0, 0)) {
      closesocket(ova->socket);
      ova->socket = INVALID_SOCKET;
      finish_accept();
      return;
    }
    if (!register_context(ctx)) {
      ctx->abort();
      finish_accept();
      return;
    }
    // Let's call user's callback to notify for new connection!
    try {
      ctx->connected(io_h_);
      ctx->notify_connection();
    } catch (const std::exception&) {
      ctx->stop();
      finish_accept();
      return;
    } catch (...) {
      ctx->stop();
      finish_accept();
      return;
    }
    // Let's arm next receive operation!
    if (!ctx->arm_next_receive_operation()) {
      ctx->stop();
      finish_accept();
      return;
    }
    finish_accept();
  }
  // +=========================================================================+
  // | [>] handle_receive                                          ( private ) |
  // +-------------------------------------------------------------------------+
  void handle_receive(std::shared_ptr<context<ENty, CNty>> ctx,
                      DWORD bytes_received) {
    try {
      if (!ctx->receive_completed(bytes_received)) return;
      if (!bytes_received) {
        if (!ctx->eof()) ctx->abort();
        ctx->stop();
        return;
      }
      if (!ctx->arm_next_receive_operation()) ctx->stop();
    } catch (...) {
      ctx->abort();
      ctx->stop();
    }
  }
  // +=========================================================================+
  // | [>] handle_send                                             ( private ) |
  // +-------------------------------------------------------------------------+
  void handle_send(overlapped_send<ENty, CNty>* ovs, DWORD bytes_sent) {
    try {
      if (ovs->get_type() == io_type::kOutput) ovs->ctx->output_ready();
      else ovs->ctx->send_completed(bytes_sent, ovs->submitted_size);
    } catch (...) {
      ovs->ctx->abort();
      ovs->ctx->stop();
    }
  }
  // +=========================================================================+
  // | [>] handle_error                                            ( private ) |
  // +-------------------------------------------------------------------------+
  void handle_error(overlapped_base* ovb) {
    switch (ovb->get_type()) {
      case io_type::kAccept: {
        auto ova = reinterpret_cast<overlapped_accept*>(ovb);
        closesocket(ova->socket);
        finish_accept();
        break;
      }
      case io_type::kReceive:
        reinterpret_cast<overlapped_receive<ENty, CNty>*>(ovb)->ctx
            ->receive_failed();
        break;
      case io_type::kSend:
      case io_type::kOutput:
        reinterpret_cast<overlapped_send<ENty, CNty>*>(ovb)->ctx->send_failed();
        break;
    }
  }
  // +=========================================================================+
  // | [>] handle_overlapped                                       ( private ) |
  // +-------------------------------------------------------------------------+
  void handle_overlapped(overlapped_base* ovb) {
    switch (ovb->get_type()) {
      case io_type::kAccept:
        delete reinterpret_cast<overlapped_accept*>(ovb);
        break;
      case io_type::kReceive:
        reinterpret_cast<overlapped_receive<ENty, CNty>*>(ovb)->ctx
            ->notify_disconnection();
        delete reinterpret_cast<overlapped_receive<ENty, CNty>*>(ovb);
        break;
      case io_type::kSend:
      case io_type::kOutput:
        reinterpret_cast<overlapped_send<ENty, CNty>*>(ovb)->ctx
            ->notify_disconnection();
        delete reinterpret_cast<overlapped_send<ENty, CNty>*>(ovb);
        break;
    }
  }
  // +=========================================================================+
  // | [>] replenish_accept_pipeline                              ( private )  |
  // +-------------------------------------------------------------------------+
  bool replenish_accept_pipeline() {
    for (;;) {
      {
        std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
        if (stopping_ || accept_socket_ == INVALID_SOCKET) return false;
        if (pending_accepts_ >= accept_depth_) return true;
      }
      if (!post_accept(false)) return false;
    }
  }
  // +=========================================================================+
  // | [>] post_accept                                             ( private ) |
  // +-------------------------------------------------------------------------+
  bool post_accept(bool replacement) {
    SOCKET soc = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                            WSA_FLAG_OVERLAPPED);
    if (soc == INVALID_SOCKET) return false;
    overlapped_accept* ova = new (std::nothrow) overlapped_accept(soc);
    if (!ova) {
      closesocket(soc);
      return false;
    }
    DWORD received = 0;
    std::unique_lock<std::mutex> lifecycle_lock(lifecycle_mutex_);
    if (stopping_ || accept_socket_ == INVALID_SOCKET) {
      lifecycle_lock.unlock();
      closesocket(soc);
      delete ova;
      return false;
    }
    if (!replacement && pending_accepts_ >= accept_depth_) {
      lifecycle_lock.unlock();
      closesocket(soc);
      delete ova;
      return true;
    }
    pending_accepts_++;
    BOOL accepted_connection =
        accept_ex_(accept_socket_, ova->socket, ova->addresses, 0,
                   kAcceptAddressBytes, kAcceptAddressBytes, &received, ova);
    if (accepted_connection == FALSE && WSAGetLastError() != ERROR_IO_PENDING) {
      // ((error)) -> Could not post AcceptEx operation!
      pending_accepts_--;
      lifecycle_lock.unlock();
      lifecycle_cv_.notify_all();
      closesocket(soc);
      delete ova;
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  const policies_type configuration_;
  const FAty create_engine_;
  const typename CNty::shared_state shared_state_;
  network::detail::environment environment_;
  HANDLE io_h_ = nullptr;
  SOCKET accept_socket_ = INVALID_SOCKET;
  LPFN_ACCEPTEX accept_ex_ = nullptr;
  std::mutex lifecycle_mutex_;
  std::condition_variable lifecycle_cv_;
  std::size_t accept_depth_ = 0;
  std::size_t pending_accepts_ = 0;
  bool starting_ = false;
  std::atomic<bool> stopping_{false};
  std::thread::id stopping_thread_{};
  std::vector<std::jthread> workers_;
  inline static thread_local const worker* current_worker_{nullptr};
  std::unordered_map<context<ENty, CNty>*,
                     std::shared_ptr<context<ENty, CNty>>> contexts_;
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
};

}  // namespace martianlabs::doba::transport::server

#endif
