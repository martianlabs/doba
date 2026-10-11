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

#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "server.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::reader;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] line_engine                                               ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct line_engine {
  struct policies_type {};

  void set_on_send(martianlabs::doba::protocol::send_delegate output) {
    on_send = std::move(output);
  }
  void set_on_close(std::function<void()>) {}
  std::size_t on_bytes_received(const char* buffer, std::size_t size,
                                std::size_t) {
    on_send("echo: ", {buffer, size}, nullptr);
    return size;
  }

  martianlabs::doba::protocol::send_delegate on_send;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] observation                                               ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct observation {
  int starts{0};
  int quiesces{0};
  int stops{0};
  int engines{0};
  bool fail_start{false};
  std::string output;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] memory_transport                                          ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ENty, typename FNty>
struct memory_transport {
  using policies_type = observation*;

  memory_transport(policies_type state, FNty factory)
      : state(state), factory(std::move(factory)) {}
  void set_on_connection(std::function<void()> callback) {
    on_connection = std::move(callback);
  }
  void set_on_disconnection(std::function<void()> callback) {
    on_disconnection = std::move(callback);
  }
  void start() {
    if (state->fail_start) throw std::runtime_error("start failed");
    state->starts++;
    on_connection();
    auto engine = factory();
    engine.set_on_send([this](std::string_view head, std::string_view body,
                              std::unique_ptr<reader>) {
      state->output.append(head);
      state->output.append(body);
    });
    engine.set_on_close([]() {});
    constexpr char input[] = "line\n";
    engine.on_bytes_received(input, sizeof(input) - 1, 64);
  }
  void quiesce() { state->quiesces++; }
  void stop() {
    state->stops++;
    on_disconnection();
  }

  policies_type state;
  FNty factory;
  std::function<void()> on_connection;
  std::function<void()> on_disconnection;
};
}  // namespace

// +===========================================================================+
// | [>] server composes a non-HTTP engine                      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("server composes a non-HTTP engine") {
  observation observed;
  auto create_engine = [&observed]() {
    observed.engines++;
    return line_engine{};
  };
  using transport_type = memory_transport<line_engine,
                                          decltype(create_engine)>;
  using server_type = martianlabs::doba::server<line_engine, transport_type>;
  static_assert(!std::is_copy_constructible_v<server_type>);
  static_assert(!std::is_move_constructible_v<server_type>);
  {
    server_type value(&observed, create_engine);
    value.start();
    value.start();
    DOBA_EXPECT_EQUAL(observed.starts, 1);
    DOBA_EXPECT_EQUAL(observed.engines, 1);
    DOBA_EXPECT_EQUAL(observed.output, "echo: line\n");
    value.stop();
    value.stop();
    DOBA_EXPECT_EQUAL(observed.quiesces, 1);
    DOBA_EXPECT_EQUAL(observed.stops, 1);
    value.start();
    DOBA_EXPECT_EQUAL(observed.starts, 2);
  }
  DOBA_EXPECT_EQUAL(observed.quiesces, 2);
  DOBA_EXPECT_EQUAL(observed.stops, 2);
}

// +===========================================================================+
// | [>] server retries after transport startup failure         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("server retries after transport startup failure") {
  observation observed;
  observed.fail_start = true;
  auto create_engine = []() { return line_engine{}; };
  using transport_type = memory_transport<line_engine,
                                          decltype(create_engine)>;
  martianlabs::doba::server<line_engine, transport_type> value(
      &observed, create_engine);
  bool failed = false;
  try {
    value.start();
  } catch (const std::runtime_error&) {
    failed = true;
  }
  DOBA_EXPECT(failed);
  DOBA_EXPECT_EQUAL(observed.stops, 0);
  observed.fail_start = false;
  value.start();
  DOBA_EXPECT_EQUAL(observed.starts, 1);
  value.stop();
  DOBA_EXPECT_EQUAL(observed.stops, 1);
}
