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

#ifndef martianlabs_doba_transport_server_tcp_linux_context_h
#define martianlabs_doba_transport_server_tcp_linux_context_h

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <span>
#include <string_view>
#include <type_traits>
#include <sys/socket.h>
#include <utility>

#include "common/task.h"
#include "platform.h"
#include "transport/server/tcp_connection.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] context                                                    ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct holds an engine's TCP connection state. It manages the        |
// | socket and input and output buffers, and handles sending and receiving    |
// | data. It also lets you connect, stop, close, or abort the connection.     |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   ENty - engine type being used.                                          |
// |   CNty - connection type being used.                                      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty, typename CNty>
struct context : public std::enable_shared_from_this<context<ENty, CNty>> {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  template <protocol::contracts::engine_factory<ENty> FAty>
  context(int in_socket, std::size_t recv_buffer_size,
          std::size_t send_buffer_size, const FAty& create_engine,
          types::on_client_connected_delegate on_connection,
          types::on_client_disconnected_delegate on_disconnection,
          typename CNty::shared_state shared_state,
          std::size_t encrypted_receive_buffer_size = 0,
          std::size_t network_bio_buffer_size = 0)
      : on_connection_{std::move(on_connection)},
        on_disconnection_{std::move(on_disconnection)},
        socket_{in_socket},
        input_{[&]() -> CNty {
          if constexpr (std::is_constructible_v<CNty, std::size_t, std::size_t,
                                                const FAty&,
                                                typename CNty::shared_state,
                                                std::size_t, std::size_t>) {
            return CNty(recv_buffer_size, send_buffer_size, create_engine,
                        shared_state, encrypted_receive_buffer_size,
                        network_bio_buffer_size);
          } else {
            return CNty(recv_buffer_size, send_buffer_size, create_engine,
                        shared_state);
          }
        }()},
        output_{send_buffer_size} {}
  context(const context&) = delete;
  context(context&&) noexcept = delete;
  ~context() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  context& operator=(const context&) = delete;
  context& operator=(context&&) noexcept = delete;
  // +=========================================================================+
  // | [>] receive                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Reads data from the socket and passes it to the engine. It returns the  |
  // | number of bytes received, or a negative value if something goes wrong.  |
  // | If the connection is closing or the input buffer is full, it returns 0. |
  // | It also catches errors during processing and aborts the connection if   |
  // | needed.                                                                 |
  // +-------------------------------------------------------------------------+
  ssize_t receive() {
    ssize_t received;
    {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      if (closing_ || input_.size == input_.capacity) return 0;
      received = ::recv(socket_, input_.buffer.get() + input_.size,
                        input_.capacity - input_.size, MSG_DONTWAIT);
      if (received <= 0) return received;
      input_.size += static_cast<std::size_t>(received);
      processing_receive_ = true;
    }
    std::size_t processed;
    try {
      processed = input_.process();
    } catch (...) {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      processing_receive_ = false;
      abort_();
      throw;
    }
    {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      processing_receive_ = false;
      if (!input_.consume(processed)) {
        abort_();
        return received;
      }
      if (input_.size == input_.capacity) abort_();
      if (input_.peer_closed() && !closing_) {
        closing_ = true;
        if (!input_.close_output()) abort_();
      }
    }
    notify_connection();
    return received;
  }
  // +=========================================================================+
  // | [>] connected                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Sets the epoll file descriptor and async scheduler on the engine, along |
  // | with its `on_close` and `on_send` callbacks. When the engine closes,    |
  // | `on_close` calls this context’s `close()` method. When the engine wants |
  // | to send data, `on_send` calls this context’s `send()` method with the   |
  // | head, body, and source it provides.                                     |
  // +-------------------------------------------------------------------------+
  void connected(int epoll_fd, common::task_scheduler scheduler = {}) {
    {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      epoll_fd_ = epoll_fd;
    }
    if constexpr (requires { input_.engine.set_async_scheduler(scheduler); }) {
      input_.engine.set_async_scheduler(scheduler);
    }
    input_.engine.set_on_close([weak = this->weak_from_this()]() {
      if (auto ctx = weak.lock()) ctx->close();
    });
    input_.engine.set_on_send([weak = this->weak_from_this()](
                                  std::string_view head, std::string_view body,
                                  std::unique_ptr<common::reader> source) {
      if (auto ctx = weak.lock()) {
        ctx->send(head, body, std::move(source));
      }
    });
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Stops the connection and closes the output. If `stopping` is true, it   |
  // | means something outside the connection triggered the stop. The method   |
  // | sets the `stopping_` and `closing_` flags, then calls `close_output()`  |
  // | on the input. If that call fails, it aborts the connection.             |
  // +-------------------------------------------------------------------------+
  void stop(bool stopping = false) {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    stopping_ = stopping_ || stopping;
    closing_ = true;
    if (!input_.close_output()) abort_();
  }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Closes the connection and its output by setting the `closing_` flag and |
  // | calling `close_output()` on the input. If that call fails, it aborts    |
  // | the connection.                                                         |
  // +-------------------------------------------------------------------------+
  void close() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    if (closing_ || socket_ == -1) return;
    closing_ = true;
    if (!input_.close_output()) {
      abort_();
      return;
    }
    if (!processing_receive_ && !update_interest_(true)) abort_();
  }
  // +=========================================================================+
  // | [>] abort                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Aborts the connection by calling the internal `abort_()` method.        |
  // +-------------------------------------------------------------------------+
  void abort() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    abort_();
  }
  // +=========================================================================+
  // | [>] can_receive                                              ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns true if the socket is valid and the connection isn’t closing,   |
  // | so it can receive data. Otherwise, it returns false.                    |
  // +-------------------------------------------------------------------------+
  bool can_receive() const {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    return socket_ != -1 && !closing_;
  }
  // +=========================================================================+
  // | [>] is_closed                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns true if the socket is invalid, which means the connection is    |
  // | closed. Otherwise, it returns false.                                    |
  // +-------------------------------------------------------------------------+
  bool is_closed() const {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    return socket_ == -1;
  }
  // +=========================================================================+
  // | [>] eof                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  bool eof() const { return input_.eof(); }
  // +=========================================================================+
  // | [>] notify_connection                                        ( public ) |
  // +-------------------------------------------------------------------------+
  // | Lets the owner know the connection is up by calling `on_connection_`.   |
  // | It locks the mutex to check whether the connection has already been     |
  // | reported or is still being set up. If it hasn’t been reported yet,      |
  // | it sets the `notified_` flag and calls the callback.                    |
  // +-------------------------------------------------------------------------+
  void notify_connection() {
    {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      if (notified_ || !input_.established()) return;
      notified_ = true;
    }
    on_connection_();
  }
  // +=========================================================================+
  // | [>] send                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Sends data over the connection, using a mutex to keep things            |
  // | thread-safe. If the connection is closing, the socket is invalid,       |
  // | or there’s no data to send, it returns right away. Otherwise, it adds   |
  // | the data to the output buffer and updates the write interest.           |
  // | If anything fails, it aborts the connection.                            |
  // +-------------------------------------------------------------------------+
  void send(std::string_view head, std::string_view body,
            std::unique_ptr<common::reader> source) {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    if (closing_ || socket_ == -1) return;
    if (head.empty() && body.empty() && !source) return;
    if (!output_.push(head, body, std::move(source))) {
      abort_();
      return;
    }
    if (!processing_receive_ && !update_interest_(true)) abort_();
  }
  // +=========================================================================+
  // | [>] send_pending                                             ( public ) |
  // +-------------------------------------------------------------------------+
  // | Sends any data waiting in the output buffer, while using a mutex to     |
  // | keep things thread-safe. If the socket is invalid, the connection has   |
  // | been aborted, or there’s nothing to send, it returns false.             |
  // | Otherwise, it gets the data ready and sends it over the socket.         |
  // | If the send succeeds, it updates the output state and checks whether    |
  // | there’s more data to send. If the connection is closing, it stops       |
  // | sending through the socket and ignores any more incoming data until     |
  // | the peer closes the connection. Returns true if everything works,       |
  // | or false if an error occurs.                                            |
  // +-------------------------------------------------------------------------+
  bool send_pending() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    if (socket_ == -1 || aborted_) return false;
    if (!input_.prepare_output(output_)) return false;
    const std::span<char> bytes = input_.output_bytes(output_);
    if (!bytes.empty()) {
      ssize_t sent;
      do {
        sent = ::send(socket_, bytes.data(),
                      std::min<std::size_t>(
                          bytes.size(), std::numeric_limits<ssize_t>::max()),
                      MSG_NOSIGNAL);
      } while (sent == -1 && errno == EINTR);
      if (sent > 0) {
        if (!input_.output_sent(output_, static_cast<std::size_t>(sent))) {
          return false;
        }
        if (input_.output_pending(output_)) return update_interest_(true);
      } else if (sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return update_interest_(true);
      } else {
        return false;
      }
    }
    if (closing_) {
      if (!socket_shutdown_) {
        if (::shutdown(socket_, SHUT_WR) == -1) return false;
        socket_shutdown_ = true;
      }
      if (stopping_ || input_.peer_closed()) return false;
    }
    if (!update_interest_(false)) return false;
    if (closing_) {
      // Discard input until peer EOF to avoid resetting the final response.
      ssize_t received;
      do {
        received =
            ::recv(socket_, input_.buffer.get(), input_.capacity, MSG_DONTWAIT);
      } while (received == -1 && errno == EINTR);
      return received > 0 ||
             (received == -1 && (errno == EAGAIN || errno == EWOULDBLOCK));
    }
    return true;
  }
  // +=========================================================================+
  // | [>] retire_socket                                            ( public ) |
  // +-------------------------------------------------------------------------+
  // | Retires the socket by locking the mutex and checking whether it’s still |
  // | valid. If it is, the method sets it to -1, marks the connection as      |
  // | closing, and clears the output buffer. It returns the retired socket,   |
  // | or -1 if the socket was already invalid.                                |
  // +-------------------------------------------------------------------------+
  int retire_socket() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    if (socket_ == -1) return -1;
    const int socket = socket_;
    socket_ = -1;
    closing_ = true;
    output_.clear();
    return socket;
  }
  // +=========================================================================+
  // | [>] notify_disconnection                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Tells the owner the connection has been disconnected by calling         |
  // | `on_disconnection_`. It locks the mutex to check whether this has       |
  // | already been reported. If not, it calls the callback and ignores any    |
  // | exceptions it throws.                                                   |
  // +-------------------------------------------------------------------------+
  void notify_disconnection() {
    {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      if (!notified_) return;
    }
    try {
      on_disconnection_();
    } catch (...) {
      // Ignore exceptions thrown by the callback to avoid crashing the server.
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  context* retirement_next{nullptr};

 private:
  // +=========================================================================+
  // | [>] update_interest_                                        ( private ) |
  // +-------------------------------------------------------------------------+
  bool update_interest_(bool enabled) {
    uint32_t events = EPOLLIN | EPOLLRDHUP;
    if (closing_ && !socket_shutdown_) events = 0;
    if (enabled) events |= EPOLLOUT;
    if (watched_events_ == events) return true;
    epoll_event event{};
    event.events = events;
    event.data.ptr = this;
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, socket_, &event) == -1) {
      return false;
    }
    watched_events_ = events;
    return true;
  }
  // +=========================================================================+
  // | [>] abort_                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  void abort_() {
    closing_ = true;
    aborted_ = true;
    // Wake the owner even when output is blocked or no data is arriving.
    if (socket_ != -1) ::shutdown(socket_, SHUT_RDWR);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
  int socket_{-1};
  CNty input_;
  int epoll_fd_{-1};
  mutable std::mutex sending_mutex_;
  send_state output_;
  uint32_t watched_events_{EPOLLIN | EPOLLRDHUP};
  bool processing_receive_{false};
  bool closing_{false};
  bool stopping_{false};
  bool socket_shutdown_{false};
  bool aborted_{false};
  bool notified_{false};
};
}  // namespace martianlabs::doba::transport::server

#endif
