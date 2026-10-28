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

#include "protocol/http/v11/engine.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
#include "test_helper.h"
#include <coroutine>
#include <string>
#include <string_view>
#include <utility>

namespace {
using namespace martianlabs::doba::protocol::http::v11;
using martianlabs::doba::common::reader;
using martianlabs::doba::protocol::deserialization_status;
using martianlabs::doba::protocol::http::router;
} // namespace

DOBA_TEST("pending HTTP bytes enforce exact boundaries before copying") {
  for (std::size_t size : {7u, 8u, 9u}) {
    detail::engine_async pending(128, {}, {}, 8);
    std::string wire;
    int closes = 0;
    pending.set_on_send([&](auto head, auto body, std::unique_ptr<reader>) {
      wire += head;
      wire += body;
    });
    pending.set_on_close([&] { ++closes; });
    DOBA_EXPECT(pending.reserve().has_value());
    DOBA_EXPECT(pending.reserve().has_value());
    std::string body(size - 1, 'b');
    pending.submit(1, {"h", body, nullptr}, true, false);
    DOBA_EXPECT_EQUAL(closes, size > 8 ? 1 : 0);
    if (size <= 8) {
      DOBA_EXPECT(pending.pressured());
      pending.submit(0, {"first", {}, nullptr}, true, false);
      DOBA_EXPECT_EQUAL(wire, "firsth" + body);
      DOBA_EXPECT(!pending.pressured());
    }
  }
}

DOBA_TEST("pending interim bytes accumulate and zero removes the byte cap") {
  for (std::size_t budget : {0u, 8u}) {
    detail::engine_async pending(0, {}, {}, budget);
    std::string wire;
    int closes = 0;
    pending.set_on_send([&](auto h, auto b, std::unique_ptr<reader>) {
      wire += h;
      wire += b;
    });
    pending.set_on_close([&] { ++closes; });
    pending.reserve();
    pending.reserve();
    pending.submit(1, {"1234", {}, nullptr}, false, false);
    pending.submit(1, {{}, "5678", nullptr}, false, false);
    pending.submit(1, {"9", {}, nullptr}, true, false);
    DOBA_EXPECT_EQUAL(closes, budget ? 1 : 0);
    pending.submit(0, {"0", {}, nullptr}, true, false);
    DOBA_EXPECT_EQUAL(wire, budget ? "" : "0123456789");
  }
}

DOBA_TEST(
    "HTTP flow pauses within a receive block and resumes without reentrancy") {
  router<request, response> routes;
  int calls = 0;
  bool pressure = false;
  routes.add("GET", "/", [&](const request &, response &res) {
    ++calls;
    res.ok_200().set_body("ok");
  });
  engine<request, response, decltype(routes)> value({}, routes);
  value.set_on_close([] {});
  value.set_on_send(
      [&](auto, auto, std::unique_ptr<reader>) { pressure = true; });
  value.set_input_flow_control([&] { return pressure; }, [] {});
  std::string request_wire = "GET / HTTP/1.1\r\nHost: a\r\n\r\n";
  value.on_bytes_received(request_wire + request_wire + request_wire);
  DOBA_EXPECT_EQUAL(calls, 1);
  DOBA_EXPECT(value.input_paused());
  value.resume_input();
  DOBA_EXPECT_EQUAL(calls, 1);
  pressure = false;
  value.resume_input();
  DOBA_EXPECT_EQUAL(calls, 2);
  pressure = false;
  value.resume_input();
  DOBA_EXPECT_EQUAL(calls, 3);
  pressure = false;
  value.resume_input();
  DOBA_EXPECT(!value.input_paused());
}

