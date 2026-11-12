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
#include <cstddef>
#include <functional>
#include <future>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "../../helpers/concurrency.h"
#include "protocol/processing_result.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "transport/tcp.h"

namespace {
using martianlabs::doba::protocol::processing_result;
namespace tr = martianlabs::doba::transport;
using martianlabs::doba::protocol::send_delegate;
using martianlabs::doba::tests::integration::tcpip_client;
using namespace std::chrono_literals;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] policy_state                                               ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct policy_state {
  // Count created engines, including ones already closed.
  std::atomic<int> created{0};
  // Count input calls while checking the receive limit.
  std::atomic<int> calls{0};
  // Flag any receive block larger than the configured limit.
  std::atomic<bool> bounded{true};
  // Set the receive limit before starting workers.
  std::size_t receive_limit{4096};
  // Choose echo, exact-size output or overflow before startup.
  bool fixed_output{false};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] policy_engine                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct policy_engine {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(send_delegate value) { send = std::move(value); }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> value) { close = std::move(value); }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view bytes) {
    ++state->calls;
    if (bytes.size() > state->receive_limit) state->bounded = false;
    if (!state->fixed_output) {
      send(bytes, {}, nullptr);
      return {};
    }
    send("123", "45", nullptr);
    if (bytes == "X")
      send("6", {}, nullptr);
    else
      close();
    return {};
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool) { return {}; }

  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Results shared with the test.
  std::shared_ptr<policy_state> state;
  // Send only during an engine call.
  send_delegate send;
  // Close only during an engine call.
  std::function<void()> close;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] rejected_endpoint                                        ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
bool rejected_endpoint(bool address, bool suffix) {
  auto state = std::make_shared<policy_state>();
  auto factory = [state] { return policy_engine{state, {}, {}}; };
  tcpip_client probe;
  const auto port = probe.find_available_port();
  if (!port) throw std::runtime_error("Cannot reserve an endpoint for test");
  tr::tcp_policies policies;
  policies.worker_count = 1;
  policies.ip = "127.0.0.1";
  policies.port = std::to_string(port);
  auto& text = address ? policies.ip : policies.port;
  text.push_back('\0');
  if (suffix) text += "ignored";
  tr::tcp<policy_engine, decltype(factory)> transport(policies, factory);
  try {
    transport.start();
  } catch (const std::runtime_error&) {
    return true;
  }
  transport.stop();
  return false;
}
}  // namespace

// +===========================================================================+
// | [>] endpoint strings are validated without truncation       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("TCP rejects embedded and trailing NUL in IPv4 text") {
  DOBA_EXPECT(rejected_endpoint(true, false));
  DOBA_EXPECT(rejected_endpoint(true, true));
}
DOBA_TEST("TCP rejects embedded and trailing NUL in decimal port text") {
  DOBA_EXPECT(rejected_endpoint(false, false));
  DOBA_EXPECT(rejected_endpoint(false, true));
}

// +===========================================================================+
// | [>] malformed endpoint text                                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("TCP rejects malformed IPv4 addresses and decimal ports") {
  auto state = std::make_shared<policy_state>();
  auto factory = [state] { return policy_engine{state, {}, {}}; };
  tcpip_client probe;
  const auto port = probe.find_available_port();
  DOBA_EXPECT(port != 0);
  auto rejects = [&](const std::string& ip, const std::string& text) {
    tr::tcp<policy_engine, decltype(factory)> transport(
        {.worker_count = 1, .ip = ip, .port = text}, factory);
    try {
      transport.start();
    } catch (const std::runtime_error&) {
      return true;
    }
    transport.stop();
    return false;
  };
  for (const std::string ip :
       {"", "localhost", "127.0.0", "256.0.0.1", "127.0.0.1 ", "::1"}) {
    DOBA_EXPECT(rejects(ip, std::to_string(port)));
  }
  for (const std::string text : {"", "0", "65536", "+1", "-1", " 80", "80 ",
                                 "80x", "99999999999999999999"}) {
    DOBA_EXPECT(rejects("127.0.0.1", text));
  }
}

