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

#include <array>
#include <atomic>
#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <thread>

#include "common/output.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "transport/server/tcpip.h"

namespace {
namespace tr = martianlabs::doba::transport::server;
using martianlabs::doba::tests::integration::tcpip_client;
using namespace std::chrono_literals;
struct byte_engine;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] byte_state                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct byte_state {
  std::function<std::size_t(byte_engine&, const char*, std::size_t,
                            std::size_t)> receive;
  std::atomic<int> connected{0};
  std::atomic<int> disconnected{0};
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] byte_engine                                                ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct byte_engine {
  using policies_type = int;
  void set_on_send(martianlabs::doba::common::send_delegate value) {
    send = std::move(value);
  }
  void set_on_close(std::function<void()> value) { close = std::move(value); }
  void echo(const char* bytes, std::size_t size) {
    auto buffer = std::make_unique<char[]>(size);
    if (size) std::memcpy(buffer.get(), bytes, size);
    send(std::move(buffer), size, std::nullopt);
  }
  std::size_t on_bytes_received(const char* bytes, std::size_t size,
                                std::size_t capacity) {
    if (!size || size > capacity) throw std::runtime_error("Invalid input");
    if (state->receive) return state->receive(*this, bytes, size, capacity);
    echo(bytes, size);
    return size;
  }
  std::shared_ptr<byte_state> state;
  martianlabs::doba::common::send_delegate send;
  std::function<void()> close;
};
bool wait_count(const std::atomic<int>& count, int expected) {
  const auto deadline = std::chrono::steady_clock::now() + 3s;
  while (count.load() < expected && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::yield();
  }
  return count.load() == expected;
}
}  // namespace
// +===========================================================================+
// | [>] tcpip serves independent loopback connections           ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip serves independent loopback connections") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  server.start();
  std::array<tcpip_client, 4> clients;
  for (std::size_t i = 0; i < clients.size(); i++) {
    DOBA_EXPECT(clients[i].connect(port));
    DOBA_EXPECT(clients[i].send_all(std::string(1, static_cast<char>('a' + i))));
  }
  for (std::size_t i = 0; i < clients.size(); i++) {
    const auto bytes = clients[i].receive(1);
    DOBA_EXPECT(bytes.has_value());
    DOBA_EXPECT_EQUAL(*bytes, std::string(1, static_cast<char>('a' + i)));
    clients[i].close();
  }
  server.stop();
  DOBA_EXPECT_EQUAL(state->connected.load(), 4);
  DOBA_EXPECT_EQUAL(state->disconnected.load(), 4);
}
// +===========================================================================+
// | [>] tcpip reuses a connection after each completed send     ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip reuses a connection after each completed send") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  server.start();
  DOBA_EXPECT(client.connect(port));
  for (const auto data : {"one", "two", "three"}) {
    DOBA_EXPECT(client.send_all(data));
    const auto bytes = client.receive(std::strlen(data));
    DOBA_EXPECT(bytes.has_value());
    DOBA_EXPECT_EQUAL(*bytes, data);
  }
  client.close();
  server.stop();
}
// +===========================================================================+
// | [>] tcpip retains unconsumed fragments for the engine       ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip retains unconsumed fragments for the engine") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  state->receive = [](byte_engine& engine, const char* bytes, std::size_t size,
                       std::size_t) -> std::size_t {
    if (size < 4) return 0;
    engine.echo(bytes, 4);
    return 4;
  };
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("ab"));
  DOBA_EXPECT(!client.has_data(30ms));
  DOBA_EXPECT(client.send_all("cdef"));
  const auto first = client.receive(4);
  DOBA_EXPECT(first.has_value());
  DOBA_EXPECT_EQUAL(*first, "abcd");
  DOBA_EXPECT(!client.has_data(30ms));
  DOBA_EXPECT(client.send_all("gh"));
  const auto second = client.receive(4);
  DOBA_EXPECT(second.has_value());
  DOBA_EXPECT_EQUAL(*second, "efgh");
  client.close();
  server.stop();
}
// +===========================================================================+
// | [>] transport contract                                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip preserves binary bytes across receive boundaries") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  configuration.recv_buffer_size = 17;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  server.start();
  DOBA_EXPECT(client.connect(port));
  std::string data;
  for (int i = 0; i < 256; i++) data += static_cast<char>(i);
  DOBA_EXPECT(client.send_all(data));
  const auto bytes = client.receive(data.size());
  DOBA_EXPECT(bytes.has_value());
  DOBA_EXPECT_EQUAL(*bytes, data);
  client.close();
  server.stop();
}
// +===========================================================================+
// | [>] tcpip rejects invalid engine consumption counts         ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip rejects invalid engine consumption counts") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  state->receive = [](byte_engine&, const char*, std::size_t size,
                       std::size_t) { return size + 1; };
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("x"));
  DOBA_EXPECT(client.wait_for_close(3s));
  client.close();
  server.stop();
}
// +===========================================================================+
// | [>] transport contract                                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip closes when unconsumed input fills the receive buffer") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  configuration.recv_buffer_size = 16;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  state->receive = [](byte_engine&, const char*, std::size_t,
                       std::size_t) -> std::size_t { return 0; };
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(std::string(15, 'x')));
  DOBA_EXPECT(!client.has_data(30ms));
  DOBA_EXPECT(client.send_all("x"));
  DOBA_EXPECT(client.wait_for_close(3s));
  client.close();
  server.stop();
}
// +===========================================================================+
// | [>] tcpip drains an engine close delivery before eof        ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip drains an engine close delivery before eof") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  state->receive = [](byte_engine& engine, const char* bytes, std::size_t size,
                       std::size_t) {
    engine.echo(bytes, size);
    engine.close();
    return size;
  };
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("close"));
  const auto bytes = client.receive_until_close(5, 3s);
  DOBA_EXPECT(bytes.has_value());
  DOBA_EXPECT_EQUAL(*bytes, "close");
  client.close();
  server.stop();
}
// +===========================================================================+
// | [>] transport contract                                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip removes an empty delivery without blocking its queue") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  state->receive = [](byte_engine& engine, const char* bytes, std::size_t size,
                       std::size_t) {
    engine.echo(bytes, 0);
    engine.echo(bytes, size);
    return size;
  };
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("ok"));
  const auto bytes = client.receive(2);
  DOBA_EXPECT(bytes.has_value());
  DOBA_EXPECT_EQUAL(*bytes, "ok");
  client.close();
  server.stop();
}
// +===========================================================================+
// | [>] tcpip closes when a delivery exceeds the send limit     ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip closes when a delivery exceeds the send limit") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  configuration.send_buffer_size = 16;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(std::string(17, 'x')));
  DOBA_EXPECT(client.wait_for_close(3s));
  client.close();
  server.stop();
}
// +===========================================================================+
// | [>] tcpip isolates engine exceptions to the connection      ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip isolates engine exceptions to the connection") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  state->receive = [](byte_engine& engine, const char* bytes, std::size_t size,
                       std::size_t) {
    if (bytes[0] == '!') throw std::runtime_error("engine failed");
    engine.echo(bytes, size);
    return size;
  };
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("!"));
  DOBA_EXPECT(client.wait_for_close(3s));
  client.close();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("ok"));
  const auto bytes = client.receive(2);
  DOBA_EXPECT(bytes.has_value());
  DOBA_EXPECT_EQUAL(*bytes, "ok");
  client.close();
  server.stop();
}
// +===========================================================================+
// | [>] tcpip drains bytes after the client half closes         ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip drains bytes after the client half closes") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("hello"));
  DOBA_EXPECT(client.shutdown_write());
  const auto bytes = client.receive_until_close(5, 3s);
  DOBA_EXPECT(bytes.has_value());
  DOBA_EXPECT_EQUAL(*bytes, "hello");
  client.close();
  server.stop();
}
// +===========================================================================+
// | [>] tcpip closes incomplete input after eof                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip closes incomplete input after eof") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  state->receive = [](byte_engine&, const char*, std::size_t,
                       std::size_t) -> std::size_t { return 0; };
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("partial"));
  DOBA_EXPECT(client.shutdown_write());
  DOBA_EXPECT(client.wait_for_close(3s));
  client.close();
  server.stop();
}
// +===========================================================================+
// | [>] tcpip restarts the same server on the same port         ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip restarts the same server on the same port") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  for (int i = 0; i < 3; i++) {
    server.start();
    server.start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("x"));
    const auto bytes = client.receive(1);
    DOBA_EXPECT(bytes.has_value());
    DOBA_EXPECT_EQUAL(*bytes, "x");
    client.close();
    server.stop();
    server.stop();
  }
  DOBA_EXPECT_EQUAL(state->connected.load(), 3);
  DOBA_EXPECT_EQUAL(state->disconnected.load(), 3);
}
// +===========================================================================+
// | [>] tcpip survives failing connection callbacks             ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip survives failing connection callbacks") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  server.set_on_connection([state]() {
    if (state->connected.fetch_add(1) == 0) {
      throw std::runtime_error("callback failed");
    }
  });
  server.start();
  DOBA_EXPECT(client.connect(port));
  const auto rejected = client.receive_until_close(1, 3s);
  DOBA_EXPECT(rejected.has_value());
  DOBA_EXPECT(rejected->empty());
  client.close();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("ok"));
  const auto bytes = client.receive(2);
  DOBA_EXPECT(bytes.has_value());
  DOBA_EXPECT_EQUAL(*bytes, "ok");
  client.close();
  server.stop();
  DOBA_EXPECT_EQUAL(state->connected.load(), 2);
}
// +===========================================================================+
// | [>] tcpip survives failing disconnection callbacks          ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip survives failing disconnection callbacks") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  server.set_on_disconnection([state]() {
    state->disconnected++;
    throw std::runtime_error("callback failed");
  });
  server.start();
  for (int i = 0; i < 2; i++) {
    DOBA_EXPECT(client.connect(port));
    client.close();
    DOBA_EXPECT(wait_count(state->disconnected, i + 1));
  }
  server.stop();
  DOBA_EXPECT_EQUAL(state->connected.load(), 2);
  DOBA_EXPECT_EQUAL(state->disconnected.load(), 2);
}
// +===========================================================================+
// | [>] tcpip rejects callback mutation while active            ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip rejects callback mutation while active") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  server.start();
  bool connection_rejected = false;
  bool disconnection_rejected = false;
  try { server.set_on_connection([]() {}); }
  catch (const std::runtime_error&) { connection_rejected = true; }
  try { server.set_on_disconnection([]() {}); }
  catch (const std::runtime_error&) { disconnection_rejected = true; }
  server.stop();
  DOBA_EXPECT(connection_rejected);
  DOBA_EXPECT(disconnection_rejected);
}
// +===========================================================================+
// | [>] tcpip drains a large ordered delivery sequence          ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip drains a large ordered delivery sequence") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  state->receive = [](byte_engine& engine, const char* bytes, std::size_t size,
                       std::size_t) {
    for (std::size_t i = 0; i < size; i++) engine.echo(bytes + i, 1);
    return size;
  };
  server.start();
  DOBA_EXPECT(client.connect(port));
  std::string data;
  for (int i = 0; i < 256; i++) data += static_cast<char>(i);
  DOBA_EXPECT(client.send_all(data));
  const auto bytes = client.receive(data.size());
  DOBA_EXPECT(bytes.has_value());
  DOBA_EXPECT_EQUAL(*bytes, data);
  client.close();
  server.stop();
}
// +===========================================================================+
// | [>] transport contract                                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip serves another client while a large delivery is blocked") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  configuration.send_buffer_size = 16 * 1024 * 1024;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  auto queued = std::make_shared<std::atomic<int>>(0);
  state->receive = [queued](byte_engine& engine, const char* bytes,
                            std::size_t size, std::size_t) {
    if (bytes[0] == 'L') {
      const std::string large(8 * 1024 * 1024, 'x');
      engine.echo(large.data(), large.size());
      queued->fetch_add(1);
    } else {
      engine.echo(bytes, size);
    }
    return size;
  };
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("L"));
  DOBA_EXPECT(wait_count(*queued, 1));
  tcpip_client other;
  DOBA_EXPECT(other.connect(port));
  DOBA_EXPECT(other.send_all("ok"));
  const auto bytes = other.receive(2);
  DOBA_EXPECT(bytes.has_value());
  DOBA_EXPECT_EQUAL(*bytes, "ok");
  client.abort();
  other.close();
  server.stop();
}
// +===========================================================================+
// | [>] transport contract                                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip recovers after a client resets a large delivery") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  configuration.send_buffer_size = 16 * 1024 * 1024;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  auto queued = std::make_shared<std::atomic<int>>(0);
  state->receive = [queued](byte_engine& engine, const char* bytes,
                            std::size_t size, std::size_t) {
    if (bytes[0] == 'L') {
      const std::string large(8 * 1024 * 1024, 'x');
      engine.echo(large.data(), large.size());
      queued->fetch_add(1);
    } else {
      engine.echo(bytes, size);
    }
    return size;
  };
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("L"));
  DOBA_EXPECT(wait_count(*queued, 1));
  client.abort();
  DOBA_EXPECT(wait_count(state->disconnected, 1));
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("ok"));
  const auto bytes = client.receive(2);
  DOBA_EXPECT(bytes.has_value());
  DOBA_EXPECT_EQUAL(*bytes, "ok");
  client.close();
  server.stop();
}
// +===========================================================================+
// | [>] tcpip recovers after a reset during byte reception      ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip recovers after a reset during byte reception") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  state->receive = [](byte_engine& engine, const char* bytes, std::size_t size,
                       std::size_t) -> std::size_t {
    if (bytes[0] == 'x') return 0;
    engine.echo(bytes, size);
    return size;
  };
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("x"));
  DOBA_EXPECT(wait_count(state->connected, 1));
  client.abort();
  DOBA_EXPECT(wait_count(state->disconnected, 1));
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("ok"));
  const auto bytes = client.receive(2);
  DOBA_EXPECT(bytes.has_value());
  DOBA_EXPECT_EQUAL(*bytes, "ok");
  client.close();
  server.stop();
}
// +===========================================================================+
// | [>] transport contract                                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip isolates interleaved fragments from concurrent clients") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([state]() { state->connected++; });
  server.set_on_disconnection([state]() { state->disconnected++; });
  state->receive = [](byte_engine& engine, const char* bytes, std::size_t size,
                       std::size_t) -> std::size_t {
    if (size < 4) return 0;
    engine.echo(bytes, 4);
    return 4;
  };
  server.start();
  std::array<tcpip_client, 4> clients;
  for (auto& connection : clients) DOBA_EXPECT(connection.connect(port));
  for (int fragment = 0; fragment < 4; fragment++) {
    for (std::size_t i = 0; i < clients.size(); i++) {
      DOBA_EXPECT(clients[i].send_all(std::string(1, static_cast<char>('a' + i))));
    }
  }
  for (std::size_t i = 0; i < clients.size(); i++) {
    const auto bytes = clients[i].receive(4);
    DOBA_EXPECT(bytes.has_value());
    DOBA_EXPECT_EQUAL(*bytes, std::string(4, static_cast<char>('a' + i)));
    clients[i].close();
  }
  server.stop();
}

