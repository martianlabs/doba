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

#ifdef DOBA_ENABLE_TLS

#include <algorithm>
#include <array>
#include <filesystem>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>

#include "test_helper.h"
#include "transport/server/tls_session.h"

namespace {
using martianlabs::doba::transport::server::make_tls_context;
using martianlabs::doba::transport::server::tls_policies;
using martianlabs::doba::transport::server::tls_session;

std::string fixture(const char* name) {
  return (std::filesystem::path(__FILE__).parent_path() / "fixtures" / name)
      .string();
}

bool rejects(const tls_policies& configuration) {
  try {
    make_tls_context(configuration);
  } catch (const std::runtime_error&) {
    return true;
  }
  return false;
}

struct test_client {
  test_client()
      : context{SSL_CTX_new(TLS_client_method()), SSL_CTX_free},
        ssl{context ? SSL_new(context.get()) : nullptr, SSL_free},
        network{nullptr, BIO_free} {
    if (!ssl) throw std::runtime_error("TLS client could not be created!");
    BIO* internal = nullptr;
    BIO* external = nullptr;
    if (BIO_new_bio_pair(&internal, 0, &external, 0) != 1) {
      throw std::runtime_error("TLS client buffers could not be created!");
    }
    SSL_set_bio(ssl.get(), internal, internal);
    network.reset(external);
    SSL_set_connect_state(ssl.get());
  }

  std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)> context;
  std::unique_ptr<SSL, decltype(&SSL_free)> ssl;
  std::unique_ptr<BIO, decltype(&BIO_free)> network;
};

tls_policies server_policies() {
  tls_policies configuration;
  configuration.certificate_file = fixture("server.crt");
  configuration.private_key_file = fixture("server.key");
  return configuration;
}

bool exchange(tls_session& server, test_client& client,
              std::size_t fragment) {
  std::array<char, 4096> bytes;
  while (BIO_ctrl_pending(client.network.get())) {
    std::size_t size = 0;
    if (BIO_read_ex(client.network.get(), bytes.data(),
                    (std::min)(fragment, bytes.size()), &size) != 1) {
      return false;
    }
    if (server.receive(std::span(bytes.data(), size)) != size) return false;
  }
  while (server.pending()) {
    const std::size_t size =
        server.drain(std::span(bytes).first(fragment));
    if (!size) return false;
    std::size_t written = 0;
    if (BIO_write_ex(client.network.get(), bytes.data(), size,
                     &written) != 1 || written != size) {
      return false;
    }
  }
  return true;
}

bool negotiate(tls_session& server, test_client& client,
               std::size_t fragment) {
  for (int step = 0; step < 64; ++step) {
    const int value = SSL_do_handshake(client.ssl.get());
    if (value != 1) {
      const int error = SSL_get_error(client.ssl.get(), value);
      if (error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE) {
        return false;
      }
    }
    if (server.handshake() == tls_session::status::failed) return false;
    if (!exchange(server, client, fragment)) return false;
    if (server.established() && SSL_is_init_finished(client.ssl.get())) {
      return true;
    }
  }
  return false;
}
}  // namespace

// +===========================================================================+
// | [>] tls context loads matching credentials                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("tls context loads matching credentials") {
  tls_policies configuration;
  configuration.certificate_file = fixture("server.crt");
  configuration.private_key_file = fixture("server.key");
  auto context = make_tls_context(configuration);
  DOBA_EXPECT(context != nullptr);
  DOBA_EXPECT(SSL_CTX_get0_certificate(context.get()) != nullptr);
}

// +===========================================================================+
// | [>] tls context requires both credential paths              ( test-case ) |
// +===========================================================================+
DOBA_TEST("tls context requires both credential paths") {
  tls_policies configuration;
  DOBA_EXPECT(rejects(configuration));
  configuration.certificate_file = fixture("server.crt");
  DOBA_EXPECT(rejects(configuration));
  configuration.certificate_file.clear();
  configuration.private_key_file = fixture("server.key");
  DOBA_EXPECT(rejects(configuration));
}

