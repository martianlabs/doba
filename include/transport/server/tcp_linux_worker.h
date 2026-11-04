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

#include <algorithm>
#include <array>
#include <chrono>
#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <netinet/tcp.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <utility>

#include "transport/server/tcp_linux_context.h"
#include "transport/server/tcp_linux_processing_queue.h"
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
        !watch(stop_, EPOLL_CTL_ADD, EPOLLIN, kStop) ||
        !watch(listener_, EPOLL_CTL_ADD, EPOLLIN, kListener)) {
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
    close_descriptor(timer_);
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
  bool watch(int socket, int operation, std::uint32_t events,
             std::uint64_t identity) noexcept {
    epoll_event event{};
    event.events = events;
    event.data.u64 = identity;
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
      if (next_identity_ == std::numeric_limits<std::uint64_t>::max()) {
        ::close(socket);
        close_descriptor(listener_);
        return;
      }
      const auto identity = next_identity_++;
      try {
        const int enabled = 1;
        if (::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &enabled,
                         sizeof(enabled)) < 0) {
          throw std::runtime_error("Cannot configure accepted socket");
        }
        auto context = std::make_unique<linux_context<ENty>>(
            socket, configuration_.receive_buffer_capacity,
            configuration_.max_buffered_send_bytes, factory_);
        if (!watch(socket, EPOLL_CTL_ADD, EPOLLIN | EPOLLRDHUP, identity)) {
          throw std::runtime_error("Cannot register accepted socket");
        }
        contexts_.emplace(identity, std::move(context));
        if (connected_) connected_();
      } catch (...) {
        if (contexts_.contains(identity)) close(identity);
        else ::close(socket);
      }
    }
  }
  // +=========================================================================+
  // | [>] close                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  void close(std::uint64_t identity) noexcept {
    auto found = contexts_.find(identity);
    if (found == contexts_.end()) return;
    if (processing_) processing_->cancel(identity);
    const auto socket = found->second->socket();
    ::epoll_ctl(epoll_, EPOLL_CTL_DEL, socket, nullptr);
    ::close(socket);
    contexts_.erase(found);
    if (disconnected_) {
      try { disconnected_(); } catch (...) {}
    }
  }
  // +=========================================================================+
  // | [>] reconcile                                               ( private ) |
  // +-------------------------------------------------------------------------+
  void reconcile(std::uint64_t identity, const linux_context<ENty>& context) {
    const auto& progress = context.progress();
    if (!progress.turn_requested && !progress.turn_at &&
        !(progress.turn_after_output_drained && !context.pending())) {
      if (processing_) processing_->cancel(identity);
      return;
    }
    if (!processing_) processing_ = std::make_unique<linux_processing_queue>();
    processing_->reconcile(identity, progress, !context.pending(),
        progress.turn_at ? clock::now() : clock::time_point{});
  }
  // +=========================================================================+
  // | [>] service                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  void service(std::uint64_t identity, linux_context<ENty>& context,
               bool was_pending) {
    if (!context.flush()) {
      close(identity);
      return;
    }
    const bool pending = context.pending();
    if (pending != was_pending &&
        !watch(context.socket(), EPOLL_CTL_MOD,
               pending ? EPOLLOUT : EPOLLIN | EPOLLRDHUP, identity)) {
      close(identity);
      return;
    }
    reconcile(identity, context);
  }
  // +=========================================================================+
  // | [>] handle                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  void handle(std::uint64_t identity, std::uint32_t events) {
    auto found = contexts_.find(identity);
    if (found == contexts_.end()) return;
    auto& context = *found->second;
    try {
      if (events & (EPOLLERR | EPOLLHUP)) {
        close(identity);
        return;
      }
      const bool was_pending = context.pending();
      if (!was_pending && !context.receive()) {
        close(identity);
        return;
      }
      service(identity, context, was_pending);
    } catch (...) {
      close(identity);
    }
  }
  // +=========================================================================+
  // | [>] process                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  void process(std::uint64_t identity) {
    auto found = contexts_.find(identity);
    if (found == contexts_.end()) return;
    auto& context = *found->second;
    try {
      const bool was_pending = context.pending();
      if (!context.process()) {
        close(identity);
        return;
      }
      service(identity, context, was_pending);
    } catch (...) {
      close(identity);
    }
  }
  // +=========================================================================+
  // | [>] read_timer                                              ( private ) |
  // +-------------------------------------------------------------------------+
  bool read_timer() noexcept {
    std::uint64_t expirations;
    ssize_t size;
    do {
      size = ::read(timer_, &expirations, sizeof(expirations));
    } while (size < 0 && errno == EINTR);
    if (size == static_cast<ssize_t>(sizeof(expirations))) {
      armed_.reset();
      return true;
    }
    return size < 0 && errno == EAGAIN;
  }
  // +=========================================================================+
  // | [>] arm_timer                                               ( private ) |
  // +-------------------------------------------------------------------------+
  bool arm_timer() noexcept {
    if (!processing_) return true;
    const auto next = processing_->next_deadline();
    // A bounded promotion may leave due identities for the next poll turn.
    if (next && *next <= clock::now()) return true;
    if (next == armed_) return true;
    if (next && timer_ < 0) {
      timer_ = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
      if (timer_ < 0 || !watch(timer_, EPOLL_CTL_ADD, EPOLLIN, kTimer)) {
        return false;
      }
    }
    itimerspec setting{};
    if (next) {
      auto remaining = std::chrono::duration_cast<std::chrono::nanoseconds>(
          *next - clock::now());
      if (remaining.count() <= 0) remaining = std::chrono::nanoseconds(1);
      const auto seconds =
          std::chrono::duration_cast<std::chrono::seconds>(remaining);
      setting.it_value.tv_sec = static_cast<time_t>(std::min<std::int64_t>(
          seconds.count(), std::numeric_limits<time_t>::max()));
      setting.it_value.tv_nsec = static_cast<long>(
          (remaining - seconds).count());
    }
    if (timer_ >= 0 && ::timerfd_settime(timer_, 0, &setting, nullptr) < 0) {
      return false;
    }
    armed_ = next;
    return true;
  }
  // +=========================================================================+
  // | [>] run                                                     ( private ) |
  // +-------------------------------------------------------------------------+
  void run() noexcept {
    std::array<epoll_event, 64> events{};
    for (;;) {
      int timeout = -1;
      if (processing_) {
        const auto due = processing_->next_deadline();
        if (processing_->ready_count() || (due && *due <= clock::now())) {
          timeout = 0;
        }
      }
      const int ready =
          ::epoll_wait(epoll_, events.data(), events.size(), timeout);
      if (ready < 0) {
        if (errno == EINTR) continue;
        break;
      }
      bool stopping = false;
      for (int i = 0; i < ready; ++i) {
        if (events[i].data.u64 == kStop) stopping = true;
      }
      if (stopping) break;
      try {
        bool healthy = true;
        for (int i = 0; i < ready; ++i) {
          const auto& event = events[i];
          if (event.data.u64 == kListener) {
            if (listener_ >= 0) accept();
          } else if (event.data.u64 == kTimer) {
            healthy = read_timer();
            if (!healthy) break;
          } else {
            handle(event.data.u64, event.events);
          }
        }
        if (!healthy) break;
        if (processing_) {
          if (processing_->next_deadline()) {
            processing_->promote(clock::now());
          }
          const auto count = std::min<std::size_t>(
              64, processing_->ready_count());
          for (std::size_t i = 0; i < count; ++i) {
            const auto identity = processing_->take_ready();
            if (!identity) break;
            process(*identity);
          }
        }
        if (!arm_timer()) break;
      } catch (...) {
        break;
      }
    }
    if (processing_) processing_->clear();
    close_descriptor(listener_);
    close_descriptor(timer_);
    armed_.reset();
    while (!contexts_.empty()) close(contexts_.begin()->first);
  }
  // +=========================================================================+
  // | [>] USINGs                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  using clock = linux_processing_queue::clock;
  // +=========================================================================+
  // | [>] CONSTANTs                                               ( private ) |
  // +-------------------------------------------------------------------------+
  static constexpr std::uint64_t kStop = 0;
  static constexpr std::uint64_t kListener = 1;
  static constexpr std::uint64_t kTimer = 2;
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
  // Lifetime identities prevent stale epoll events from following FD reuse.
  std::uint64_t next_identity_{3};
  std::unordered_map<std::uint64_t,
                     std::unique_ptr<linux_context<ENty>>> contexts_;
  // Allocated only when a connection requests runnable work or a deadline.
  std::unique_ptr<linux_processing_queue> processing_;
  // One monotonic timer descriptor, created only for future deadlines.
  int timer_{-1};
  std::optional<clock::time_point> armed_;
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
