//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef martianlabs_doba_tests_integration_tcpip_client_h
#define martianlabs_doba_tests_integration_tcpip_client_h

#include <algorithm>
#include <chrono>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "network/environment.h"

namespace martianlabs::doba::tests::integration {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcpip_client                                                ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class tcpip_client {
 public:
  tcpip_client() = default;
  tcpip_client(const tcpip_client&) = delete;
  tcpip_client(tcpip_client&&) noexcept = delete;
  ~tcpip_client() { close(); }
  tcpip_client& operator=(const tcpip_client&) = delete;
  tcpip_client& operator=(tcpip_client&&) noexcept = delete;
  uint16_t find_available_port() const {
    socket_type socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket == invalid_socket()) return 0;
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = 0;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (::bind(socket, reinterpret_cast<sockaddr*>(&address),
               sizeof(address)) != 0) {
      close_socket(socket);
      return 0;
    }
#ifdef _WIN32
    int size = sizeof(address);
#else
    socklen_t size = sizeof(address);
#endif
    if (::getsockname(socket, reinterpret_cast<sockaddr*>(&address), &size) !=
        0) {
      close_socket(socket);
      return 0;
    }
    close_socket(socket);
    return ntohs(address.sin_port);
  }
  bool connect(uint16_t port,
               std::chrono::milliseconds timeout = std::chrono::seconds(3)) {
    close();
    operation_ = "connect";
    error_ = {};
    native_error_ = 0;
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ == invalid_socket()) return fail_socket();
#ifdef _WIN32
    u_long nonblocking = 1;
    if (::ioctlsocket(socket_, FIONBIO, &nonblocking) != 0) {
#else
    if (::fcntl(socket_, F_SETFL, O_NONBLOCK) != 0) {
#endif
      fail_socket();
      close();
      return false;
    }
    if (receive_buffer_size_ != 0 &&
        !set_receive_buffer_size(receive_buffer_size_)) {
      fail_socket();
      close();
      return false;
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (::connect(socket_, reinterpret_cast<sockaddr*>(&address),
                  sizeof(address)) == 0) {
      return true;
    }
    if (!pending(socket_error()) || !wait_ready(true, deadline)) {
      if (error_.empty()) fail_socket();
      close();
      return false;
    }
    int error = 0;
#ifdef _WIN32
    int size = sizeof(error);
#else
    socklen_t size = sizeof(error);
#endif
    if (::getsockopt(socket_, SOL_SOCKET, SO_ERROR,
                     reinterpret_cast<char*>(&error), &size) != 0) {
      fail_socket();
    } else if (error != 0) {
      error_ = "socket";
      native_error_ = error;
    } else {
      return true;
    }
    close();
    return false;
  }
  bool send_all(
      std::string_view value,
      std::chrono::milliseconds timeout = std::chrono::seconds(3)) {
    operation_ = "send";
    error_ = {};
    native_error_ = 0;
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    std::size_t sent = 0;
    while (sent < value.size()) {
      if (!wait_ready(true, deadline)) return false;
      const int size = static_cast<int>(
          std::min(value.size() - sent, static_cast<std::size_t>(INT_MAX)));
#ifdef _WIN32
      int count = ::send(socket_, value.data() + sent, size, 0);
#else
      int count = ::send(socket_, value.data() + sent, size, MSG_NOSIGNAL);
#endif
      if (count < 0 && pending(socket_error())) continue;
      if (count <= 0) return fail_socket();
      sent += static_cast<std::size_t>(count);
    }
    return true;
  }
  std::optional<std::string> receive(
      std::size_t size,
      std::chrono::milliseconds timeout = std::chrono::seconds(3)) {
    operation_ = "receive";
    error_ = {};
    native_error_ = 0;
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    std::string result(size, '\0');
    std::size_t received = 0;
    while (received < size) {
      if (!wait_ready(false, deadline)) return std::nullopt;
      int count = ::recv(
          socket_, result.data() + received,
          static_cast<int>(std::min(size - received,
                                    static_cast<std::size_t>(INT_MAX))), 0);
      if (count < 0 && pending(socket_error())) continue;
      if (count == 0) {
        error_ = "eof";
        return std::nullopt;
      }
      if (count < 0) {
        fail_socket();
        return std::nullopt;
      }
      received += static_cast<std::size_t>(count);
    }
    return result;
  }
  bool has_data(std::chrono::milliseconds timeout) const {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(socket_, &read_set);
    timeval value{};
    value.tv_sec = static_cast<long>(timeout.count() / 1000);
    value.tv_usec =
        static_cast<long>((timeout.count() % 1000) * 1000);
#ifdef _WIN32
    int ready = ::select(0, &read_set, nullptr, nullptr, &value);
#else
    int ready = ::select(socket_ + 1, &read_set, nullptr, nullptr, &value);
#endif
    return ready > 0 && FD_ISSET(socket_, &read_set);
  }
  bool wait_for_close(std::chrono::milliseconds timeout) {
    operation_ = "receive";
    error_ = {};
    native_error_ = 0;
    if (!wait_ready(false, std::chrono::steady_clock::now() + timeout)) {
      return false;
    }
    char value = 0;
    int count = ::recv(socket_, &value, 1, 0);
    if (count < 0) return fail_socket();
    if (count == 0) error_ = "eof";
    return count == 0;
  }
  std::optional<std::string> receive_until_close(
      std::size_t maximum,
      std::chrono::milliseconds timeout = std::chrono::seconds(3)) {
    operation_ = "receive";
    error_ = {};
    native_error_ = 0;
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    std::string result;
    char buffer[8192];
    for (;;) {
      if (!wait_ready(false, deadline)) return std::nullopt;
      int count = ::recv(socket_, buffer, sizeof(buffer), 0);
      if (count < 0 && pending(socket_error())) continue;
      if (count == 0) {
        error_ = "eof";
        return result;
      }
      if (count < 0) {
        fail_socket();
        return std::nullopt;
      }
      if (static_cast<std::size_t>(count) > maximum - result.size()) {
        error_ = "limit";
        return std::nullopt;
      }
      result.append(buffer, static_cast<std::size_t>(count));
    }
  }
  std::string_view operation() const { return operation_; }
  std::string_view error() const { return error_; }
  int native_error() const { return native_error_; }
  bool shutdown_write() {
    if (socket_ == invalid_socket()) return false;
#ifdef _WIN32
    return ::shutdown(socket_, SD_SEND) == 0;
#else
    return ::shutdown(socket_, SHUT_WR) == 0;
#endif
  }
  bool set_receive_buffer_size(int size) {
    if (size <= 0) return false;
    receive_buffer_size_ = size;
    return socket_ == invalid_socket() ||
           ::setsockopt(socket_, SOL_SOCKET, SO_RCVBUF,
                        reinterpret_cast<const char*>(&size), sizeof(size)) ==
               0;
  }
  void abort() {
    if (socket_ == invalid_socket()) return;
    linger value{1, 0};
    ::setsockopt(socket_, SOL_SOCKET, SO_LINGER,
                 reinterpret_cast<const char*>(&value), sizeof(value));
    close_socket(socket_);
    socket_ = invalid_socket();
  }
  void close() {
    if (socket_ == invalid_socket()) return;
#ifdef _WIN32
    ::shutdown(socket_, SD_BOTH);
#else
    ::shutdown(socket_, SHUT_RDWR);
#endif
    close_socket(socket_);
    socket_ = invalid_socket();
  }

 private:
#ifdef _WIN32
  using socket_type = SOCKET;
  static socket_type invalid_socket() { return INVALID_SOCKET; }
  static void close_socket(socket_type socket) { ::closesocket(socket); }
#else
  using socket_type = int;
  static socket_type invalid_socket() { return -1; }
  static void close_socket(socket_type socket) { ::close(socket); }
#endif
  static int socket_error() {
#ifdef _WIN32
    return ::WSAGetLastError();
#else
    return errno;
#endif
  }
  static bool pending(int error) {
#ifdef _WIN32
    return error == WSAEWOULDBLOCK || error == WSAEINPROGRESS ||
           error == WSAEINTR;
#else
    return error == EWOULDBLOCK || error == EAGAIN || error == EINPROGRESS ||
           error == EINTR;
#endif
  }
  bool fail_socket() {
    error_ = "socket";
    native_error_ = socket_error();
    return false;
  }
  bool wait_ready(bool writing,
                  std::chrono::steady_clock::time_point deadline) {
    for (;;) {
      const auto remaining =
          std::chrono::duration_cast<std::chrono::microseconds>(
          deadline - std::chrono::steady_clock::now());
      if (remaining.count() <= 0) {
        error_ = "timeout";
        return false;
      }
      fd_set selected;
      fd_set errors;
      FD_ZERO(&selected);
      FD_ZERO(&errors);
      FD_SET(socket_, &selected);
      FD_SET(socket_, &errors);
      timeval timeout{};
      timeout.tv_sec = static_cast<long>(remaining.count() / 1000000);
      timeout.tv_usec = static_cast<long>(remaining.count() % 1000000);
#ifdef _WIN32
      const int count = ::select(0, writing ? nullptr : &selected,
                                 writing ? &selected : nullptr, &errors,
                                 &timeout);
#else
      const int count = ::select(socket_ + 1, writing ? nullptr : &selected,
                                 writing ? &selected : nullptr, &errors,
                                 &timeout);
#endif
      if (count > 0) return true;
      if (count == 0) {
        error_ = "timeout";
        return false;
      }
      if (!pending(socket_error())) return fail_socket();
    }
  }
  [[maybe_unused]] network::detail::environment environment_;
  socket_type socket_{invalid_socket()};
  int receive_buffer_size_ = 0;
  std::string_view operation_;
  std::string_view error_;
  int native_error_ = 0;
};
}  // namespace martianlabs::doba::tests::integration

#endif
