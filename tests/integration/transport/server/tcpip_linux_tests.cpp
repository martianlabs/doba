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

#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>

#include "protocol/processing_result.h"
#include "common/reader.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "transport/server/tcp.h"

namespace {
using martianlabs::doba::protocol::processing_result;
namespace common = martianlabs::doba::common;
namespace tr = martianlabs::doba::transport::server;
using martianlabs::doba::tests::integration::tcpip_client;
using namespace std::chrono_literals;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] synchronous_engine                                         ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct synchronous_engine {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(martianlabs::doba::protocol::send_delegate callback) {
    send = std::move(callback);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> callback) {
    close = std::move(callback);
  }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view bytes) {
    action(*this, bytes);
    return {};
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool) { return {}; }

  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Per-test behavior, called only on the owning worker.
  std::function<void(synchronous_engine&, std::string_view)> action;
  // Transport output and closure notifications.
  martianlabs::doba::protocol::send_delegate send;
  std::function<void()> close;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] output_source                                            ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
std::unique_ptr<common::reader> output_source(const std::string& bytes) {
  return std::make_unique<common::reader>(common::reader::borrowed(
      std::as_bytes(std::span(bytes.data(), bytes.size()))));
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] descriptor_count                                         ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
std::size_t descriptor_count() {
  std::size_t count = 0;
  for ([[maybe_unused]] const auto& entry :
       std::filesystem::directory_iterator("/proc/self/fd")) ++count;
  return count;
}
}  // namespace

// +===========================================================================+
// | [>] Linux preserves ordered output across RX blocks         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux preserves ordered output across RX blocks") {
  auto factory = []() {
    return synchronous_engine{[](auto& engine, std::string_view bytes) {
      for (const char byte : bytes) {
        engine.send("R", std::string_view(&byte, 1), nullptr);
      }
    }, {}, {}};
  };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<decltype(factory()), decltype(factory)> transport(
      {.receive_buffer_capacity = 3, .worker_count = 1,
       .ip = "127.0.0.1", .port = std::to_string(port)}, factory);
  transport.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("0123456789"));
  const auto received = client.receive(20);
  DOBA_EXPECT(received.has_value());
  DOBA_EXPECT_EQUAL(*received, "R0R1R2R3R4R5R6R7R8R9");
  DOBA_EXPECT(client.send_all("AB"));
  const auto second = client.receive(4);
  DOBA_EXPECT(second.has_value());
  DOBA_EXPECT_EQUAL(*second, "RARB");
}

// +===========================================================================+
// | [>] Linux drains prefix and source before engine close      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux drains prefix and source before engine close") {
  const std::string body(128 * 1024, 's');
  auto factory = [&]() {
    return synchronous_engine{[&](auto& engine, std::string_view) {
      auto reader = common::reader::borrowed(
          std::as_bytes(std::span(body.data(), body.size())));
      engine.send("prefix", {},
                  std::make_unique<common::reader>(std::move(reader)));
      engine.close();
    }, {}, {}};
  };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<decltype(factory()), decltype(factory)> transport(
      {.worker_count = 1, .max_buffered_send_bytes = 256,
       .ip = "127.0.0.1", .port = std::to_string(port)}, factory);
  transport.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("x"));
  const auto received = client.receive(body.size() + 6);
  DOBA_EXPECT(received.has_value());
  DOBA_EXPECT_EQUAL(*received, "prefix" + body);
  DOBA_EXPECT(client.wait_for_close(3s));
}

// +===========================================================================+
// | [>] Linux isolates engine failures and supports restart     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux isolates engine failures and supports restart") {
  auto factory = []() {
    return synchronous_engine{[](auto& engine, std::string_view input) {
      if (input == "!") throw std::runtime_error("test failure");
      engine.send(input, {}, nullptr);
    }, {}, {}};
  };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<decltype(factory()), decltype(factory)> transport(
      {.worker_count = 1, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  for (int run = 0; run < 3; ++run) {
    transport.start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("!"));
    DOBA_EXPECT(client.wait_for_close(3s));
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("ok"));
    const auto received = client.receive(2);
    DOBA_EXPECT(received.has_value());
    DOBA_EXPECT_EQUAL(*received, "ok");
    transport.stop();
    DOBA_EXPECT(client.wait_for_close(3s));
  }
}

