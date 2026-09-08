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

#include <atomic>
#include <chrono>
#include <string>
#include <string_view>
#include <thread>

#include "test_helper.h"
#include "tcpip_client.h"

namespace {
// +===========================================================================+
// | [>] native_peer                                                 ( class ) |
// +===========================================================================+
class native_peer {
 public:
  explicit native_peer(bool listening = true) {
    listener_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener_ == invalid_socket()) return;
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (::bind(listener_, reinterpret_cast<sockaddr*>(&address),
               sizeof(address)) != 0) return;
#ifdef _WIN32
    int size = sizeof(address);
#else
    socklen_t size = sizeof(address);
#endif
    if (::getsockname(listener_, reinterpret_cast<sockaddr*>(&address),
                      &size) != 0) return;
    if (listening && ::listen(listener_, 1) != 0) return;
    port_ = ntohs(address.sin_port);
  }
  native_peer(const native_peer&) = delete;
  native_peer& operator=(const native_peer&) = delete;
  ~native_peer() {
    close();
    if (listener_ != invalid_socket()) close_socket(listener_);
  }
  uint16_t port() const { return port_; }
  bool accept() {
    fd_set selected;
    FD_ZERO(&selected);
    FD_SET(listener_, &selected);
    timeval timeout{3, 0};
#ifdef _WIN32
    int ready = ::select(0, &selected, nullptr, nullptr, &timeout);
#else
    int ready = ::select(listener_ + 1, &selected, nullptr, nullptr, &timeout);
#endif
    if (ready <= 0) return false;
    peer_ = ::accept(listener_, nullptr, nullptr);
    return peer_ != invalid_socket();
  }
  bool send(std::string_view value) {
#ifdef _WIN32
    int count = ::send(peer_, value.data(), static_cast<int>(value.size()), 0);
#else
    int count = ::send(peer_, value.data(), static_cast<int>(value.size()),
                       MSG_NOSIGNAL);
#endif
    return count == static_cast<int>(value.size());
  }
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
#ifdef _WIN32
  using socket_type = SOCKET;
  static socket_type invalid_socket() { return INVALID_SOCKET; }
  static void close_socket(socket_type socket) { ::closesocket(socket); }
#else
  using socket_type = int;
  static socket_type invalid_socket() { return -1; }
  static void close_socket(socket_type socket) { ::close(socket); }
#endif
  [[maybe_unused]] martianlabs::doba::network::detail::environment environment_;
  socket_type listener_{invalid_socket()};
  socket_type peer_{invalid_socket()};
  uint16_t port_ = 0;
};
}  // namespace

// +===========================================================================+
// | [>] refused connection diagnostic                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("client reports refused connections") {
  native_peer peer(false);
  martianlabs::doba::tests::integration::tcpip_client client;
  DOBA_EXPECT(peer.port() != 0);
  const auto started = std::chrono::steady_clock::now();
  DOBA_EXPECT(!client.connect(peer.port()));
  DOBA_EXPECT(std::chrono::steady_clock::now() - started <
              std::chrono::seconds(4));
  DOBA_EXPECT_EQUAL(client.operation(), "connect");
  DOBA_EXPECT_EQUAL(client.error(), "socket");
  DOBA_EXPECT(client.native_error() != 0);
}

// +===========================================================================+
// | [>] silent receive deadline                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("client bounds silent receive operations") {
  native_peer peer;
  martianlabs::doba::tests::integration::tcpip_client client;
  DOBA_EXPECT(peer.port() != 0);
  DOBA_EXPECT(client.connect(peer.port()));
  DOBA_EXPECT(peer.accept());
  const auto started = std::chrono::steady_clock::now();
  auto result = client.receive(2, std::chrono::milliseconds(200));
  const auto elapsed = std::chrono::steady_clock::now() - started;
  DOBA_EXPECT(!result.has_value());
  DOBA_EXPECT_EQUAL(client.operation(), "receive");
  DOBA_EXPECT_EQUAL(client.error(), "timeout");
  DOBA_EXPECT(elapsed >= std::chrono::milliseconds(150));
  DOBA_EXPECT(elapsed < std::chrono::seconds(2));
}

// +===========================================================================+
// | [>] partial receive deadline                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("client preserves the deadline after a partial receive") {
  native_peer peer;
  martianlabs::doba::tests::integration::tcpip_client client;
  DOBA_EXPECT(peer.port() != 0);
  DOBA_EXPECT(client.connect(peer.port()));
  DOBA_EXPECT(peer.accept());
  std::atomic<bool> sent = false;
  const auto started = std::chrono::steady_clock::now();
  std::jthread producer([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    sent = peer.send("a");
  });
  auto result = client.receive(2, std::chrono::milliseconds(600));
  const auto elapsed = std::chrono::steady_clock::now() - started;
  producer.join();
  DOBA_EXPECT(sent.load());
  DOBA_EXPECT(!result.has_value());
  DOBA_EXPECT_EQUAL(client.error(), "timeout");
  DOBA_EXPECT(elapsed >= std::chrono::milliseconds(550));
  DOBA_EXPECT(elapsed < std::chrono::milliseconds(850));
}

// +===========================================================================+
// | [>] progress does not extend deadline                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("client bounds receive until close despite progress") {
  native_peer peer;
  martianlabs::doba::tests::integration::tcpip_client client;
  DOBA_EXPECT(peer.port() != 0);
  DOBA_EXPECT(client.connect(peer.port()));
  DOBA_EXPECT(peer.accept());
  std::atomic<std::size_t> sent = 0;
  std::jthread producer([&](std::stop_token stop) {
    for (std::size_t i = 0; i < 20 && !stop.stop_requested(); i++) {
      if (peer.send("a")) sent.fetch_add(1);
      std::this_thread::sleep_for(std::chrono::milliseconds(60));
    }
  });
  const auto started = std::chrono::steady_clock::now();
  auto result = client.receive_until_close(100, std::chrono::milliseconds(250));
  const auto elapsed = std::chrono::steady_clock::now() - started;
  producer.request_stop();
  producer.join();
  DOBA_EXPECT(sent.load() >= 2);
  DOBA_EXPECT(!result.has_value());
  DOBA_EXPECT_EQUAL(client.error(), "timeout");
  DOBA_EXPECT(elapsed >= std::chrono::milliseconds(200));
  DOBA_EXPECT(elapsed < std::chrono::milliseconds(600));
}

// +===========================================================================+
// | [>] eof and reset diagnostics                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("client distinguishes eof from reset") {
  for (bool reset : {false, true}) {
    native_peer peer;
    martianlabs::doba::tests::integration::tcpip_client client;
    DOBA_EXPECT(peer.port() != 0);
    DOBA_EXPECT(client.connect(peer.port()));
    DOBA_EXPECT(peer.accept());
    DOBA_EXPECT(peer.send("a"));
    peer.close(reset);
    auto result = client.receive(2);
    DOBA_EXPECT(!result.has_value());
    DOBA_EXPECT_EQUAL(client.operation(), "receive");
    DOBA_EXPECT_EQUAL(client.error(), reset ? "socket" : "eof");
    DOBA_EXPECT_EQUAL(client.native_error() != 0, reset);
  }
}