// +===========================================================================+
// | [>] isolated empty deliveries                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip completes isolated empty deliveries") {
  for (const bool null_buffer : {false, true}) {
    tcpip_client client;
    const auto port = client.find_available_port();
    DOBA_EXPECT(port != 0);
    auto state = std::make_shared<byte_state>();
    auto factory = [state]() { return byte_engine{state, {}, {}}; };
    tr::policies configuration;
    configuration.ip = "127.0.0.1";
    configuration.port = std::to_string(port);
    configuration.worker_count = 2;
    configuration.send_buffer_size = 2;
    tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
    server.set_on_connection([]() {});
    server.set_on_disconnection([]() {});
    std::atomic<int> received{0};
    state->receive = [&](byte_engine& engine, const char* bytes,
                         std::size_t size, std::size_t) {
      if (bytes[0] == 'e') {
        for (int i = 0; i < 3; i++) {
          if (null_buffer) engine.send(nullptr, 0, std::nullopt);
          else engine.echo(bytes, 0);
        }
      } else {
        engine.echo("a", 1);
        engine.send(nullptr, 0, std::nullopt);
        engine.echo("b", 1);
        engine.send(nullptr, 0, std::nullopt);
        engine.close();
      }
      received++;
      return size;
    };
    server.start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("e"));
    DOBA_EXPECT(wait_count(received, 1));
    DOBA_EXPECT(client.send_all("x"));
    const auto bytes = client.receive_until_close(2, 3s);
    DOBA_EXPECT(bytes.has_value());
    if (bytes) DOBA_EXPECT_EQUAL(*bytes, "ab");
    client.close();
    server.stop();
  }
}
// +===========================================================================+
// | [>] rejected send notification                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip rejects invalid deliveries and ignores later output") {
  for (const int mode : {0, 1, 2}) {
    tcpip_client client;
    const auto port = client.find_available_port();
    DOBA_EXPECT(port != 0);
    auto state = std::make_shared<byte_state>();
    auto factory = [state]() { return byte_engine{state, {}, {}}; };
    tr::policies configuration;
    configuration.ip = "127.0.0.1";
    configuration.port = std::to_string(port);
    configuration.worker_count = 2;
    configuration.send_buffer_size = 16;
    tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
    server.set_on_connection([]() {});
    server.set_on_disconnection([]() {});
    state->receive = [&](byte_engine& engine, const char*, std::size_t size,
                         std::size_t) {
      if (mode == 1) engine.send(nullptr, 1, std::nullopt);
      else engine.echo("01234567890123456", 17);
      engine.echo("ignored", 7);
      engine.send(nullptr, 0, std::nullopt);
      if (mode == 2) throw std::runtime_error("After rejection");
      return size;
    };
    server.start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("x"));
    DOBA_EXPECT(client.wait_for_close(3s));
    client.close();
    server.stop();
    server.stop();
  }
}
// +===========================================================================+
// | [>] tcpip recovers receive space after partial consumption  ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip recovers receive space after partial consumption") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  std::atomic<int> full{0};
  std::atomic<int> invalid{0};
  state->receive = [&](byte_engine& engine, const char* bytes, std::size_t size,
                       std::size_t capacity) -> std::size_t {
    if (!size || size > capacity || capacity != 16) invalid++;
    if (!full.load()) {
      if (size < capacity) return 0;
      full++;
      engine.echo(bytes, 8);
      return 8;
    }
    if (size < 16) return 0;
    engine.echo(bytes, size);
    return size;
  };
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::tcpip<byte_engine, decltype(factory)> transport(
      {.recv_buffer_size = 16, .worker_count = 2, .ip = "127.0.0.1",
        .port = std::to_string(port)}, factory);
  transport.set_on_connection([state]() { state->connected++; });
  transport.set_on_disconnection([state]() { state->disconnected++; });
  transport.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("abcdefghijklmnop"));
  const auto first = client.receive(8);
  DOBA_EXPECT(first.has_value());
  if (first) DOBA_EXPECT_EQUAL(*first, "abcdefgh");
  DOBA_EXPECT(client.send_all("qrstuvwx"));
  const auto second = client.receive(16);
  DOBA_EXPECT(second.has_value());
  if (second) DOBA_EXPECT_EQUAL(*second, "ijklmnopqrstuvwx");
  client.close();
  transport.stop();
  DOBA_EXPECT_EQUAL(full.load(), 1);
  DOBA_EXPECT_EQUAL(invalid.load(), 0);
}
// +===========================================================================+
// | [>] tcpip batches output after the receive callback         ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip batches output after the receive callback") {
  for (const std::size_t count : {1U, 16U, 256U}) {
    tcpip_client client;
    const auto port = client.find_available_port();
    DOBA_EXPECT(port != 0);
    auto state = std::make_shared<byte_state>();
    std::atomic<int> queued{0};
    std::atomic<bool> release{false};
    const std::string payload(37, 'x');
    state->receive = [&](byte_engine& engine, const char*, std::size_t size,
                          std::size_t) {
      for (std::size_t i = 0; i < count; i++) {
        engine.echo(payload.data(), payload.size());
      }
      queued++;
      const auto deadline = std::chrono::steady_clock::now() + 5s;
      while (!release.load()) {
        if (std::chrono::steady_clock::now() >= deadline) {
          throw std::runtime_error("Test callback was not released");
        }
        std::this_thread::yield();
      }
      return size;
    };
    auto factory = [state]() { return byte_engine{state, {}, {}}; };
    tr::policies configuration;
    configuration.ip = "127.0.0.1";
    configuration.port = std::to_string(port);
    configuration.worker_count = 2;
    configuration.send_buffer_size = count * payload.size();
    tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
    server.set_on_connection([]() {});
    server.set_on_disconnection([]() {});
    server.start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("x"));
    const bool enqueued = wait_count(queued, 1);
    const bool early = client.has_data(50ms);
    release = true;
    const auto bytes = client.receive(count * payload.size(), 5s);
    client.close();
    server.stop();
    DOBA_EXPECT(enqueued);
    DOBA_EXPECT(!early);
    DOBA_EXPECT(bytes.has_value());
    if (bytes) {
      DOBA_EXPECT_EQUAL(*bytes, std::string(count * payload.size(), 'x'));
    }
  }
}
// +===========================================================================+
// | [>] tcpip accepts concurrent producers without new input    ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip accepts concurrent producers without new input") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  martianlabs::doba::common::send_delegate output;
  std::atomic<int> ready{0};
  state->receive = [&](byte_engine& engine, const char*, std::size_t size,
                        std::size_t) {
    output = engine.send;
    ready++;
    return size;
  };
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 4;
  configuration.send_buffer_size = 64 * 1024;
  {
    tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
    server.set_on_connection([]() {});
    server.set_on_disconnection([]() {});
    server.start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("x"));
    DOBA_EXPECT(wait_count(ready, 1));
    for (int round = 0; round < 3; round++) {
      std::array<std::jthread, 6> producers;
      for (std::size_t i = 0; i < producers.size(); i++) {
        producers[i] = std::jthread([output, i]() {
          for (int sequence = 0; sequence < 128; sequence++) {
            auto bytes = std::make_unique<char[]>(16);
            std::memset(bytes.get(), static_cast<int>('a' + i), 16);
            bytes[1] = static_cast<char>(sequence);
            output(std::move(bytes), 16, std::nullopt);
          }
        });
      }
      for (auto& producer : producers) producer.join();
      const auto bytes = client.receive(6 * 128 * 16, 5s);
      DOBA_EXPECT(bytes.has_value());
      if (!bytes) break;
      std::array<int, 6> next{};
      for (std::size_t offset = 0; offset < bytes->size(); offset += 16) {
        const auto i = static_cast<std::size_t>((*bytes)[offset] - 'a');
        DOBA_EXPECT(i < next.size());
        if (i >= next.size()) break;
        DOBA_EXPECT_EQUAL(static_cast<unsigned char>((*bytes)[offset + 1]),
                          next[i]++);
        DOBA_EXPECT_EQUAL(bytes->substr(offset + 2, 14),
                          std::string(14, static_cast<char>('a' + i)));
      }
      for (const int sequence : next) DOBA_EXPECT_EQUAL(sequence, 128);
    }
    client.close();
    server.stop();
  }
  output(std::make_unique<char[]>(16), 16, std::nullopt);
}
// +===========================================================================+
// | [>] tcpip limits the sum of queued deliveries               ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip limits the sum of queued deliveries") {
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  auto state = std::make_shared<byte_state>();
  state->receive = [](byte_engine& engine, const char*, std::size_t size,
                       std::size_t) {
    const std::string payload(32, 'x');
    for (int i = 0; i < 3; i++) engine.echo(payload.data(), payload.size());
    return size;
  };
  auto factory = [state]() { return byte_engine{state, {}, {}}; };
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  configuration.send_buffer_size = 64;
  tr::tcpip<byte_engine, decltype(factory)> server(configuration, factory);
  server.set_on_connection([]() {});
  server.set_on_disconnection([]() {});
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("x"));
  const auto bytes = client.receive_until_close(65, 5s);
  const std::string error(client.error());
  client.close();
  server.stop();
  DOBA_EXPECT(bytes.has_value() || error == "socket");
  if (bytes) DOBA_EXPECT(bytes->empty());
}
// +===========================================================================+
// | [>] tcpip closes safely while producers enqueue             ( test-case ) |
// +===========================================================================+
DOBA_TEST("tcpip closes safely while producers enqueue") {
  for (const bool destroy : {false, true}) {
    tcpip_client client;
    const auto port = client.find_available_port();
    DOBA_EXPECT(port != 0);
    auto state = std::make_shared<byte_state>();
    martianlabs::doba::common::send_delegate output;
    std::atomic<int> ready{0};
    std::atomic<bool> release{false};
    state->receive = [&](byte_engine& engine, const char*, std::size_t size,
                          std::size_t) {
      output = engine.send;
      ready++;
      return size;
    };
    auto factory = [state]() { return byte_engine{state, {}, {}}; };
    tr::policies configuration;
    configuration.ip = "127.0.0.1";
    configuration.port = std::to_string(port);
    configuration.worker_count = 4;
    auto server = std::make_unique<tr::tcpip<byte_engine, decltype(factory)>>(
        configuration, factory);
    server->set_on_connection([]() {});
    server->set_on_disconnection([state, &output]() {
      output(std::make_unique<char[]>(32), 32, std::nullopt);
      state->disconnected++;
    });
    server->start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("x"));
    DOBA_EXPECT(wait_count(ready, 1));
    std::array<std::jthread, 4> producers;
    for (auto& producer : producers) {
      producer = std::jthread([&]() {
        while (!release.load()) std::this_thread::yield();
        for (int i = 0; i < 1024; i++) {
          output(std::make_unique<char[]>(32), 32, std::nullopt);
        }
      });
    }
    release = true;
    if (destroy) server.reset();
    else client.abort();
    for (auto& producer : producers) producer.join();
    client.close();
    if (server) server->stop();
    DOBA_EXPECT_EQUAL(state->disconnected.load(), 1);
    output(std::make_unique<char[]>(32), 32, std::nullopt);
  }
}
