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


#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include "platform.h"
#include <openssl/ssl.h>

#include "protocol/send_delegate.h"
#include "tcpip_client.h"
#include "tls_client.h"
#include "tls_server_policies.h"
#include "test_helper.h"
#include "transport/server/tls.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] general                                                    ( usings ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
namespace tr = martianlabs::doba::transport::server;
using martianlabs::doba::tests::integration::server_policies;
using martianlabs::doba::tests::integration::tcpip_client;
using martianlabs::doba::tests::integration::tls_client;
using namespace std::chrono_literals;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] byte_engine                                                ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct byte_engine {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  martianlabs::doba::protocol::send_delegate send;
  std::function<void()> close;
  bool split{false};
  bool close_after{false};
  bool fail_source{false};
  std::atomic<bool>* failed_source_received{nullptr};
  std::string body;
  std::function<std::size_t(byte_engine&, const char*, std::size_t,
                            std::size_t)>
      receive;
  std::function<void(const martianlabs::doba::protocol::send_delegate&)>
      capture;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(martianlabs::doba::protocol::send_delegate value) {
    send = std::move(value);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> value) { close = std::move(value); }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  std::size_t on_bytes_received(const char* bytes, std::size_t size,
                                std::size_t capacity) {
    if (receive) return receive(*this, bytes, size, capacity);
    if (capture) {
      capture(send);
      return size;
    }
    if (fail_source) {
      if (failed_source_received) failed_source_received->store(true);
      martianlabs::doba::common::reader source(
          martianlabs::doba::common::filesystem_file{});
      std::byte byte{};
      source.read(std::span(&byte, 1));
      send("pre", {},
           std::make_unique<martianlabs::doba::common::reader>(
               std::move(source)));
      return size;
    }
    if (!body.empty()) {
      send("x", {},
           std::make_unique<martianlabs::doba::common::reader>(
               martianlabs::doba::common::reader::borrowed(
                   std::as_bytes(std::span(body)))));
      return size;
    }
    if (split) {
      for (std::size_t i = 0; i < size; ++i) {
        send(std::string(1, bytes[i]), {}, nullptr);
      }
      return size;
    }
    send(std::string(bytes, size), {}, nullptr);
    if (close_after) close();
    return size;
  }
};
}  // namespace

// +===========================================================================+
// | [>] tls exchanges encrypted bytes on loopback               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls exchanges encrypted bytes on loopback") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() { return byte_engine{}; };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  DOBA_EXPECT(client.send("hello"));
  const auto response = client.receive(5);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "hello");
  client.socket.close();
  server.stop();
}

// +===========================================================================+
// | [>] tls quiesces and restarts on the same port              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls quiesces and restarts on the same port") {
  tcpip_client probe;
  const auto port = probe.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() { return byte_engine{}; };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  std::atomic<int> connected{0};
  std::atomic<int> disconnected{0};
  server.set_on_connection([&]() { connected++; });
  server.set_on_disconnection([&]() { disconnected++; });
  for (int iteration = 1; iteration <= 2; iteration++) {
    tls_client client;
    server.start();
    DOBA_EXPECT(client.socket.connect(port));
    DOBA_EXPECT(client.negotiate());
    DOBA_EXPECT(client.send("ready"));
    const auto response = client.receive(5);
    DOBA_EXPECT(response.has_value());
    if (response) DOBA_EXPECT_EQUAL(*response, "ready");
    server.quiesce();
    client.socket.close();
    server.stop();
    DOBA_EXPECT_EQUAL(connected.load(), iteration);
    DOBA_EXPECT_EQUAL(disconnected.load(), iteration);
  }
}

// +===========================================================================+
// | [>] tls accepts fragmented handshake and request            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls accepts fragmented handshake and request") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() { return byte_engine{}; };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate(3));
  DOBA_EXPECT(client.send("fragmented", 2));
  const auto response = client.receive(10);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "fragmented");
  client.socket.close();
  server.stop();
}