// +===========================================================================+
// | [>] tls context rejects missing credential files            ( test-case ) |
// +===========================================================================+
DOBA_TEST("tls context rejects missing credential files") {
  tls_policies configuration;
  configuration.certificate_file = fixture("missing.crt");
  configuration.private_key_file = fixture("server.key");
  DOBA_EXPECT(rejects(configuration));
  configuration.certificate_file = fixture("server.crt");
  configuration.private_key_file = fixture("missing.key");
  DOBA_EXPECT(rejects(configuration));
}

// +===========================================================================+
// | [>] tls context rejects a mismatched private key            ( test-case ) |
// +===========================================================================+
DOBA_TEST("tls context rejects a mismatched private key") {
  tls_policies configuration;
  configuration.certificate_file = fixture("server.crt");
  configuration.private_key_file = fixture("other.key");
  DOBA_EXPECT(rejects(configuration));
}

// +===========================================================================+
// | [>] tls session negotiates fragmented input                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("tls session negotiates fragmented input") {
  tls_session server(make_tls_context(server_policies()), 32768);
  test_client client;
  DOBA_EXPECT_EQUAL(server.handshake(), tls_session::status::need_input);
  const int value = SSL_do_handshake(client.ssl.get());
  DOBA_EXPECT_EQUAL(SSL_get_error(client.ssl.get(), value),
                    SSL_ERROR_WANT_READ);
  DOBA_EXPECT(exchange(server, client, 7));
  DOBA_EXPECT_EQUAL(server.handshake(), tls_session::status::need_output);
  DOBA_EXPECT(server.pending() > 0);
  DOBA_EXPECT(negotiate(server, client, 7));
  DOBA_EXPECT(server.established());
  DOBA_EXPECT(!server.failed());
}

// +===========================================================================+
// | [>] tls session exchanges application data                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("tls session exchanges application data") {
  tls_session server(make_tls_context(server_policies()), 32768);
  test_client client;
  DOBA_EXPECT(negotiate(server, client, 1024));
  std::size_t written = 0;
  DOBA_EXPECT(SSL_write_ex(client.ssl.get(), "request", 7,
                           &written) == 1);
  DOBA_EXPECT_EQUAL(written, 7);
  DOBA_EXPECT(exchange(server, client, 3));
  std::array<char, 32> bytes;
  const auto input = server.read(bytes);
  DOBA_EXPECT_EQUAL(input.state, tls_session::status::ready);
  DOBA_EXPECT_EQUAL(std::string(bytes.data(), input.size), "request");
  const auto output = server.write(std::span("response", 8));
  DOBA_EXPECT_EQUAL(output.state, tls_session::status::ready);
  DOBA_EXPECT_EQUAL(output.size, 8);
  DOBA_EXPECT(exchange(server, client, 3));
  std::size_t received = 0;
  DOBA_EXPECT(SSL_read_ex(client.ssl.get(), bytes.data(), bytes.size(),
                          &received) == 1);
  DOBA_EXPECT_EQUAL(std::string(bytes.data(), received), "response");
}

// +===========================================================================+
// | [>] tls session bounds encrypted input                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("tls session bounds encrypted input") {
  tls_session server(make_tls_context(server_policies()), 32768);
  const std::string bytes(64 * 1024, 'x');
  const std::size_t received = server.receive(bytes);
  DOBA_EXPECT(received > 0);
  DOBA_EXPECT(received <= 17 * 1024);
  DOBA_EXPECT_EQUAL(server.receive(bytes), 0);
  DOBA_EXPECT(!server.failed());
}

// +===========================================================================+
// | [>] tls session bounds encrypted output                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("tls session bounds encrypted output") {
  tls_session server(make_tls_context(server_policies()), 4096);
  test_client client;
  DOBA_EXPECT(negotiate(server, client, 1024));
  std::array<char, 1024> bytes{};
  DOBA_EXPECT_EQUAL(server.read(bytes).state,
                    tls_session::status::need_input);
  bool blocked = false;
  for (int step = 0; step < 16; ++step) {
    const auto result = server.write(bytes);
    DOBA_EXPECT(result.state != tls_session::status::failed);
    DOBA_EXPECT(server.pending() <= 4096);
    if (result.state == tls_session::status::need_output) {
      blocked = true;
      break;
    }
  }
  DOBA_EXPECT(blocked);
  DOBA_EXPECT(exchange(server, client, 1024));
  DOBA_EXPECT_EQUAL(server.write(bytes).state, tls_session::status::ready);
}

