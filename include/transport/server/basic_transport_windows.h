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

#ifndef martianlabs_doba_transport_server_basic_transport_windows_h
#define martianlabs_doba_transport_server_basic_transport_windows_h

#include <algorithm>
#include <array>
#include <atomic>
#include <charconv>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <new>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>

#include "network/environment.h"
#include "platform.h"
#include "transport/server/tcp_connection.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] CONSTANTs                                                  ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
static constexpr DWORD kAcceptAddressBytes =
    static_cast<DWORD>(sizeof(sockaddr_storage) + 16);

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] io_type                                                ( enum-class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class io_type : uint8_t { kAccept, kSend, kReceive, kOutput };

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] FORWARDs                                                   ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty, typename CNty>
struct context;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] overlapped_base                                            ( struct ) |
// +---------------------------------------------------------------------------+
// | Base state for Windows overlapped I/O.                                    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct overlapped_base : OVERLAPPED {
  overlapped_base(io_type in_type) : OVERLAPPED{}, type{in_type} {}
  io_type get_type() const { return type; }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  const io_type type;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] overlapped_accept                                          ( struct ) |
// +---------------------------------------------------------------------------+
// | Windows overlapped accept operation.                                      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct overlapped_accept : overlapped_base {
  overlapped_accept(SOCKET in_socket)
      : overlapped_base(io_type::kAccept), socket{in_socket} {}
  SOCKET socket{INVALID_SOCKET};
  CHAR addresses[(kAcceptAddressBytes * 2)]{0};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] overlapped_receive                                         ( struct ) |
// +---------------------------------------------------------------------------+
// | Windows overlapped receive operation.                                     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty, typename CNty>
struct overlapped_receive : overlapped_base {
  overlapped_receive(std::shared_ptr<context<ENty, CNty>> context)
      : overlapped_base(io_type::kReceive), ctx{context} {}
  std::shared_ptr<context<ENty, CNty>> ctx;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] overlapped_send                                            ( struct ) |
// +---------------------------------------------------------------------------+
// | Windows overlapped send operation.                                        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty, typename CNty>
struct overlapped_send : overlapped_base {
  overlapped_send(std::shared_ptr<context<ENty, CNty>> context,
                  io_type type = io_type::kSend)
      : overlapped_base(type), ctx{context} {}
  std::shared_ptr<context<ENty, CNty>> ctx;
  std::array<WSABUF, 1> buffers{};
  DWORD buffer_count{0};
  std::size_t submitted_size{0};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] context [windowsTM]                                        ( struct ) |