// +===========================================================================+
// | [>] Linux stop releases pending reader output               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux stop releases a reader with pending socket output") {
  std::promise<void> entered;
  auto ready = entered.get_future();
  int released = 0;
  auto factory = [&]() {
    return synchronous_engine{[&](auto& engine, std::string_view) {
      auto owner = std::shared_ptr<const std::string>(
          new std::string(16 * 1024 * 1024, 's'),
          [&](const std::string* bytes) { ++released; delete bytes; });
      auto reader = common::reader::shared(owner,
          std::as_bytes(std::span(owner->data(), owner->size())));
      engine.send("prefix", {},
                  std::make_unique<common::reader>(std::move(reader)));
      entered.set_value();
    }, {}, {}};
  };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<decltype(factory()), decltype(factory)> transport(
      {.worker_count = 1, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  transport.start();
  DOBA_EXPECT(client.set_receive_buffer_size(1024));
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("x"));
  DOBA_EXPECT(ready.wait_for(3s) == std::future_status::ready);
  transport.stop();
  DOBA_EXPECT_EQUAL(released, 1);
}

// +===========================================================================+
// | [>] Linux drains a reader after client half close           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux drains a reader after client half close") {
  const std::string body(128 * 1024, 's');
  auto factory = [&]() {
    return synchronous_engine{[&](auto& engine, std::string_view) {
      engine.send("prefix", {}, output_source(body));
    }, {}, {}};
  };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<decltype(factory()), decltype(factory)> transport(
      {.worker_count = 1, .max_buffered_send_bytes = 256,
       .ip = "127.0.0.1", .port = std::to_string(port)}, factory);
  transport.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("x"));
  DOBA_EXPECT(client.shutdown_write());
  const auto received = client.receive(body.size() + 6);
  DOBA_EXPECT(received.has_value());
  DOBA_EXPECT_EQUAL(*received, "prefix" + body);
  DOBA_EXPECT(client.wait_for_close(3s));
}

