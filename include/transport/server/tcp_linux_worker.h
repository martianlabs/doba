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

#ifndef martianlabs_doba_transport_server_tcp_linux_worker_h
#define martianlabs_doba_transport_server_tcp_linux_worker_h

#include <array>
#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <netinet/tcp.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <utility>

#include "transport/server/tcp_linux_context.h"
#include "transport/server/tcp_policies.h"

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] linux_worker                                               ( class ) |
// +---------------------------------------------------------------------------+
// | Owns socket I/O and connection contexts on one thread.                    |
// | The event descriptor carries only a stop signal, never application work. |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
class linux_worker {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  linux_worker(tcp_policies configuration, const FAty& factory,
               const std::function<void()>& connected,
               const std::function<void()>& disconnected)
      : configuration_(std::move(configuration)), factory_(factory),
        connected_(connected), disconnected_(disconnected) {}
  linux_worker(const linux_worker&) = delete;
  ~linux_worker() { stop(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  linux_worker& operator=(const linux_worker&) = delete;
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void start(in_addr ip, std::uint16_t port) {
    epoll_ = ::epoll_create1(EPOLL_CLOEXEC);
    stop_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    listener_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                         IPPROTO_TCP);
    if (epoll_ < 0 || stop_ < 0 || listener_ < 0) {
      throw std::runtime_error("Cannot create Linux transport descriptors");
    }
    const int enabled = 1;
    if (::setsockopt(listener_, SOL_SOCKET, SO_REUSEADDR, &enabled,
                     sizeof(enabled)) < 0 ||
        ::setsockopt(listener_, SOL_SOCKET, SO_REUSEPORT, &enabled,
                     sizeof(enabled)) < 0) {
      throw std::runtime_error("Cannot configure Linux listener");
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr = ip;
    address.sin_port = htons(port);
    const int backlog = configuration_.listen_backlog;
    if (::bind(listener_, reinterpret_cast<sockaddr*>(&address),
               sizeof(address)) < 0 ||
        ::listen(listener_, backlog ? backlog : SOMAXCONN) < 0 ||
        !watch(stop_, EPOLL_CTL_ADD, EPOLLIN) ||
        !watch(listener_, EPOLL_CTL_ADD, EPOLLIN)) {
      throw std::runtime_error("Cannot start Linux listener");
    }
    thread_ = std::thread([this]() { run(); });
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // Only the external owner calls stop; descriptors stay open until join.
  void stop() {
    if (thread_.joinable()) {
      const std::uint64_t signal = 1;
      ssize_t size;
      do {
        size = ::write(stop_, &signal, sizeof(signal));
      } while (size < 0 && errno == EINTR);
      thread_.join();
    }
    close_descriptor(listener_);
    close_descriptor(stop_);
    close_descriptor(epoll_);
  }
  // +=========================================================================+
  // | [>] is_current_thread                                        ( public ) |
  // +-------------------------------------------------------------------------+
  bool is_current_thread() const noexcept {
    return thread_.get_id() == std::this_thread::get_id();
  }

 private:
  // +=========================================================================+
  // | [>] close_descriptor                                        ( private ) |
  // +-------------------------------------------------------------------------+
  static void close_descriptor(int& descriptor) noexcept {
    if (descriptor >= 0) ::close(descriptor);
    descriptor = -1;
  }
  // +=========================================================================+
  // | [>] watch                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  bool watch(int socket, int operation, std::uint32_t events) noexcept {
    epoll_event event{};
    event.events = events;
    event.data.fd = socket;
    return ::epoll_ctl(epoll_, operation, socket, &event) == 0;
  }
  // +=========================================================================+
  // | [>] accept                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  void accept() {
    for (std::size_t count = 0; count < 64; ++count) {
      const int socket = ::accept4(listener_, nullptr, nullptr,
                                    SOCK_NONBLOCK | SOCK_CLOEXEC);
      if (socket < 0) {
        if (errno == EINTR) continue;
        // Resource/system failures end admission until an explicit restart.
        // Keeping a readable listener here would spin without making progress.
        if (errno != EAGAIN && errno != EWOULDBLOCK &&
            errno != ECONNABORTED && errno != EPROTO) {
          close_descriptor(listener_);
        }
        return;
      }
      if (configuration_.max_connections &&
          contexts_.size() >= configuration_.max_connections) {
        ::close(socket);
        continue;
      }
      try {
        const int enabled = 1;
        if (::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &enabled,
                         sizeof(enabled)) < 0) {
          throw std::runtime_error("Cannot configure accepted socket");
        }
        auto context = std::make_unique<linux_context<ENty>>(
            socket, configuration_.receive_buffer_capacity,
            configuration_.max_buffered_send_bytes, factory_);
        if (!watch(socket, EPOLL_CTL_ADD, EPOLLIN | EPOLLRDHUP)) {
          throw std::runtime_error("Cannot register accepted socket");
        }
        contexts_.emplace(socket, std::move(context));
        if (connected_) connected_();
      } catch (...) {
        if (contexts_.contains(socket)) close(socket);
        else ::close(socket);
      }
    }
  }
  // +=========================================================================+
  // | [>] close                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  void close(int socket) noexcept {
    ::epoll_ctl(epoll_, EPOLL_CTL_DEL, socket, nullptr);
    ::close(socket);
    contexts_.erase(socket);
    if (disconnected_) {
      try { disconnected_(); } catch (...) {}
    }
  }
  // +=========================================================================+
  // | [>] handle                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  void handle(int socket, std::uint32_t events) {
    auto found = contexts_.find(socket);
    if (found == contexts_.end()) return;
    auto& context = *found->second;
    try {
      if (events & (EPOLLERR | EPOLLHUP)) {
        close(socket);
        return;
      }
      const bool was_pending = context.pending();
      if (!was_pending && !context.receive()) {
        close(socket);
        return;
      }
      if (!context.flush()) {
        close(socket);
        return;
      }
      const bool pending = context.pending();
      if (pending != was_pending &&
          !watch(socket, EPOLL_CTL_MOD,
                 pending ? EPOLLOUT : EPOLLIN | EPOLLRDHUP)) {
        close(socket);
      }
    } catch (...) {
      close(socket);
    }
  }
  // +=========================================================================+
  // | [>] run                                                     ( private ) |
  // +-------------------------------------------------------------------------+
  void run() noexcept {
    std::array<epoll_event, 64> events{};
    for (;;) {
      const int ready = ::epoll_wait(epoll_, events.data(), events.size(), -1);
      if (ready < 0) {
        if (errno == EINTR) continue;
        break;
      }
      bool stopping = false;
      for (int i = 0; i < ready; ++i) {
        if (events[i].data.fd == stop_) stopping = true;
      }
      if (stopping) break;
      for (int i = 0; i < ready; ++i) {
        const auto& event = events[i];
        if (event.data.fd == listener_) accept();
        else handle(event.data.fd, event.events);
      }
    }
    while (!contexts_.empty()) close(contexts_.begin()->first);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Immutable worker configuration, including its admission capacity.
  const tcp_policies configuration_;
  // Engine factory, alive until the worker has joined.
  const FAty& factory_;
  // Optional connection notification executed by this worker.
  const std::function<void()>& connected_;
  // Optional notification after context destruction.
  const std::function<void()>& disconnected_;
  // Readiness set, kept open until the worker has joined.
  int epoll_{-1};
  // External owner writes here to wake and stop the worker.
  int stop_{-1};
  // Nonblocking listening socket.
  int listener_{-1};
  // The sole owner of connection processing.
  std::thread thread_;
  // Socket-indexed contexts, accessed only by the worker thread.
  std::unordered_map<int, std::unique_ptr<linux_context<ENty>>> contexts_;
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