// +---------------------------------------------------------------------------+
// | Windows server transport connection state.                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty, typename CNty>
struct context : public std::enable_shared_from_this<context<ENty, CNty>> {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  template <protocol::contracts::engine_factory<ENty> FAty>
  context(SOCKET in_socket, std::size_t recv_buffer_size,
          std::size_t send_buffer_size, const FAty& create_engine,
          const std::atomic<bool>& stopping,
          types::on_client_connected_delegate on_connection,
          types::on_client_disconnected_delegate on_disconnection,
          std::function<void(context*)> on_retirement,
          typename CNty::shared_state shared_state)
      : socket_{in_socket},
        on_connection_{on_connection},
        on_disconnection_{on_disconnection},
        on_retirement_{on_retirement},
        input_{recv_buffer_size, send_buffer_size, create_engine,
               shared_state},
        stopping_{stopping},
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
  // | [>] send_completed                                           ( public ) |
  // +-------------------------------------------------------------------------+
  void send_completed(std::size_t size, std::size_t submitted_size) {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    send_state_ = send_status::kIdle;
    if (socket_ == INVALID_SOCKET) {
      retire_();
      return;
    }
    if (!size || size > submitted_size ||
        !input_.output_sent(output_, size)) {
      abort_();
      return;
    }
    send_pending_();
  }
  // +=========================================================================+
  // | [>] output_ready                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void output_ready() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    send_state_ = send_status::kIdle;
    send_pending_();
  }
  // +=========================================================================+
  // | [>] send                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void send(std::string_view head, std::string_view body,
            std::unique_ptr<common::reader> source) {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    if (closing_ || socket_ == INVALID_SOCKET) return;
    if (head.empty() && body.empty() && !source) return;
    if (!output_.push(head, body, std::move(source))) {
      closing_ = true;
      aborted_ = true;
      ::shutdown(socket_, SD_BOTH);
      request_output_();
      return;
    }
    request_output_();
  }
  // +=========================================================================+
  // | [>] arm_next_receive_operation                               ( public ) |
  // +-------------------------------------------------------------------------+
  bool arm_next_receive_operation() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    processing_receive_ = false;
    if (stopping_.load()) {
      closing_ = true;
      if (!input_.close_output()) abort_();
    }
    send_pending_();
    if (closing_ || socket_ == INVALID_SOCKET) {
      cleanup_resources_();
      return false;
    }
    if (input_.size == input_.capacity || !receive_()) {
      abort_();
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] receive_completed                                        ( public ) |
  // +-------------------------------------------------------------------------+
  bool receive_completed(std::size_t size) {
    std::lock_guard<std::mutex> receive_lock(receive_mutex_);
    {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      receiving_ = false;
      if (!size) receive_closed_ = true;
      if (stopping_.load()) {
        closing_ = true;
        if (!input_.close_output()) abort_();
      }
      if (aborted_) abort_();
      if (socket_ == INVALID_SOCKET) {
        retire_();
        return false;
      }
      if (closing_) {
        cleanup_resources_();
        return false;
      }
      processing_receive_ = size != 0;
    }
    if (!size) return true;
    input_.size += size;
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
      if (!input_.consume(processed) || input_.size == input_.capacity) {
        processing_receive_ = false;
        abort_();
        return false;
      }
      if (input_.peer_closed() && !closing_) {
        closing_ = true;
        if (!input_.close_output()) abort_();
      }
    }
    notify_connection();
    return true;
  }
  // +=========================================================================+
  // | [>] receive_failed                                           ( public ) |
  // +-------------------------------------------------------------------------+
  void receive_failed() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    receiving_ = false;
    abort_();
  }
  // +=========================================================================+
  // | [>] send_failed                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void send_failed() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    send_state_ = send_status::kIdle;
    abort_();
  }
  // +=========================================================================+
  // | [>] abort                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void abort() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    abort_();
  }
  // +=========================================================================+
  // | [>] connected                                                ( public ) |
  // +-------------------------------------------------------------------------+
  void connected(HANDLE completion_port) {
    {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      io_h_ = completion_port;
      connected_ = true;
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
  // | [>] close                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void close() {
    std::lock_guard<std::mutex> lock(sending_mutex_);
    if (closing_ || socket_ == INVALID_SOCKET) return;
    closing_ = true;
    if (!input_.close_output()) {
      abort_();
      return;
    }
    request_output_();
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void stop() {
    // Wait for the active handler before closing the connection.
    {
      std::lock_guard<std::mutex> receive_lock(receive_mutex_);
      std::lock_guard<std::mutex> lock(sending_mutex_);
      closing_ = true;
      if (!input_.close_output()) abort_();
      processing_receive_ = false;
      send_pending_();
      cleanup_resources_();
    }
    notify_disconnection();
  }
  // +=========================================================================+
  // | [>] notify_disconnection                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void notify_disconnection() {
    {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      if (!notified_ || disconnected_ || socket_ != INVALID_SOCKET) return;
      disconnected_ = true;
    }
    try {
      // Let's call user's callback to notify for disconnection!
      on_disconnection_();
    } catch (const std::exception&) {
      // [to-do] -> add support for this!
    } catch (...) {
      // [to-do] -> add support for this!
    }
  }
  // +=========================================================================+
  // | [>] notify_connection                                       ( public )  |
  // +-------------------------------------------------------------------------+
  void notify_connection() {
    {
      std::lock_guard<std::mutex> lock(sending_mutex_);
      if (!connected_ || notified_ || !input_.established()) return;
      notified_ = true;
    }
    on_connection_();
  }
  // +=========================================================================+
  // | [>] eof                                                     ( public )  |
  // +-------------------------------------------------------------------------+
  bool eof() const { return input_.eof(); }

 private:
  // +=========================================================================+
  // | [>] receive_                                                ( private ) |
  // +-------------------------------------------------------------------------+
  bool receive_() {
    DWORD f = 0, r = 0;
    overlapped_receive<ENty, CNty>* ovr = new (std::nothrow)
        overlapped_receive<ENty, CNty>(this->shared_from_this());
    if (!ovr) return false;
    ovr_wsa_.buf = input_.buffer.get() + input_.size;
    ovr_wsa_.len = static_cast<ULONG>(input_.capacity - input_.size);
    int res = WSARecv(socket_, &ovr_wsa_, 1, &r, &f, ovr, 0);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      delete ovr;
      return false;
    }
    receiving_ = true;
    return true;
  }
  // +=========================================================================+
  // | [>] request_output_                                         ( private ) |
  // +-------------------------------------------------------------------------+
  void request_output_() {
    if (processing_receive_ || send_state_ != send_status::kIdle) return;
    auto output = new (std::nothrow) overlapped_send<ENty, CNty>(
        this->shared_from_this(), io_type::kOutput);
    if (output && PostQueuedCompletionStatus(io_h_, 0, 0, output)) {
      send_state_ = send_status::kQueued;
    } else {
      delete output;
      closing_ = true;
      aborted_ = true;
      ::shutdown(socket_, SD_BOTH);
    }
  }
  // +=========================================================================+
  // | [>] send_pending_                                           ( private ) |
  // +-------------------------------------------------------------------------+
  void send_pending_() {
    if (aborted_) {
      abort_();
      return;
    }
    if (send_state_ != send_status::kIdle) return;
    if (socket_ == INVALID_SOCKET) {
      retire_();
      return;
    }
    if (processing_receive_) return;
    if (!input_.prepare_output(output_)) {
      abort_();
      return;
    }
    std::array<std::span<char>, 1> bytes{};
    const std::size_t count = input_.output_buffers(output_, bytes);
    if (!count) {
      cleanup_resources_();
      return;
    }
    auto ovs = new (std::nothrow) overlapped_send<ENty, CNty>(
        this->shared_from_this());
    if (!ovs) {
      abort_();
      return;
    }
    for (std::size_t i = 0; i < count; i++) {
      const std::size_t size = std::min<std::size_t>(
          bytes[i].size(),
          std::numeric_limits<DWORD>::max() - ovs->submitted_size);
      if (!size) break;
      ovs->buffers[i] = {static_cast<ULONG>(size), bytes[i].data()};
      ovs->buffer_count++;
      ovs->submitted_size += size;
    }
    const int result = WSASend(socket_, ovs->buffers.data(),
                               ovs->buffer_count, nullptr, 0, ovs, nullptr);
    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      delete ovs;
      abort_();
      return;
    }
    send_state_ = send_status::kPending;
  }
  // +=========================================================================+
  // | [>] cleanup_resources_                                      ( private ) |
  // +-------------------------------------------------------------------------+
  void cleanup_resources_() {
    if (socket_ == INVALID_SOCKET) {
      retire_();
      return;
    }
    if (!closing_ || processing_receive_ ||
        send_state_ != send_status::kIdle ||
        input_.output_pending(output_)) return;
    if (!socket_shutdown_) {
      if (::shutdown(socket_, SD_SEND) == SOCKET_ERROR) {
        abort_();
        return;
      }
      socket_shutdown_ = true;
    }
    if (stopping_.load() || receive_closed_ || input_.peer_closed()) {
      close_socket_();
      retire_();
      return;
    }
    if (!receiving_) {
      // Discard input until peer EOF to avoid resetting the final response.
      input_.size = 0;
      if (!receive_()) abort_();
    }
  }
  // +=========================================================================+
  // | [>] abort_                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  void abort_() {
    closing_ = true;
    aborted_ = true;
    close_socket_();
    retire_();
  }
  // +=========================================================================+
  // | [>] close_socket_                                           ( private ) |
  // +-------------------------------------------------------------------------+
  void close_socket_() {
    if (socket_ != INVALID_SOCKET) {
      closesocket(socket_);
      socket_ = INVALID_SOCKET;
    }
  }
  // +=========================================================================+
  // | [>] retire_                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  void retire_() {
    if (retired_ || socket_ != INVALID_SOCKET || receiving_ ||
        processing_receive_ || send_state_ != send_status::kIdle) return;
    retired_ = true;
    if (on_retirement_) on_retirement_(this);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  enum class send_status { kIdle, kQueued, kPending };
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
  std::function<void(context*)> on_retirement_;
  SOCKET socket_{INVALID_SOCKET};
  HANDLE io_h_{nullptr};
  CNty input_;
  const std::atomic<bool>& stopping_;
  std::mutex receive_mutex_;
  std::mutex sending_mutex_;
  send_state output_;
  send_status send_state_{send_status::kIdle};
  bool closing_{false};
  bool aborted_{false};
  bool connected_{false};
  bool notified_{false};
  bool disconnected_{false};
  bool receiving_{false};
  bool receive_closed_{false};
  bool processing_receive_{false};
  bool socket_shutdown_{false};
  bool retired_{false};
  WSABUF ovr_wsa_{0};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] basic_transport [windowsTM]                                 ( class ) |
