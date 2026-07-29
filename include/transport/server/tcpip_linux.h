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
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <sys/eventfd.h>
#include <unordered_map>

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
struct epoll_registration;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] response_data                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct response_data {
  uint64_t id{0};
  std::unique_ptr<protocol::serialization_result> response;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] context [linux]                                            ( struct ) |
// +---------------------------------------------------------------------------+
// | This specification holds for the Linux server transport context.         |
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
  context(int in_socket, uint64_t in_id) : socket{in_socket}, id{in_id} {}
  context(const context&) = delete;
  context(context&&) noexcept = delete;
  ~context() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  context& operator=(const context&) = delete;
  context& operator=(context&&) noexcept = delete;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  // [common] section!
  bool closing{false};
  bool close_requested{false};
  bool processing{false};
  bool read_closed{false};
  int socket{-1};
  epoll_registration<RQty, RSty, DEty>* registration{nullptr};
  uint64_t id{0};
  // [decoder] section!
  DEty<RQty, RSty> decoder{};
  // [responses] section!
  uint64_t next_request_id{0};
  std::vector<response_data> responses;
  uint64_t expected_response_id{0};
  std::size_t pending_responses{0};
  bool close_after_sending{false};
  uint64_t close_after_response_id{0};
  std::string sending_buffer;
  std::size_t sending_offset{0};
  std::mutex sending_mutex;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] epoll_registration                                      ( struct ) |
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
// | [>] tcpip [linux]                                              ( class ) |
// +---------------------------------------------------------------------------+
// | This specification holds for the Linux server transport.                 |
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
  // | [>] PROPERTIEs                                               ( public ) |
  // +=========================================================================+
  types::on_request_delegate<RQty, RSty> on_request;
  types::on_bad_request_delegate<RSty> on_bad_request;
  types::on_client_connected_delegate on_connection;
  types::on_client_disconnected_delegate on_disconnection;
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +=========================================================================+
  void start(const char port[]) {
    if (epoll_fd_.load() != -1) return;
    try {
      setup_workers(setup_listener(port));
    } catch (...) {
      stop();
      throw;
    }
  }

  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() {
    int epoll_fd = epoll_fd_.load();
    if (epoll_fd == -1) return;
    if (!stopping_.exchange(true)) {
      int listener_fd = listener_fd_.exchange(-1);
      if (listener_fd != -1) {
        ::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, listener_fd, nullptr);
        ::close(listener_fd);
      }
      uint64_t workers = workers_.size();
      if (workers > 0) {
        while (::write(wake_fd_.load(), &workers, sizeof(workers)) == -1 &&
               errno == EINTR) {
        }
      }
    }
    workers_.clear();
    close_contexts();
    int wake_fd = wake_fd_.exchange(-1);
    if (wake_fd != -1) ::close(wake_fd);
    epoll_fd = epoll_fd_.exchange(-1);
    if (epoll_fd != -1) ::close(epoll_fd);
    stopping_.store(false);
  }

  // +=========================================================================+
  // | [>] is_running                                               ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool is_running() const { return epoll_fd_.load() != -1; }

 private:
  // +=========================================================================+
  // | [>] setup_listener                                          ( private ) |
  // +=========================================================================+
  std::size_t setup_listener(const char port[]) {
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
    int epoll_fd = ::epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1) {
      throw std::runtime_error("Epoll instance could not be created!");
    }
    int wake_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC | EFD_SEMAPHORE);
    if (wake_fd == -1) {
      ::close(epoll_fd);
      throw std::runtime_error("Stop event could not be created!");
    }
    int listener_fd =
        ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                 IPPROTO_TCP);
    if (listener_fd == -1) {
      ::close(wake_fd);
      ::close(epoll_fd);
      throw std::runtime_error("Socket could not be created!");
    }
    int reuse_address = 1;
    if (::setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_address,
                     sizeof(reuse_address)) == -1) {
      ::close(listener_fd);
      ::close(wake_fd);
      ::close(epoll_fd);
      throw std::runtime_error("Listener socket could not be configured!");
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<uint16_t>(port_number));
    if (::bind(listener_fd, reinterpret_cast<const sockaddr*>(&address),
               sizeof(address)) == -1 ||
        ::listen(listener_fd, SOMAXCONN) == -1) {
      ::close(listener_fd);
      ::close(wake_fd);
      ::close(epoll_fd);
      throw std::runtime_error("Listener socket could not be started!");
    }
    epoll_event wake_event{};
    wake_event.events = EPOLLIN;
    wake_event.data.u64 = kWakeEventId;
    epoll_event listener_event{};
    listener_event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    listener_event.data.u64 = kListenerEventId;
    if (::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, wake_fd, &wake_event) == -1 ||
        ::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listener_fd, &listener_event) ==
            -1) {
      ::close(listener_fd);
      ::close(wake_fd);
      ::close(epoll_fd);
      throw std::runtime_error("Listener could not be registered!");
    }
    stopping_.store(false);
    wake_fd_.store(wake_fd);
    listener_fd_.store(listener_fd);
    epoll_fd_.store(epoll_fd);
    return std::max<std::size_t>(1, std::thread::hardware_concurrency());
  }

  // +=========================================================================+
  // | [>] setup_workers                                           ( private ) |
  // +=========================================================================+
  void setup_workers(std::size_t number_of_workers) {
    for (std::size_t i = 0; i < number_of_workers; i++) {
      workers_.emplace_back(std::jthread([this]() { worker_loop(); }));
    }
  }

  // +=========================================================================+
  // | [>] worker_loop                                             ( private ) |
  // +=========================================================================+
  void worker_loop() {
    std::array<epoll_event, 64> events{};
    while (!stopping_.load()) {
      const int epoll_fd = epoll_fd_.load();
      if (epoll_fd == -1) return;
      const int ready =
          ::epoll_wait(epoll_fd, events.data(), events.size(), -1);
      if (ready == -1) {
        if (errno == EINTR) continue;
        return;
      }
      for (int i = 0; i < ready; i++) {
        const uint64_t event_id = events[i].data.u64;
        if (event_id == kWakeEventId) {
          handle_wake();
          if (stopping_.load()) return;
        } else if (event_id == kListenerEventId) {
          handle_listener();
        } else {
          epoll_registration<RQty, RSty, DEty>* registration =
              static_cast<epoll_registration<RQty, RSty, DEty>*>(
                  events[i].data.ptr);
          std::shared_ptr<context<RQty, RSty, DEty>> ctx = registration->ctx;
          handle_context(ctx, events[i].events);
        }
      }
    }
  }

  // +=========================================================================+
  // | [>] handle_wake                                             ( private ) |
  // +=========================================================================+
  void handle_wake() {
    uint64_t value = 0;
    while (::read(wake_fd_.load(), &value, sizeof(value)) == -1 &&
           errno == EINTR) {
    }
  }

  // +=========================================================================+
  // | [>] handle_listener                                         ( private ) |
  // +=========================================================================+
  void handle_listener() {
    const int listener_fd = listener_fd_.load();
    while (!stopping_.load() && listener_fd != -1) {
      int socket = ::accept4(listener_fd, nullptr, nullptr,
                             SOCK_NONBLOCK | SOCK_CLOEXEC);
      if (socket == -1) {
        if (errno == EINTR) continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
        break;
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
    rearm_listener();
  }

  // +=========================================================================+
  // | [>] register_context                                        ( private ) |
  // +=========================================================================+
  void register_context(int socket) {
    const uint64_t id = next_context_id_.fetch_add(1);
    std::shared_ptr<context<RQty, RSty, DEty>> ctx =
        std::make_shared<context<RQty, RSty, DEty>>(socket, id);
    std::unique_ptr<epoll_registration<RQty, RSty, DEty>> registration =
        std::make_unique<epoll_registration<RQty, RSty, DEty>>(ctx);
    ctx->registration = registration.get();
    {
      std::lock_guard<std::mutex> registrations_lock(registrations_mutex_);
      registrations_.emplace(id, std::move(registration));
    }
    try {
      on_connection();
    } catch (...) {
      discard_context(ctx);
      return;
    }
    epoll_event event{};
    event.events = EPOLLIN | EPOLLRDHUP | EPOLLET | EPOLLONESHOT;
    event.data.ptr = ctx->registration;
    if (::epoll_ctl(epoll_fd_.load(), EPOLL_CTL_ADD, socket, &event) == -1) {
      discard_context(ctx);
      try {
        on_disconnection();
      } catch (...) {
      }
    }
  }

  // +=========================================================================+
  // | [>] handle_context                                          ( private ) |
  // +=========================================================================+
  void handle_context(std::shared_ptr<context<RQty, RSty, DEty>> ctx,
                      uint32_t events) {
    bool close_context = false;
    {
      std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
      if (ctx->closing || ctx->processing) return;
      ctx->processing = true;
      close_context = ctx->close_requested || events & EPOLLERR;
    }
    if (close_context) {
      {
        std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
        ctx->processing = false;
      }
      mark_context_for_closing(ctx);
    } else {
      if (events & (EPOLLIN | EPOLLRDHUP | EPOLLHUP)) handle_receive(ctx);
      if (!ctx->closing &&
          ((events & EPOLLOUT) || has_sendable_response(ctx))) {
        flush_send(ctx);
      }
      finish_context(ctx);
    }
  }

  // +=========================================================================+
  // | [>] handle_receive                                          ( private ) |
  // +=========================================================================+
  void handle_receive(std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    std::array<char, kReceiveBufferSz> buffer{};
    const int socket = ctx->socket;
    while (!ctx->closing && !ctx->read_closed) {
      const ssize_t received =
          ::recv(socket, buffer.data(), buffer.size(), MSG_DONTWAIT);
      if (received > 0) {
        if (!consume_received(ctx, buffer.data(),
                              static_cast<std::size_t>(received))) {
          return;
        }
        continue;
      }
      if (received == 0) {
        set_context_read_closed(ctx);
        return;
      }
      if (errno == EINTR) continue;
      if (errno == EAGAIN || errno == EWOULDBLOCK) return;
      mark_context_for_closing(ctx);
      return;
    }
  }

  // +=========================================================================+
  // | [>] consume_received                                        ( private ) |
  // +=========================================================================+
  bool consume_received(std::shared_ptr<context<RQty, RSty, DEty>> ctx,
                        char* buffer, std::size_t size) {
    std::size_t consumed = 0;
    while (consumed < size) {
      std::size_t accepted =
          ctx->decoder.accumulate(buffer + consumed, size - consumed);
      if (accepted == 0) {
        if (!handle_deserialized_requests(ctx)) return false;
        accepted = ctx->decoder.accumulate(buffer + consumed, size - consumed);
        if (accepted == 0) {
          queue_error_and_close(ctx, "Invalid source deserialization content!");
          return false;
        }
      }
      consumed += accepted;
      if (!handle_deserialized_requests(ctx)) return false;
    }
    return true;
  }

  // +=========================================================================+
  // | [>] handle_deserialized_requests                            ( private ) |
  // +=========================================================================+
  bool handle_deserialized_requests(
      std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    while (!ctx->closing) {
      protocol::deserialization_result<RQty> result =
          ctx->decoder.deserialize();
      if (result.code == protocol::deserialization_status::kMoreBytesNeeded) {
        return true;
      }
      if (result.code == protocol::deserialization_status::kInvalidSource ||
          result.request == nullptr) {
        queue_error_and_close(ctx, "Invalid source deserialization content!");
        return false;
      }
      const uint64_t response_id = ctx->next_request_id++;
      if (result.channel == protocol::channel_intent::kUpgrade) {
        mark_context_for_closing(ctx);
        return false;
      }
      const bool close_after_sending =
          result.channel == protocol::channel_intent::kClose;
      if (close_after_sending) {
        set_close_after_response(ctx, response_id);
        set_context_read_closed(ctx);
      }
      std::shared_ptr<RSty> response = std::make_shared<RSty>();
      std::shared_ptr<std::atomic<bool>> response_sent =
          std::make_shared<std::atomic<bool>>(false);
      {
        std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
        ctx->pending_responses++;
      }
      try {
        on_request(result.request, response,
                   [this, ctx, response_id, response_sent](
                       std::shared_ptr<RSty> response) {
                      if (response_sent->exchange(true)) return;
                      if (response == nullptr) {
                        request_context_closing(ctx);
                        return;
                     }
                     try {
                       add_response_to_queue(ctx, response_id,
                                             std::move(response->serialize()),
                                             true);
                      } catch (...) {
                        request_context_closing(ctx);
                     }
                   });
      } catch (...) {
        mark_context_for_closing(ctx);
        return false;
      }
      if (close_after_sending) return false;
    }
    return false;
  }

  // +=========================================================================+
  // | [>] queue_error_and_close                                  ( private ) |
  // +=========================================================================+
  void queue_error_and_close(std::shared_ptr<context<RQty, RSty, DEty>> ctx,
                             std::string_view reason) {
    const uint64_t response_id = ctx->next_request_id++;
    set_close_after_response(ctx, response_id);
    set_context_read_closed(ctx);
    std::shared_ptr<RSty> response = std::make_shared<RSty>();
    try {
      on_bad_request(reason, response);
      add_response_to_queue(ctx, response_id, std::move(response->serialize()),
                            false);
    } catch (...) {
      mark_context_for_closing(ctx);
    }
  }

  // +=========================================================================+
  // | [>] set_close_after_response                               ( private ) |
  // +=========================================================================+
  void set_close_after_response(
      std::shared_ptr<context<RQty, RSty, DEty>> ctx, uint64_t response_id) {
    std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
    ctx->close_after_sending = true;
    ctx->close_after_response_id = response_id;
  }

  // +=========================================================================+
  // | [>] add_response_to_queue                                   ( private ) |
  // +=========================================================================+
  void add_response_to_queue(
      std::shared_ptr<context<RQty, RSty, DEty>> ctx, uint64_t response_id,
      std::unique_ptr<protocol::serialization_result> response,
      bool completes_request) {
    {
      std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
      if (completes_request) {
        if (ctx->pending_responses == 0) return;
        ctx->pending_responses--;
      }
      if (response == nullptr || ctx->closing) return;
      if (ctx->close_after_sending &&
          response_id > ctx->close_after_response_id) {
        return;
      }
      auto itr = std::lower_bound(
          ctx->responses.begin(), ctx->responses.end(), response_id,
          [](const response_data& data, uint64_t id) { return data.id < id; });
      if (itr != ctx->responses.end() && itr->id == response_id) return;
      ctx->responses.insert(itr, {response_id, std::move(response)});
    }
    notify_output(ctx);
  }

  // +=========================================================================+
  // | [>] request_context_closing                                ( private ) |
  // +=========================================================================+
  void request_context_closing(
      std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    {
      std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
      if (ctx->closing) return;
      ctx->close_requested = true;
    }
    notify_output(ctx);
  }

  // +=========================================================================+
  // | [>] notify_output                                           ( private ) |
  // +=========================================================================+
  void notify_output(std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    {
      std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
      if (ctx->closing || ctx->processing) return;
      if (should_close_locked(*ctx)) ctx->close_requested = true;
      rearm_context_locked(*ctx);
    }
  }

  // +=========================================================================+
  // | [>] has_sendable_response                                  ( private ) |
  // +=========================================================================+
  bool has_sendable_response(
      std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
    return has_sendable_response_locked(*ctx);
  }

  // +=========================================================================+
  // | [>] has_sendable_response_locked                            ( private ) |
  // +=========================================================================+
  static bool has_sendable_response_locked(context<RQty, RSty, DEty>& ctx) {
    if (ctx.sending_offset < ctx.sending_buffer.size()) return true;
    return std::any_of(ctx.responses.begin(), ctx.responses.end(),
                       [&ctx](const response_data& response) {
                         return response.id == ctx.expected_response_id;
                       });
  }

  // +=========================================================================+
  // | [>] flush_send                                              ( private ) |
  // +=========================================================================+
  void flush_send(std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    while (!ctx->closing) {
      const char* data = nullptr;
      std::size_t size = 0;
      bool close_context = false;
      int socket = -1;
      {
        std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
        if (ctx->sending_offset == ctx->sending_buffer.size()) {
          ctx->sending_buffer.clear();
          ctx->sending_offset = 0;
          append_sendable_responses_locked(*ctx);
        }
        if (ctx->sending_buffer.empty()) {
          close_context = should_close_locked(*ctx);
        } else {
          data = ctx->sending_buffer.data() + ctx->sending_offset;
          size = ctx->sending_buffer.size() - ctx->sending_offset;
          socket = ctx->socket;
        }
      }
      if (close_context) {
        mark_context_for_closing(ctx);
        return;
      }
      if (data == nullptr) return;
      const ssize_t sent = ::send(socket, data, size, MSG_NOSIGNAL);
      if (sent > 0) {
        std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
        ctx->sending_offset += static_cast<std::size_t>(sent);
        continue;
      }
      if (sent == -1 && errno == EINTR) continue;
      if (sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) return;
      mark_context_for_closing(ctx);
      return;
    }
  }

  // +=========================================================================+
  // | [>] append_sendable_responses_locked                        ( private ) |
  // +=========================================================================+
  static void append_sendable_responses_locked(context<RQty, RSty, DEty>& ctx) {
    while (true) {
      auto itr = std::find_if(
          ctx.responses.begin(), ctx.responses.end(),
          [&ctx](const response_data& data) {
            return data.id == ctx.expected_response_id;
          });
      if (itr == ctx.responses.end()) return;
      ctx.sending_buffer.append(itr->response->prefix);
      ctx.expected_response_id++;
      ctx.responses.erase(itr);
    }
  }

  // +=========================================================================+
  // | [>] should_close_locked                                    ( private ) |
  // +=========================================================================+
  static bool should_close_locked(context<RQty, RSty, DEty>& ctx) {
    if (ctx.sending_offset < ctx.sending_buffer.size()) return false;
    if (ctx.close_after_sending &&
        ctx.expected_response_id > ctx.close_after_response_id) {
      return true;
    }
    return ctx.read_closed && ctx.pending_responses == 0 &&
           ctx.responses.empty();
  }

  // +=========================================================================+
  // | [>] rearm_listener                                          ( private ) |
  // +=========================================================================+
  void rearm_listener() {
    const int epoll_fd = epoll_fd_.load();
    const int listener_fd = listener_fd_.load();
    if (stopping_.load() || epoll_fd == -1 || listener_fd == -1) return;
    epoll_event event{};
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    event.data.u64 = kListenerEventId;
    ::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, listener_fd, &event);
  }

  // +=========================================================================+
  // | [>] set_context_read_closed                                ( private ) |
  // +=========================================================================+
  void set_context_read_closed(
      std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
    ctx->read_closed = true;
  }

  // +=========================================================================+
  // | [>] finish_context                                          ( private ) |
  // +=========================================================================+
  void finish_context(std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    bool close_context = false;
    {
      std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
      if (!ctx->closing) {
        close_context =
            ctx->close_requested || should_close_locked(*ctx) ||
            !rearm_context_locked(*ctx);
      }
      ctx->processing = false;
    }
    if (close_context) mark_context_for_closing(ctx);
  }

  // +=========================================================================+
  // | [>] rearm_context_locked                                    ( private ) |
  // +=========================================================================+
  bool rearm_context_locked(context<RQty, RSty, DEty>& ctx) {
    if (ctx.closing) return true;
    uint32_t events = EPOLLONESHOT | EPOLLRDHUP | EPOLLET;
    if (ctx.close_requested) {
      events |= EPOLLOUT;
    } else if (!ctx.read_closed) {
      events |= EPOLLIN;
    }
    if (has_sendable_response_locked(ctx)) events |= EPOLLOUT;
    if (!(events & (EPOLLIN | EPOLLOUT))) return true;
    if (ctx.registration == nullptr || ctx.socket == -1) return false;
    epoll_event event{};
    event.events = events;
    event.data.ptr = ctx.registration;
    return ::epoll_ctl(epoll_fd_.load(), EPOLL_CTL_MOD, ctx.socket, &event) !=
           -1;
  }

  // +=========================================================================+
  // | [>] discard_context                                         ( private ) |
  // +=========================================================================+
  void discard_context(std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    int socket = -1;
    {
      std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
      ctx->closing = true;
      ctx->registration = nullptr;
      socket = ctx->socket;
      ctx->socket = -1;
    }
    if (socket != -1) ::close(socket);
    std::lock_guard<std::mutex> registrations_lock(registrations_mutex_);
    registrations_.erase(ctx->id);
  }

  // +=========================================================================+
  // | [>] mark_context_for_closing                                ( private ) |
  // +=========================================================================+
  void mark_context_for_closing(
      std::shared_ptr<context<RQty, RSty, DEty>> ctx) {
    int socket = -1;
    {
      std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
      if (ctx->closing) return;
      ctx->closing = true;
      ctx->registration = nullptr;
      socket = ctx->socket;
      ctx->socket = -1;
    }
    if (socket != -1) {
      ::epoll_ctl(epoll_fd_.load(), EPOLL_CTL_DEL, socket, nullptr);
      ::close(socket);
    }
    {
      std::lock_guard<std::mutex> registrations_lock(registrations_mutex_);
      registrations_.erase(ctx->id);
    }
    try {
      on_disconnection();
    } catch (...) {
    }
  }

  // +=========================================================================+
  // | [>] close_contexts                                          ( private ) |
  // +=========================================================================+
  void close_contexts() {
    std::vector<std::unique_ptr<epoll_registration<RQty, RSty, DEty>>>
        registrations;
    {
      std::lock_guard<std::mutex> registrations_lock(registrations_mutex_);
      registrations.reserve(registrations_.size());
      for (auto& [id, registration] : registrations_) {
        registrations.push_back(std::move(registration));
      }
      registrations_.clear();
    }
    for (const auto& registration : registrations) {
      const std::shared_ptr<context<RQty, RSty, DEty>>& ctx =
          registration->ctx;
      int socket = -1;
      {
        std::lock_guard<std::mutex> sending_lock(ctx->sending_mutex);
        if (ctx->closing) continue;
        ctx->closing = true;
        ctx->registration = nullptr;
        socket = ctx->socket;
        ctx->socket = -1;
      }
      if (socket != -1) {
        ::epoll_ctl(epoll_fd_.load(), EPOLL_CTL_DEL, socket, nullptr);
        ::close(socket);
      }
      try {
        on_disconnection();
      } catch (...) {
      }
    }
  }

  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::atomic<int> epoll_fd_{-1};
  std::atomic<int> listener_fd_{-1};
  std::atomic<int> wake_fd_{-1};
  std::atomic<bool> stopping_{false};
  std::atomic<uint64_t> next_context_id_{1};
  std::unordered_map<
      uint64_t, std::unique_ptr<epoll_registration<RQty, RSty, DEty>>>
      registrations_;
  std::mutex registrations_mutex_;
  std::vector<std::jthread> workers_;
};
}  // namespace martianlabs::doba::transport::server
#endif
