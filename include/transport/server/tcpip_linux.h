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

#ifndef martianlabs_doba_transport_server_tcpip_linux_h
#define martianlabs_doba_transport_server_tcpip_linux_h

#include <algorithm>
#include <arpa/inet.h>
#include <array>
#include <atomic>
#include <charconv>
#include <cerrno>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <list>
#include <memory>
#include <mutex>
#include <new>
#include <optional>
#include <span>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "network/environment.h"
#include "platform.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] CONSTANTs                                                  ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
static constexpr uint64_t kWakeEventId = 0;
static constexpr uint64_t kListenerEventId =
    std::numeric_limits<uint64_t>::max();
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] context [linux]                                            ( struct ) |
// +---------------------------------------------------------------------------+
// | This specification holds for the Linux server transport context.          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct context {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  template <protocol::contracts::engine_factory<ENty> FAty>
  context(int in_socket, std::size_t recv_buffer_size,
          std::size_t send_buffer_size, const FAty& create_engine,
          types::on_client_disconnected_delegate on_disconnection)
      : on_disconnection_{std::move(on_disconnection)},
        socket_{in_socket},
        engine_{create_engine()},
        send_capacity_{send_buffer_size},
        receive_buffer_{std::make_unique<char[]>(recv_buffer_size)},
        receive_capacity_{recv_buffer_size} {}
  context(const context&) = delete;
  context(context&&) noexcept = delete;
  ~context() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  context& operator=(const context&) = delete;
  context& operator=(context&&) noexcept = delete;
  // +=========================================================================+
  // | [>] receive                                                  ( public ) |
  // +=========================================================================+
  ssize_t receive() {
    if (receive_size_ == receive_capacity_) {
      abort();
      return 0;
    }
    ssize_t received = ::recv(socket_, receive_buffer_.get() + receive_size_,
                              receive_capacity_ - receive_size_, MSG_DONTWAIT);
    if (received > 0) {
      receive_size_ += static_cast<std::size_t>(received);
      std::size_t processed =
          engine_.on_bytes_received(receive_buffer_.get(), receive_size_,
                                    receive_capacity_);
      if (processed > receive_size_) {
        abort();
        return received;
      }
      receive_size_ -= processed;
      if (processed && receive_size_) {
        std::memmove(receive_buffer_.get(), receive_buffer_.get() + processed,
                     receive_size_);
      }
      if (receive_size_ == receive_capacity_) abort();
      if (send_reserved_ && send_offset_ == send_size_ && !send_pending()) abort();
    }
    return received;
  }
  // +=========================================================================+
  // | [>] connected                                                ( public ) |
  // +=========================================================================+
  void connected(int epoll_fd) {
    epoll_fd_ = epoll_fd;
    connected_ = true;
    engine_.set_on_close([this]() { close(); });
    engine_.set_on_send(
        [this](std::unique_ptr<char[]> buffer, std::size_t size,
               std::optional<common::reader> source) {
          send(std::move(buffer), size, std::move(source));
        });
    if (send_reserved_ && send_offset_ == send_size_ && !send_pending()) abort();
  }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +=========================================================================+
  void close() {
    closing_ = true;
  }
  // +=========================================================================+
  // | [>] abort                                                    ( public ) |
  // +=========================================================================+
  void abort() {
    closing_ = true;
    aborted_ = true;
  }
  // +=========================================================================+
  // | [>] can_receive                                              ( public ) |
  // +=========================================================================+
  bool can_receive() const {
    return socket_ != -1 && !closing_;
  }
  // +=========================================================================+
  // | [>] is_closed                                                ( public ) |
  // +=========================================================================+
  bool is_closed() const { return socket_ == -1; }
  // +=========================================================================+
  // | [>] send                                                     ( public ) |
  // +=========================================================================+
  void send(std::unique_ptr<char[]> buffer, std::size_t size,
             std::optional<common::reader> source) {
    if (closing_ || socket_ == -1) return;
    const std::size_t reserved = source
        ? std::max(size, std::min<std::size_t>(8192, send_capacity_)) : size;
    if ((size && !buffer) || (!size && !source) ||
        reserved > send_capacity_ - send_bytes_) {
      abort();
      return;
    }
    if (send_reserved_) {
      send_queue_.emplace_back(std::move(buffer), size, std::move(source));
    } else {
      send_buffer_ = std::move(buffer);
      send_source_ = std::move(source);
      send_reserved_ = reserved;
      send_streaming_ = false;
      send_size_ = size;
      send_offset_ = 0;
    }
    send_bytes_ += reserved;
    if (!send_pending(false)) abort();
  }
  // +=========================================================================+
  // | [>] send_pending                                             ( public ) |
  // +=========================================================================+
  bool send_pending(bool notify = true) {
    while (send_reserved_ && !aborted_) {
      while (send_offset_ < send_size_) {
        std::size_t size = std::min<std::size_t>(
            send_size_ - send_offset_, std::numeric_limits<ssize_t>::max());
        ssize_t sent = ::send(socket_, send_buffer_.get() + send_offset_, size,
                              MSG_NOSIGNAL);
        if (sent > 0) {
          send_offset_ += static_cast<std::size_t>(sent);
          continue;
        }
        if (sent == -1 && errno == EINTR) continue;
        if (sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
          if (!send_waiting_) {
            epoll_event event{};
            event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET;
            event.data.ptr = this;
            if (::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, socket_, &event) == -1) {
              return false;
            }
            send_waiting_ = true;
          }
          return true;
        }
        return false;
      }
      if (send_source_) {
        if (send_source_->failed()) return false;
        if (!send_source_->eof()) {
          const std::size_t capacity = std::min<std::size_t>(8192, send_capacity_);
          if (!send_streaming_) {
            send_buffer_.reset();
            send_buffer_.reset(new (std::nothrow) char[capacity]);
            if (!send_buffer_) return false;
            send_streaming_ = true;
          }
          send_size_ = send_source_->read(std::span<std::byte>(
              reinterpret_cast<std::byte*>(send_buffer_.get()), capacity));
          send_offset_ = 0;
          if (send_source_->failed() || (!send_size_ && !send_source_->eof())) {
            return false;
          }
          if (send_size_) continue;
        }
        send_source_.reset();
      }
      if (!notify) break;
      send_bytes_ -= send_reserved_;
      send_reserved_ = 0;
      send_buffer_.reset();
      if (!send_queue_.empty()) {
        send_buffer_ = std::move(std::get<0>(send_queue_.front()));
        send_source_ = std::move(std::get<2>(send_queue_.front()));
        send_size_ = std::get<1>(send_queue_.front());
        send_reserved_ = send_source_
            ? std::max(send_size_, std::min<std::size_t>(8192, send_capacity_))
            : send_size_;
        send_streaming_ = false;
        send_offset_ = 0;
        send_queue_.pop_front();
      }
      engine_.on_send_completed(true);
    }
    if (aborted_ || closing_) return false;
    if (send_waiting_) {
      epoll_event event{};
      event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
      event.data.ptr = this;
      if (::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, socket_, &event) == -1) {
        return false;
      }
      send_waiting_ = false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] retire_socket                                           ( public )  |
  // +=========================================================================+
  int retire_socket() {
    if (socket_ == -1) return -1;
    int socket = socket_;
    socket_ = -1;
    closing_ = true;
    return socket;
  }
  // +=========================================================================+
  // | [>] notify_disconnection                                     ( public ) |
  // +=========================================================================+
  void notify_disconnection() {
    bool pending = send_reserved_ != 0;
    bool succeeded = send_offset_ == send_size_ && !send_source_;
    send_buffer_.reset();
    send_source_.reset();
    send_reserved_ = 0;
    std::size_t cancelled = send_queue_.size();
    send_queue_.clear();
    send_bytes_ = 0;
    if (pending) {
      try {
        engine_.on_send_completed(succeeded);
      } catch (...) {
      }
    }
    for (std::size_t i = 0; i < cancelled; i++) {
      try {
        engine_.on_send_completed(false);
      } catch (...) {
      }
    }
    if (!connected_ || disconnected_ || socket_ != -1) return;
    disconnected_ = true;
    try {
      on_disconnection_();
    } catch (...) {
    }
  }
  // +=========================================================================+
  // | ATTRIBUTEs                                                   ( public ) |
  // +=========================================================================+
  context* retirement_next{nullptr};

 private:
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +=========================================================================+
  types::on_client_disconnected_delegate on_disconnection_;
  int socket_{-1};
  ENty engine_;
  int epoll_fd_{-1};
  std::unique_ptr<char[]> send_buffer_;
  std::optional<common::reader> send_source_;
  std::list<std::tuple<std::unique_ptr<char[]>, std::size_t,
                       std::optional<common::reader>>> send_queue_;
  const std::size_t send_capacity_;
  std::size_t send_bytes_{0};
  std::size_t send_reserved_{0};
  std::size_t send_size_{0};
  std::size_t send_offset_{0};
  bool send_streaming_{false};
  bool send_waiting_{false};
  bool closing_{false};
  bool aborted_{false};
  bool connected_{false};
  bool disconnected_{false};
  std::unique_ptr<char[]> receive_buffer_;
  const std::size_t receive_capacity_;
  std::size_t receive_size_{0};
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] worker [linux]                                             ( struct ) |
// +---------------------------------------------------------------------------+
// | This specification holds for a Linux server transport worker.             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
struct worker {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  worker(policies configuration, const FAty& create_engine,
         types::on_client_connected_delegate on_connection,
         types::on_client_disconnected_delegate on_disconnection)
      : configuration_{configuration},
        create_engine_{create_engine},
        on_connection_{std::move(on_connection)},
        on_disconnection_{std::move(on_disconnection)} {}
  worker(const worker&) = delete;
  worker(worker&&) noexcept = delete;
  ~worker() { stop(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  worker& operator=(const worker&) = delete;
  worker& operator=(worker&&) noexcept = delete;
  // +=========================================================================+
  // | [>] setup                                                    ( public ) |
  // +=========================================================================+
  void setup(in_addr ip, uint16_t port) {
    epoll_fd_ = ::epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ == -1) {
      throw std::runtime_error("Epoll instance could not be created!");
    }
    wake_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wake_fd_ == -1) {
      close_resources();
      throw std::runtime_error("Stop event could not be created!");
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
    epoll_event wake_event{};
    wake_event.events = EPOLLIN;
    wake_event.data.u64 = kWakeEventId;
    epoll_event listener_event{};
    listener_event.events = EPOLLIN | EPOLLET;
    listener_event.data.u64 = kListenerEventId;
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, wake_fd_, &wake_event) == -1 ||
        ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listener_fd_, &listener_event) ==
            -1) {
      close_resources();
      throw std::runtime_error("Listener could not be registered!");
    }
  }
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +=========================================================================+
  void start() {
    thread_ = std::jthread([this]() {
      current_worker_ = this;
      run();
      current_worker_ = nullptr;
    });
  }
  // +=========================================================================+
  // | [>] is_current_thread                                        ( public ) |
  // +=========================================================================+
  bool is_current_thread() const {
    return current_worker_ == this;
  }
  static bool is_current_thread(const FAty& create_engine) {
    return current_worker_ &&
           &current_worker_->create_engine_ == &create_engine;
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
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
  // +=========================================================================+
  void request_stop() {
    if (stopping_.exchange(true)) return;
    uint64_t wake = 1;
    ssize_t written = 0;
    do {
      written = ::write(wake_fd_, &wake, sizeof(wake));
    } while (written == -1 && errno == EINTR);
  }

 private:
  // +=========================================================================+
  // | [>] run                                                     ( private ) |
  // +=========================================================================+
  void run() {
    std::array<epoll_event, 64> events{};
    for (;;) {
      int ready = ::epoll_wait(epoll_fd_, events.data(), events.size(), -1);
      if (ready == -1) {
        if (errno == EINTR) continue;
        break;
      }
      for (std::size_t i = 0; i < static_cast<std::size_t>(ready); i++) {
        if (events[i].data.u64 == kWakeEventId) {
          handle_wake();
        } else if (events[i].data.u64 == kListenerEventId) {
          if (!stopping_.load()) handle_listener();
        } else {
          auto ctx = static_cast<context<ENty>*>(events[i].data.ptr);
          if (!ctx->is_closed()) {
            try {
              handle_context(ctx, events[i].events);
            } catch (...) {
              close_context(ctx);
            }
          }
        }
      }
      retire_contexts();
      if (stopping_.load() && contexts_.empty()) break;
    }
    close_contexts();
  }
  // +=========================================================================+
  // | [>] handle_wake                                             ( private ) |
  // +=========================================================================+
  void handle_wake() {
    uint64_t wake = 0;
    ssize_t received = 0;
    do {
      received = ::read(wake_fd_, &wake, sizeof(wake));
    } while (received == -1 && errno == EINTR);
    if (!stopping_.load()) return;
    if (listener_fd_ != -1) {
      ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, listener_fd_, nullptr);
      ::close(listener_fd_);
      listener_fd_ = -1;
    }
    for (const auto& item : contexts_) {
      auto ctx = item.first;
      if (ctx->is_closed()) continue;
      ctx->close();
      try {
        if (!ctx->send_pending()) close_context(ctx);
      } catch (...) {
        close_context(ctx);
      }
    }
  }
  // +=========================================================================+
  // | [>] handle_listener                                         ( private ) |
  // +=========================================================================+
  void handle_listener() {
    while (!stopping_.load()) {
      int socket = ::accept4(listener_fd_, nullptr, nullptr,
                             SOCK_NONBLOCK | SOCK_CLOEXEC);
      if (socket == -1) {
        if (errno == EINTR || errno == ECONNABORTED || errno == EPROTO ||
            errno == ENETDOWN || errno == ENOPROTOOPT || errno == EHOSTDOWN ||
            errno == ENONET || errno == EHOSTUNREACH || errno == EOPNOTSUPP ||
            errno == ENETUNREACH) {
          continue;
        }
        return;
      }
      if (stopping_.load()) {
        ::close(socket);
        return;
      }
      int no_delay = 1;
      if (::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &no_delay,
                       sizeof(no_delay)) == -1) {
        ::close(socket);
        continue;
      }
      try {
        register_context(socket);
      } catch (...) {
        ::close(socket);
      }
    }
  }
  // +=========================================================================+
  // | [>] register_context                                        ( private ) |
  // +=========================================================================+
  void register_context(int socket) {
    auto ctx = std::make_unique<context<ENty>>(
        socket, configuration_.recv_buffer_size, configuration_.send_buffer_size,
        create_engine_, on_disconnection_);
    auto ctx_ptr = ctx.get();
    if (!contexts_.emplace(ctx_ptr, std::move(ctx)).second) {
      throw std::runtime_error("Context could not be registered!");
    }
    epoll_event event{};
    event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
    event.data.ptr = ctx_ptr;
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket, &event) == -1) {
      close_context(ctx_ptr);
      return;
    }
    try {
      on_connection_();
      ctx_ptr->connected(epoll_fd_);
      if (!ctx_ptr->can_receive() && !ctx_ptr->send_pending()) {
        close_context(ctx_ptr);
      }
    } catch (...) {
      close_context(ctx_ptr);
      return;
    }
  }
  // +=========================================================================+
  // | [>] handle_context                                          ( private ) |
  // +=========================================================================+
  void handle_context(context<ENty>* ctx, uint32_t events) {
    if (events & EPOLLERR) {
      close_context(ctx);
      return;
    }
    if (stopping_.load()) ctx->close();
    if (ctx->can_receive() && (events & (EPOLLIN | EPOLLRDHUP | EPOLLHUP))) {
      if (!handle_receive(ctx)) {
        close_context(ctx);
        return;
      }
    }
    if (!ctx->can_receive() || (events & EPOLLOUT)) {
      if (!ctx->send_pending()) close_context(ctx);
    }
  }
  // +=========================================================================+
  // | [>] handle_receive                                          ( private ) |
  // +=========================================================================+
  bool handle_receive(context<ENty>* ctx) {
    while (ctx->can_receive() && !stopping_.load()) {
      ssize_t received = ctx->receive();
      if (received > 0) continue;
      if (received == 0) {
        ctx->close();
        return true;
      }
      if (errno == EINTR) continue;
      if (errno == EAGAIN || errno == EWOULDBLOCK) return true;
      return false;
    }
    if (stopping_.load()) ctx->close();
    return true;
  }
  // +=========================================================================+
  // | [>] close_context                                           ( private ) |
  // +=========================================================================+
  void close_context(context<ENty>* ctx) {
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
  // +=========================================================================+
  void retire_contexts() {
    while (retired_contexts_) {
      context<ENty>* ctx = retired_contexts_;
      retired_contexts_ = ctx->retirement_next;
      contexts_.erase(ctx);
    }
  }
  // +=========================================================================+
  // | [>] close_contexts                                          ( private ) |
  // +=========================================================================+
  void close_contexts() {
    for (auto& item : contexts_) {
      context<ENty>* ctx = item.first;
      if (ctx->is_closed()) continue;
      close_context(ctx);
    }
    retire_contexts();
  }
  // +=========================================================================+
  // | [>] close_resources                                         ( private ) |
  // +=========================================================================+
  void close_resources() {
    if (listener_fd_ != -1) ::close(listener_fd_);
    if (wake_fd_ != -1) ::close(wake_fd_);
    if (epoll_fd_ != -1) ::close(epoll_fd_);
    listener_fd_ = -1;
    wake_fd_ = -1;
    epoll_fd_ = -1;
  }
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +=========================================================================+
  const policies configuration_;
  const FAty& create_engine_;
  int epoll_fd_{-1};
  int wake_fd_{-1};
  int listener_fd_{-1};
  std::atomic<bool> stopping_{false};
  std::jthread thread_;
  inline static thread_local const worker* current_worker_{nullptr};
  std::unordered_map<context<ENty>*, std::unique_ptr<context<ENty>>> contexts_;
  context<ENty>* retired_contexts_{nullptr};
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcpip [linux]                                               ( class ) |
// +---------------------------------------------------------------------------+
// | This specification holds for the Linux server transport.                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
class tcpip {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  using policies_type = policies;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  explicit tcpip(policies_type configuration, FAty create_engine)
      : configuration_{configuration},
        create_engine_{std::move(create_engine)} {
    if (!configuration_.recv_buffer_size ||
        configuration_.recv_buffer_size >
            static_cast<std::size_t>(std::numeric_limits<ssize_t>::max())) {
      throw std::runtime_error("Invalid receive buffer size!");
    }
    if (!configuration_.send_buffer_size) {
      throw std::runtime_error("Invalid send buffer size!");
    }
    if (configuration_.listen_backlog < 0) {
      throw std::runtime_error("Invalid listen backlog!");
    }
  }
  tcpip(const tcpip&) = delete;
  tcpip(tcpip&&) noexcept = delete;
  ~tcpip() { stop(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  tcpip& operator=(const tcpip&) = delete;
  tcpip& operator=(tcpip&&) noexcept = delete;
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +=========================================================================+
  void start() {
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      if (starting_ || stopping_ || !workers_.empty()) return;
      starting_ = true;
    }
    try {
      in_addr ip_address{};
      if (::inet_pton(AF_INET, configuration_.ip.c_str(), &ip_address) != 1) {
        throw std::runtime_error("Invalid IPv4 address!");
      }
      uint16_t port_number = parse_port(configuration_.port.c_str());
      std::size_t number_of_workers = configuration_.worker_count;
      if (!number_of_workers) {
        number_of_workers =
            std::max<std::size_t>(1, std::thread::hardware_concurrency());
      }
      for (std::size_t i = 0; i < number_of_workers; i++) {
        auto entry = std::make_unique<worker<ENty, FAty>>(
            configuration_, create_engine_, on_connection_, on_disconnection_);
        entry->setup(ip_address, port_number);
        workers_.emplace_back(std::move(entry));
      }
      for (const auto& entry : workers_) entry->start();
    } catch (...) {
      stop_(true);
      throw;
    }
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      starting_ = false;
    }
    lifecycle_condition_.notify_all();
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() { stop_(false); }
  // +=========================================================================+
  // | [>] set_on_connection                                        ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_connection(FNty&& fn) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    ensure_stopped();
    on_connection_ = std::forward<FNty>(fn);
  }
  // +=========================================================================+
  // | [>] set_on_disconnection                                     ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_disconnection(FNty&& fn) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    ensure_stopped();
    on_disconnection_ = std::forward<FNty>(fn);
  }

 private:
  // +=========================================================================+
  // | [>] stop_                                                   ( private ) |
  // +=========================================================================+
  void stop_(bool starting_failure) {
    if (worker<ENty, FAty>::is_current_thread(create_engine_)) {
      throw std::runtime_error(
          "Transport cannot be stopped from a worker!");
    }
    std::vector<std::unique_ptr<worker<ENty, FAty>>> workers;
    {
      std::unique_lock<std::mutex> lifecycle_lock(lifecycle_mutex_);
      if (starting_failure) {
        starting_ = false;
        lifecycle_condition_.notify_all();
      } else {
        lifecycle_condition_.wait(lifecycle_lock,
                                  [this]() { return !starting_; });
      }
      if (stopping_) {
        if (stopping_thread_ == std::this_thread::get_id()) return;
        lifecycle_condition_.wait(lifecycle_lock,
                                  [this]() { return !stopping_; });
        return;
      }
      if (workers_.empty()) return;
      stopping_ = true;
      stopping_thread_ = std::this_thread::get_id();
      workers.swap(workers_);
    }
    try {
      for (const auto& entry : workers) entry->request_stop();
      for (const auto& entry : workers) entry->stop();
    } catch (...) {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      stopping_ = false;
      stopping_thread_ = {};
      lifecycle_condition_.notify_all();
      throw;
    }
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      stopping_ = false;
      stopping_thread_ = {};
    }
    lifecycle_condition_.notify_all();
  }
  // +=========================================================================+
  // | [>] parse_port                                              ( private ) |
  // +=========================================================================+
  static uint16_t parse_port(const char port[]) {
    if (!port || !*port) {
      throw std::runtime_error("Invalid port (range from 1 to 65535)!");
    }
    unsigned int port_number = 0;
    const char* end = port + std::strlen(port);
    auto result = std::from_chars(port, end, port_number);
    if (result.ec != std::errc() || result.ptr != end || port_number < 1 ||
        port_number > 65535) {
      throw std::runtime_error("Invalid port (range from 1 to 65535)!");
    }
    return static_cast<uint16_t>(port_number);
  }
  // +=========================================================================+
  // | [>] ensure_stopped                                          ( private ) |
  // +=========================================================================+
  void ensure_stopped() const {
    if (starting_ || stopping_ || !workers_.empty()) {
      throw std::runtime_error(
          "Transport callbacks cannot change while active!");
    }
  }
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +=========================================================================+
  const policies_type configuration_;
  const FAty create_engine_;
  network::detail::environment environment_;
  std::vector<std::unique_ptr<worker<ENty, FAty>>> workers_;
  std::mutex lifecycle_mutex_;
  std::condition_variable lifecycle_condition_;
  bool starting_{false};
  bool stopping_{false};
  std::thread::id stopping_thread_{};
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
};
}  // namespace martianlabs::doba::transport::server

#endif
