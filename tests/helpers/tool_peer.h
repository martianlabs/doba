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
// Copyright 2026 martianLabs
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

#ifndef martianlabs_doba_tests_helpers_tool_peer_h
#define martianlabs_doba_tests_helpers_tool_peer_h

// Include Winsock before windows.h, even through another helper.
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "tool_support.h"

#include <climits>
#include <cstdint>
#include <memory>

#ifdef DOBA_TOOL_TLS
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif

namespace martianlabs::doba::tests::tools {

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] peer                                                        ( class ) |
// +---------------------------------------------------------------------------+
// | Loopback peer with socket timeouts. Memory BIOs let tests split TLS       |
// | records without passing Windows sockets to OpenSSL.                       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class peer {
#ifdef _WIN32
  using socket_type = SOCKET;
  static constexpr auto invalid = INVALID_SOCKET;
  // +=========================================================================+
  // | [>] initialize                                              ( private ) |
  // +-------------------------------------------------------------------------+
  static void initialize() {
    struct winsock {
      winsock() {
        WSADATA data{};
        require(WSAStartup(MAKEWORD(2, 2), &data) == 0, "WSAStartup failed");
      }
      ~winsock() { WSACleanup(); }
    };
    static winsock environment;
  }
  // +=========================================================================+
  // | [>] close_socket                                            ( private ) |
  // +-------------------------------------------------------------------------+
  static void close_socket(socket_type socket) { closesocket(socket); }
#else
  using socket_type = int;
  static constexpr auto invalid = -1;
  // +=========================================================================+
  // | [>] initialize                                              ( private ) |
  // +-------------------------------------------------------------------------+
  static void initialize() {}
  // +=========================================================================+
  // | [>] close_socket                                            ( private ) |
  // +-------------------------------------------------------------------------+
  static void close_socket(socket_type socket) { ::close(socket); }
#endif
  socket_type socket_{invalid};
#ifdef DOBA_TOOL_TLS
  SSL_CTX* context_{};
  SSL* ssl_{};
  bool fragment_{};

  // +=========================================================================+
  // | [>] drain                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  void drain() {
    char data[65536];
    BIO* output = SSL_get_wbio(ssl_);
    while (BIO_ctrl_pending(output)) {
      int count = BIO_read(output, data, sizeof(data));
      require(count > 0, "TLS output BIO failed");
      if (fragment_) {
        for (int index = 0; index < count; ++index) raw_send(
          std::string_view(data + index, 1));
      } else raw_send(std::string_view(data, static_cast<size_t>(count)));
    }
  }
  // +=========================================================================+
  // | [>] feed                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  bool feed() {
    auto data = raw_receive(65536);
    if (data.empty()) {
      BIO_set_mem_eof_return(SSL_get_rbio(ssl_), 0);
      return false;
    }
    require(BIO_write(SSL_get_rbio(ssl_), data.data(),
      static_cast<int>(data.size())) == static_cast<int>(data.size()),
      "TLS input BIO failed");
    return true;
  }
#endif

 public:
  // +=========================================================================+
  // | [>] PUBLIC INTERFACE                                         ( public ) |
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +-------------------------------------------------------------------------+
  explicit peer(uint16_t port) {
    initialize();
    socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    require(socket_ != invalid, "Cannot create client socket");
    timeout(5s);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port);
    if (::connect(socket_, reinterpret_cast<sockaddr*>(&address),
        sizeof(address)) != 0) {
      close_socket(socket_);
      socket_ = invalid;
      throw std::runtime_error("Loopback connection failed");
    }
  }
  peer(const peer&) = delete;
  peer& operator=(const peer&) = delete;
  ~peer() {
    close();
#ifdef DOBA_TOOL_TLS
    if (ssl_) SSL_free(ssl_);
    if (context_) SSL_CTX_free(context_);
#endif
  }
  // +=========================================================================+
  // | [>] close                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void close() {
    if (socket_ != invalid) close_socket(socket_);
    socket_ = invalid;
  }
  // +=========================================================================+
  // | [>] available_port                                           ( public ) |
  // +-------------------------------------------------------------------------+
  static uint16_t available_port() {
    initialize();
    auto probe = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    require(probe != invalid, "Cannot create port probe");
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    int result = bind(probe, reinterpret_cast<sockaddr*>(&address),
      sizeof(address));
#ifdef _WIN32
    int size = sizeof(address);
#else
    socklen_t size = sizeof(address);
#endif
    int named = getsockname(probe,
      reinterpret_cast<sockaddr*>(&address), &size);
    close_socket(probe);
    require(result == 0 && named == 0, "Cannot allocate loopback port");
    return ntohs(address.sin_port);
  }
  // +=========================================================================+
  // | [>] timeout                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  void timeout(std::chrono::milliseconds duration) {