// +---------------------------------------------------------------------------+
// | This specification holds for the WindowsTM server transport.              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty, typename CNty,
          typename PTy>
class basic_transport {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = PTy;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit basic_transport(
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
  basic_transport(const basic_transport&) = delete;
  basic_transport(basic_transport&&) noexcept = delete;
  ~basic_transport() { stop(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  basic_transport& operator=(const basic_transport&) = delete;
  basic_transport& operator=(basic_transport&&) noexcept = delete;
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
    if (current_transport_ == this) {
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
        current_transport_ = this;
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
        current_transport_ = nullptr;
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
      ctx = std::make_shared<context<ENty, CNty>>(
          ova->socket, configuration_.recv_buffer_size,
          configuration_.max_send_buffer_size, create_engine_, stopping_,
          on_connection_, on_disconnection_,
          [this](context<ENty, CNty>* context) { retire_context(context); },
          shared_state_);
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
  inline static thread_local const basic_transport* current_transport_{nullptr};
  std::unordered_map<context<ENty, CNty>*,
                     std::shared_ptr<context<ENty, CNty>>> contexts_;
  types::on_client_connected_delegate on_connection_;
  types::on_client_disconnected_delegate on_disconnection_;
};

}  // namespace martianlabs::doba::transport::server

#endif
