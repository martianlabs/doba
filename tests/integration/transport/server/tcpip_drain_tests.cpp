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
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include "common/byte_storage.h"
#include "common/reader.h"
#include "protocol/http/v11/server.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "transport/server/tcpip.h"

namespace {
namespace tr = martianlabs::doba::transport::server;
using martianlabs::doba::common::send_delegate;
using martianlabs::doba::tests::integration::tcpip_client;
using namespace std::chrono_literals;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] drain_state                                                ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct drain_state {
  std::atomic<int> entered{0};
  std::atomic<int> queued{0};
  std::atomic<int> destroyed{0};
  std::atomic<bool> release{true};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] drain_engine                                               ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct drain_engine {
  using policies_type = int;
  explicit drain_engine(std::shared_ptr<drain_state> state)
      : state_(std::move(state)) {}
  drain_engine(const drain_engine&) = delete;
  drain_engine(drain_engine&&) = delete;
  ~drain_engine() { state_->destroyed++; }
  void set_on_send(send_delegate output) { output_ = std::move(output); }
  void set_on_close(std::function<void()>) {}
  void set_on_wake(std::function<void()>) {}
  void on_wake() {}
  void on_stop() {}
  void on_send_completed(bool) {}
  std::size_t on_bytes_received(const char* bytes, std::size_t size,
                                std::size_t) {
    state_->entered++;
    const auto deadline = std::chrono::steady_clock::now() + 5s;
    while (!state_->release.load()) {
      if (std::chrono::steady_clock::now() >= deadline) {
        throw std::runtime_error("Test callback was not released");
      }
      std::this_thread::yield();
    }
    for (int i = 0; i < 8; i++) {
      auto block = std::make_unique_for_overwrite<char[]>(1024 * 1024);
      std::memset(block.get(), bytes[0], 1024 * 1024);
      output_(std::move(block), 1024 * 1024, std::nullopt);
    }
    state_->queued++;
    return size;
  }
  std::shared_ptr<drain_state> state_;
  send_delegate output_;
};

bool wait_count(const std::atomic<int>& value, int expected) {
  const auto deadline = std::chrono::steady_clock::now() + 5s;
  while (value.load() < expected &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::yield();
  }
  return value.load() == expected;
}