// +===========================================================================+
// | [>] tls retains unconsumed plaintext                        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls retains unconsumed plaintext") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() {
    byte_engine engine;
    engine.receive = [](byte_engine& value, const char* bytes, std::size_t size,
                        std::size_t) {
      if (size < 4) return std::size_t{0};
      value.send(std::string(bytes, 4), {}, nullptr);
      return std::size_t{4};
    };
    return engine;
  };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  const std::string first("ab\0cde", 6);
  DOBA_EXPECT(client.send(first, 2));
  const auto response = client.receive(4);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, first.substr(0, 4));
  DOBA_EXPECT(client.send("fg", 2));
  const auto next = client.receive(4);
  DOBA_EXPECT(next.has_value());
  DOBA_EXPECT_EQUAL(*next, "defg");
  client.socket.close();
  server.stop();
}

// +===========================================================================+
// | [>] tls receives binary data beyond one encrypted buffer    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls receives binary data beyond one encrypted buffer") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() { return byte_engine{}; };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  std::string bytes(32 * 1024 + 13, '\0');
  for (std::size_t i = 0; i < bytes.size(); ++i) {
    bytes[i] = static_cast<char>(i % 251);
  }
  DOBA_EXPECT(client.send(bytes));
  const auto response = client.receive(bytes.size());
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, bytes);
  client.socket.close();
  server.stop();
}

DOBA_TEST("tls uses configured encrypted and BIO capacities") {
  auto configuration = server_policies(8443);
  configuration.encrypted_receive_buffer_size = 8192;
  configuration.network_bio_buffer_size = 4096;
  auto factory = []() { return byte_engine{}; };
  tr::tls_connection<byte_engine> connection(
      4096, 65536, factory, tr::make_tls_context(configuration),
      configuration.encrypted_receive_buffer_size,
      configuration.network_bio_buffer_size);
  DOBA_EXPECT_EQUAL(connection.capacity, 8192);
  for (const bool encrypted : {false, true}) {
    auto invalid = configuration;
    if (encrypted) {
      invalid.encrypted_receive_buffer_size = 0;
    } else {
      invalid.network_bio_buffer_size = 0;
    }
    bool rejected = false;
    try {
      tr::make_tls_context(invalid);
    } catch (const std::runtime_error&) {
      rejected = true;
    }
    DOBA_EXPECT(rejected);
  }
}

// +===========================================================================+
// | [>] tls closes when unconsumed plaintext fills the buffer   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls closes when unconsumed plaintext fills the buffer") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() {
    byte_engine engine;
    engine.receive = [](byte_engine&, const char*, std::size_t, std::size_t) {
      return std::size_t{0};
    };
    return engine;
  };
  auto configuration = server_policies(port);
  configuration.recv_buffer_size = 16;
  tr::tls<byte_engine, decltype(factory)> server(configuration, factory);
  std::atomic<int> connected{0};
  std::atomic<int> disconnected{0};
  server.set_on_connection([&]() { ++connected; });
  server.set_on_disconnection([&]() { ++disconnected; });
  server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  std::size_t written = 0;
  DOBA_EXPECT(
      SSL_write_ex(client.ssl.get(), "xxxxxxxxxxxxxxxx", 16, &written) == 1);
  DOBA_EXPECT_EQUAL(written, 16);
  std::array<char, 128> bytes;
  std::size_t size = 0;
  DOBA_EXPECT(BIO_read_ex(client.network.get(), bytes.data(), bytes.size(),
                          &size) == 1);
  DOBA_EXPECT(client.socket.send_all(std::string_view(bytes.data(), size)));
  DOBA_EXPECT(client.socket.wait_for_close(3s));
  client.socket.close();
  server.stop();
  DOBA_EXPECT_EQUAL(connected.load(), 1);
  DOBA_EXPECT_EQUAL(disconnected.load(), 1);
}

