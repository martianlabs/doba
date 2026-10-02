//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses-2.0
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

#ifdef DOBA_ENABLE_TLS

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "platform.h"
#include <openssl/ssl.h>

#include "protocol/http/v11/server.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "transport/server/tls.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] general                                                    ( usings ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::protocol::http::v11::server;
using martianlabs::doba::tests::integration::tcpip_client;
using martianlabs::doba::transport::server::tls;
using martianlabs::doba::transport::server::tls_policies;
using namespace std::chrono_literals;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls_client                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct tls_client {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  tcpip_client socket;
  std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)> context{
      SSL_CTX_new(TLS_client_method()), SSL_CTX_free};
  std::unique_ptr<SSL, decltype(&SSL_free)> ssl{
      context ? SSL_new(context.get()) : nullptr, SSL_free};
  std::unique_ptr<BIO, decltype(&BIO_free)> network{nullptr, BIO_free};
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  tls_client() {
    BIO* internal = nullptr;
    BIO* external = nullptr;
    if (!ssl || BIO_new_bio_pair(&internal, 0, &external, 0) != 1) {
      throw std::runtime_error("TLS client could not be created!");
    }
    SSL_set_bio(ssl.get(), internal, internal);
    network.reset(external);
    SSL_set_connect_state(ssl.get());
  }
  // +=========================================================================+
  // | [>] exchange                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  bool exchange() {
    std::array<char, 4096> bytes;
    while (BIO_ctrl_pending(network.get())) {
      std::size_t size = 0;
      if (BIO_read_ex(network.get(), bytes.data(), bytes.size(), &size) != 1 ||
          !socket.send_all(std::string_view(bytes.data(), size)))
        return false;
    }
    if (!socket.has_data(10ms)) return true;
    auto received = socket.receive_some(bytes.size());
    if (!received) return false;
    std::size_t written = 0;
    return BIO_write_ex(network.get(), received->data(), received->size(),
                        &written) == 1 &&
           written == received->size();
  }
  // +=========================================================================+
  // | [>] negotiate                                                ( public ) |
  // +-------------------------------------------------------------------------+
  bool negotiate() {
    for (int step = 0; step < 100; ++step) {
      const int result = SSL_do_handshake(ssl.get());
      if (result != 1) {
        const int error = SSL_get_error(ssl.get(), result);
        if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE)
          return false;
      }
      if (!exchange()) return false;
      if (SSL_is_init_finished(ssl.get())) return true;
    }
    return false;
  }
  // +=========================================================================+
  // | [>] send                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  bool send(std::string_view value) {
    for (int step = 0; step < 100; ++step) {
      std::size_t written = 0;
      const int result =
          SSL_write_ex(ssl.get(), value.data(), value.size(), &written);
      const int error = result == 1 ? 0 : SSL_get_error(ssl.get(), result);
      std::array<char, 4096> bytes;
      while (BIO_ctrl_pending(network.get())) {
        std::size_t size = 0;
        if (BIO_read_ex(network.get(), bytes.data(), bytes.size(), &size) !=
                1 ||
            !socket.send_all(std::string_view(bytes.data(), size))) {
          return false;
        }
      }
      if (result == 1) return written == value.size();
      if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE)
        return false;
    }
    return false;
  }
  // +=========================================================================+
  // | [>] receive                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  std::optional<std::string> receive(std::size_t size) {
    std::string result(size, '\0');
    std::size_t total = 0;
    for (int step = 0; step < 400 && total < size; ++step) {
      std::size_t received = 0;
      const int value = SSL_read_ex(ssl.get(), result.data() + total,
                                    size - total, &received);
      total += received;
      if (value == 1) {
        if (total == size) return result;
        continue;
      }
      const int error = SSL_get_error(ssl.get(), value);
      if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE)
        return std::nullopt;
      if (!exchange()) return std::nullopt;
    }
    return std::nullopt;
  }
  // +=========================================================================+
  // | [>] receive_close                                            ( public ) |
  // +-------------------------------------------------------------------------+
  bool receive_close() {
    std::array<char, 32> bytes;
    for (int step = 0; step < 100; ++step) {
      std::size_t received = 0;
      const int result =
          SSL_read_ex(ssl.get(), bytes.data(), bytes.size(), &received);
      if (result == 1) return false;
      const int error = SSL_get_error(ssl.get(), result);
      if (error == SSL_ERROR_ZERO_RETURN) return true;
      if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE)
        return false;
      if (!exchange()) return false;
    }
    return false;
  }
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] server_policies                                          ( function ) |
// +---------------------------------------------------------------------------+
// | This function returns a TLS server configuration with a self-signed       |
// | certificate for testing purposes. It sets the IP address, port, and       |
// | worker count for the server.                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
tls_policies server_policies(uint16_t port) {
  tls_policies configuration;
  const auto fixtures = std::filesystem::path(__FILE__).parent_path() /
                        "../../../unit/transport/server/fixtures";
  configuration.certificate_file = (fixtures / "server.crt").string();
  configuration.private_key_file = (fixtures / "server.key").string();
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  return configuration;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] receive_response                                         ( function ) |
// +---------------------------------------------------------------------------+
// | This function receives an HTTP response from a TLS client. It reads the   |
// | response headers and body, ensuring that the response is valid and        |
// | complete. The function returns the body of the response as a string if    |
// | successful, or std::nullopt if there is an error or if the response is    |
// | incomplete. It checks for the "Content-Length" header to determine how    |
// | many bytes to read for the body.                                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
std::optional<std::string> receive_response(tls_client& client) {
  std::string wire;
  while (!wire.ends_with("\r\n\r\n")) {
    if (wire.size() >= 16384) return std::nullopt;
    const auto byte = client.receive(1);
    if (!byte) return std::nullopt;
    wire += *byte;
  }
  if (!wire.starts_with("HTTP/1.1 200 OK\r\n")) return std::nullopt;
  const auto field = wire.find("\r\nContent-Length: ");
  if (field == std::string::npos) return std::nullopt;
  const auto begin = field + sizeof("\r\nContent-Length: ") - 1;
  const auto end = wire.find("\r\n", begin);
  if (end == std::string::npos) return std::nullopt;
  std::size_t size = 0;
  const auto parsed =
      std::from_chars(wire.data() + begin, wire.data() + end, size);
  if (parsed.ec != std::errc{} || parsed.ptr != wire.data() + end ||
      size > 1024 * 1024)
    return std::nullopt;
  const auto body = client.receive(size);
  if (!body) return std::nullopt;
  return *body;
}
}  // namespace