void check_drain(bool destroy, bool active_callback) {
  auto state = std::make_shared<drain_state>();
  state->release = !active_callback;
  auto factory = [state]() -> drain_engine { return drain_engine{state}; };
  std::array<tcpip_client, 2> clients;
  const auto port = clients[0].find_available_port();
  DOBA_EXPECT(port != 0);
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  configuration.send_buffer_size = 32 * 1024 * 1024;
  auto transport = std::make_unique<tr::tcpip<drain_engine, decltype(factory)>>(
      configuration, std::move(factory));
  transport->set_on_connection([]() {});
  transport->set_on_disconnection([]() {});
  transport->start();
  const int count = active_callback ? 1 : 2;
  for (int i = 0; i < count; i++) {
    DOBA_EXPECT(clients[i].connect(port));
    DOBA_EXPECT(clients[i].send_all(
        std::string(1, static_cast<char>('a' + i))));
  }
  DOBA_EXPECT(wait_count(active_callback ? state->entered : state->queued,
                         count));
  auto stopped = std::async(std::launch::async, [&]() {
    if (destroy) {
      transport.reset();
    } else {
      transport->stop();
    }
  });
  const bool waited = stopped.wait_for(150ms) == std::future_status::timeout;
  bool rejects_connections = true;
  if (!active_callback) {
    tcpip_client later;
    rejects_connections = !later.connect(port, 500ms);
  }
  state->release = true;
  bool intact = true;
  bool closed = true;
  for (int i = 0; i < count; i++) {
    const auto bytes = clients[i].receive(8 * 1024 * 1024, 10s);
    intact = intact && bytes.has_value() &&
             *bytes == std::string(8 * 1024 * 1024, static_cast<char>('a' + i));
    closed = closed && clients[i].wait_for_close(5s);
    clients[i].close();
  }
  stopped.get();
  DOBA_EXPECT(waited);
  DOBA_EXPECT(rejects_connections);
  DOBA_EXPECT(intact);
  DOBA_EXPECT(closed);
  DOBA_EXPECT_EQUAL(state->entered.load(), count);
  DOBA_EXPECT_EQUAL(state->destroyed.load(), count);
  if (transport) {
    transport->start();
    transport->stop();
  }
}
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] reader_state                                               ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct reader_state {
  std::atomic<int> queued{0};
  std::atomic<int> succeeded{0};
  std::atomic<int> failed{0};
  std::atomic<int> reentered{0};
  bool close{false};
  bool single{false};
  int source_error{0};
  std::size_t size{8 * 1024 * 1024};
  std::string later = std::string(1025, 'c');
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] reader_engine                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct reader_engine {
  using policies_type = int;
  explicit reader_engine(std::shared_ptr<reader_state> state)
      : state_(std::move(state)) {}
  void set_on_send(send_delegate output) { output_ = std::move(output); }
  void set_on_close(std::function<void()> close) { close_ = std::move(close); }
  void set_on_wake(std::function<void()>) {}
  void on_wake() {}
  void on_stop() {}
  void on_send_completed(bool succeeded) {
    if (in_callback_) state_->reentered++;
    if (succeeded) state_->succeeded++;
    else state_->failed++;
  }
  std::size_t on_bytes_received(const char*, std::size_t size, std::size_t) {
    in_callback_ = true;
    namespace common = martianlabs::doba::common;
    common::reader source;
    std::size_t prefix_size = state_->single ? 0 : 1;
    if (state_->source_error == 1) {
      source = common::reader(common::filesystem_file{});
      std::byte byte;
      source.read(std::span<std::byte>(&byte, 1));
      prefix_size = state_->size;
    } else {
      common::byte_storage storage({.spill_threshold = 1, .spill_dir = {}});
      const std::string body(state_->size, 'a');
      if (!storage.write(body.data(), body.size())) {
        throw std::runtime_error("Unable to prepare source");
      }
      if (state_->source_error != 3) {
        storage.finish(body.size() + (state_->source_error == 2 ? 1 : 0));
      }
      source = common::reader(std::move(storage));
    }
    auto prefix = std::make_unique_for_overwrite<char[]>(prefix_size);
    std::memset(prefix.get(), 'A', prefix_size);
    output_(std::move(prefix), prefix_size, std::move(source));
    if (!state_->single) {
      auto next = std::make_unique<char[]>(1);
      next[0] = 'B';
      output_(std::move(next), 1, std::nullopt);
      auto last = std::make_unique<char[]>(1);
      last[0] = 'C';
      output_(std::move(last), 1, common::reader::borrowed(
          std::as_bytes(std::span(state_->later.data(), state_->later.size()))));
      output_(nullptr, 0, common::reader{});
      auto tail = std::make_unique<char[]>(2);
      tail[0] = 'D';
      tail[1] = 'E';
      output_(std::move(tail), 2, std::nullopt);
      // The queued source must not have been read during this callback.
      std::fill(state_->later.begin(), state_->later.end(), 'd');
    }
    if (state_->close) close_();
    in_callback_ = false;
    state_->queued++;
    return size;
  }
  std::shared_ptr<reader_state> state_;
  send_delegate output_;
  std::function<void()> close_;
  bool in_callback_{false};
};

void check_reader_drain(bool destroy, bool close) {
  auto state = std::make_shared<reader_state>();
  state->close = close;
  auto factory = [state]() -> reader_engine { return reader_engine{state}; };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  configuration.send_buffer_size = 32 * 1024;
  auto transport = std::make_unique<tr::tcpip<reader_engine, decltype(factory)>>(
      configuration, std::move(factory));
  transport->set_on_connection([]() {});
  transport->set_on_disconnection([]() {});
  transport->start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("x"));
  DOBA_EXPECT(wait_count(state->queued, 1));
  DOBA_EXPECT_EQUAL(state->succeeded.load(), 0);
  auto stopped = std::async(std::launch::async, [&]() {
    if (destroy) transport.reset();
    else if (!close) transport->stop();
  });
  bool waited = true;
  if (!close) {
    waited = stopped.wait_for(150ms) == std::future_status::timeout;
  }
  const std::string expected =
      "A" + std::string(state->size, 'a') + "BC" + std::string(1025, 'd') + "DE";
  const auto bytes = client.receive(expected.size(), 15s);
  const bool closed = client.wait_for_close(5s);
  client.close();
  stopped.get();
  if (transport) transport->stop();
  DOBA_EXPECT(waited);
  DOBA_EXPECT(bytes.has_value());
  if (bytes) DOBA_EXPECT_EQUAL(*bytes, expected);
  DOBA_EXPECT(closed);
  DOBA_EXPECT_EQUAL(state->succeeded.load(), 5);
  DOBA_EXPECT_EQUAL(state->failed.load(), 0);
  DOBA_EXPECT_EQUAL(state->reentered.load(), 0);
}
}  // namespace