// +===========================================================================+
// | [>] copied RX and backlog policies                          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("TCP copies policies and bounds input with kernel backlog requests") {
  for (const int backlog : {0, 1, std::numeric_limits<int>::max()}) {
    auto state = std::make_shared<policy_state>();
    state->receive_limit = 3;
    auto factory = [state] { return policy_engine{state, {}, {}}; };
    tcpip_client client;
    const auto port = client.find_available_port();
    DOBA_EXPECT(port != 0);
    tr::tcp_policies policies;
    policies.receive_buffer_capacity = 3;
    policies.worker_count = 1;
    policies.listen_backlog = backlog;
    policies.max_buffered_send_bytes = 64;
    policies.max_connections = 1;
    policies.ip = "127.0.0.1";
    policies.port = std::to_string(port);
    tr::tcp<policy_engine, decltype(factory)> transport(policies, factory);
    policies.receive_buffer_capacity = policies.max_buffered_send_bytes = 0;
    policies.ip = policies.port = "invalid";
    martianlabs::doba::tests::scope_exit cleanup([&] {
      client.close();
      transport.stop();
    });
    transport.start();
    DOBA_EXPECT(client.connect(port));
    const std::string input = "abcdefghijklmnopqrstuvwx";
    DOBA_EXPECT(client.send_all(input));
    const auto output = client.receive(input.size(), 5s);
    DOBA_EXPECT(output && *output == input);
    DOBA_EXPECT(state->bounded.load());
    DOBA_EXPECT(state->calls.load() >= 8);
  }
}

// +===========================================================================+
// | [>] automatic workers and unlimited admission               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("TCP automatic workers serve input within the default RX block") {
  auto state = std::make_shared<policy_state>();
  auto factory = [state] { return policy_engine{state, {}, {}}; };
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<policy_engine, decltype(factory)> transport(
      {.worker_count = 0, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  martianlabs::doba::tests::scope_exit cleanup([&] {
    client.close();
    transport.stop();
  });
  transport.start();
  DOBA_EXPECT(client.connect(port));
  const std::string input(5000, 'x');
  DOBA_EXPECT(client.send_all(input));
  const auto output = client.receive(input.size(), 5s);
  DOBA_EXPECT(output && *output == input);
  DOBA_EXPECT(state->bounded.load());
  DOBA_EXPECT(state->calls.load() >= 2);
}

DOBA_TEST("TCP zero admission limit accepts multiple live connections") {
  auto state = std::make_shared<policy_state>();
  auto factory = [state] {
    ++state->created;
    return policy_engine{state, {}, {}};
  };
  tcpip_client probe;
  const auto port = probe.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<policy_engine, decltype(factory)> transport(
      {.worker_count = 4,
       .max_connections = 0,
       .ip = "127.0.0.1",
       .port = std::to_string(port)},
      factory);
  std::vector<std::unique_ptr<tcpip_client>> peers;
  martianlabs::doba::tests::scope_exit cleanup([&] {
    peers.clear();
    transport.stop();
  });
  transport.start();
  for (int i = 0; i < 8; ++i) {
    auto peer = std::make_unique<tcpip_client>();
    DOBA_EXPECT(peer->connect(port));
    DOBA_EXPECT(peer->send_all("ok"));
    const auto output = peer->receive(2, 5s);
    DOBA_EXPECT(output && *output == "ok");
    peers.push_back(std::move(peer));
  }
  DOBA_EXPECT_EQUAL(state->created.load(), 8);
}

// +===========================================================================+
// | [>] exact TX budget and overflow isolation                  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("TCP accepts exact TX capacity and isolates one-byte overflow") {
  auto state = std::make_shared<policy_state>();
  state->fixed_output = true;
  auto factory = [state] { return policy_engine{state, {}, {}}; };
  tcpip_client probe;
  const auto port = probe.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<policy_engine, decltype(factory)> transport(
      {.receive_buffer_capacity = 1,
       .worker_count = 1,
       .max_buffered_send_bytes = 5,
       .ip = "127.0.0.1",
       .port = std::to_string(port)},
      factory);
  martianlabs::doba::tests::scope_exit cleanup([&] { transport.stop(); });
  transport.start();
  for (const std::string command : {"E", "X", "E"}) {
    tcpip_client client;
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all(command));
    if (command == "X") {
      const auto output = client.receive_until_close(1, 5s);
      DOBA_EXPECT((output && output->empty()) || client.connection_reset());
      DOBA_EXPECT(client.error() != "timeout");
    } else {
      const auto output = client.receive(5, 5s);
      DOBA_EXPECT(output && *output == "12345");
      DOBA_EXPECT(client.wait_for_close(5s));
    }
  }
}

