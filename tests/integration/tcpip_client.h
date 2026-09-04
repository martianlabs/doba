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

#include <chrono>
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
  bool connect(uint16_t port) {
    close();
    socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ == invalid_socket()) return false;
    set_receive_timeout();
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (::connect(socket_, reinterpret_cast<sockaddr*>(&address),
                  sizeof(address)) == 0) {
      return true;
    }
    close();
    return false;
  }
  bool send_all(std::string_view value) {
    std::size_t sent = 0;
    while (sent < value.size()) {
      int count = ::send(socket_, value.data() + sent,
                         static_cast<int>(value.size() - sent), 0);
      if (count <= 0) return false;
      sent += static_cast<std::size_t>(count);
    }
    return true;
  }
  std::optional<std::string> receive(std::size_t size) {
    std::string result(size, '\0');
    std::size_t received = 0;
    while (received < size) {
      int count = ::recv(socket_, result.data() + received,
                         static_cast<int>(size - received), 0);
      if (count <= 0) return std::nullopt;
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
    if (!has_data(timeout)) return false;
    char value = 0;
    return ::recv(socket_, &value, 1, 0) == 0;
  }
  std::optional<std::string> receive_until_close(std::size_t maximum) {
    std::string result;
    char buffer[8192];
    for (;;) {
      int count = ::recv(socket_, buffer, sizeof(buffer), 0);
      if (count == 0) return result;
      if (count < 0 || result.size() + static_cast<std::size_t>(count) >
                           maximum) {
        return std::nullopt;
      }
      result.append(buffer, static_cast<std::size_t>(count));
    }
  }
  bool shutdown_write() {
    if (socket_ == invalid_socket()) return false;
#ifdef _WIN32
    return ::shutdown(socket_, SD_SEND) == 0;
#else
    return ::shutdown(socket_, SHUT_WR) == 0;
#endif
  }
  bool set_receive_buffer_size(int size) {
    return socket_ != invalid_socket() &&
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
  void set_receive_timeout() {
#ifdef _WIN32
    DWORD timeout = 3000;
    ::setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO,
                 reinterpret_cast<const char*>(&timeout), sizeof(timeout));
#else
    timeval timeout{3, 0};
    ::setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif
  }
  [[maybe_unused]] network::detail::environment environment_;
  socket_type socket_{invalid_socket()};
};
}  // namespace martianlabs::doba::tests::integration

#endif