DOBA_TEST(
    "decoder bounds huge incomplete heads and accepts exact complete heads") {
  policies p;
  p.max_request_head_size = 64;
  for (std::size_t size : {63u, 64u, 65u, 1024u * 1024u}) {
    decoder<request, response> value(p);
    std::string prefix = "GET / HTTP/1.1\r\nHost: a\r\nX: ";
    std::string wire =
        prefix + std::string(size - prefix.size() - 4, 'x') + "\r\n\r\n";
    std::string_view input(wire);
    auto result = value.deserialize(input);
    DOBA_EXPECT_EQUAL(result.code,
                      size <= 64 ? deserialization_status::kSucceeded
                                 : deserialization_status::kInvalidSource);
    if (size > 64) {
      DOBA_EXPECT(result.response.has_value());
      auto output = result.response->serialize();
      DOBA_EXPECT(output.head.starts_with("HTTP/1.1 431"));
    }
  }
  decoder<request, response> incomplete(p);
  std::string wire =
      "GET / HTTP/1.1\r\nHost: a\r\nX: " + std::string(1000000, 'x');
  std::string_view input(wire.data(), 20);
  DOBA_EXPECT_EQUAL(incomplete.deserialize(input).code,
                    deserialization_status::kMoreBytesNeeded);
  input = std::string_view(wire).substr(20);
  auto result = incomplete.deserialize(input);
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
  DOBA_EXPECT(result.response->serialize().head.starts_with("HTTP/1.1 431"));
}

DOBA_TEST("HTTP flow does not poll pressure without input and still retains a pressured block") {
  router<request, response> routes;
  int calls = 0, polls = 0;
  bool pressure = false;
  routes.add("GET", "/", [&](const request&, response& res) {
    ++calls;
    res.ok_200().set_body("ok");
  });
  engine<request, response, decltype(routes)> value({}, routes);
  value.set_on_close([] {});
  value.set_on_send([](auto, auto, std::unique_ptr<reader>) {});
  value.set_input_flow_control(
      [&] {
        ++polls;
        return pressure;
      },
      [] {});
  value.on_bytes_received({});
  value.resume_input();
  DOBA_EXPECT_EQUAL(polls, 0);
  const std::string wire = "GET / HTTP/1.1\r\nHost: a\r\n\r\n";
  value.on_bytes_received(wire);
  DOBA_EXPECT_EQUAL(calls, 1);
  DOBA_EXPECT_EQUAL(polls, 1);
  pressure = true;
  value.on_bytes_received(wire + wire);
  DOBA_EXPECT(value.input_paused());
  DOBA_EXPECT_EQUAL(calls, 1);
  pressure = false;
  value.resume_input();
  DOBA_EXPECT_EQUAL(calls, 3);
  DOBA_EXPECT(!value.input_paused());
  const auto before = polls;
  value.resume_input();
  value.on_bytes_received({});
  DOBA_EXPECT_EQUAL(polls, before);
}

namespace {
struct demand_gate {
  std::coroutine_handle<> handle;
  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> next) noexcept { handle = next; }
  void await_resume() const noexcept {}
  void resume() {
    if (auto next = std::exchange(handle, {})) next.resume();
  }
};
}  // namespace

DOBA_TEST(
    "HTTP demand pressure subscribes on arriving input and resumes after async "
    "credit") {
  router<request, response> routes;
  demand_gate gate;
  int calls = 0, replies = 0, wakes = 0, closes = 0;
  routes.add("GET", "/",
             [&](const request&,
                 response& res) -> martianlabs::doba::common::task<void> {
               ++calls;
               co_await gate;
               res.ok_200().set_body("ok");
             });
  policies limits;
  limits.max_pending_requests = 1;
  engine<request, response, decltype(routes)> value(limits, routes);
  struct cleanup {
    demand_gate& gate;
    ~cleanup() { gate.resume(); }
  } finish{gate};
  value.set_on_close([&] { ++closes; });
  value.set_on_send([&](auto, auto, std::unique_ptr<reader>) { ++replies; });
  value.set_input_flow_control([] { return false; }, [&] { ++wakes; });
  const std::string wire = "GET / HTTP/1.1\r\nHost: a\r\n\r\n";
  value.on_bytes_received(wire);
  DOBA_EXPECT_EQUAL(calls, 1);
  DOBA_EXPECT(!value.input_paused());
  value.on_bytes_received(wire);
  DOBA_EXPECT_EQUAL(calls, 1);
  DOBA_EXPECT(value.input_paused());
  gate.resume();
  DOBA_EXPECT_EQUAL(wakes, 1);
  value.resume_input();
  DOBA_EXPECT_EQUAL(calls, 2);
  DOBA_EXPECT(!value.input_paused());
  gate.resume();
  DOBA_EXPECT_EQUAL(replies, 2);
  DOBA_EXPECT_EQUAL(wakes, 1);
  DOBA_EXPECT_EQUAL(closes, 0);
}