// +===========================================================================+
// | [>] tls serves an HTTP response                             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls serves an HTTP response") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  server<request, response,
         martianlabs::doba::protocol::http::router<request, response>,
         martianlabs::doba::protocol::http::v11::engine<request, response>, tls>
      http_server(server_policies(port));
  http_server.add_route("GET", "/ready", [](const request&, response& res) {
    res.ok_200();
    res.set_body("ready");
    return;
  });
  http_server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  DOBA_EXPECT(client.send("GET /ready HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto received = receive_response(client);
  DOBA_EXPECT(received.has_value());
  DOBA_EXPECT_EQUAL(*received, "ready");
  client.socket.close();
  http_server.stop();
}

// +===========================================================================+
// | [>] tls preserves a fragmented binary HTTP body             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls preserves a fragmented binary HTTP body") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  server<request, response,
         martianlabs::doba::protocol::http::router<request, response>,
         martianlabs::doba::protocol::http::v11::engine<request, response>, tls>
      http_server(server_policies(port));
  http_server.add_route("POST", "/echo", [](const request& req, response& res) {
    if (!req.has_body_reader()) {
      res.bad_request_400();
      return;
    }
    std::array<std::byte, 1024> buffer{};
    std::string body;
    for (int i = 0; i < 32; ++i) {
      const auto state = req.get_body_reader()->read(buffer);
      if (state.has_error) {
        res.bad_request_400();
        return;
      }
      body.append(reinterpret_cast<const char*>(buffer.data()), state.produced);
      if (state.complete) {
        res.ok_200();
        res.set_body(body);
        return;
      }
    }
    {
      res.bad_request_400();
      return;
    }
  });
  http_server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  std::string body(513, '\0');
  for (std::size_t i = 0; i < body.size(); ++i) {
    body[i] = static_cast<char>(i % 251);
  }
  DOBA_EXPECT(client.send("POST /echo HTTP/1.1\r\nHost: example.com\r\n"));
  DOBA_EXPECT(client.send("Content-Length: 513\r\n\r\n"));
  for (std::size_t i = 0; i < body.size(); i += 17) {
    DOBA_EXPECT(client.send(std::string_view(body).substr(i, 17)));
  }
  const auto received = receive_response(client);
  DOBA_EXPECT(received.has_value());
  DOBA_EXPECT_EQUAL(*received, body);
  client.socket.close();
  http_server.stop();
}

// +===========================================================================+
// | [>] tls orders HTTP responses before closing                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls serves sequential HTTP requests before closing") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  server<request, response,
         martianlabs::doba::protocol::http::router<request, response>,
         martianlabs::doba::protocol::http::v11::engine<request, response>, tls>
      http_server(server_policies(port));
  http_server.add_route("GET", "/one", [](const request&, response& res) {
    res.ok_200();
    res.set_body("one");
    return;
  });
  http_server.add_route("GET", "/two", [](const request&, response& res) {
    res.ok_200();
    res.set_body("two");
    return;
  });
  http_server.add_route("GET", "/three", [](const request&, response& res) {
    res.ok_200();
    res.set_body("three");
    return;
  });
  http_server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  DOBA_EXPECT(client.send("GET /one HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto first = receive_response(client);
  DOBA_EXPECT(first.has_value());
  DOBA_EXPECT_EQUAL(*first, "one");
  DOBA_EXPECT(client.send("GET /two HTTP/1.1\r\nHost: example.com\r\n\r\n"));
  const auto second = receive_response(client);
  DOBA_EXPECT(second.has_value());
  DOBA_EXPECT_EQUAL(*second, "two");
  DOBA_EXPECT(
      client.send("GET /three HTTP/1.1\r\nHost: example.com\r\n"
                  "Connection: close\r\n\r\n"));
  const auto third = receive_response(client);
  DOBA_EXPECT(third.has_value());
  DOBA_EXPECT_EQUAL(*third, "three");
  DOBA_EXPECT(client.receive_close());
  client.socket.close();
  http_server.stop();
}

#endif