// +===========================================================================+
// | [>] tls session encrypts a large body in segments           ( test-case ) |
// +===========================================================================+
DOBA_TEST("tls session encrypts a large body in segments") {
  tls_session server(make_tls_context(server_policies()), 4096);
  test_client client;
  DOBA_EXPECT(negotiate(server, client, 1024));
  const std::string body(32 * 1024, 'x');
  std::string received;
  std::array<char, 4096> bytes;
  std::size_t offset = 0;
  for (int step = 0; step < 128 && received.size() < body.size();
       ++step) {
    if (offset < body.size()) {
      const auto result = server.write(std::span(
          body.data() + offset, body.size() - offset));
      DOBA_EXPECT(result.state != tls_session::status::failed);
      offset += result.size;
    }
    DOBA_EXPECT(server.pending() <= 4096);
    while (server.pending()) {
      const std::size_t size = server.drain(bytes);
      DOBA_EXPECT(size > 0);
      std::size_t written = 0;
      DOBA_EXPECT(BIO_write_ex(client.network.get(), bytes.data(), size,
                               &written) == 1);
      DOBA_EXPECT_EQUAL(written, size);
    }
    while (true) {
      std::size_t size = 0;
      const int value = SSL_read_ex(client.ssl.get(), bytes.data(),
                                    bytes.size(), &size);
      if (value == 1) {
        received.append(bytes.data(), size);
        continue;
      }
      DOBA_EXPECT_EQUAL(SSL_get_error(client.ssl.get(), value),
                        SSL_ERROR_WANT_READ);
      break;
    }
  }
  DOBA_EXPECT_EQUAL(offset, body.size());
  DOBA_EXPECT_EQUAL(received, body);
}

// +===========================================================================+
// | [>] tls session sends close notify                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("tls session sends close notify") {
  tls_session server(make_tls_context(server_policies()), 32768);
  test_client client;
  DOBA_EXPECT(negotiate(server, client, 1024));
  DOBA_EXPECT_EQUAL(server.shutdown(), tls_session::status::need_output);
  DOBA_EXPECT(exchange(server, client, 1024));
  std::array<char, 32> bytes;
  std::size_t received = 0;
  const int result = SSL_read_ex(client.ssl.get(), bytes.data(),
                                  bytes.size(), &received);
  DOBA_EXPECT_EQUAL(SSL_get_error(client.ssl.get(), result),
                    SSL_ERROR_ZERO_RETURN);
  DOBA_EXPECT_EQUAL(server.shutdown(), tls_session::status::ready);
  DOBA_EXPECT_EQUAL(server.write(std::span("x", 1)).state,
                    tls_session::status::failed);
}

// +===========================================================================+
// | [>] tls session receives close notify                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("tls session receives close notify") {
  tls_session server(make_tls_context(server_policies()), 32768);
  test_client client;
  DOBA_EXPECT(negotiate(server, client, 1024));
  DOBA_EXPECT_EQUAL(SSL_shutdown(client.ssl.get()), 0);
  DOBA_EXPECT(exchange(server, client, 1024));
  std::array<char, 32> bytes;
  DOBA_EXPECT_EQUAL(server.read(bytes).state,
                    tls_session::status::closed);
  DOBA_EXPECT_EQUAL(server.shutdown(), tls_session::status::need_output);
  DOBA_EXPECT(exchange(server, client, 1024));
  DOBA_EXPECT_EQUAL(SSL_shutdown(client.ssl.get()), 1);
}

// +===========================================================================+
// | [>] tls session rejects invalid negotiation                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("tls session rejects invalid negotiation") {
  tls_session server(make_tls_context(server_policies()), 32768);
  DOBA_EXPECT_EQUAL(server.receive(std::span("GET / HTTP/1.1\r\n", 16)),
                    16);
  DOBA_EXPECT_EQUAL(server.handshake(), tls_session::status::failed);
  DOBA_EXPECT(server.failed());
  DOBA_EXPECT_EQUAL(server.handshake(), tls_session::status::failed);
}

#endif