// +===========================================================================+
// | [>] admission before engine construction and recovery       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("TCP finite admission rejects before allocation and recovers") {
  auto state = std::make_shared<policy_state>();
  auto factory = [state] {
    ++state->created;
    return policy_engine{state, {}, {}};
  };
  tcpip_client live, excess, healthy;
  const auto port = live.find_available_port();
  DOBA_EXPECT(port != 0);
  tr::tcp<policy_engine, decltype(factory)> transport(
      {.worker_count = 1,
       .max_connections = 1,
       .ip = "127.0.0.1",
       .port = std::to_string(port)},
      factory);
  martianlabs::doba::tests::scope_exit cleanup([&] {
    live.close();
    excess.close();
    healthy.close();
    transport.stop();
  });
  transport.start();
  DOBA_EXPECT(live.connect(port));
  DOBA_EXPECT(live.send_all("ok"));
  const auto first = live.receive(2, 5s);
  DOBA_EXPECT(first && *first == "ok");
  DOBA_EXPECT(excess.connect(port));
  DOBA_EXPECT(excess.wait_for_close(5s));
  DOBA_EXPECT_EQUAL(state->created.load(), 1);
  live.close();
  bool recovered = false;
  const auto deadline = std::chrono::steady_clock::now() + 5s;
  do {
    healthy.close();
    if (healthy.connect(port, 500ms) && healthy.send_all("ok", 500ms)) {
      const auto reply = healthy.receive(2, 500ms);
      recovered = reply && *reply == "ok";
    }
  } while (!recovered && std::chrono::steady_clock::now() < deadline);
  DOBA_EXPECT(recovered);
}

DOBA_TEST(
    "TCP saturated admission preserves live peers and recovers repeatedly") {
  for (const std::size_t workers : {1U, 4U}) {
    constexpr std::size_t capacity = 16;
    auto state = std::make_shared<policy_state>();
    state->receive_limit = 3;
    auto factory = [state] {
      ++state->created;
      return policy_engine{state, {}, {}};
    };
    tcpip_client probe;
    const auto port = probe.find_available_port();
    DOBA_EXPECT(port != 0);
    tr::tcp<policy_engine, decltype(factory)> transport(
        {.receive_buffer_capacity = 3,
         .worker_count = workers,
         .max_buffered_send_bytes = 64,
         .max_connections = capacity,
         .ip = "127.0.0.1",
         .port = std::to_string(port)},
        factory);
    std::vector<std::unique_ptr<tcpip_client>> live;
    martianlabs::doba::tests::scope_exit cleanup([&] {
      live.clear();
      transport.stop();
    });
    transport.start();
    for (unsigned cycle = 0; cycle < 8; ++cycle) {
      const auto deadline = std::chrono::steady_clock::now() + 10s;
      while (live.size() < capacity &&
             std::chrono::steady_clock::now() < deadline) {
        auto peer = std::make_unique<tcpip_client>();
        if (!peer->connect(port, 500ms) || !peer->send_all("ok", 500ms)) {
          continue;
        }
        const auto output = peer->receive(2, 500ms);
        if (output && *output == "ok") live.push_back(std::move(peer));
      }
      DOBA_EXPECT_EQUAL(live.size(), capacity);
      const auto created = state->created.load();
      martianlabs::doba::tests::manual_event begin;
      std::vector<std::future<bool>> excess;
      martianlabs::doba::tests::scope_exit release_start([&] { begin.open(); });
      for (unsigned i = 0; i < 32; ++i) {
        excess.push_back(std::async(std::launch::async, [&] {
          begin.wait();
          tcpip_client peer;
          return peer.connect(port) && peer.wait_for_close(5s);
        }));
      }
      begin.open();
      for (std::size_t i = 0; i < live.size(); ++i) {
        std::string payload(33, static_cast<char>(i + cycle));
        payload.front() = '\0';
        DOBA_EXPECT(live[i]->send_all(payload));
        const auto output = live[i]->receive(payload.size(), 5s);
        DOBA_EXPECT(output && *output == payload);
      }
      for (auto& result : excess) DOBA_EXPECT(result.get());
      DOBA_EXPECT_EQUAL(state->created.load(), created);
      DOBA_EXPECT(state->bounded.load());
      // Free half the slots; the next cycle must refill them without a restart.
      live.resize(capacity / 2);
    }
  }
}
