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
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "../../../helpers/concurrency.h"
#include "http_server.h"
#include "http_test_helper.h"
#include "tcpip_client.h"
#include "test_helper.h"

namespace {
using namespace martianlabs::doba;
using protocol::processing_result;
using clock_type = processing_result::clock;
using tests::integration::tcpip_client;
using tests::integration::receive_http_response;
using tests::integration::wait_for_http_count;
using protocol::http::v11::request;
using protocol::http::v11::response;
using namespace std::chrono_literals;
namespace tr = transport::server;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] turn_engine                                                ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct turn_engine {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  protocol::send_delegate send;
  std::function<void()> close;
  std::function<processing_result(turn_engine&, std::string_view)> input;
  std::function<processing_result(turn_engine&, bool)> turn;
  int step{0};
  clock_type::time_point due{};
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(protocol::send_delegate value) { send = std::move(value); }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> value) { close = std::move(value); }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view bytes) {
    return input(*this, bytes);
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool idle) { return turn(*this, idle); }
};

}  // namespace

DOBA_TEST(
    "Windows delivers output-idle processing without a socket write edge") {
  std::atomic<std::size_t> calls{0};
  auto factory = [&] {
    turn_engine engine;
    engine.input = [](auto&, auto) {
      return processing_result{false, {}, true};
    };
    engine.turn = [&](auto& current, bool idle) {
      if (idle) ++calls;
      current.send("idle", {}, nullptr);
      current.close();
      return processing_result{true, clock_type::time_point{}, true};
    };
    return engine;
  };
  tcpip_client peer;
  const auto port = peer.find_available_port();
  tr::tcp<turn_engine, decltype(factory)> server(
      {.worker_count = 1, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  server.start();
  DOBA_EXPECT(peer.connect(port));
  DOBA_EXPECT(peer.send_all("x"));
  const auto bytes = peer.receive(4);
  DOBA_EXPECT(bytes.has_value());
  DOBA_EXPECT_EQUAL(*bytes, "idle");
  DOBA_EXPECT(peer.wait_for_close(3s));
  server.stop();
  DOBA_EXPECT_EQUAL(calls.load(), 1U);
}

DOBA_TEST("Windows preserves a future deadline while an immediate turn runs") {
  std::atomic<bool> early{false};
  std::atomic<std::size_t> calls{0};
  auto factory = [&] {
    turn_engine engine;
    engine.input = [](auto& current, auto) {
      current.due = clock_type::now() + 5ms;
      return processing_result{true, current.due, false};
    };
    engine.turn = [&](auto& current, bool) {
      ++calls;
      if (current.step++ == 0) {
        current.send("A", {}, nullptr);
        return processing_result{false, current.due, false};
      }
      early = clock_type::now() < current.due;
      current.send("B", {}, nullptr);
      current.close();
      return processing_result{};
    };
    return engine;
  };
  tcpip_client peer;
  const auto port = peer.find_available_port();
  tr::tcp<turn_engine, decltype(factory)> server(
      {.worker_count = 1, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  server.start();
  DOBA_EXPECT(peer.connect(port));
  DOBA_EXPECT(peer.send_all("x"));
  const auto bytes = peer.receive(2);
  DOBA_EXPECT(bytes.has_value());
  DOBA_EXPECT_EQUAL(*bytes, "AB");
  server.stop();
  DOBA_EXPECT_EQUAL(calls.load(), 2U);
  DOBA_EXPECT(!early.load());
}

DOBA_TEST(
    "Windows HTTP runs concurrent timed handlers with ordered mixed output") {
  tcpip_client peer;
  const auto port = peer.find_available_port();
  std::atomic<std::size_t> started{0}, completed{0};
  http_server<> server(
      {.worker_count = 1, .ip = "127.0.0.1", .port = std::to_string(port)});
  server.add_route("GET", "/wait/:id",
                   [&](const request&, response& res,
                       int id) -> common::task<> {
    ++started;
    co_await common::sleep_for(id == 1 ? 5ms : 1ms);
    ++completed;
    res.ok_200().set_body(std::to_string(id));
  });
  server.add_route("GET", "/sync", [](const request&, response& res) {
    res.ok_200().set_body("sync");
  });
  server.start();
  DOBA_EXPECT(peer.connect(port));
  std::string batch;
  for (int i = 1; i <= 3; ++i) {
    batch += "GET /wait/" + std::to_string(i) +
             " HTTP/1.1\r\nHost: local\r\n\r\n";
  }
  batch += "GET /sync HTTP/1.1\r\nHost: local\r\n\r\n";
  DOBA_EXPECT(peer.send_all(batch));
  for (const auto expected : {"1", "2", "3", "sync"}) {
    const auto result = receive_http_response(peer);
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->body, expected);
  }
  server.stop();
  DOBA_EXPECT_EQUAL(started.load(), 3U);
  DOBA_EXPECT_EQUAL(completed.load(), 3U);
}

DOBA_TEST("Windows sleeping HTTP connection leaves the same worker available") {
  tcpip_client slow, fast;
  const auto port = slow.find_available_port();
  std::atomic<std::size_t> started{0}, live{0};
  http_server<> server(
      {.worker_count = 1, .ip = "127.0.0.1", .port = std::to_string(port)});
  server.add_route("GET", "/wait",
                   [&](const request&, response&) -> common::task<> {
    ++live;
    auto guard = std::shared_ptr<int>(new int, [&](int* value) {
      delete value;
      --live;
    });
    ++started;
    co_await common::sleep_for(1h);
  });
  server.add_route("GET", "/fast", [](const request&, response& res) {
    res.ok_200().set_body("fast");
  });
  server.start();
  DOBA_EXPECT(slow.connect(port));
  DOBA_EXPECT(slow.send_all("GET /wait HTTP/1.1\r\nHost: local\r\n\r\n"));
  DOBA_EXPECT(wait_for_http_count(started, 1));
  DOBA_EXPECT(fast.connect(port));
  DOBA_EXPECT(fast.send_all("GET /fast HTTP/1.1\r\nHost: local\r\n\r\n"));
  const auto result = receive_http_response(fast);
  DOBA_EXPECT(result.has_value());
  DOBA_EXPECT_EQUAL(result->body, "fast");
  DOBA_EXPECT_EQUAL(live.load(), 1U);
  server.stop();
  DOBA_EXPECT_EQUAL(live.load(), 0U);
}

DOBA_TEST(
    "Windows delivers supported interest combinations and honors max time") {
  for (unsigned condition : {1U, 2U, 4U, 7U}) {
    std::atomic<std::size_t> turns{0};
    auto factory = [&] {
      turn_engine engine;
      engine.input = [condition](auto& current, auto) {
        current.send("rx", {}, nullptr);
        const auto due = (condition & 2)
            ? std::optional{clock_type::time_point::max()} : std::nullopt;
        return processing_result{(condition & 1) != 0, due,
                                  (condition & 4) != 0};
      };
      engine.turn = [&](auto& current, bool) {
        ++turns;
        current.send("turn", {}, nullptr);
        current.close();
        return processing_result{};
      };
      return engine;
    };
    tcpip_client peer;
    const auto port = peer.find_available_port();
    tr::tcp<turn_engine, decltype(factory)> server(
        {.worker_count = 1, .ip = "127.0.0.1", .port = std::to_string(port)},
        factory);
    server.start();
    DOBA_EXPECT(peer.connect(port));
    DOBA_EXPECT(peer.send_all("x"));
    const auto acknowledgement = peer.receive(2);
    DOBA_EXPECT(acknowledgement.has_value());
    DOBA_EXPECT_EQUAL(*acknowledgement, "rx");
    if (condition == 2) {
      DOBA_EXPECT_EQUAL(turns.load(), 0U);
      peer.close();
    } else {
      const auto output = peer.receive(4);
      DOBA_EXPECT(output.has_value());
      DOBA_EXPECT_EQUAL(*output, "turn");
      DOBA_EXPECT(peer.wait_for_close(3s));
    }
    server.stop();
    DOBA_EXPECT_EQUAL(turns.load(), condition == 2 ? 0U : 1U);
  }
}

DOBA_TEST("Windows deadlines progress while a source keeps output busy") {
  const std::string body(16 * 1024 * 1024, 's');
  std::promise<bool> timer_fired;
  auto fired = timer_fired.get_future();
  auto factory = [&] {
    turn_engine engine;
    engine.input = [&](auto& current, auto) {
      current.send("prefix", {}, std::make_unique<common::reader>(
          common::reader::borrowed(std::as_bytes(std::span(body)))));
      current.due = clock_type::now() + 5ms;
      return processing_result{false, current.due, true};
    };
    engine.turn = [&](auto&, bool idle) {
      timer_fired.set_value(idle);
      return processing_result{};
    };
    return engine;
  };
  tcpip_client peer;
  DOBA_EXPECT(peer.set_receive_buffer_size(4096));
  const auto port = peer.find_available_port();
  tr::tcp<turn_engine, decltype(factory)> server(
      {.worker_count = 2, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  server.start();
  tests::scope_exit close_peer([&] { peer.close(); });
  DOBA_EXPECT(peer.connect(port));
  DOBA_EXPECT(peer.send_all("x"));
  DOBA_EXPECT(fired.wait_for(3s) == std::future_status::ready);
  DOBA_EXPECT(!fired.get());
  // Windows stop drains output; disconnect the peer that isn't reading.
  peer.close();
  server.stop();
}

DOBA_TEST("Windows replaces deadline interests and cancels across restart") {
  std::atomic<std::size_t> turns{0};
  auto factory = [&] {
    turn_engine engine;
    engine.input = [](auto& current, auto bytes) {
      current.send("ok", {}, nullptr);
      if (bytes == "a") {
        return processing_result{false, clock_type::time_point::max(), false};
      }
      return processing_result{};
    };
    engine.turn = [&](auto&, bool) { ++turns; return processing_result{}; };
    return engine;
  };
  tcpip_client peer;
  const auto port = peer.find_available_port();
  tr::tcp<turn_engine, decltype(factory)> server(
      {.worker_count = 4, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  for (int run = 0; run < 3; ++run) {
    server.start();
    DOBA_EXPECT(peer.connect(port));
    for (const auto bytes : {"a", "b", "a"}) {
      DOBA_EXPECT(peer.send_all(bytes));
      const auto reply = peer.receive(2);
      DOBA_EXPECT(reply && *reply == "ok");
    }
    server.stop();
    peer.close();
  }
  DOBA_EXPECT_EQUAL(turns.load(), 0U);
}

DOBA_TEST("Windows requeued immediate turns preserve serialized progress") {
  std::atomic<std::size_t> turns{0};
  auto factory = [&] {
    turn_engine engine;
    engine.input = [](auto&, auto) {
      return processing_result{true, {}, false};
    };
    engine.turn = [&](auto& current, bool) {
      ++turns;
      if (++current.step < 1024) return processing_result{true, {}, false};
      current.send("done", {}, nullptr);
      current.close();
      return processing_result{};
    };
    return engine;
  };
  tcpip_client peer;
  const auto port = peer.find_available_port();
  tr::tcp<turn_engine, decltype(factory)> server(
      {.worker_count = 4, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  server.start();
  DOBA_EXPECT(peer.connect(port));
  DOBA_EXPECT(peer.send_all("x"));
  const auto reply = peer.receive(4);
  DOBA_EXPECT(reply && *reply == "done");
  DOBA_EXPECT(peer.wait_for_close(3s));
  server.stop();
  DOBA_EXPECT_EQUAL(turns.load(), 1024U);
}

DOBA_TEST("Windows delivers overdue results immediately without dropping any") {
  std::atomic<std::size_t> admitted{0}, turns{0};
  const auto due = clock_type::now() - 1ms;
  auto factory = [&] {
    turn_engine engine;
    engine.input = [&](auto&, auto) {
      ++admitted;
      return processing_result{false, due, false};
    };
    engine.turn = [&](auto& current, bool) {
      ++turns;
      current.send("ok", {}, nullptr);
      return processing_result{};
    };
    return engine;
  };
  tcpip_client probe;
  const auto port = probe.find_available_port();
  tr::tcp<turn_engine, decltype(factory)> server(
      {.worker_count = 4, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  server.start();
  std::vector<std::unique_ptr<tcpip_client>> peers;
  for (int i = 0; i < 65; ++i) {
    auto peer = std::make_unique<tcpip_client>();
    DOBA_EXPECT(peer->connect(port));
    DOBA_EXPECT(peer->send_all("x"));
    peers.push_back(std::move(peer));
  }
  for (const auto& peer : peers) {
    const auto reply = peer->receive(2);
    DOBA_EXPECT(reply && *reply == "ok");
  }
  server.stop();
  DOBA_EXPECT_EQUAL(admitted.load(), 65U);
  DOBA_EXPECT_EQUAL(turns.load(), 65U);
}
