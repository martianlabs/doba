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
static constexpr uint64_t kWorkEventId = 1;
static constexpr uint64_t kLstnrEventId = std::numeric_limits<uint64_t>::max();

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] worker                                                     ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct represents a worker thread that handles TCP connections for   |
// | an engine. It manages the `epoll` instance, listener socket, and          |
// | event file descriptors. It also sets up, starts, and stops the worker,    |
// | runs its thread, checks whether the current thread is that worker, and    |
// | handles connection events.                                                |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   ENty - engine type being used.                                          |
// |   FAty - engine factory type being used.                                  |
// |   CNty - connection type being used.                                      |
// |   POty - policies type being used.                                        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty, typename CNty,
          typename POty>
struct worker {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  worker(POty configuration, const FAty& create_engine,
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
  // | Sets up the worker by creating the `epoll` instance, event file         |
  // | descriptors, and listener socket. It configures the listener, binds it  |
  // | to the given IP address and port, and registers its events with         |
  // | `epoll`. If anything goes wrong, it cleans up the resources and         |
  // | throws a runtime error.                                                 |
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
    work_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (work_fd_ == -1) {
      close_resources();
      throw std::runtime_error("Work event could not be created!");
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
    epoll_event work_event{};
    work_event.events = EPOLLIN;
    work_event.data.u64 = kWorkEventId;
    epoll_event lsn_event{};
    lsn_event.events = EPOLLIN;
    lsn_event.data.u64 = kLstnrEventId;
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, stop_fd_, &stop_event) == -1 ||
        ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, work_fd_, &work_event) == -1 ||
        ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listener_fd_, &lsn_event) == -1) {
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
  // +=========================================================================+
  // | [>] is_current_thread                                        ( public ) |
  // +-------------------------------------------------------------------------+
  static bool is_current_thread(const FAty& cr_engine) {
    return current_worker_ && &current_worker_->create_engine_ == &cr_engine;
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
  // | Asks the worker to stop by setting the `stopping_` flag and writing to  |
  // | the stop event file descriptor. If it’s already stopping, it returns    |
  // | right away. The method is thread-safe and can be called from any        |
  // | thread.                                                                 |
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
  // | [>] request_quiesce                                          ( public ) |
  // +-------------------------------------------------------------------------+
  // | Requests quiescence and signals the worker's stop event descriptor.     |
  // | Call while the worker is active and coordinate with `stop()`.           |
  // | Use `wait_quiesced()` to wait for the request to be handled.            |
  // +-------------------------------------------------------------------------+
  void request_quiesce() {
    quiescing_.store(true);
    uint64_t sgn = 1;
    while (::write(stop_fd_, &sgn, sizeof(sgn)) == -1 && errno == EINTR) {
    }
  }
  // +=========================================================================+
  // | [>] wait_quiesced                                            ( public ) |
  // +-------------------------------------------------------------------------+
  // | Waits on `quiesced_` until the worker handles the request.              |
  // | Call `request_quiesce()` first.                                         |
  // +-------------------------------------------------------------------------+
  void wait_quiesced() const {
    while (!quiesced_.load()) quiesced_.wait(false);
  }

 private:
  // +=========================================================================+
  // | [>] run                                                     ( private ) |
  // +-------------------------------------------------------------------------+
  // | Runs the worker thread’s main loop. It waits for events from `epoll`    |
  // | and handles stop, work, listener, and context events as they come in.   |
  // | It also manages timed and ready work, and cleans up contexts            |
  // | when needed. The loop keeps running until the worker is stopping and    |
  // | all its contexts are closed.                                            |
  // +-------------------------------------------------------------------------+
  void run() {
    std::array<epoll_event, 64> events{};
    for (;;) {
      int tout = -1;
      if (!ready_.empty()) {
        tout = 0;
      } else if (!timers_.empty()) {
        auto remaining = timers_.top().first - std::chrono::steady_clock::now();
        auto milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(remaining);
        if (remaining <= std::chrono::steady_clock::duration::zero()) {
          tout = 0;
        } else if (milliseconds.count() >= INT_MAX) {
          tout = INT_MAX;
        } else {
          tout = static_cast<int>(milliseconds.count() + 1);
        }
      }
      int ready = ::epoll_wait(epoll_fd_, events.data(), events.size(), tout);
      if (ready == -1) {
        if (errno == EINTR) continue;
        break;
      }
      for (std::size_t i = 0; i < static_cast<std::size_t>(ready); i++) {
        if (events[i].data.u64 == kStopEventId) {
          handle_stop();
        } else if (events[i].data.u64 == kWorkEventId) {
          handle_work();
        } else if (events[i].data.u64 == kLstnrEventId) {
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
        auto work = ready_.front();
        ready_.pop_front();
        work.run(work.context);
      }
      retire_contexts();
      if (stopping_.load() && contexts_.empty()) break;
    }
    close_contexts();
  }
  // +=========================================================================+
  // | [>] handle_stop                                             ( private ) |
  // +-------------------------------------------------------------------------+
  // | Handles a stop event by reading from the stop event file descriptor and |
  // | checking whether the worker is stopping or quiescing. If it is, the     |
  // | method removes the listener from `epoll`, closes it, and stops all      |
  // | contexts. It then sets the `quiesced_` flag and notifies any waiting    |
  // | threads. This runs on the worker thread when it receives a stop event.  |
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
  // | [>] handle_work                                             ( private ) |
  // +-------------------------------------------------------------------------+
  // | Moves work submitted by other threads to the worker's queues.          |
  // +-------------------------------------------------------------------------+
  void handle_work() {
    uint64_t sig = 0;
    while (::read(work_fd_, &sig, sizeof(sig)) == -1 && errno == EINTR) {
    }
    std::lock_guard<std::mutex> lock(work_mutex_);
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
  // | [>] execution                                               ( private ) |
  // +-------------------------------------------------------------------------+
  // | Returns this worker's capacity for executing opaque work.              |
  // +-------------------------------------------------------------------------+
  execution_capacity execution() {
    return {this,
            [](void* owner, std::chrono::steady_clock::time_point due,
               execution_work work) {
              static_cast<worker*>(owner)->enqueue(due, work);
            }};
  }
  // +=========================================================================+
  // | [>] enqueue                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  // | Adds work to this worker's queue and wakes it when needed.             |
  // +-------------------------------------------------------------------------+
  void enqueue(std::chrono::steady_clock::time_point due,
               execution_work work) {
    if (is_current_thread()) {
      if (due == std::chrono::steady_clock::time_point::min()) {
        ready_.push_back(work);
      } else {
        timers_.push({due, work});
      }
      return;
    }
    std::lock_guard<std::mutex> lock(work_mutex_);
    incoming_.push_back({due, work});
    uint64_t sig = 1;
    ssize_t written;
    do {
      written = ::write(work_fd_, &sig, sizeof(sig));
    } while (written == -1 && errno == EINTR);
    if (written != static_cast<ssize_t>(sizeof(sig)) &&
        !(written == -1 && errno == EAGAIN)) {
      incoming_.pop_back();
      throw std::runtime_error("Work could not be queued!");
    }
  }
  // +=========================================================================+
  // | [>] handle_listener                                         ( private ) |
  // +-------------------------------------------------------------------------+
  // | Handles a listener event by accepting a new connection. If the worker   |
  // | is stopping or quiescing, it skips the connection. Otherwise, it        |
  // | accepts it, sets `TCP_NODELAY`, and creates a context for it. If        |
  // | anything goes wrong, it closes the socket. This runs on the worker      |
  // | thread when a listener event comes in.                                  |
  // +-------------------------------------------------------------------------+
  void handle_listener() {
    if (stopping_.load() || quiescing_.load()) return;
    int socket =
        ::accept4(listener_fd_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
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
  // | Sets up a new context for the socket. It adds the context to the        |
  // | `contexts_` map, registers it with `epoll`, then calls `connected()`    |
  // | and `notify_connection()`. If anything goes wrong, it closes the        |
  // | socket and cleans up. This runs on the worker thread when a new         |
  // | connection is accepted.                                                 |
  // +-------------------------------------------------------------------------+
  void register_context(int socket) {
    std::size_t encrypted_receive_buffer_size = 0;
    std::size_t network_bio_buffer_size = 0;
    if constexpr (requires(const POty& value) {
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
      ctx_ptr->connected(epoll_fd_, execution());
      ctx_ptr->notify_connection();
    } catch (...) {
      close_context(ctx_ptr);
      return;
    }
  }
  // +=========================================================================+
  // | [>] handle_context                                          ( private ) |
  // +-------------------------------------------------------------------------+
  // | Handles a context event by checking its flags and calling the           |
  // | appropriate methods. If the worker is stopping or quiescing, it stops   |
  // | the context. If the context can receive data, it calls `receive()`.     |
  // | If it can send or has data waiting, it calls `send_pending()`. If       |
  // | anything goes wrong, it closes the context. This runs on the worker     |
  // | thread when a context event comes in.                                   |
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
        } else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
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
  // | Closes a context by retiring its socket, removing it from `epoll`, and  |
  // | moving it to the `retired_contexts_` list. It also calls                |
  // | `notify_disconnection()` to let the owner know. This runs on the        |
  // | worker thread when a context needs to be closed.                        |
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
  // | Cleans up all contexts in `retired_contexts_` by removing them from     |
  // | the `contexts_` map and deleting them. This runs on the worker thread   |
  // | after the contexts have been closed.                                    |
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
  // | Closes all contexts by iterating through the `contexts_` map and        |
  // | calling `close_context()` on each one. This runs on the worker thread.  |
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
  // | Closes the listener, stop, work, and epoll descriptors.                 |
  // +-------------------------------------------------------------------------+
  void close_resources() {
    if (listener_fd_ != -1) ::close(listener_fd_);
    if (stop_fd_ != -1) ::close(stop_fd_);
    if (work_fd_ != -1) ::close(work_fd_);
    if (epoll_fd_ != -1) ::close(epoll_fd_);
    listener_fd_ = -1;
    stop_fd_ = -1;
    work_fd_ = -1;
    epoll_fd_ = -1;
  }
  // +=========================================================================+
  // | USINGs                                                      ( private ) |
  // +-------------------------------------------------------------------------+
  using work_entry =
      std::pair<std::chrono::steady_clock::time_point, execution_work>;
  // +=========================================================================+
  // | TYPEs                                                       ( private ) |
  // +-------------------------------------------------------------------------+
  struct later_work {
    bool operator()(const work_entry& left, const work_entry& right) const {
      return left.first > right.first;
    }
  };
  // +=========================================================================+
  // | USINGs                                                      ( private ) |
  // +-------------------------------------------------------------------------+
  using internal_queue =
      std::priority_queue<work_entry, std::vector<work_entry>, later_work>;
  using contexts_map = std::unordered_map<context<ENty, CNty>*,
                                          std::shared_ptr<context<ENty, CNty>>>;
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  const POty configuration_;
  const FAty& create_engine_;
  const typename CNty::shared_state shared_state_;
  int epoll_fd_{-1};
  int stop_fd_{-1};
  int work_fd_{-1};
  int listener_fd_{-1};
  std::atomic<bool> stopping_{false};
  std::atomic<bool> quiescing_{false};
  std::atomic<bool> quiesced_{false};
  std::deque<execution_work> ready_;
  internal_queue timers_;
  std::deque<work_entry> incoming_;
  std::mutex work_mutex_;
  std::jthread thread_;
  inline static thread_local const worker* current_worker_{nullptr};
  contexts_map contexts_;
  context<ENty, CNty>* retired_contexts_{nullptr};
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
};
}  // namespace martianlabs::doba::transport::server

#endif
