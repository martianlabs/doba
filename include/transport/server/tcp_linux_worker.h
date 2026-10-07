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
#include <atomic>
#include <cerrno>
#include <chrono>
#include <climits>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "platform.h"
#include "transport/server/tcp_linux_context.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] CONSTANTs                                                  ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
static constexpr uint64_t kStopEventId = 0;
static constexpr uint64_t kAsyncEventId = 1;
static constexpr uint64_t kListenerEventId =
    std::numeric_limits<uint64_t>::max();

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] worker [linux]                                             ( struct ) |
// +---------------------------------------------------------------------------+
// | This specification holds for a Linux server transport worker.             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty, typename CNty,
          typename PTy>
struct worker {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  worker(PTy configuration, const FAty& create_engine,
         types::on_client_connected_delegate on_connection,
         types::on_client_disconnected_delegate on_disconnection,
         typename CNty::shared_state shared_state)
      : configuration_{configuration},
        create_engine_{create_engine},
        shared_state_{std::move(shared_state)},
        on_connection_{std::move(on_connection)},
        on_disconnection_{std::move(on_disconnection)} {}
  worker(const worker&) = delete;
  worker(worker&&) noexcept = delete;
  ~worker() { stop(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  worker& operator=(const worker&) = delete;
  worker& operator=(worker&&) noexcept = delete;
  // +=========================================================================+
  // | [>] setup                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void setup(in_addr ip, uint16_t port) {
    epoll_fd_ = ::epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ == -1) {
      throw std::runtime_error("Epoll instance could not be created!");
    }
    stop_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (stop_fd_ == -1) {
      close_resources();
      throw std::runtime_error("Stop event could not be created!");
    }
    async_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (async_fd_ == -1) {
      close_resources();
      throw std::runtime_error("Async event could not be created!");
    }
    listener_fd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                            IPPROTO_TCP);
    if (listener_fd_ == -1) {
      close_resources();
      throw std::runtime_error("Socket could not be created!");
    }
    int reuse_address = 1;
    if (::setsockopt(listener_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse_address,
                     sizeof(reuse_address)) == -1 ||
        ::setsockopt(listener_fd_, SOL_SOCKET, SO_REUSEPORT, &reuse_address,
                     sizeof(reuse_address)) == -1) {
      close_resources();
      throw std::runtime_error("Listener socket could not be configured!");
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr = ip;
    address.sin_port = htons(port);
    if (::bind(listener_fd_, reinterpret_cast<const sockaddr*>(&address),
               sizeof(address)) == -1 ||
        ::listen(listener_fd_, configuration_.listen_backlog
                                   ? configuration_.listen_backlog
                                   : SOMAXCONN) == -1) {
      close_resources();
      throw std::runtime_error("Listener socket could not be started!");
    }
    epoll_event stop_event{};
    stop_event.events = EPOLLIN;
    stop_event.data.u64 = kStopEventId;
    epoll_event async_event{};
    async_event.events = EPOLLIN;
    async_event.data.u64 = kAsyncEventId;
    epoll_event listener_event{};
    listener_event.events = EPOLLIN;
    listener_event.data.u64 = kListenerEventId;
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, stop_fd_, &stop_event) == -1 ||
        ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, async_fd_, &async_event) == -1 ||
        ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listener_fd_, &listener_event) ==
            -1) {
      close_resources();
      throw std::runtime_error("Listener could not be registered!");
    }
  }
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void start() {
    thread_ = std::jthread([this]() {
      current_worker_ = this;
      run();
      current_worker_ = nullptr;
    });
  }
  // +=========================================================================+
  // | [>] is_current_thread                                        ( public ) |
  // +-------------------------------------------------------------------------+
  bool is_current_thread() const { return current_worker_ == this; }
  static bool is_current_thread(const FAty& create_engine) {
    return current_worker_ &&
           &current_worker_->create_engine_ == &create_engine;
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void stop() {
    if (is_current_thread()) {
      throw std::runtime_error("Worker cannot be stopped from itself!");
    }
    if (epoll_fd_ == -1) return;
    request_stop();
    if (thread_.joinable()) thread_.join();
    close_contexts();
    close_resources();
  }
  // +=========================================================================+
  // | [>] request_stop                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void request_stop() {
    if (stopping_.exchange(true)) return;
    uint64_t stop = 1;
    ssize_t written = 0;
    do {
      written = ::write(stop_fd_, &stop, sizeof(stop));
    } while (written == -1 && errno == EINTR);
  }
  // +=========================================================================+
  // | [>] request_quiesce                                         ( public ) |
  // +-------------------------------------------------------------------------+
  void request_quiesce() {
    quiescing_.store(true);
    uint64_t signal = 1;
    while (::write(stop_fd_, &signal, sizeof(signal)) == -1 &&
           errno == EINTR) {}
  }
  // +=========================================================================+
  // | [>] wait_quiesced                                           ( public ) |
  // +-------------------------------------------------------------------------+
  void wait_quiesced() const {
    while (!quiesced_.load()) quiesced_.wait(false);
  }

 private:
  // +=========================================================================+
  // | [>] run                                                     ( private ) |
  // +-------------------------------------------------------------------------+
  void run() {
    std::array<epoll_event, 64> events{};
    for (;;) {
      int timeout = -1;
      if (!ready_.empty()) {
        timeout = 0;
      } else if (!timers_.empty()) {
        auto remaining = timers_.top().first -
                         std::chrono::steady_clock::now();
        auto milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(remaining);
        if (remaining <= std::chrono::steady_clock::duration::zero()) {
          timeout = 0;
        } else if (milliseconds.count() >= INT_MAX) {
          timeout = INT_MAX;
        } else {
          timeout = static_cast<int>(milliseconds.count() + 1);
        }
      }
      int ready = ::epoll_wait(epoll_fd_, events.data(), events.size(),
                               timeout);
      if (ready == -1) {
        if (errno == EINTR) continue;
        break;
      }
      for (std::size_t i = 0; i < static_cast<std::size_t>(ready); i++) {
        if (events[i].data.u64 == kStopEventId) {
          handle_stop();
        } else if (events[i].data.u64 == kAsyncEventId) {
          handle_async();
        } else if (events[i].data.u64 == kListenerEventId) {
          if (!quiescing_.load() && !stopping_.load()) handle_listener();
        } else {
          auto ctx = static_cast<context<ENty, CNty>*>(events[i].data.ptr);
          if (!ctx->is_closed()) {
            try {
              handle_context(ctx, events[i].events);
            } catch (...) {
              close_context(ctx);
            }
          }
        }
      }
      if (!timers_.empty()) {
        const auto now = std::chrono::steady_clock::now();
        while (!timers_.empty() && timers_.top().first <= now) {
          ready_.push_back(timers_.top().second);
          timers_.pop();
        }
      }
      std::size_t count = ready_.size();
      while (count--) {
        auto handle = ready_.front();
        ready_.pop_front();
        handle.resume();
      }
      retire_contexts();
      if (stopping_.load() && contexts_.empty()) break;
    }
    close_contexts();
  }
  // +=========================================================================+
  // | [>] handle_stop                                             ( private ) |
  // +-------------------------------------------------------------------------+
  void handle_stop() {
    uint64_t stop = 0;
    ssize_t received = 0;
    do {
      received = ::read(stop_fd_, &stop, sizeof(stop));
    } while (received == -1 && errno == EINTR);
    if (!stopping_.load() && !quiescing_.load()) return;
    if (listener_fd_ != -1) {
      ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, listener_fd_, nullptr);
      ::close(listener_fd_);
      listener_fd_ = -1;
    }
    for (const auto& item : contexts_) {
      auto ctx = item.first;
      if (ctx->is_closed()) continue;
      ctx->stop(true);
      try {
        if (!ctx->send_pending()) close_context(ctx);
      } catch (...) {
        close_context(ctx);
      }
    }
    quiesced_.store(true);
    quiesced_.notify_all();
  }
  // +=========================================================================+
  // | [>] handle_async                                           ( private ) |
  // +-------------------------------------------------------------------------+
  void handle_async() {
    uint64_t signal = 0;
    while (::read(async_fd_, &signal, sizeof(signal)) == -1 &&
           errno == EINTR) {}
    std::lock_guard<std::mutex> lock(async_mutex_);
    while (!incoming_.empty()) {
      auto item = incoming_.front();
      incoming_.pop_front();
      if (item.first == std::chrono::steady_clock::time_point::min()) {
        ready_.push_back(item.second);
      } else {
        timers_.push(item);
      }
    }
  }
  // +=========================================================================+
  // | [>] scheduler                                              ( private ) |
  // +-------------------------------------------------------------------------+
  common::task_scheduler scheduler() {
    return {this,
            [](void* owner, std::coroutine_handle<> handle) {
              static_cast<worker*>(owner)->enqueue(
                  std::chrono::steady_clock::time_point::min(), handle);
            },
            [](void* owner, std::chrono::steady_clock::time_point due,
               std::coroutine_handle<> handle) {
              static_cast<worker*>(owner)->enqueue(due, handle);
            }};
  }
  // +=========================================================================+
  // | [>] enqueue                                                ( private ) |
  // +-------------------------------------------------------------------------+
  void enqueue(std::chrono::steady_clock::time_point due,
               std::coroutine_handle<> handle) {
    if (is_current_thread()) {
      if (due == std::chrono::steady_clock::time_point::min()) {
        ready_.push_back(handle);
      } else {
        timers_.push({due, handle});
      }
      return;
    }
    {
      std::lock_guard<std::mutex> lock(async_mutex_);
      incoming_.push_back({due, handle});
    }
    uint64_t signal = 1;
    while (::write(async_fd_, &signal, sizeof(signal)) == -1 &&
           errno == EINTR) {}
  }
  // +=========================================================================+
  // | [>] handle_listener                                         ( private ) |
  // +-------------------------------------------------------------------------+
  void handle_listener() {
    if (stopping_.load() || quiescing_.load()) return;
    int socket = ::accept4(listener_fd_, nullptr, nullptr,
                           SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (socket == -1) return;
    if (stopping_.load() || quiescing_.load()) {
      ::close(socket);
      return;
    }
    int no_delay = 1;
    if (::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &no_delay,
                     sizeof(no_delay)) == -1) {
      ::close(socket);
      return;
    }
    try {
      register_context(socket);
    } catch (...) {
      ::close(socket);
    }
  }
  // +=========================================================================+
  // | [>] register_context                                        ( private ) |
  // +-------------------------------------------------------------------------+
  void register_context(int socket) {
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
    auto ctx = std::make_shared<context<ENty, CNty>>(
        socket, configuration_.recv_buffer_size,
        configuration_.max_send_buffer_size, create_engine_, on_connection_,
        on_disconnection_, shared_state_, encrypted_receive_buffer_size,
        network_bio_buffer_size);
    auto ctx_ptr = ctx.get();
    if (!contexts_.emplace(ctx_ptr, std::move(ctx)).second) {
      throw std::runtime_error("Context could not be registered!");
    }
    epoll_event event{};
    event.events = EPOLLIN | EPOLLRDHUP;
    event.data.ptr = ctx_ptr;
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket, &event) == -1) {
      close_context(ctx_ptr);
      return;
    }
    try {
      ctx_ptr->connected(epoll_fd_, scheduler());
      ctx_ptr->notify_connection();
    } catch (...) {
      close_context(ctx_ptr);
      return;
    }
  }
  // +=========================================================================+
  // | [>] handle_context                                          ( private ) |
  // +-------------------------------------------------------------------------+
  void handle_context(context<ENty, CNty>* ctx, uint32_t events) {
    if (events & EPOLLERR) {
      close_context(ctx);
      return;
    }
    if (stopping_.load() || quiescing_.load()) ctx->stop(true);
    bool should_send = (events & EPOLLOUT) != 0;
    if (ctx->can_receive() && (events & (EPOLLIN | EPOLLRDHUP | EPOLLHUP))) {
      if (stopping_.load() || quiescing_.load()) {
        ctx->stop(true);
      } else {
        const ssize_t received = ctx->receive();
        if (received > 0) {
          should_send = true;
        } else if (received == 0) {
          if (!ctx->eof()) {
            close_context(ctx);
            return;
          }
          ctx->stop();
        } else if (errno != EINTR && errno != EAGAIN &&
                   errno != EWOULDBLOCK) {
          close_context(ctx);
          return;
        }
      }
    }
    if (should_send || !ctx->can_receive()) {
      if (!ctx->send_pending()) close_context(ctx);
    }
  }
  // +=========================================================================+
  // | [>] close_context                                           ( private ) |
  // +-------------------------------------------------------------------------+
  void close_context(context<ENty, CNty>* ctx) {
    int socket = ctx->retire_socket();
    if (socket == -1) return;
    ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket, nullptr);
    ::close(socket);
    ctx->retirement_next = retired_contexts_;
    retired_contexts_ = ctx;
    ctx->notify_disconnection();
  }
  // +=========================================================================+
  // | [>] retire_contexts                                         ( private ) |
  // +-------------------------------------------------------------------------+
  void retire_contexts() {
    while (retired_contexts_) {
      context<ENty, CNty>* ctx = retired_contexts_;
      retired_contexts_ = ctx->retirement_next;
      contexts_.erase(ctx);
    }
  }
  // +=========================================================================+
  // | [>] close_contexts                                          ( private ) |
  // +-------------------------------------------------------------------------+
  void close_contexts() {
    for (auto& item : contexts_) {
      context<ENty, CNty>* ctx = item.first;
      if (ctx->is_closed()) continue;
      close_context(ctx);
    }
    retire_contexts();
  }
  // +=========================================================================+
  // | [>] close_resources                                         ( private ) |
  // +-------------------------------------------------------------------------+
  void close_resources() {
    if (listener_fd_ != -1) ::close(listener_fd_);
    if (stop_fd_ != -1) ::close(stop_fd_);
    if (async_fd_ != -1) ::close(async_fd_);
    if (epoll_fd_ != -1) ::close(epoll_fd_);
    listener_fd_ = -1;
    stop_fd_ = -1;
    async_fd_ = -1;
    epoll_fd_ = -1;
  }
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  const PTy configuration_;
  const FAty& create_engine_;
  const typename CNty::shared_state shared_state_;
  int epoll_fd_{-1};
  int stop_fd_{-1};
  int async_fd_{-1};
  int listener_fd_{-1};
  std::atomic<bool> stopping_{false};
  std::atomic<bool> quiescing_{false};
  std::atomic<bool> quiesced_{false};
  using async_entry = std::pair<std::chrono::steady_clock::time_point,
                                std::coroutine_handle<>>;
  struct later_async {
    bool operator()(const async_entry& left,
                    const async_entry& right) const {
      return left.first > right.first;
    }
  };
  std::deque<std::coroutine_handle<>> ready_;
  std::priority_queue<async_entry, std::vector<async_entry>, later_async>
      timers_;
  std::deque<async_entry> incoming_;
  std::mutex async_mutex_;
  std::jthread thread_;
  inline static thread_local const worker* current_worker_{nullptr};
  std::unordered_map<context<ENty, CNty>*, std::shared_ptr<context<ENty, CNty>>>
      contexts_;
  context<ENty, CNty>* retired_contexts_{nullptr};
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
};

}  // namespace martianlabs::doba::transport::server

#endif
