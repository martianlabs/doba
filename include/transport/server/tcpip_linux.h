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
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/eventfd.h>
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
template <typename RQty, typename RSty,
          template <typename, typename> class DEty>
struct epoll_registration;
template <typename RQty, typename RSty,
          template <typename, typename> class DEty>
void notify_worker(std::shared_ptr<worker<RQty, RSty, DEty>> owner,
                   std::shared_ptr<context<RQty, RSty, DEty>> ctx);
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] response_data                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct response_data {
  uint64_t id{0};
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
struct context {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  context(int in_socket, std::weak_ptr<worker<RQty, RSty, DEty>> in_owner,
          types::on_client_disconnected_delegate on_disconnection)
      : socket_{in_socket},
        owner{in_owner},
        on_disconnection_{std::move(on_disconnection)} {}
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
    return ::recv(socket_, ovr_buf_, kReceiveBufferSz, MSG_DONTWAIT);
  }
  // +=========================================================================+
  // | [>] get_receive_buffer                                       ( public ) |
  // +=========================================================================+
  char* get_receive_buffer() { return ovr_buf_; }
  // +=========================================================================+
  // | [>] get_socket                                               ( public ) |
  // +=========================================================================+
  int get_socket() const { return socket_; }
  // +=========================================================================+
  // | [>] get_next_response_id                                     ( public ) |
  // +=========================================================================+
  uint64_t get_next_response_id() { return next_response_id_++; }
  // +=========================================================================+
  // | [>] enqueue_response                                         ( public ) |
  // +=========================================================================+
  void enqueue_response(
      std::unique_ptr<protocol::serialization_result> response,
      uint64_t response_id, bool close_this_context_after_sending = false) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    enqueue_response_(std::move(response), response_id);
    if (close_this_context_after_sending) {
      closing_rid_ = response_id;
      close_requested_ = true;
      responses_.erase(
          std::remove_if(responses_.begin(), responses_.end(),
                         [response_id](const response_data& response) {
                           return response.id > response_id;
                         }),
          responses_.end());
      receiving_ = false;
    }
  }
  // +=========================================================================+
  // | [>] enqueue_error_response                                   ( public ) |
  // +=========================================================================+
  void enqueue_error_response(std::shared_ptr<RSty> error_response) {
    if (!error_response) {
      close();
      return;
    }
    uint64_t id = get_next_response_id();
    enqueue_response(std::move(error_response->serialize()), id, true);
  }
  // +=========================================================================+
  // | [>] set_closing_rid                                          ( public ) |
  // +=========================================================================+
  // | Marks the last response identifier to be sent before closing; the       |
  // | context stops receiving but stays alive until that response is fully    |
  // | flushed.                                                                |
  // +=========================================================================+
  void set_closing_rid(uint64_t rid) {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    closing_rid_ = rid;
    close_requested_ = true;
    receiving_ = false;
  }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +=========================================================================+
  // | Requests context closing; the context stops receiving but stays alive   |
  // | until every already queued response has been fully flushed.             |
  // +=========================================================================+
  void close() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    close_requested_ = true;
    receiving_ = false;
  }
  // +=========================================================================+
  // | [>] can_receive                                              ( public ) |
  // +=========================================================================+
  bool can_receive() const { return !closing_.load() && receiving_; }
  // +=========================================================================+
  // | [>] is_closing                                               ( public ) |
  // +=========================================================================+
  bool is_closing() const { return closing_.load(); }
  // +=========================================================================+
  // | [>] flush_send                                               ( public ) |
  // +=========================================================================+
  bool flush_send() {
    while (true) {
      const char* data = nullptr;
      std::size_t size = 0;
      {
        std::lock_guard<std::mutex> sending_lock(sending_mutex_);
        if (closing_) return false;
        if (sending_offset_ == sending_buffer_.size()) {
          sending_buffer_.clear();
          sending_offset_ = 0;
          append_sendable_responses_();
        }
        if (should_close_()) return false;
        if (sending_buffer_.empty()) return true;
        data = sending_buffer_.data() + sending_offset_;
        size = sending_buffer_.size() - sending_offset_;
      }
      ssize_t sent = ::send(socket_, data, size, MSG_NOSIGNAL);
      if (sent > 0) {
        std::lock_guard<std::mutex> sending_lock(sending_mutex_);
        sending_offset_ += static_cast<std::size_t>(sent);
        continue;
      }
      if (sent == -1 && errno == EINTR) continue;
      if (sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) return true;
      return false;
    }
  }
  // +=========================================================================+
  // | [>] should_close                                             ( public ) |
  // +=========================================================================+
  bool should_close() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    return should_close_();
  }
  // +=========================================================================+
  // | [>] has_sendable_response                                    ( public ) |
  // +=========================================================================+
  bool has_sendable_response() {
    std::lock_guard<std::mutex> sending_lock(sending_mutex_);
    return has_sendable_response_();
  }
  // +=========================================================================+
  // | [>] mark_context_for_closing                                 ( public ) |
  // +=========================================================================+
  int mark_context_for_closing() {
    if (closing_.exchange(true)) return -1;
    receiving_ = false;
    int result = socket_;
    socket_ = -1;
    return result;
  }
  // +=========================================================================+
  // | [>] notify_disconnection                                     ( public ) |
  // +=========================================================================+
  void notify_disconnection() {
    try {
      on_disconnection_();
    } catch (...) {
    }
  }
  // +=========================================================================+
  // | ATTRIBUTEs                                                   ( public ) |
  // +=========================================================================+
  std::weak_ptr<worker<RQty, RSty, DEty>> owner;
  epoll_registration<RQty, RSty, DEty>* registration{nullptr};

 private:
  // +=========================================================================+
  // | [>] enqueue_response_                                       ( private ) |
  // +=========================================================================+
  void enqueue_response_(
      std::unique_ptr<protocol::serialization_result> response,
      uint64_t response_id) {
    if (!response || closing_ ||
        (closing_rid_ && response_id > *closing_rid_)) {
      return;
    }
    response_data data{response_id, std::move(response)};
    auto itr = responses_.begin();
    while (itr != responses_.end() && itr->id < data.id) itr++;
    if (itr != responses_.end() && itr->id == data.id) return;
    responses_.insert(itr, std::move(data));
  }
  // +=========================================================================+
  // | [>] has_sendable_response_                                  ( private ) |
  // +=========================================================================+
  bool has_sendable_response_() {
    if (sending_offset_ < sending_buffer_.size()) return true;
    // The responses queue is kept ordered, so only its head can be sent!
    return !responses_.empty() &&
           responses_.front().id == expected_response_id_;
  }
  // +=========================================================================+
  // | [>] append_sendable_responses_                              ( private ) |
  // +=========================================================================+
  void append_sendable_responses_() {
    while (true) {
      if (closing_rid_ && expected_response_id_ > *closing_rid_) return;
      if (sending_buffer_.size() >= kSendBufferMaxSz) return;
      // The responses queue is kept ordered, so only its head can be sent!
      auto itr = responses_.begin();
      if (itr == responses_.end() || itr->id != expected_response_id_) return;
      if (!itr->prefix_written) {
        sending_buffer_.append(itr->response->prefix);
        itr->prefix_written = true;
        continue;
      }
      auto& source = itr->response->source;
      if (source.has_value() && !source->eof()) {
        // Let's pour, at most, the remaining outgoing buffer capacity!
        std::byte chunk[kSendChunkSz];
        std::size_t room = kSendBufferMaxSz - sending_buffer_.size();
        if (room > kSendChunkSz) room = kSendChunkSz;
        std::size_t read = source->read(std::span<std::byte>(chunk, room));
        if (source->failed()) {
          close_requested_ = true;
          return;
        }
        if (!read) {
          // Sources are synchronous readers, so a zero-byte read means there
          // is nothing else to pour: let's retire this response right below!
          source.reset();
          continue;
        }
        sending_buffer_.append(reinterpret_cast<const char*>(chunk), read);
        continue;
      }
      expected_response_id_++;
      responses_.erase(itr);
      if (closing_rid_ && expected_response_id_ > *closing_rid_) return;
    }
  }
  // +=========================================================================+
  // | [>] should_close_                                           ( private ) |
  // +=========================================================================+
  // | Mirrors the WindowsTM cleanup criteria: a closing context is only torn  |
  // | down once every pending byte and queued response has been flushed.      |
  // +=========================================================================+
  bool should_close_() {
    if (!close_requested_) return false;
    if (sending_offset_ != sending_buffer_.size()) return false;
    if (!responses_.empty()) return false;
    if (closing_rid_ && expected_response_id_ <= *closing_rid_) return false;
    return true;
  }
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +=========================================================================+
  types::on_client_disconnected_delegate on_disconnection_;
  std::atomic<bool> closing_{false};
  bool close_requested_{false};
  bool receiving_{true};
  int socket_{-1};
  DEty<RQty, RSty> decoder_{};
  char ovr_buf_[kReceiveBufferSz];
  uint64_t next_response_id_{0};
  std::vector<response_data> responses_;
  uint64_t expected_response_id_{0};
  std::optional<uint64_t> closing_rid_;
  std::string sending_buffer_;
  std::size_t sending_offset_{0};
  std::mutex sending_mutex_;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] epoll_registration                                         ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty,
          template <typename, typename> class DEty>