// +===========================================================================+
// | [>] Linux peers progress during pending output              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux serves peers while another connection has pending output") {
  const std::string body(8 * 1024 * 1024, 's');
  std::promise<void> entered;
  auto ready = entered.get_future();
  auto factory = [&]() {
    return synchronous_engine{[&](auto& engine, std::string_view bytes) {
      if (bytes == "s") {
        engine.send("prefix", {}, output_source(body));
        entered.set_value();
      } else {
        engine.send("fast", {}, nullptr);
        engine.close();
      }
    }, {}, {}};
  };
  tcpip_client slow;
  const auto port = slow.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<decltype(factory()), decltype(factory)> transport(
      {.worker_count = 1, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  transport.start();
  DOBA_EXPECT(slow.set_receive_buffer_size(1024));
  DOBA_EXPECT(slow.connect(port));
  DOBA_EXPECT(slow.send_all("s"));
  DOBA_EXPECT(ready.wait_for(3s) == std::future_status::ready);
  tcpip_client fast;
  DOBA_EXPECT(fast.connect(port));
  DOBA_EXPECT(fast.send_all("f"));
  const auto received = fast.receive(4);
  DOBA_EXPECT(received.has_value());
  DOBA_EXPECT_EQUAL(*received, "fast");
  DOBA_EXPECT(fast.wait_for_close(3s));
}

// +===========================================================================+
// | [>] Linux recovers from engine factory failure              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux recovers from engine factory failure") {
  unsigned creations = 0;
  auto factory = [&]() {
    if (++creations == 1) throw std::runtime_error("factory failure");
    return synchronous_engine{[](auto& engine, std::string_view bytes) {
      engine.send(bytes, {}, nullptr);
    }, {}, {}};
  };
  tcpip_client failed;
  const auto port = failed.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<decltype(factory()), decltype(factory)> transport(
      {.worker_count = 1, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  transport.start();
  if (failed.connect(port)) {
    DOBA_EXPECT(failed.wait_for_close(3s) || failed.connection_reset());
  } else {
    DOBA_EXPECT(failed.connection_reset());
  }
  tcpip_client recovered;
  DOBA_EXPECT(recovered.connect(port));
  DOBA_EXPECT(recovered.send_all("ok"));
  const auto received = recovered.receive(2);
  DOBA_EXPECT(received.has_value());
  DOBA_EXPECT_EQUAL(*received, "ok");
  transport.stop();
  DOBA_EXPECT_EQUAL(creations, 2);
}

// +===========================================================================+
// | [>] Linux isolates failing connection notifications         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux isolates failing connection notifications") {
  unsigned notifications = 0;
  auto factory = []() {
    return synchronous_engine{[](auto& engine, std::string_view bytes) {
      engine.send(bytes, {}, nullptr);
    }, {}, {}};
  };
  tcpip_client failed;
  const auto port = failed.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<decltype(factory()), decltype(factory)> transport(
      {.worker_count = 1, .max_connections = 1,
       .ip = "127.0.0.1", .port = std::to_string(port)}, factory);
  transport.set_on_connection([&]() {
    if (++notifications == 1) throw std::runtime_error("connect failure");
  });
  transport.set_on_disconnection([]() {
    throw std::runtime_error("disconnect failure");
  });
  transport.start();
  if (failed.connect(port)) {
    DOBA_EXPECT(failed.wait_for_close(3s) || failed.connection_reset());
  } else {
    DOBA_EXPECT(failed.connection_reset());
  }
  tcpip_client recovered;
  DOBA_EXPECT(recovered.connect(port));
  DOBA_EXPECT(recovered.send_all("ok"));
  const auto received = recovered.receive(2);
  DOBA_EXPECT(received.has_value());
  DOBA_EXPECT_EQUAL(*received, "ok");
  transport.stop();
  DOBA_EXPECT_EQUAL(notifications, 2);
}

// +===========================================================================+
// | [>] Linux releases admission capacity after EOF             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux releases admission capacity after EOF") {
  std::promise<void> disconnected;
  auto released = disconnected.get_future();
  bool notified = false;
  auto factory = []() {
    return synchronous_engine{[](auto& engine, std::string_view bytes) {
      engine.send(bytes, {}, nullptr);
    }, {}, {}};
  };
  tcpip_client first;
  const auto port = first.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<decltype(factory()), decltype(factory)> transport(
      {.worker_count = 1, .max_connections = 1,
       .ip = "127.0.0.1", .port = std::to_string(port)}, factory);
  transport.set_on_disconnection([&]() {
    if (!notified) {
      notified = true;
      disconnected.set_value();
    }
  });
  transport.start();
  DOBA_EXPECT(first.connect(port));
  DOBA_EXPECT(first.send_all("one"));
  DOBA_EXPECT(first.receive(3) == std::optional<std::string>("one"));
  tcpip_client excess;
  if (excess.connect(port)) {
    DOBA_EXPECT(excess.wait_for_close(3s) || excess.connection_reset());
  } else {
    DOBA_EXPECT(excess.connection_reset());
  }
  first.close();
  DOBA_EXPECT(released.wait_for(3s) == std::future_status::ready);
  tcpip_client next;
  DOBA_EXPECT(next.connect(port));
  DOBA_EXPECT(next.send_all("two"));
  DOBA_EXPECT(next.receive(3) == std::optional<std::string>("two"));
}