// +===========================================================================+
// | [>] tls isolates invalid engine input                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls isolates invalid engine input") {
  for (const bool throwing : {false, true}) {
    tls_client failed;
    const auto port = failed.socket.find_available_port();
    DOBA_EXPECT(port != 0);
    std::atomic<int> created{0};
    auto factory = [&]() {
      byte_engine engine;
      if (created++ == 0) {
        engine.receive = [throwing](byte_engine&, const char*, std::size_t size,
                                    std::size_t) -> std::size_t {
          if (throwing) throw std::runtime_error("receive failed");
          return size + 1;
        };
      }
      return engine;
    };
    tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                   factory);
    std::atomic<int> connected{0};
    std::atomic<int> disconnected{0};
    server.set_on_connection([&]() { ++connected; });
    server.set_on_disconnection([&]() { ++disconnected; });
    server.start();
    DOBA_EXPECT(failed.socket.connect(port));
    DOBA_EXPECT(failed.negotiate());
    std::size_t written = 0;
    DOBA_EXPECT(SSL_write_ex(failed.ssl.get(), "x", 1, &written) == 1);
    DOBA_EXPECT_EQUAL(written, 1);
    std::array<char, 128> bytes;
    std::size_t size = 0;
    DOBA_EXPECT(BIO_read_ex(failed.network.get(), bytes.data(), bytes.size(),
                            &size) == 1);
    DOBA_EXPECT(failed.socket.send_all(std::string_view(bytes.data(), size)));
    DOBA_EXPECT(failed.socket.wait_for_close(3s));
    failed.socket.close();
    tls_client healthy;
    DOBA_EXPECT(healthy.socket.connect(port));
    DOBA_EXPECT(healthy.negotiate());
    DOBA_EXPECT(healthy.send("ok"));
    const auto response = healthy.receive(2);
    DOBA_EXPECT(response.has_value());
    DOBA_EXPECT_EQUAL(*response, "ok");
    healthy.socket.close();
    server.stop();
    DOBA_EXPECT_EQUAL(connected.load(), 2);
    DOBA_EXPECT_EQUAL(disconnected.load(), 2);
  }
}

// +===========================================================================+
// | [>] tls preserves queued response order                     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls preserves queued response order") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() {
    byte_engine engine;
    engine.split = true;
    return engine;
  };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  DOBA_EXPECT(client.send("abcdef"));
  const auto response = client.receive(6);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "abcdef");
  client.socket.close();
  server.stop();
}

// +===========================================================================+
// | [>] tls streams a body beyond the send buffer               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls streams a body beyond the send buffer") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() {
    byte_engine engine;
    engine.body.assign(50000, 'b');
    return engine;
  };
  auto configuration = server_policies(port);
  configuration.max_send_buffer_size = 4096;
  tr::tls<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  server.start();
  DOBA_EXPECT(client.socket.set_receive_buffer_size(1024));
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  DOBA_EXPECT(client.send("request"));
  const auto response = client.receive(50001);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "x" + std::string(50000, 'b'));
  client.socket.close();
  server.stop();
}

// +===========================================================================+
// | [>] tls accepts concurrent response producers               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls accepts concurrent response producers") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  martianlabs::doba::protocol::send_delegate output;
  std::atomic<bool> ready{false};
  auto factory = [&]() {
    byte_engine engine;
    engine.capture = [&](const auto& value) {
      output = value;
      ready.store(true);
    };
    return engine;
  };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  DOBA_EXPECT(client.send("x"));
  const auto deadline = std::chrono::steady_clock::now() + 3s;
  while (!ready.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::yield();
  }
  DOBA_EXPECT(ready.load());
  std::array<std::jthread, 2> producers;
  for (std::size_t i = 0; i < producers.size(); ++i) {
    producers[i] = std::jthread([output, i]() {
      for (int item = 0; item < 100; ++item) {
        output(std::string(1, static_cast<char>('a' + i)), {}, nullptr);
      }
    });
  }
  for (auto& producer : producers) producer.join();
  const auto response = client.receive(200);
  DOBA_EXPECT(response.has_value());
  if (response) {
    DOBA_EXPECT_EQUAL((std::count)(response->begin(), response->end(), 'a'),
                      100);
    DOBA_EXPECT_EQUAL((std::count)(response->begin(), response->end(), 'b'),
                      100);
  }
  client.socket.close();
  server.stop();
}

// +===========================================================================+
// | [>] tls closes after a failed response source               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls closes after a failed response source") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<bool> received{false};
  auto factory = [&received]() {
    byte_engine engine;
    engine.fail_source = true;
    engine.failed_source_received = &received;
    return engine;
  };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  // The expected abort may race with the client's final TLS exchange.
  client.send("x");
  const auto response = client.receive(4);
  DOBA_EXPECT(received.load());
  DOBA_EXPECT(!response.has_value());
  DOBA_EXPECT(client.socket.error() != "timeout");
  client.socket.close();
  server.stop();
}