#ifdef _WIN32
    DWORD value = static_cast<DWORD>(duration.count());
#else
    timeval value{static_cast<long>(duration.count() / 1000),
      static_cast<long>((duration.count() % 1000) * 1000)};
#endif
    for (int option : {SO_RCVTIMEO, SO_SNDTIMEO}) {
      require(setsockopt(socket_, SOL_SOCKET, option,
        reinterpret_cast<const char*>(&value), sizeof(value)) == 0,
        "Cannot set peer deadline");
    }
  }
  // +=========================================================================+
  // | [>] receive_buffer                                           ( public ) |
  // +-------------------------------------------------------------------------+
  void receive_buffer(int size) {
    require(setsockopt(socket_, SOL_SOCKET, SO_RCVBUF,
      reinterpret_cast<const char*>(&size), sizeof(size)) == 0,
      "Cannot set receive buffer");
  }
  // +=========================================================================+
  // | [>] raw_send                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  void raw_send(std::string_view data) {
    while (!data.empty()) {
#ifdef _WIN32
      int count = ::send(socket_, data.data(),
        static_cast<int>(std::min(data.size(), size_t{INT_MAX})), 0);
#else
      auto count = ::send(socket_, data.data(), data.size(), MSG_NOSIGNAL);
      if (count < 0 && errno == EINTR) continue;
#endif
      require(count > 0, "Peer socket send failed or timed out");
      data.remove_prefix(static_cast<size_t>(count));
    }
  }
  // +=========================================================================+
  // | [>] raw_receive                                              ( public ) |
  // +-------------------------------------------------------------------------+
  std::string raw_receive(size_t maximum) {
    std::string data(std::min(maximum, size_t{65536}), '\0');
#ifdef _WIN32
    int count = recv(socket_, data.data(), static_cast<int>(data.size()), 0);
#else
    ssize_t count;
    do { count = recv(socket_, data.data(), data.size(), 0); }
    while (count < 0 && errno == EINTR);
#endif
    require(count >= 0, "Peer socket receive failed or timed out");
    data.resize(static_cast<size_t>(count));
    return data;
  }