// +===========================================================================+
// | [>] Linux unwinds failed startup without descriptor leaks   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux unwinds failed startup without descriptor leaks") {
  auto factory = []() {
    return synchronous_engine{[](auto& engine, std::string_view bytes) {
      engine.send(bytes, {}, nullptr);
    }, {}, {}};
  };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  const auto release = [](int* socket) { ::close(*socket); delete socket; };
  std::unique_ptr<int, decltype(release)> occupied(
      new int(::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP)),
      release);
  DOBA_EXPECT(*occupied >= 0);
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  address.sin_port = htons(port);
  DOBA_EXPECT(::bind(*occupied, reinterpret_cast<sockaddr*>(&address),
                     sizeof(address)) == 0);
  DOBA_EXPECT(::listen(*occupied, 8) == 0);
  tr::tcp<decltype(factory()), decltype(factory)> second(
      {.worker_count = 4, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  const auto before = descriptor_count();
  bool rejected = false;
  try { second.start(); }
  catch (const std::runtime_error&) { rejected = true; }
  DOBA_EXPECT(rejected);
  DOBA_EXPECT_EQUAL(descriptor_count(), before);
  occupied.reset();
  second.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("ok"));
  DOBA_EXPECT(client.receive(2) == std::optional<std::string>("ok"));
}

// +===========================================================================+
// | [>] Linux releases descriptors after reset and restart      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux releases descriptors after reset and restart") {
  const std::string body(8 * 1024 * 1024, 's');
  auto factory = [&]() {
    return synchronous_engine{[&](auto& engine, std::string_view) {
      engine.send("prefix", {}, output_source(body));
    }, {}, {}};
  };
  tcpip_client probe;
  const auto port = probe.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<decltype(factory()), decltype(factory)> transport(
      {.worker_count = 4, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  const auto before = descriptor_count();
  for (int iteration = 0; iteration < 8; ++iteration) {
    transport.start();
    tcpip_client client;
    DOBA_EXPECT(client.set_receive_buffer_size(1024));
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("x"));
    DOBA_EXPECT(client.receive(6) == std::optional<std::string>("prefix"));
    client.abort();
    transport.stop();
    DOBA_EXPECT_EQUAL(descriptor_count(), before);
  }
}

// +===========================================================================+
// | [>] Linux workers own independent connections and sources   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux workers own independent connections and sources") {
  const std::string body(8192, 's');
  std::atomic<unsigned> connected{0};
  std::atomic<unsigned> disconnected{0};
  auto factory = [&]() {
    const auto owner = std::this_thread::get_id();
    const auto value = std::hash<std::thread::id>{}(owner);
    const std::string prefix(reinterpret_cast<const char*>(&value),
                             sizeof(value));
    return synchronous_engine{[&, owner, prefix](auto& engine,
                                               std::string_view) {
      if (owner != std::this_thread::get_id()) {
        throw std::runtime_error("Connection changed workers");
      }
      engine.send(prefix, {}, output_source(body));
    }, {}, {}};
  };
  tcpip_client probe;
  const auto port = probe.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<decltype(factory()), decltype(factory)> transport(
      {.worker_count = 4, .max_buffered_send_bytes = 128,
       .max_connections = 0,
       .ip = "127.0.0.1", .port = std::to_string(port)}, factory);
  transport.set_on_connection([&]() { ++connected; });
  transport.set_on_disconnection([&]() { ++disconnected; });
  transport.start();
  transport.start();
  std::vector<std::unique_ptr<tcpip_client>> peers;
  std::vector<std::string> prefixes;
  std::set<std::string> owners;
  for (unsigned index = 0; index < 128; ++index) {
    auto peer = std::make_unique<tcpip_client>();
    DOBA_EXPECT(peer->connect(port));
    DOBA_EXPECT(peer->send_all("x"));
    const auto received = peer->receive(sizeof(std::size_t) + body.size());
    DOBA_EXPECT(received.has_value());
    DOBA_EXPECT_EQUAL(received->substr(sizeof(std::size_t)), body);
    prefixes.push_back(received->substr(0, sizeof(std::size_t)));
    owners.insert(prefixes.back());
    peers.push_back(std::move(peer));
  }
  DOBA_EXPECT_EQUAL(owners.size(), std::size_t{4});
  for (std::size_t index = 0; index < peers.size(); ++index) {
    DOBA_EXPECT(peers[index]->send_all("y"));
    const auto received = peers[index]->receive(prefixes[index].size() +
                                               body.size());
    DOBA_EXPECT(received == std::optional<std::string>(prefixes[index] + body));
  }
  transport.stop();
  DOBA_EXPECT_EQUAL(connected.load(), 128u);
  DOBA_EXPECT_EQUAL(disconnected.load(), 128u);
  transport.stop();
}

