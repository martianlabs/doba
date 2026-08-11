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
#include <array>
#include <atomic>
#include <charconv>
#include <cerrno>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <memory>
#include <mutex>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "platform.h"
#include "protocol/deserialization.h"
#include "protocol/serialization.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] CONSTANTs                                                  ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
static constexpr inline std::size_t kReceiveBufferSz = 8192;
static constexpr inline std::size_t kSendBufferMaxSz = 65536;
static constexpr inline std::size_t kSendChunkSz = 8192;
static constexpr uint64_t kWakeEventId = 0;
static constexpr uint64_t kListenerEventId =
    std::numeric_limits<uint64_t>::max();
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] FORWARDs                                                   ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty,
          template <typename, typename> class DEty>
struct context;
template <typename RQty, typename RSty,
          template <typename, typename> class DEty>
struct worker;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] response_data                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct response_data {
  std::unique_ptr<protocol::serialization_result> response;
  bool prefix_written{false};
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] context [linux]                                            ( struct ) |
// +---------------------------------------------------------------------------+
// | This specification holds for the Linux server transport context.          |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used.                                              |
// |   RSty - response being used.                                             |
// |   DEty - decoder (deserializer) being used.                               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty,
          template <typename, typename> class DEty>
struct context
    : public std::enable_shared_from_this<context<RQty, RSty, DEty>> {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  context(int in_socket, std::weak_ptr<worker<RQty, RSty, DEty>> in_owner,
          types::on_client_disconnected_delegate on_disconnection)
      : owner{std::move(in_owner)},
        on_disconnection_{std::move(on_disconnection)},
        socket_{in_socket} {}
  context(const context&) = delete;
  context(context&&) noexcept = delete;
  ~context() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  context& operator=(const context&) = delete;
  context& operator=(context&&) noexcept = delete;
  // +=========================================================================+
  // | [>] accumulate                                               ( public ) |
  // +=========================================================================+
  std::size_t accumulate(char* buffer, std::size_t size) {
    return decoder_.accumulate(buffer, size);
  }
  // +=========================================================================+
  // | [>] deserialize                                              ( public ) |
  // +=========================================================================+
  protocol::deserialization_result<RQty> deserialize() {
    return decoder_.deserialize();
  }
  // +=========================================================================+
  // | [>] receive                                                  ( public ) |
  // +=========================================================================+
  ssize_t receive() {
    return ::recv(socket_, receive_buffer_, kReceiveBufferSz, MSG_DONTWAIT);
  }
  // +=========================================================================+
  // | [>] get_receive_buffer                                       ( public ) |
  // +=========================================================================+
  char* get_receive_buffer() { return receive_buffer_; }
  // +=========================================================================+
  // | [>] get_socket                                               ( public ) |
  // +=========================================================================+
  int get_socket() const { return socket_; }
  // +=========================================================================+
  // | [>] get_next_response_id                                     ( public ) |
  // +=========================================================================+
  uint64_t get_next_response_id() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    uint64_t response_id = next_response_id_++;
    if (!closing_ && socket_ != -1) responses_.emplace_back();
    return response_id;
  }
  // +=========================================================================+
  // | [>] enqueue_response                                         ( public ) |
  // +=========================================================================+
  bool enqueue_response(
      std::unique_ptr<protocol::serialization_result> response,
      uint64_t response_id) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    return enqueue_response_(std::move(response), response_id);
  }
  // +=========================================================================+
  // | [>] enqueue_error_response                                   ( public ) |
  // +=========================================================================+
  bool enqueue_error_response(
      std::unique_ptr<protocol::serialization_result> response,
      uint64_t response_id) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    if (!enqueue_response_(std::move(response), response_id)) {
      return fail_response_(response_id);
    }
    auto first = responses_.begin() +
                 static_cast<std::ptrdiff_t>(response_id -
                                             expected_response_id_) +
                 1;
    responses_.erase(first, responses_.end());
    closing_ = true;
    return true;
  }
  // +=========================================================================+
  // | [>] fail_response                                            ( public ) |
  // +=========================================================================+
  bool fail_response(uint64_t response_id) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    return fail_response_(response_id);
  }
  // +=========================================================================+
  // | [>] connected                                                ( public ) |
  // +=========================================================================+
  void connected() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    connected_ = true;
  }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +=========================================================================+
  void close() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    closing_ = true;
  }
  // +=========================================================================+
  // | [>] abort                                                    ( public ) |
  // +=========================================================================+
  void abort() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    abort_();
  }
  // +=========================================================================+
  // | [>] can_receive                                              ( public ) |
  // +=========================================================================+
  bool can_receive() const {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    return socket_ != -1 && !closing_;
  }
  // +=========================================================================+
  // | [>] is_closed                                                ( public ) |
  // +=========================================================================+
  bool is_closed() const { return socket_ == -1; }
  // +=========================================================================+
  // | [>] flush_send                                               ( public ) |
  // +=========================================================================+
  bool flush_send() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    if (socket_ == -1 || aborted_) return false;
    for (;;) {
      if (sending_offset_ == sending_buffer_.size()) {
        sending_buffer_.clear();
        sending_offset_ = 0;
        if (!append_sendable_responses_()) return false;
      }
      if (sending_buffer_.empty()) return true;
      ssize_t sent = ::send(socket_, sending_buffer_.data() + sending_offset_,
                            sending_buffer_.size() - sending_offset_,
                            MSG_NOSIGNAL);
      if (sent > 0) {
        sending_offset_ += static_cast<std::size_t>(sent);
        continue;
      }
      if (sent == -1 && errno == EINTR) continue;
      if (sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) return true;
      return false;
    }
  }
  // +=========================================================================+
  // | [>] get_event_mask                                           ( public ) |
  // +=========================================================================+
  uint32_t get_event_mask() const {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    if (socket_ == -1 || aborted_) return 0;
    if (closing_ && sending_offset_ == sending_buffer_.size() &&
        responses_.empty()) {
      return 0;
    }
    uint32_t events = EPOLLRDHUP | EPOLLET;
    if (!closing_) events |= EPOLLIN;
    if (sending_offset_ < sending_buffer_.size() ||
        (!responses_.empty() && responses_.front().response)) {
      events |= EPOLLOUT;
    }
    return events;
  }
  // +=========================================================================+
  // | [>] retire_socket                                           ( public )  |
  // +=========================================================================+
  int retire_socket() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
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
    {
      std::lock_guard<std::mutex> sending_lock(sending_mutex_);
      if (!connected_ || disconnected_ || socket_ != -1) return;
      disconnected_ = true;
    }
    try {
      on_disconnection_();
    } catch (...) {
    }
  }
  // +=========================================================================+
  // | ATTRIBUTEs                                                   ( public ) |
  // +=========================================================================+
  std::weak_ptr<worker<RQty, RSty, DEty>> owner;
  context* retirement_next{nullptr};

 private:
  // +=========================================================================+
  // | [>] enqueue_response_                                       ( private ) |
  // +=========================================================================+
  bool enqueue_response_(
      std::unique_ptr<protocol::serialization_result> response,
      uint64_t response_id) {
    if (!response || socket_ == -1 || aborted_ ||
        response_id < expected_response_id_) {
      return false;
    }
    uint64_t offset = response_id - expected_response_id_;
    if (offset >= responses_.size()) return false;
    response_data& data = responses_[static_cast<std::size_t>(offset)];
    if (data.response) return false;
    data.response = std::move(response);
    return true;
  }
  // +=========================================================================+
  // | [>] fail_response_                                          ( private ) |
  // +=========================================================================+
  bool fail_response_(uint64_t response_id) {
    if (socket_ == -1 || aborted_ || response_id < expected_response_id_) {
      return false;
    }
    uint64_t offset = response_id - expected_response_id_;
    if (offset >= responses_.size()) return false;
    auto first = responses_.begin() + static_cast<std::ptrdiff_t>(offset);
    if (first->response) return false;
    responses_.erase(first, responses_.end());
    closing_ = true;
    return true;
  }
  // +=========================================================================+
  // | [>] append_sendable_responses_                              ( private ) |
  // +=========================================================================+
  bool append_sendable_responses_() {
    while (!responses_.empty() &&
           sending_buffer_.size() < kSendBufferMaxSz) {
      response_data& data = responses_.front();
      if (!data.response) break;
      if (!data.prefix_written) {
        sending_buffer_.append(data.response->prefix);
        data.prefix_written = true;
        continue;
      }
      auto& source = data.response->source;
      if (source.has_value() && !source->eof()) {
        std::byte chunk[kSendChunkSz];
        std::size_t room = kSendBufferMaxSz - sending_buffer_.size();
        if (room > kSendChunkSz) room = kSendChunkSz;
        std::size_t read = source->read(std::span<std::byte>(chunk, room));
        if (source->failed() || read > room) {
          abort_();
          return false;
        }
        if (!read) {
          source.reset();
          continue;
        }
        sending_buffer_.append(reinterpret_cast<const char*>(chunk), read);
        continue;
      }
      expected_response_id_++;
      responses_.pop_front();
    }
    return true;
  }
  // +=========================================================================+
  // | [>] abort_                                                  ( private ) |
  // +=========================================================================+
  void abort_() {
    aborted_ = true;
    closing_ = true;
    responses_.clear();
    sending_buffer_.clear();
    sending_offset_ = 0;
  }
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +=========================================================================+
  types::on_client_disconnected_delegate on_disconnection_;
  mutable std::mutex sending_mutex_;
  int socket_{-1};
  bool closing_{false};
  bool connected_{false};
  bool disconnected_{false};
  bool aborted_{false};
  DEty<RQty, RSty> decoder_{};
  char receive_buffer_[kReceiveBufferSz];
  std::string sending_buffer_;
  std::size_t sending_offset_{0};
  std::deque<response_data> responses_;
  uint64_t expected_response_id_{0};
  uint64_t next_response_id_{0};
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] worker [linux]                                             ( struct ) |
// +---------------------------------------------------------------------------+
// | This specification holds for a Linux server transport worker.             |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used.                                              |
// |   RSty - response being used.                                             |
// |   DEty - decoder (deserializer) being used.                               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty,
          template <typename, typename> class DEty>
