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

#ifndef martianlabs_doba_tests_integration_native_peer_linux_h
#define martianlabs_doba_tests_integration_native_peer_linux_h

#include <string_view>

#include "tcpip_client.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] native_peer                                                 ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class native_peer {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit native_peer(bool listening = true) {
    listener_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener_ == invalid_socket()) return;
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (::bind(listener_, reinterpret_cast<sockaddr*>(&address),
               sizeof(address)) != 0)
      return;
    socklen_t size = sizeof(address);
    if (::getsockname(listener_, reinterpret_cast<sockaddr*>(&address),
                      &size) != 0)
      return;
    if (listening && ::listen(listener_, 1) != 0) return;
    port_ = ntohs(address.sin_port);
  }
  native_peer(const native_peer&) = delete;
  ~native_peer() {
    close();
    if (listener_ != invalid_socket()) close_socket(listener_);
  }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  native_peer& operator=(const native_peer&) = delete;
  // +=========================================================================+
  // | [>] port                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  uint16_t port() const { return port_; }
  // +=========================================================================+
  // | [>] accept                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  bool accept() {
    fd_set selected;
    FD_ZERO(&selected);
    FD_SET(listener_, &selected);
    timeval timeout{3, 0};
    int ready = ::select(listener_ + 1, &selected, nullptr, nullptr, &timeout);
    if (ready <= 0) return false;
    peer_ = ::accept(listener_, nullptr, nullptr);
    return peer_ != invalid_socket();
  }
  // +=========================================================================+
  // | [>] send                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  bool send(std::string_view value) {
    int count = ::send(peer_, value.data(), static_cast<int>(value.size()),
                       MSG_NOSIGNAL);
    return count == static_cast<int>(value.size());
  }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void close(bool reset = false) {
    if (peer_ == invalid_socket()) return;
    if (reset) {
      linger value{1, 0};
      ::setsockopt(peer_, SOL_SOCKET, SO_LINGER,
                   reinterpret_cast<const char*>(&value), sizeof(value));
    }
    close_socket(peer_);
    peer_ = invalid_socket();
  }

 private:
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  using socket_type = int;
  static socket_type invalid_socket() { return -1; }
  static void close_socket(socket_type socket) { ::close(socket); }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  [[maybe_unused]] martianlabs::doba::network::detail::environment environment_;
  socket_type listener_{invalid_socket()};
  socket_type peer_{invalid_socket()};
  uint16_t port_ = 0;
};
}  // namespace

#endif