struct epoll_registration {
  epoll_registration(std::shared_ptr<context<RQty, RSty, DEty>> in_context)
      : ctx{std::move(in_context)} {}
  std::shared_ptr<context<RQty, RSty, DEty>> ctx;
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
    wake_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC | EFD_SEMAPHORE);
    if (wake_fd_ == -1) {
      ::close(epoll_fd_);
      epoll_fd_ = -1;
      throw std::runtime_error("Stop event could not be created!");
    }
    listener_fd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                            IPPROTO_TCP);
    if (listener_fd_ == -1) {
      ::close(wake_fd_);
      ::close(epoll_fd_);
      wake_fd_ = -1;
      epoll_fd_ = -1;
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
  void start() {
    thread_ = std::jthread([this]() { run(); });
  }
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
      while (::write(wake_fd_, &wake, sizeof(wake)) == -1 && errno == EINTR) {
      }
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
    pending_contexts_.emplace_back(std::move(ctx));
    uint64_t wake = 1;
    while (::write(wake_fd_, &wake, sizeof(wake)) == -1 && errno == EINTR) {
    }
  }

 private:
  // +=========================================================================+
  // | [>] run                                                     ( private ) |
  // +=========================================================================+
  void run() {
    std::array<epoll_event, 64> events{};
    while (!stopping_.load()) {
      int ready = ::epoll_wait(epoll_fd_, events.data(), events.size(), -1);
      if (ready == -1) {
        if (errno == EINTR) continue;
        return;
      }
      for (int i = 0; i < ready; i++) {
        if (events[i].data.u64 == kWakeEventId) {
          handle_wake();
          continue;
        }
        if (events[i].data.u64 == kListenerEventId) {
          handle_listener();
          continue;
        }
        epoll_registration<RQty, RSty, DEty>* registration =
            static_cast<epoll_registration<RQty, RSty, DEty>*>(
                events[i].data.ptr);
        auto itr = registrations_.find(registration);
        if (itr == registrations_.end()) continue;
        handle_context(itr->second->ctx, events[i].events);
      }
      retired_registrations_.clear();
    }
  }
  // +=========================================================================+
  // | [>] handle_wake                                             ( private ) |
  // +=========================================================================+
  void handle_wake() {
    uint64_t wake = 0;
    while (::read(wake_fd_, &wake, sizeof(wake)) != -1 || errno == EINTR) {
      if (stopping_.load()) return;
    }
    std::vector<std::shared_ptr<context<RQty, RSty, DEty>>> contexts;
    {
      std::lock_guard<std::mutex> pending_lock(pending_mutex_);
      contexts.swap(pending_contexts_);
    }
    for (const auto& ctx : contexts) {
      if (ctx->is_closing()) continue;
      if (!ctx->flush_send()) {
        close_context(ctx);
      } else {
        rearm_context(ctx);
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
        if (errno == EINTR) continue;
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
    std::unique_ptr<epoll_registration<RQty, RSty, DEty>> registration =
        std::make_unique<epoll_registration<RQty, RSty, DEty>>(ctx);
    ctx->registration = registration.get();
    try {
      registrations_.emplace(ctx->registration, std::move(registration));
    } catch (...) {
      int failed_socket = ctx->mark_context_for_closing();
      if (failed_socket != -1) ::close(failed_socket);
      throw;
    }
    epoll_event event{};
    event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
    event.data.ptr = ctx->registration;
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket, &event) == -1) {
      registrations_.erase(ctx->registration);
      int failed_socket = ctx->mark_context_for_closing();
      if (failed_socket != -1) ::close(failed_socket);
      return;
    }
    try {
      on_connection_();
    } catch (...) {
      close_context(ctx);
    }
  }
  // +=========================================================================+
  // | [>] handle_context                                          ( private ) |
  // +=========================================================================+
  void handle_context(std::shared_ptr<context<RQty, RSty, DEty>> ctx,
                      uint32_t events) {
    if (ctx->is_closing()) return;
    if (events & EPOLLERR) {
      close_context(ctx);
      return;
    }
    if (events & (EPOLLIN | EPOLLRDHUP | EPOLLHUP)) handle_receive(ctx);
    if (ctx->is_closing()) return;
    if (!ctx->flush_send()) {
      close_context(ctx);
      return;
    }
    rearm_context(ctx);
  }
  // +=========================================================================+
  // | [>] handle_receive                                          ( private ) |
  // +=========================================================================+
  void handle_receive(std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    while (ctx->can_receive()) {
      ssize_t received = ctx->receive();
      if (received > 0) {
        consume_received(ctx, ctx->get_receive_buffer(),
                         static_cast<std::size_t>(received));
        continue;
      }
      if (received == 0) {
        ctx->close();
        return;
      }
      if (errno == EINTR) continue;
      if (errno == EAGAIN || errno == EWOULDBLOCK) return;
      ctx->close();
      return;
    }
  }
  // +=========================================================================+
  // | [>] consume_received                                        ( private ) |
  // +=========================================================================+
  void consume_received(std::shared_ptr<context<RQty, RSty, DEty>> ctx,
                        char* buffer, std::size_t size) {
    std::size_t consumed = 0;
    do {
      // Let's accumulate the received bytes into the decoder!
      std::size_t accepted =
          ctx->accumulate(buffer + consumed, size - consumed);
      if (accepted == 0) {
        // The decoder is full: let's drain the pending requests and retry!
        handle_deserialized_requests(ctx);
        if (!ctx->can_receive()) return;
        accepted = ctx->accumulate(buffer + consumed, size - consumed);
      }
      if (accepted == 0 || accepted > (size - consumed)) {
        enqueue_error_response(ctx, "Invalid request content!");
        return;
      }
      // Let's try to deserialize some requests!
      handle_deserialized_requests(ctx);
      consumed += accepted;
    } while (ctx->can_receive() && consumed < size);
  }
  // +=========================================================================+
  // | [>] handle_deserialized_requests                            ( private ) |
  // +=========================================================================+
  void handle_deserialized_requests(
      std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    while (ctx->can_receive()) {
      protocol::deserialization_result<RQty> result = ctx->deserialize();
      if (result.code == protocol::deserialization_status::kMoreBytesNeeded) {
        return;
      }
      if (result.code == protocol::deserialization_status::kInvalidSource) {
        enqueue_error_response(ctx, "Invalid request content!");
        return;
      }
      if (result.request == nullptr) {
        enqueue_error_response(ctx, "Decoder error!");
        return;
      }
      try {
        std::thread::id tid = std::this_thread::get_id();
        uint64_t id = ctx->get_next_response_id();
        // When the protocol requests channel closing, this response becomes
        // the last one to be sent and no more requests must be decoded!
        bool close_after_this =
            result.channel == protocol::channel_intent::kClose;
        if (close_after_this) ctx->set_closing_rid(id);
        on_request_(
            result.request, std::make_shared<RSty>(),
            [ctx, id, tid](std::shared_ptr<RSty> response) {
              if (response) {
                try {
                  std::unique_ptr<protocol::serialization_result> serialized =
                      response->serialize();
                  // Duplicated identifiers are dropped downstream, so
                  // a repeated completion is harmless here!
                  if (serialized) {
                    ctx->enqueue_response(std::move(serialized), id);
                  }
                } catch (...) {
                  ctx->close();
                }
              }
              if (std::this_thread::get_id() != tid) {
                std::shared_ptr<worker<RQty, RSty, DEty>> owner =
                    ctx->owner.lock();
                if (owner) notify_worker(std::move(owner), ctx);
              }
            });
        if (close_after_this) return;
      } catch (...) {
        ctx->close();
        return;
      }
    }
  }
  // +=========================================================================+
  // | [>] enqueue_error_response                                  ( private ) |
  // +=========================================================================+
  void enqueue_error_response(std::shared_ptr<context<RQty, RSty, DEty>> ctx,
                              std::string_view reason) {
    try {
      std::shared_ptr<RSty> response = std::make_shared<RSty>();
      on_bad_request_(reason, response);
      ctx->enqueue_error_response(response);
    } catch (...) {
      ctx->close();
    }
  }
  // +=========================================================================+
  // | [>] rearm_context                                           ( private ) |
  // +=========================================================================+
  void rearm_context(std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    if (ctx->should_close()) {
      close_context(ctx);
      return;
    }
    uint32_t events = EPOLLRDHUP | EPOLLET;
    if (ctx->can_receive()) events |= EPOLLIN;
    if (ctx->has_sendable_response()) events |= EPOLLOUT;
    epoll_event event{};
    event.events = events;
    event.data.ptr = ctx->registration;
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, ctx->get_socket(), &event) ==
        -1) {
      close_context(ctx);
    }
  }
  // +=========================================================================+
  // | [>] close_context                                           ( private ) |
  // +=========================================================================+
  void close_context(std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    epoll_registration<RQty, RSty, DEty>* registration = ctx->registration;
    int socket = ctx->mark_context_for_closing();
    if (socket == -1) return;
    ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket, nullptr);
    ::close(socket);
    auto itr = registrations_.find(registration);
    if (itr != registrations_.end()) {
      retired_registrations_.emplace_back(std::move(itr->second));
      registrations_.erase(itr);
    }
    ctx->notify_disconnection();
  }
  // +=========================================================================+
  // | [>] close_contexts                                          ( private ) |
  // +=========================================================================+
  void close_contexts() {
    for (auto& [registration, entry] : registrations_) {
      int socket = entry->ctx->mark_context_for_closing();
      if (socket != -1) {
        ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket, nullptr);
        ::close(socket);
        entry->ctx->notify_disconnection();
      }
    }
    registrations_.clear();
    retired_registrations_.clear();
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
  std::unordered_map<epoll_registration<RQty, RSty, DEty>*,
                     std::unique_ptr<epoll_registration<RQty, RSty, DEty>>>
      registrations_;
  std::vector<std::unique_ptr<epoll_registration<RQty, RSty, DEty>>>
      retired_registrations_;
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
// | [>] notify_worker                                            ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty,
          template <typename, typename> class DEty>