// +===========================================================================+
// | [>] Linux partitions finite admission without exceeding it  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux partitions finite admission without exceeding it") {
  std::atomic<unsigned> created{0};
  auto factory = [&]() {
    ++created;
    return synchronous_engine{[](auto& engine, std::string_view bytes) {
      engine.send(bytes, {}, nullptr);
    }, {}, {}};
  };
  tcpip_client probe;
  const auto port = probe.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<decltype(factory()), decltype(factory)> transport(
      {.worker_count = 4, .max_connections = 7,
       .ip = "127.0.0.1", .port = std::to_string(port)}, factory);
  transport.start();
  std::vector<std::unique_ptr<tcpip_client>> peers;
  for (unsigned index = 0; index < 128 && peers.size() < 7; ++index) {
    auto peer = std::make_unique<tcpip_client>();
    DOBA_EXPECT(peer->connect(port));
    if (peer->send_all("x") &&
        peer->receive(1) == std::optional<std::string>("x")) {
      peers.push_back(std::move(peer));
    }
  }
  DOBA_EXPECT_EQUAL(peers.size(), std::size_t{7});
  tcpip_client excess;
  DOBA_EXPECT(excess.connect(port));
  if (excess.send_all("x")) {
    DOBA_EXPECT(!excess.receive(1).has_value());
  }
  transport.stop();
  DOBA_EXPECT_EQUAL(created.load(), 7u);
  bool rejected = false;
  try {
    tr::tcp<decltype(factory()), decltype(factory)> invalid(
        {.worker_count = 4, .max_connections = 3,
         .ip = "127.0.0.1", .port = std::to_string(port)}, factory);
  } catch (const std::runtime_error&) { rejected = true; }
  DOBA_EXPECT(rejected);
}

// +===========================================================================+
// | [>] Linux default worker serves loopback                    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux worker_count zero serves synchronous loopback") {
  auto factory = []() {
    return synchronous_engine{[](auto& engine, std::string_view bytes) {
      engine.send(bytes, {}, nullptr);
    }, {}, {}};
  };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<decltype(factory()), decltype(factory)> transport(
      {.worker_count = 0,
       .ip = "127.0.0.1", .port = std::to_string(port)}, factory);
  transport.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("echo"));
  DOBA_EXPECT(client.receive(4) == std::optional<std::string>("echo"));
}

// +===========================================================================+
// | [>] Linux rejects callback mutation and worker stop         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Linux rejects active callback mutation and worker stop") {
  std::function<void()> stop;
  auto factory = [&]() {
    return synchronous_engine{[&](auto& engine, std::string_view) {
      try { stop(); }
      catch (const std::runtime_error&) {
        engine.send("rejected", {}, nullptr);
      }
    }, {}, {}};
  };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<decltype(factory()), decltype(factory)> transport(
      {.worker_count = 4, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  stop = [&]() { transport.stop(); };
  transport.start();
  bool connection_rejected = false;
  bool disconnection_rejected = false;
  try { transport.set_on_connection([]() {}); }
  catch (const std::runtime_error&) { connection_rejected = true; }
  try { transport.set_on_disconnection([]() {}); }
  catch (const std::runtime_error&) { disconnection_rejected = true; }
  DOBA_EXPECT(connection_rejected && disconnection_rejected);
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("x"));
  DOBA_EXPECT(client.receive(8) == std::optional<std::string>("rejected"));
  transport.stop();
  transport.set_on_connection([]() {});
  transport.set_on_disconnection([]() {});
  transport.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("x"));
  DOBA_EXPECT(client.receive(8) == std::optional<std::string>("rejected"));
}