#ifdef DOBA_TOOL_TLS
  // +=========================================================================+
  // | [>] tls                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  void tls(const fs::path& certificate, int version = 0,
           const std::string& alpn = "doba-echo", bool fragmented = false,
           const std::string& hostname = "localhost") {
    fragment_ = fragmented;
    context_ = SSL_CTX_new(TLS_client_method());
    require(context_ != nullptr, "Cannot allocate TLS client context");
    SSL_CTX_set_verify(context_, SSL_VERIFY_PEER, nullptr);
    if (certificate.empty()) {
      require(SSL_CTX_set_default_verify_paths(context_) == 1,
        "Cannot load default TLS trust roots");
    } else {
      require(SSL_CTX_load_verify_locations(context_,
        certificate.string().c_str(), nullptr) == 1, "Cannot load TLS CA");
    }
    if (version) {
      require(SSL_CTX_set_min_proto_version(context_, version) == 1 &&
        SSL_CTX_set_max_proto_version(context_, version) == 1,
        "Cannot select TLS version");
    }
    ssl_ = SSL_new(context_);
    require(ssl_ != nullptr, "Cannot allocate TLS client");
    SSL_set_bio(ssl_, BIO_new(BIO_s_mem()), BIO_new(BIO_s_mem()));
    require(SSL_get_rbio(ssl_) && SSL_get_wbio(ssl_), "Cannot allocate BIOs");
    BIO_set_mem_eof_return(SSL_get_rbio(ssl_), -1);
    require(SSL_set_tlsext_host_name(ssl_, hostname.c_str()) == 1 &&
      SSL_set1_host(ssl_, hostname.c_str()) == 1, "Cannot set verified host");
    if (!alpn.empty()) {
      require(alpn.size() <= 255, "ALPN name too large");
      std::string wire(1, static_cast<char>(alpn.size()));
      wire += alpn;
      require(SSL_set_alpn_protos(ssl_,
        reinterpret_cast<const unsigned char*>(wire.data()),
        static_cast<unsigned>(wire.size())) == 0, "Cannot select ALPN");
    }
    auto deadline = clock::now() + 10s;
    for (;;) {
      int result = SSL_connect(ssl_);
      int error = result == 1 ? SSL_ERROR_NONE : SSL_get_error(ssl_, result);
      drain();
      if (result == 1) break;
      require(error == SSL_ERROR_WANT_READ && clock::now() < deadline,
        "TLS handshake rejected: " +
        std::to_string(SSL_get_verify_result(ssl_)));
      require(feed(), "EOF during TLS handshake");
    }
    require(SSL_get_verify_result(ssl_) == X509_V_OK,
      "TLS certificate verification failed");
    require(!version || SSL_version(ssl_) == version,
      "TLS negotiated the wrong version");
  }
  // +=========================================================================+
  // | [>] alpn                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  std::string alpn() const {
    const unsigned char* data{};
    unsigned size{};
    SSL_get0_alpn_selected(ssl_, &data, &size);
    return size ? std::string(reinterpret_cast<const char*>(data), size) : "";
  }
  // +=========================================================================+
  // | [>] shutdown_tls                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void shutdown_tls() {
    auto deadline = clock::now() + 10s;
    for (;;) {
      int result = SSL_shutdown(ssl_);
      int error = result >= 0 ? SSL_ERROR_NONE : SSL_get_error(ssl_, result);
      drain();
      if (result == 1) return;
      require((result == 0 || error == SSL_ERROR_WANT_READ) &&
        clock::now() < deadline, "TLS close notification failed");
      require(feed(), "Missing TLS close notification");
    }
  }
#endif
  // +=========================================================================+
  // | [>] send                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void send(std::string_view data) {
#ifdef DOBA_TOOL_TLS
    if (ssl_) {
      auto deadline = clock::now() + 30s;
      while (!data.empty()) {
        int count = SSL_write(ssl_, data.data(),
          static_cast<int>(std::min(data.size(), size_t{16384})));
        int error = count > 0 ? SSL_ERROR_NONE : SSL_get_error(ssl_, count);
        drain();
        if (count > 0) data.remove_prefix(static_cast<size_t>(count));
        else {
          require(error == SSL_ERROR_WANT_READ && clock::now() < deadline,
            "TLS write failed");
          require(feed(), "EOF during TLS write");
        }
      }
      return;
    }
#endif
    raw_send(data);
  }
  // +=========================================================================+
  // | [>] receive                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  std::string receive(size_t maximum) {
#ifdef DOBA_TOOL_TLS
    if (ssl_) {
      auto deadline = clock::now() + 30s;
      std::string data(std::min(maximum, size_t{65536}), '\0');
      bool eof = false;
      for (;;) {
        int count = SSL_read(ssl_, data.data(), static_cast<int>(data.size()));
        int error = count > 0 ? SSL_ERROR_NONE : SSL_get_error(ssl_, count);
        drain();
        if (count > 0) {
          data.resize(static_cast<size_t>(count));
          return data;
        }
        if (error == SSL_ERROR_ZERO_RETURN) return {};
        // Rejection tests may close without close_notify.
        if (error != SSL_ERROR_WANT_READ && eof) return {};
        require(error == SSL_ERROR_WANT_READ && clock::now() < deadline,
          "TLS read failed before peer EOF");
        eof = !feed();
      }
    }
#endif
    return raw_receive(maximum);
  }
  // +=========================================================================+
  // | [>] exact                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  std::string exact(size_t size) {
    std::string result;
    while (result.size() < size) {
      auto data = receive(size - result.size());
      require(!data.empty(), "Truncated peer response");
      result += data;
    }
    return result;
  }
  // +=========================================================================+
  // | [>] expect                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  void expect(std::string_view expected) {
    require(exact(expected.size()) == expected, "Peer response bytes differ");
  }
};

} // namespace martianlabs::doba::tests::tools

#endif