void notify_worker(std::shared_ptr<worker<RQty, RSty, DEty>> owner,
                   std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
  owner->notify(std::move(ctx));
}
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
    std::unique_lock<std::mutex> lifecycle_lock(lifecycle_mutex_);
    if (!workers_.empty() || stopping_) return;
    uint16_t port_number = get_port(port);
    std::size_t number_of_workers =
        std::max<std::size_t>(1, std::thread::hardware_concurrency());
    try {
      for (std::size_t i = 0; i < number_of_workers; i++) {
        std::shared_ptr<worker<RQty, RSty, DEty>> entry =
            std::make_shared<worker<RQty, RSty, DEty>>(
                on_request_, on_bad_request_, on_connection_,
                on_disconnection_);
        entry->setup(port_number);
        workers_.emplace_back(std::move(entry));
      }
      for (const auto& worker : workers_) worker->start();
    } catch (...) {
      stopping_ = true;
      lifecycle_lock.unlock();
      try {
        for (const auto& worker : workers_) worker->stop();
      } catch (...) {
        lifecycle_lock.lock();
        stopping_ = false;
        lifecycle_lock.unlock();
        lifecycle_condition_.notify_all();
        throw;
      }
      lifecycle_lock.lock();
      workers_.clear();
      stopping_ = false;
      lifecycle_lock.unlock();
      lifecycle_condition_.notify_all();
      throw;
    }
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    std::unique_lock<std::mutex> lifecycle_lock(lifecycle_mutex_);
    for (const auto& worker : workers_) {
      if (worker->is_current_thread()) {
        throw std::runtime_error("Transport cannot be stopped from a worker!");
      }
    }
    if (stopping_) {
      lifecycle_condition_.wait(lifecycle_lock,
                                [this]() { return !stopping_; });
    }
    if (workers_.empty()) return;
    stopping_ = true;
    lifecycle_lock.unlock();
    try {
      for (const auto& worker : workers_) worker->stop();
    } catch (...) {
      lifecycle_lock.lock();
      stopping_ = false;
      lifecycle_lock.unlock();
      lifecycle_condition_.notify_all();
      throw;
    }
    lifecycle_lock.lock();
    workers_.clear();
    stopping_ = false;
    lifecycle_lock.unlock();
    lifecycle_condition_.notify_all();
  }
  // +=========================================================================+
  // | [>] set_on_request                                           ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_request(FNty&& fn) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    on_request_ = std::forward<FNty>(fn);
  }
  // +=========================================================================+
  // | [>] set_on_bad_request                                       ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_bad_request(FNty&& fn) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    on_bad_request_ = std::forward<FNty>(fn);
  }
  // +=========================================================================+
  // | [>] set_on_connection                                        ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_connection(FNty&& fn) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    on_connection_ = std::forward<FNty>(fn);
  }
  // +=========================================================================+
  // | [>] set_on_disconnection                                     ( public ) |
  // +=========================================================================+
  template <typename FNty>
  void set_on_disconnection(FNty&& fn) {
    std::lock_guard<std::mutex> lifecycle_lock(lifecycle_mutex_);
    on_disconnection_ = std::forward<FNty>(fn);
  }

 private:
  // +=========================================================================+
  // | [>] get_port                                                ( private ) |
  // +=========================================================================+
  static uint16_t get_port(const char port[]) {
    if (port == nullptr || *port == '\0') {
      throw std::runtime_error("Invalid port (range from 1 to 65535)!");
    }
    int port_number = 0;
    const char* const port_end = port + std::strlen(port);
    const auto [parsed_at, parse_error] =
        std::from_chars(port, port_end, port_number);
    if (parse_error != std::errc{} || parsed_at != port_end ||
        port_number < 1 || port_number > 65535) {
      throw std::runtime_error("Invalid port (range from 1 to 65535)!");
    }
    return static_cast<uint16_t>(port_number);
  }
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +=========================================================================+
  std::vector<std::shared_ptr<worker<RQty, RSty, DEty>>> workers_;
  std::mutex lifecycle_mutex_;
  std::condition_variable lifecycle_condition_;
  bool stopping_{false};
  types::on_request_delegate<RQty, RSty> on_request_;
  types::on_bad_request_delegate<RSty> on_bad_request_;
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
};
}  // namespace martianlabs::doba::transport::server

#endif