struct worker : public std::enable_shared_from_this<worker<RQty, RSty, DEty>> {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  worker(types::on_request_delegate<RQty, RSty> on_request,
         types::on_bad_request_delegate<RSty> on_bad_request,
         types::on_client_connected_delegate on_connection,
         types::on_client_disconnected_delegate on_disconnection)
      : on_request_{std::move(on_request)},
        on_bad_request_{std::move(on_bad_request)},
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
  void setup(uint16_t port) {
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
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    if (::bind(listener_fd_, reinterpret_cast<const sockaddr*>(&address),
               sizeof(address)) == -1 ||
        ::listen(listener_fd_, SOMAXCONN) == -1) {
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
    std::lock_guard<std::mutex> pending_lock(pending_mutex_);
    accepting_notifications_ = true;
  }
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +=========================================================================+
  void start() { thread_ = std::jthread([this]() { run(); }); }
  // +=========================================================================+
  // | [>] is_current_thread                                        ( public ) |
  // +=========================================================================+
  bool is_current_thread() const {
    return thread_.joinable() && thread_.get_id() == std::this_thread::get_id();
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    if (is_current_thread()) {
      throw std::runtime_error("Worker cannot be stopped from itself!");
    }
    if (epoll_fd_ == -1) return;
    stopping_.store(true);
    {
      std::lock_guard<std::mutex> pending_lock(pending_mutex_);
      accepting_notifications_ = false;
      uint64_t wake = 1;
      ssize_t written = 0;
      do {
        written = ::write(wake_fd_, &wake, sizeof(wake));
      } while (written == -1 && errno == EINTR);
    }
    if (thread_.joinable()) thread_.join();
    close_contexts();
    {
      std::lock_guard<std::mutex> pending_lock(pending_mutex_);
      pending_contexts_.clear();
      close_resources();
    }
  }
  // +=========================================================================+
  // | [>] notify                                                   ( public ) |
  // +=========================================================================+
  void notify(std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    std::lock_guard<std::mutex> pending_lock(pending_mutex_);
    if (!accepting_notifications_) return;
    bool wake_worker = pending_contexts_.empty();
    pending_contexts_.emplace_back(std::move(ctx));
    if (!wake_worker) return;
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
          auto ctx = static_cast<context<RQty, RSty, DEty>*>(
              events[i].data.ptr);
          if (!ctx->is_closed()) {
            try {
              handle_context(ctx, events[i].events);
            } catch (...) {
              abort_context(ctx);
            }
          }
        }
        if (stopping_.load()) break;
      }
      retire_contexts();
      if (stopping_.load()) break;
    }
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
    if (stopping_.load()) return;
    std::vector<std::shared_ptr<context<RQty, RSty, DEty>>> pending;
    {
      std::lock_guard<std::mutex> pending_lock(pending_mutex_);
      pending.swap(pending_contexts_);
    }
    for (const auto& ctx : pending) {
      if (ctx->is_closed()) continue;
      try {
        if (!ctx->flush_send()) {
          abort_context(ctx.get());
        } else {
          rearm_context(ctx.get());
        }
      } catch (...) {
        abort_context(ctx.get());
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
            errno == ENONET || errno == EHOSTUNREACH ||
            errno == EOPNOTSUPP || errno == ENETUNREACH) {
          continue;
        }
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
    std::shared_ptr<context<RQty, RSty, DEty>> ctx =
        std::make_shared<context<RQty, RSty, DEty>>(
            socket, this->shared_from_this(), on_disconnection_);
    if (!contexts_.emplace(ctx.get(), ctx).second) {
      throw std::runtime_error("Context could not be registered!");
    }
    epoll_event event{};
    event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
    event.data.ptr = ctx.get();
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket, &event) == -1) {
      ctx->abort();
      close_context(ctx.get());
      return;
    }
    try {
      on_connection_();
    } catch (...) {
      close_context(ctx.get());
      return;
    }
    ctx->connected();
  }
  // +=========================================================================+
  // | [>] handle_context                                          ( private ) |
  // +=========================================================================+
  void handle_context(context<RQty, RSty, DEty>* ctx, uint32_t events) {
    if (events & EPOLLERR) {
      abort_context(ctx);
      return;
    }
    if (events & (EPOLLIN | EPOLLRDHUP | EPOLLHUP)) {
      if (!handle_receive(ctx)) {
        abort_context(ctx);
        return;
      }
    }
    if (ctx->is_closed()) return;
    if (!ctx->flush_send()) {
      abort_context(ctx);
      return;
    }
    rearm_context(ctx);
  }
  // +=========================================================================+
  // | [>] handle_receive                                          ( private ) |
  // +=========================================================================+
  bool handle_receive(context<RQty, RSty, DEty>* ctx) {
    while (ctx->can_receive()) {
      ssize_t received = ctx->receive();
      if (received > 0) {
        consume_received(ctx, ctx->get_receive_buffer(),
                         static_cast<std::size_t>(received));
        continue;
      }
      if (received == 0) {
        ctx->close();
        return true;
      }
      if (errno == EINTR) continue;
      if (errno == EAGAIN || errno == EWOULDBLOCK) return true;
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] consume_received                                        ( private ) |
  // +=========================================================================+
  void consume_received(context<RQty, RSty, DEty>* ctx, char* buffer,
                        std::size_t size) {
    std::size_t consumed = 0;
    do {
      std::size_t accepted =
          ctx->accumulate(buffer + consumed, size - consumed);
      if (!accepted) {
        handle_deserialized_requests(ctx);
        if (!ctx->can_receive()) return;
        accepted = ctx->accumulate(buffer + consumed, size - consumed);
      }
      if (!accepted || accepted > (size - consumed)) {
        enqueue_error_response(ctx, ctx->get_next_response_id(), 0,
                               "Invalid request content!");
        return;
      }
      handle_deserialized_requests(ctx);
      consumed += accepted;
    } while (ctx->can_receive() && consumed < size);
  }
  // +=========================================================================+
  // | [>] handle_deserialized_requests                            ( private ) |
  // +=========================================================================+
  void handle_deserialized_requests(context<RQty, RSty, DEty>* ctx) {
    while (ctx->can_receive()) {
      protocol::deserialization_result<RQty> result = ctx->deserialize();
      if (result.code == protocol::deserialization_status::kMoreBytesNeeded) {
        // The protocol may need some bytes on the wire before it can go on
        // (their meaning is opaque here); they take their own response slot
        // so they are written ahead of any later response.
        if (!result.interim.empty()) {
          auto interim = std::make_unique<protocol::serialization_result>();
          interim->prefix.assign(result.interim);
          ctx->enqueue_response(std::move(interim),
                                ctx->get_next_response_id());
        }
        return;
      }
      uint64_t response_id = ctx->get_next_response_id();
      if (result.code == protocol::deserialization_status::kInvalidSource) {
        enqueue_error_response(ctx, response_id, result.reason,
                               "Invalid request content!");
        return;
      }
      if (!result.request) {
        enqueue_error_response(ctx, response_id, 0, "Decoder error!");
        return;
      }
      if (result.channel == protocol::channel_intent::kClose) ctx->close();
      std::thread::id this_thread_id = std::this_thread::get_id();
      std::shared_ptr<context<RQty, RSty, DEty>> context_owner =
          ctx->shared_from_this();
      try {
        on_request_(
            result.request, std::make_shared<RSty>(),
            [context_owner, response_id,
             this_thread_id](std::shared_ptr<RSty> response) {
              bool completed = false;
              if (response) {
                try {
                  auto serialized = response->serialize();
                  if (serialized) {
                    completed = context_owner->enqueue_response(
                        std::move(serialized), response_id);
                  } else {
                    completed = context_owner->fail_response(response_id);
                  }
                } catch (...) {
                  completed = context_owner->fail_response(response_id);
                }
              } else {
                completed = context_owner->fail_response(response_id);
              }
              if (completed && std::this_thread::get_id() != this_thread_id) {
                auto owner = context_owner->owner.lock();
                if (owner) owner->notify(context_owner);
              }
            });
      } catch (const std::exception& ex) {
        enqueue_error_response(ctx, response_id, 7, ex.what());
        return;
      } catch (...) {
        enqueue_error_response(ctx, response_id, 7,
                               "Request handler error!");
        return;
      }
      if (result.channel == protocol::channel_intent::kClose) return;
    }
  }
  // +=========================================================================+
  // | [>] enqueue_error_response                                  ( private ) |
  // +=========================================================================+
  void enqueue_error_response(context<RQty, RSty, DEty>* ctx,
                              uint64_t response_id, int reason_code,
                              std::string_view reason) {
    try {
      std::shared_ptr<RSty> response = std::make_shared<RSty>();
      on_bad_request_(reason_code, reason, response);
      auto serialized = response->serialize();
      if (!serialized ||
          !ctx->enqueue_error_response(std::move(serialized), response_id)) {
        ctx->fail_response(response_id);
      }
    } catch (...) {
      ctx->fail_response(response_id);
    }
  }
  // +=========================================================================+
  // | [>] rearm_context                                           ( private ) |
  // +=========================================================================+
  void rearm_context(context<RQty, RSty, DEty>* ctx) {
    uint32_t events = ctx->get_event_mask();
    if (!events) {
      close_context(ctx);
      return;
    }
    epoll_event event{};
    event.events = events;
    event.data.ptr = ctx;
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, ctx->get_socket(), &event) ==
        -1) {
      abort_context(ctx);
    }
  }
  // +=========================================================================+
  // | [>] abort_context                                           ( private ) |
  // +=========================================================================+
  void abort_context(context<RQty, RSty, DEty>* ctx) {
    ctx->abort();
    close_context(ctx);
  }
  // +=========================================================================+
  // | [>] close_context                                           ( private ) |
  // +=========================================================================+
  void close_context(context<RQty, RSty, DEty>* ctx) {
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
      context<RQty, RSty, DEty>* ctx = retired_contexts_;
      retired_contexts_ = ctx->retirement_next;
      contexts_.erase(ctx);
    }
  }
  // +=========================================================================+
  // | [>] close_contexts                                          ( private ) |
  // +=========================================================================+
  void close_contexts() {
    for (auto& item : contexts_) {
      context<RQty, RSty, DEty>* ctx = item.first;
      if (ctx->is_closed()) continue;
      ctx->abort();
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
  int epoll_fd_{-1};
  int wake_fd_{-1};
  int listener_fd_{-1};
  std::atomic<bool> stopping_{false};
  std::jthread thread_;
  std::unordered_map<context<RQty, RSty, DEty>*,
                     std::shared_ptr<context<RQty, RSty, DEty>>>
      contexts_;
  context<RQty, RSty, DEty>* retired_contexts_{nullptr};
  std::vector<std::shared_ptr<context<RQty, RSty, DEty>>> pending_contexts_;
  std::mutex pending_mutex_;
  bool accepting_notifications_{false};
  types::on_request_delegate<RQty, RSty> on_request_;
  types::on_bad_request_delegate<RSty> on_bad_request_;
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcpip [linux]                                               ( class ) |
// +---------------------------------------------------------------------------+
// | This specification holds for the Linux server transport.                  |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used.                                              |
// |   RSty - response being used.                                             |
// |   DEty - decoder (deserializer) being used.                               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty,
          template <typename, typename> class DEty>
class tcpip {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  tcpip() = default;
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
  void start(const char port[]) {
    {
      std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
      if (starting_ || stopping_ || !workers_.empty()) return;
      starting_ = true;
    }
    try {
      uint16_t port_number = parse_port(port);
      std::size_t number_of_workers =
          std::max<std::size_t>(1, std::thread::hardware_concurrency());
      for (std::size_t i = 0; i < number_of_workers; i++) {
        auto entry = std::make_shared<worker<RQty, RSty, DEty>>(
            on_request_, on_bad_request_, on_connection_, on_disconnection_);
        entry->setup(port_number);
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
  // | [>] set_on_request                                           ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_request(FNty&& fn) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    ensure_stopped();
    on_request_ = std::forward<FNty>(fn);
  }
  // +=========================================================================+
  // | [>] set_on_bad_request                                       ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_bad_request(FNty&& fn) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    ensure_stopped();
    on_bad_request_ = std::forward<FNty>(fn);
  }
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
    std::vector<std::shared_ptr<worker<RQty, RSty, DEty>>> workers;
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
      for (const auto& entry : workers_) {
        if (entry->is_current_thread()) {
          throw std::runtime_error(
              "Transport cannot be stopped from a worker!");
        }
      }
      stopping_ = true;
      stopping_thread_ = std::this_thread::get_id();
      workers.swap(workers_);
    }
    try {
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
  std::vector<std::shared_ptr<worker<RQty, RSty, DEty>>> workers_;
  std::mutex lifecycle_mutex_;
  std::condition_variable lifecycle_condition_;
  bool starting_{false};
  bool stopping_{false};
  std::thread::id stopping_thread_{};
  types::on_request_delegate<RQty, RSty> on_request_;
  types::on_bad_request_delegate<RSty> on_bad_request_;
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
};
}  // namespace martianlabs::doba::transport::server

#endif