// +===========================================================================+
// | [>] tls sends close notify after queued output              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls sends close notify after queued output") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() {
    byte_engine engine;
    engine.close_after = true;
    return engine;
  };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  std::atomic<int> connected{0};
  std::atomic<int> disconnected{0};
  server.set_on_connection([&]() { ++connected; });
  server.set_on_disconnection([&]() { ++disconnected; });
  server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT_EQUAL(connected.load(), 0);
  DOBA_EXPECT(client.negotiate());
  DOBA_EXPECT(client.send("bye"));
  const auto response = client.receive(3);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "bye");
  DOBA_EXPECT(client.receive_close());
  DOBA_EXPECT_EQUAL(connected.load(), 1);
  client.socket.close();
  server.stop();
  DOBA_EXPECT_EQUAL(disconnected.load(), 1);
}

// +===========================================================================+
// | [>] tls answers peer close notify                           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls answers peer close notify") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() { return byte_engine{}; };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  std::atomic<int> disconnected{0};
  server.set_on_connection([]() {});
  server.set_on_disconnection([&]() { ++disconnected; });
  server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  DOBA_EXPECT_EQUAL(SSL_shutdown(client.ssl.get()), 0);
  DOBA_EXPECT(client.exchange());
  DOBA_EXPECT(client.receive_close());
  client.socket.close();
  server.stop();
  DOBA_EXPECT_EQUAL(disconnected.load(), 1);
}

// +===========================================================================+
// | [>] tls aborts on peer EOF without close notify             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls aborts on peer EOF without close notify") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() { return byte_engine{}; };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  std::atomic<int> connected{0};
  std::atomic<int> disconnected{0};
  server.set_on_connection([&]() { ++connected; });
  server.set_on_disconnection([&]() { ++disconnected; });
  server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  DOBA_EXPECT(client.send("x"));
  DOBA_EXPECT(client.receive(1).has_value());
  client.socket.close();
  server.stop();
  DOBA_EXPECT_EQUAL(connected.load(), 1);
  DOBA_EXPECT_EQUAL(disconnected.load(), 1);
}

// +===========================================================================+
// | [>] tls omits callbacks for incomplete negotiation          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls omits callbacks for incomplete negotiation") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() { return byte_engine{}; };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  std::atomic<int> connected{0};
  std::atomic<int> disconnected{0};
  server.set_on_connection([&]() { ++connected; });
  server.set_on_disconnection([&]() { ++disconnected; });
  server.start();
  DOBA_EXPECT(client.socket.connect(port));
  const int result = SSL_do_handshake(client.ssl.get());
  DOBA_EXPECT_EQUAL(SSL_get_error(client.ssl.get(), result),
                    SSL_ERROR_WANT_READ);
  std::array<char, 16> bytes;
  std::size_t size = 0;
  DOBA_EXPECT(BIO_read_ex(client.network.get(), bytes.data(), bytes.size(),
                          &size) == 1);
  DOBA_EXPECT(size > 0);
  DOBA_EXPECT(client.socket.send_all(std::string_view(bytes.data(), 1)));
  server.stop();
  DOBA_EXPECT_EQUAL(connected.load(), 0);
  DOBA_EXPECT_EQUAL(disconnected.load(), 0);
}

// +===========================================================================+
// | [>] tls survives an invalid handshake                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls survives an invalid handshake") {
  tcpip_client invalid;
  const auto port = invalid.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() { return byte_engine{}; };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  std::atomic<int> connected{0};
  std::atomic<int> disconnected{0};
  server.set_on_connection([&]() { ++connected; });
  server.set_on_disconnection([&]() { ++disconnected; });
  server.start();
  DOBA_EXPECT(invalid.connect(port));
  DOBA_EXPECT(invalid.send_all("GET / HTTP/1.1\r\n\r\n"));
  DOBA_EXPECT(invalid.wait_for_close(3s));
  invalid.close();
  tls_client healthy;
  DOBA_EXPECT(healthy.socket.connect(port));
  DOBA_EXPECT(healthy.negotiate());
  DOBA_EXPECT(healthy.send("ok"));
  const auto response = healthy.receive(2);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "ok");
  healthy.socket.close();
  server.stop();
  DOBA_EXPECT_EQUAL(connected.load(), 1);
  DOBA_EXPECT_EQUAL(disconnected.load(), 1);
}