// +===========================================================================+
// | [>] stop drains pending bytes                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("stop drains pending bytes") {
  check_drain(false, false);
}
// +===========================================================================+
// | [>] destruction drains pending bytes                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("destruction drains pending bytes") {
  check_drain(true, false);
}
// +===========================================================================+
// | [>] stop lets an active engine callback finish              ( test-case ) |
// +===========================================================================+
DOBA_TEST("stop lets an active engine callback finish") {
  check_drain(false, true);
}
// +===========================================================================+
// | [>] stop handles failed and overflowing sends               ( test-case ) |
// +===========================================================================+
DOBA_TEST("stop handles failed and overflowing sends") {
  for (const bool overflow : {false, true}) {
    auto state = std::make_shared<drain_state>();
    auto factory = [state]() -> drain_engine { return drain_engine{state}; };
    tcpip_client client;
    const auto port = client.find_available_port();
    DOBA_EXPECT(port != 0);
    tr::policies configuration;
    configuration.ip = "127.0.0.1";
    configuration.port = std::to_string(port);
    configuration.worker_count = 2;
    configuration.send_buffer_size = overflow ? 1024 : 32 * 1024 * 1024;
    tr::tcpip<drain_engine, decltype(factory)> transport(configuration,
                                                       std::move(factory));
    transport.set_on_connection([]() {});
    transport.set_on_disconnection([]() {});
    transport.start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("x"));
    DOBA_EXPECT(wait_count(state->queued, 1));
    if (overflow) {
      const auto bytes = client.receive_until_close(1, 5s);
      DOBA_EXPECT(!bytes || bytes->empty());
      DOBA_EXPECT(client.error() != "timeout");
    }
    client.abort();
    transport.stop();
    DOBA_EXPECT_EQUAL(state->destroyed.load(), 1);
  }
}
// +===========================================================================+
// | [>] stop rejects new engine input                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("stop rejects new engine input") {
  auto state = std::make_shared<drain_state>();
  auto factory = [state]() -> drain_engine { return drain_engine{state}; };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  configuration.send_buffer_size = 32 * 1024 * 1024;
  tr::tcpip<drain_engine, decltype(factory)> transport(configuration,
                                                     std::move(factory));
  transport.set_on_connection([]() {});
  transport.set_on_disconnection([]() {});
  transport.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("x"));
  DOBA_EXPECT(wait_count(state->queued, 1));
  auto stopped = std::async(std::launch::async, [&]() { transport.stop(); });
  const bool draining = stopped.wait_for(150ms) == std::future_status::timeout;
  const bool sent = client.send_all("new request");
  std::this_thread::sleep_for(50ms);
  const int entered = state->entered.load();
  client.abort();
  stopped.get();
  DOBA_EXPECT(draining);
  DOBA_EXPECT(sent);
  DOBA_EXPECT_EQUAL(entered, 1);
}
// +===========================================================================+
// | [>] stop drains ordered byte sources                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("stop drains ordered byte sources") {
  check_reader_drain(false, false);
}
// +===========================================================================+
// | [>] destruction drains ordered byte sources                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("destruction drains ordered byte sources") {
  check_reader_drain(true, false);
}
// +===========================================================================+
// | [>] close drains ordered byte sources                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("close drains ordered byte sources") {
  check_reader_drain(false, true);
}
// +===========================================================================+
// | [>] sources work with a small send buffer                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("sources work with a small send buffer") {
  for (const std::size_t capacity : {1U, 7U, 8192U}) {
    for (const std::size_t length : {0U, 37U}) {
      auto state = std::make_shared<reader_state>();
      state->single = true;
      state->close = true;
      state->size = length;
      auto factory = [state]() -> reader_engine { return reader_engine{state}; };
      tcpip_client client;
      const auto port = client.find_available_port();
      DOBA_EXPECT(port != 0);
      tr::policies configuration;
      configuration.ip = "127.0.0.1";
      configuration.port = std::to_string(port);
      configuration.worker_count = 2;
      configuration.send_buffer_size = capacity;
      tr::tcpip<reader_engine, decltype(factory)> transport(configuration,
                                                          std::move(factory));
      transport.set_on_connection([]() {});
      transport.set_on_disconnection([]() {});
      transport.start();
      DOBA_EXPECT(client.connect(port));
      DOBA_EXPECT(client.send_all("x"));
      const auto bytes = client.receive(length, 5s);
      const bool closed = client.wait_for_close(5s);
      client.close();
      transport.stop();
      DOBA_EXPECT(bytes.has_value());
      if (bytes) DOBA_EXPECT_EQUAL(*bytes, std::string(length, 'a'));
      DOBA_EXPECT(closed);
      DOBA_EXPECT_EQUAL(state->succeeded.load(), 1);
      DOBA_EXPECT_EQUAL(state->failed.load(), 0);
      DOBA_EXPECT_EQUAL(state->reentered.load(), 0);
    }
  }
}
// +===========================================================================+
// | [>] source failure cancels later deliveries                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("source failure cancels later deliveries") {
  for (const int source_error : {1, 2}) {
    auto state = std::make_shared<reader_state>();
    state->source_error = source_error;
    auto factory = [state]() -> reader_engine { return reader_engine{state}; };
    tcpip_client client;
    const auto port = client.find_available_port();
    DOBA_EXPECT(port != 0);
    tr::policies configuration;
    configuration.ip = "127.0.0.1";
    configuration.port = std::to_string(port);
    configuration.worker_count = 2;
    configuration.send_buffer_size = 16 * 1024 * 1024;
    tr::tcpip<reader_engine, decltype(factory)> transport(configuration,
                                                        std::move(factory));
    transport.set_on_connection([]() {});
    transport.set_on_disconnection([]() {});
    transport.start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("x"));
    DOBA_EXPECT(wait_count(state->queued, 1));
    const auto bytes = client.receive_until_close(state->size + 10000, 15s);
    const std::string error(client.error());
    client.close();
    transport.stop();
    // Aborting a failed source may reset the socket before its prefix drains.
    DOBA_EXPECT(bytes.has_value() || error == "socket");
    if (bytes) {
      const std::string expected = source_error == 1
          ? std::string(state->size, 'A') : "A" + std::string(state->size, 'a');
      DOBA_EXPECT_EQUAL(*bytes, expected);
    }
    DOBA_EXPECT_EQUAL(state->succeeded.load(), 0);
    DOBA_EXPECT_EQUAL(state->failed.load(), 5);
    DOBA_EXPECT_EQUAL(state->reentered.load(), 0);
  }
}
// +===========================================================================+
// | [>] sources without progress close the connection           ( test-case ) |
// +===========================================================================+
DOBA_TEST("sources without progress close the connection") {
  auto state = std::make_shared<reader_state>();
  state->single = true;
  state->source_error = 3;
  state->size = 37;
  auto factory = [state]() -> reader_engine { return reader_engine{state}; };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  tr::tcpip<reader_engine, decltype(factory)> transport(configuration,
                                                      std::move(factory));
  transport.set_on_connection([]() {});
  transport.set_on_disconnection([]() {});
  transport.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("x"));
  const auto bytes = client.receive_until_close(1, 5s);
  const std::string error(client.error());
  client.close();
  transport.stop();
  DOBA_EXPECT(!bytes || bytes->empty());
  DOBA_EXPECT(error != "timeout");
  DOBA_EXPECT_EQUAL(state->succeeded.load(), 0);
  DOBA_EXPECT_EQUAL(state->failed.load(), 1);
  DOBA_EXPECT_EQUAL(state->reentered.load(), 0);
}
// +===========================================================================+
// | [>] source buffers respect the send limit                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("source buffers respect the send limit") {
  auto state = std::make_shared<reader_state>();
  auto factory = [state]() -> reader_engine { return reader_engine{state}; };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  configuration.send_buffer_size = 8192;
  tr::tcpip<reader_engine, decltype(factory)> transport(configuration,
                                                      std::move(factory));
  transport.set_on_connection([]() {});
  transport.set_on_disconnection([]() {});
  transport.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("x"));
  DOBA_EXPECT(wait_count(state->queued, 1));
  const auto bytes = client.receive_until_close(state->size + 1, 5s);
  const std::string error(client.error());
  client.close();
  transport.stop();
  DOBA_EXPECT(!bytes || bytes->size() < state->size + 1);
  DOBA_EXPECT(error != "timeout");
  DOBA_EXPECT_EQUAL(state->succeeded.load(), 0);
  DOBA_EXPECT_EQUAL(state->failed.load(), 1);
}
// +===========================================================================+
// | [>] HTTP sources precede the next pipelined response        ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP sources precede the next pipelined response") {
  namespace http = martianlabs::doba::protocol::http::v11;
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  configuration.send_buffer_size = 32768;
  http::server<> server(configuration);
  const std::string body(2 * 1024 * 1024, 'h');
  server.add_route("GET", "/large", [&body](const http::request&) {
    auto result = http::response::ok_200();
    result.set_header("Date", "fixed").set_body(body);
    return result;
  });
  server.add_route("GET", "/chunked", [](const http::request&) {
    auto writer = http::body::body_writer::chunked();
    if (!writer.write("abc")) throw std::runtime_error("body write failed");
    auto result = http::response::ok_200();
    result.set_header("Date", "fixed").set_body(std::move(writer));
    return result;
  });
  std::atomic<int> unexpected{0};
  server.add_route("GET", "/later", [&unexpected](const http::request&) {
    unexpected++;
    return http::response::ok_200();
  });
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /large HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /chunked HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
      "GET /later HTTP/1.1\r\nHost: localhost\r\n\r\n"));
  const auto bytes = client.receive_until_close(body.size() + 4096, 10s);
  client.close();
  server.stop();
  DOBA_EXPECT(bytes.has_value());
  DOBA_EXPECT(bytes->starts_with("HTTP/1.1 200 OK\r\n"));
  const auto begin = bytes->find("\r\n\r\n") + 4;
  DOBA_EXPECT_EQUAL(bytes->substr(begin, body.size()), body);
  DOBA_EXPECT(bytes->substr(begin + body.size()).starts_with("HTTP/1.1 200 OK"));
  DOBA_EXPECT(bytes->ends_with("\r\n\r\n3\r\nabc\r\n0\r\n\r\n"));
  DOBA_EXPECT_EQUAL(unexpected.load(), 0);
}
// +===========================================================================+
// | [>] concurrent stops drain before returning                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("concurrent stops drain before returning") {
  auto state = std::make_shared<drain_state>();
  auto factory = [state]() -> drain_engine { return drain_engine{state}; };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 8;
  configuration.send_buffer_size = 32 * 1024 * 1024;
  tr::tcpip<drain_engine, decltype(factory)> transport(configuration,
                                                     std::move(factory));
  transport.set_on_connection([]() {});
  transport.set_on_disconnection([]() {});
  for (int iteration = 0; iteration < 16; iteration++) {
    transport.start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("x"));
    DOBA_EXPECT(wait_count(state->queued, iteration + 1));
    std::promise<void> release;
    auto ready = release.get_future().share();
    std::atomic<int> entered{0};
    std::atomic<int> stopped{0};
    std::array<std::jthread, 8> callers;
    for (auto& caller : callers) {
      caller = std::jthread([&]() {
        ready.wait();
        entered++;
        try {
          for (int i = 0; i < 8; i++) transport.stop();
          stopped++;
        } catch (...) {
        }
      });
    }
    release.set_value();
    const bool concurrent = wait_count(entered, 8);
    const auto bytes = client.receive(8 * 1024 * 1024, 10s);
    const bool closed = client.wait_for_close(5s);
    client.close();
    for (auto& caller : callers) caller.join();
    DOBA_EXPECT(concurrent);
    DOBA_EXPECT_EQUAL(stopped.load(), 8);
    DOBA_EXPECT(bytes.has_value());
    DOBA_EXPECT_EQUAL(*bytes, std::string(8 * 1024 * 1024, 'x'));
    DOBA_EXPECT(closed);
    DOBA_EXPECT_EQUAL(state->destroyed.load(), iteration + 1);
  }
}
// +===========================================================================+
// | [>] stop from a worker is rejected                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("stop from a worker is rejected") {
  auto state = std::make_shared<drain_state>();
  auto factory = [state]() -> drain_engine { return drain_engine{state}; };
  tcpip_client client;
  const auto other_port = client.find_available_port();
  DOBA_EXPECT(other_port != 0);
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(other_port);
  configuration.worker_count = 2;
  std::promise<bool> rejected;
  auto result = rejected.get_future();
  tr::tcpip<drain_engine, decltype(factory)> other(configuration, factory);
  other.set_on_connection([]() {});
  other.set_on_disconnection([]() {});
  other.start();
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  configuration.port = std::to_string(port);
  tr::tcpip<drain_engine, decltype(factory)> transport(configuration, factory);
  transport.set_on_connection([&]() {
    other.stop();
    bool threw = false;
    try {
      transport.stop();
    } catch (const std::runtime_error&) {
      threw = true;
    }
    rejected.set_value(threw);
  });
  transport.set_on_disconnection([]() {});
  transport.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(result.wait_for(5s) == std::future_status::ready);
  DOBA_EXPECT(result.get());
  client.close();
  transport.stop();
  transport.start();
  transport.stop();
}
// +===========================================================================+
// | [>] disconnection can reenter an external stop              ( test-case ) |
// +===========================================================================+
DOBA_TEST("disconnection can reenter an external stop") {
  auto state = std::make_shared<drain_state>();
  auto factory = [state]() -> drain_engine { return drain_engine{state}; };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  std::promise<void> connected;
  auto connection = connected.get_future();
  std::atomic<int> disconnected{0};
  std::atomic<bool> correct{false};
  const auto caller = std::this_thread::get_id();
  tr::tcpip<drain_engine, decltype(factory)> transport(configuration,
                                                     std::move(factory));
  transport.set_on_connection([&]() { connected.set_value(); });
  transport.set_on_disconnection([&]() {
    bool rejected = false;
    try {
      transport.stop();
    } catch (const std::runtime_error&) {
      rejected = true;
    }
    correct = rejected == (std::this_thread::get_id() != caller);
    disconnected++;
  });
  transport.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(connection.wait_for(5s) == std::future_status::ready);
  transport.stop();
  DOBA_EXPECT_EQUAL(disconnected.load(), 1);
  DOBA_EXPECT(correct.load());
  DOBA_EXPECT(client.wait_for_close(5s));
  client.close();
  transport.start();
  transport.stop();
}
// +===========================================================================+
// | [>] invalid endpoint policies prevent startup               ( test-case ) |
// +===========================================================================+
DOBA_TEST("invalid endpoint policies prevent startup") {
  const std::array<std::pair<const char*, const char*>, 13> endpoints{{
      {"", ""},
      {"", "8080"},
      {"127.0.0.256", "8080"},
      {"::1", "8080"},
      {"127.0.0.1", ""},
      {"127.0.0.1", "0"},
      {"127.0.0.1", "65536"},
      {"127.0.0.1", "-1"},
      {"127.0.0.1", "80x"},
      {"127.0.0.1", " 80"},
      {"127.0.0.1", "80 "},
      {"127.0.0.1", "+80"},
      {"127.0.0.1", "999999999999999999999999999999999999"},
  }};
  auto state = std::make_shared<drain_state>();
  auto factory = [state]() -> drain_engine { return drain_engine{state}; };
  for (const auto& [ip, port] : endpoints) {
    tr::policies configuration;
    configuration.ip = ip;
    configuration.port = port;
    configuration.worker_count = 1;
    tr::tcpip<drain_engine, decltype(factory)> transport(configuration, factory);
    transport.set_on_connection([]() {});
    transport.set_on_disconnection([]() {});
    for (int attempt = 0; attempt < 2; attempt++) {
      std::string error;
      try {
        transport.start();
      } catch (const std::runtime_error& exception) {
        error = exception.what();
      }
      transport.stop();
      DOBA_EXPECT_EQUAL(error, configuration.ip == "127.0.0.1"
                                   ? "Invalid port (range from 1 to 65535)!"
                                   : "Invalid IPv4 address!");
    }
  }
}
// +===========================================================================+
// | [>] endpoint policies persist across restarts               ( test-case ) |
// +===========================================================================+
DOBA_TEST("endpoint policies persist across restarts") {
  auto state = std::make_shared<drain_state>();
  auto factory = [state]() -> drain_engine { return drain_engine{state}; };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::policies configuration;
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  std::atomic<int> connected{0};
  tr::tcpip<drain_engine, decltype(factory)> transport(configuration, factory);
  transport.set_on_connection([&]() { connected++; });
  transport.set_on_disconnection([]() {});
  configuration.ip = "invalid";
  configuration.port = "0";
  for (int iteration = 0; iteration < 2; iteration++) {
    transport.start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(wait_count(connected, iteration + 1));
    transport.stop();
    DOBA_EXPECT(client.wait_for_close(5s));
    client.close();
    DOBA_EXPECT_EQUAL(state->destroyed.load(), iteration + 1);
  }
}