// +===========================================================================+
// | [>] tls rejects an oversized record on the wire             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls rejects an oversized record on the wire") {
  tcpip_client invalid;
  const auto port = invalid.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() { return byte_engine{}; };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  std::atomic<int> connected{0};
  std::atomic<int> disconnected{0};
  server.set_on_connection([&]() { ++connected; });
  server.set_on_disconnection([&]() { ++disconnected; });
  server.start();
  DOBA_EXPECT(invalid.connect(port));
  DOBA_EXPECT(invalid.send_all(std::string_view("\x16\x03\x01\xff\xff", 5)));
  const auto rejected = invalid.receive_until_close(1024, 3s);
  DOBA_EXPECT(rejected.has_value() || invalid.error() == "socket");
  invalid.close();
  tls_client healthy;
  DOBA_EXPECT(healthy.socket.connect(port));
  DOBA_EXPECT(healthy.negotiate());
  DOBA_EXPECT(healthy.send("ok"));
  const auto response = healthy.receive(2);
  DOBA_EXPECT(response.has_value());
  if (response) DOBA_EXPECT_EQUAL(*response, "ok");
  healthy.socket.close();
  server.stop();
  DOBA_EXPECT_EQUAL(connected.load(), 1);
  DOBA_EXPECT_EQUAL(disconnected.load(), 1);
}

// +===========================================================================+
// | [>] tls stop drains queued output                           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls stop drains queued output") {
  tls_client client;
  const auto port = client.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  martianlabs::doba::protocol::send_delegate output;
  std::atomic<bool> ready{false};
  auto factory = [&]() {
    byte_engine engine;
    engine.capture = [&](const auto& value) {
      output = value;
      ready.store(true);
    };
    return engine;
  };
  auto configuration = server_policies(port);
  configuration.max_send_buffer_size = 65536;
  tr::tls<byte_engine, decltype(factory)> server(configuration, factory);
  std::atomic<int> connected{0};
  std::atomic<int> disconnected{0};
  server.set_on_connection([&]() { ++connected; });
  server.set_on_disconnection([&]() { ++disconnected; });
  server.start();
  DOBA_EXPECT(client.socket.connect(port));
  DOBA_EXPECT(client.negotiate());
  DOBA_EXPECT(client.send("request"));
  const auto deadline = std::chrono::steady_clock::now() + 3s;
  while (!ready.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::yield();
  }
  DOBA_EXPECT(ready.load());
  output(std::string(50000, 'x'), {}, nullptr);
  std::jthread stopping([&]() { server.stop(); });
  const auto response = client.receive(50000);
  DOBA_EXPECT(response.has_value());
  if (response) DOBA_EXPECT_EQUAL(*response, std::string(50000, 'x'));
  DOBA_EXPECT(client.receive_close());
  client.socket.close();
  stopping.join();
  DOBA_EXPECT_EQUAL(connected.load(), 1);
  DOBA_EXPECT_EQUAL(disconnected.load(), 1);
}

// +===========================================================================+
// | [>] tls restarts after stopping                             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tls restarts after a stopped connection") {
  tls_client first;
  const auto port = first.socket.find_available_port();
  DOBA_EXPECT(port != 0);
  auto factory = []() { return byte_engine{}; };
  tr::tls<byte_engine, decltype(factory)> server(server_policies(port),
                                                 factory);
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  server.start();
  DOBA_EXPECT(first.socket.connect(port));
  DOBA_EXPECT(first.negotiate());
  first.socket.close();
  server.stop();
  tls_client second;
  server.start();
  DOBA_EXPECT(second.socket.connect(port));
  DOBA_EXPECT(second.negotiate());
  DOBA_EXPECT(second.send("again"));
  const auto response = second.receive(5);
  DOBA_EXPECT(response.has_value());
  DOBA_EXPECT_EQUAL(*response, "again");
  second.socket.close();
  server.stop();
}
