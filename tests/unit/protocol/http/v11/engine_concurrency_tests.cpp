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
#include <barrier>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

#include "../../../../helpers/concurrency.h"
#include "common/task.h"
#include "protocol/http/v11/engine.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
#include "test_helper.h"

namespace {
// +---------------------------------------------------------------------------+
// | [>] general                                                    ( usings ) |
// +---------------------------------------------------------------------------+
using namespace martianlabs::doba;
using namespace martianlabs::doba::protocol::http::v11;
using protocol::http::router;
using protocol::http::v11::detail::engine_async;
using tests::manual_event;
using tests::scope_exit;

constexpr std::string_view slow = "GET /slow HTTP/1.1\r\nHost: a\r\n\r\n";
constexpr std::string_view fast = "GET /fast HTTP/1.1\r\nHost: a\r\n\r\n";

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] output_state                                               ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct output_state {
  std::mutex mutex;
  std::string wire;
  std::atomic<int> sends{0};
  std::atomic<int> closes{0};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] fixture                                                    ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct fixture {
  std::shared_ptr<output_state> output = std::make_shared<output_state>();
  engine<request, response> value;
  explicit fixture(const router<request, response>& routes,
                   policies config = {})
      : value(config, routes) {
    value.set_on_send([state = output](std::string_view head,
                                       std::string_view body,
                                       std::unique_ptr<common::reader>) {
      std::lock_guard lock(state->mutex);
      state->wire.append(head).append(body);
      ++state->sends;
    });
    value.set_on_close([state = output] { ++state->closes; });
  }
  void receive(std::string_view wire) { value.on_bytes_received(wire); }
};

// +---------------------------------------------------------------------------+
// | [>] respond                                                  ( function ) |
// +---------------------------------------------------------------------------+
void respond(response& res, std::string_view text) {
  res.ok_200().set_header("Date", "fixed").set_body(text);
}
}  // namespace

// +===========================================================================+
// | [>] completion contract covers both controlled stop orders  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("completion contract covers both controlled stop orders") {
  for (bool finish_first : {false, true}) {
    engine_async state(32, {}, {});
    int callbacks = 0;
    state.begin();
    if (finish_first) state.end();
    const bool immediate = state.stop([&] { ++callbacks; });
    DOBA_EXPECT_EQUAL(immediate, finish_first);
    DOBA_EXPECT_EQUAL(callbacks, 0);
    if (!finish_first) state.end();
    DOBA_EXPECT_EQUAL(callbacks, finish_first ? 0 : 1);
    DOBA_EXPECT(state.stop([&] { ++callbacks; }));
    DOBA_EXPECT_EQUAL(callbacks, finish_first ? 0 : 1);
  }
}

// +===========================================================================+
// | [>] last completion races stop without losing or            ( test-case ) |
// |     duplicating callbacks                                                 |
// +---------------------------------------------------------------------------+
DOBA_TEST(
    "last completion races stop without losing or duplicating callbacks") {
  for (int iteration = 0; iteration < 8; ++iteration) {
    engine_async state(32, {}, {});
    std::barrier start(3);
    std::atomic<int> callbacks{0};
    bool immediate = false;
    state.begin();
    std::jthread stopping([&] {
      start.arrive_and_wait();
      immediate = state.stop([&] { ++callbacks; });
    });
    std::jthread finishing([&] {
      start.arrive_and_wait();
      state.end();
    });
    start.arrive_and_wait();
    stopping.join();
    finishing.join();
    DOBA_EXPECT_EQUAL(callbacks.load(), immediate ? 0 : 1);
  }
}

DOBA_TEST("completion publication survives repeated stop races on persistent threads") {
  constexpr int rounds = 2048;
  std::unique_ptr<engine_async> current;
  std::barrier phase(3);
  std::atomic<int> callbacks{0};
  bool immediate = false;
  int mismatches = 0;
  std::jthread stopping([&] {
    for (int i = 0; i < rounds; ++i) {
      phase.arrive_and_wait();
      immediate = current->stop([&] { ++callbacks; });
      phase.arrive_and_wait();
    }
  });
  std::jthread finishing([&] {
    for (int i = 0; i < rounds; ++i) {
      phase.arrive_and_wait();
      current->end();
      phase.arrive_and_wait();
    }
  });
  for (int i = 0; i < rounds; ++i) {
    current = std::make_unique<engine_async>(32, std::function<void()>{},
                                           std::function<void()>{});
    callbacks = 0;
    current->begin();
    phase.arrive_and_wait();
    phase.arrive_and_wait();
    if (callbacks.load() != (immediate ? 0 : 1)) ++mismatches;
  }
  stopping.join();
  finishing.join();
  DOBA_EXPECT_EQUAL(mismatches, 0);
}

// +===========================================================================+
// | [>] completion sends while a successor is inside receive    ( test-case ) |
// |     dispatch                                                              |
// +---------------------------------------------------------------------------+
DOBA_TEST("completion sends while a successor is inside receive dispatch") {
  router<request, response> routes;
  manual_event pending, entered, release;
  std::atomic<bool> parsing{false};
  routes.add("GET", "/slow",
             [&](const request&, response& res) -> common::task<void> {
               co_await pending;
               respond(res, "slow");
             });
  routes.add("GET", "/fast", [&](const request&, response& res) {
    parsing = true;
    entered.open();
    release.wait();
    respond(res, "fast");
    parsing = false;
  });
  fixture current(routes);
  scope_exit resume([&] { pending.open(); });
  current.receive(slow);
  DOBA_EXPECT(pending.wait_for_waiters(1));
  std::jthread receiver([&] { current.receive(fast); });
  scope_exit unblock([&] { release.open(); });
  DOBA_EXPECT(entered.wait_for());
  pending.open();
  DOBA_EXPECT(parsing.load());
  DOBA_EXPECT_EQUAL(current.output->sends.load(), 1);
  release.open();
  receiver.join();
  DOBA_EXPECT_EQUAL(current.output->sends.load(), 2);
  const auto first = current.output->wire.find("\r\n\r\nslow");
  const auto second = current.output->wire.find("\r\n\r\nfast");
  DOBA_EXPECT(first != std::string::npos);
  DOBA_EXPECT(second != std::string::npos && first < second);
  DOBA_EXPECT_EQUAL(current.output->closes.load(), 0);
}

// +===========================================================================+
// | [>] async error closes during receive and suppresses later  ( test-case ) |
// |     dispatch                                                              |
// +---------------------------------------------------------------------------+
DOBA_TEST("async error closes during receive and suppresses later dispatch") {
  router<request, response> routes;
  manual_event pending, entered, release;
  std::atomic<int> later{0};
  routes.add("GET", "/slow",
             [&](const request&, response&) -> common::task<void> {
               co_await pending;
               throw std::runtime_error("after suspension");
             });
  routes.add("GET", "/fast", [&](const request&, response& res) {
    entered.open();
    release.wait();
    respond(res, "fast");
  });
  routes.add("GET", "/later", [&](const request&, response& res) {
    ++later;
    respond(res, "unexpected");
  });
  fixture current(routes);
  scope_exit resume([&] { pending.open(); });
  current.receive(slow);
  DOBA_EXPECT(pending.wait_for_waiters(1));
  std::jthread receiver([&] {
    current.receive(std::string(fast) +
                    "GET /later HTTP/1.1\r\nHost: a\r\n\r\n");
  });
  scope_exit unblock([&] { release.open(); });
  DOBA_EXPECT(entered.wait_for());
  pending.open();
  DOBA_EXPECT_EQUAL(current.output->closes.load(), 1);
  release.open();
  receiver.join();
  DOBA_EXPECT_EQUAL(later.load(), 0);
  DOBA_EXPECT_EQUAL(current.output->sends.load(), 1);
  DOBA_EXPECT(current.output->wire.starts_with("HTTP/1.1 500 "));
  DOBA_EXPECT_EQUAL(current.output->closes.load(), 1);
}

// +===========================================================================+
// | [>] pending limit retains frames until late completion then ( test-case ) |
// |     releases                                                              |
// +---------------------------------------------------------------------------+
DOBA_TEST("pending limit retains frames until late completion then releases") {
  router<request, response> routes;
  manual_event pending;
  std::weak_ptr<int> frame;
  routes.add("GET", "/slow",
             [&](const request&, response& res) -> common::task<void> {
               auto lifetime = std::make_shared<int>(1);
               frame = lifetime;
               co_await pending;
               respond(res, "late");
             });
  routes.add("GET", "/fast",
             [](const request&, response& res) { respond(res, "fast"); });
  policies config;
  config.max_pending_requests = 2;
  fixture current(routes, config);
  scope_exit resume([&] { pending.open(); });
  current.receive(slow);
  DOBA_EXPECT(pending.wait_for_waiters(1));
  DOBA_EXPECT(!frame.expired());
  current.receive(fast);
  current.receive(fast);
  DOBA_EXPECT_EQUAL(current.output->closes.load(), 1);
  int completions = 0;
  DOBA_EXPECT(!current.value.stop([&] { ++completions; }));
  pending.open();
  DOBA_EXPECT(frame.expired());
  DOBA_EXPECT_EQUAL(completions, 1);
  DOBA_EXPECT_EQUAL(current.output->sends.load(), 0);
}

// +===========================================================================+
// | [>] owned callbacks and frames survive facade destruction   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("owned callbacks and frames survive facade destruction") {
  router<request, response> routes;
  manual_event pending;
  std::weak_ptr<int> frame;
  routes.add("GET", "/slow",
             [&](const request&, response& res) -> common::task<void> {
               auto lifetime = std::make_shared<int>(1);
               frame = lifetime;
               co_await pending;
               respond(res, "owned");
             });
  auto current = std::make_unique<fixture>(routes);
  auto output = current->output;
  scope_exit resume([&] { pending.open(); });
  current->receive(slow);
  DOBA_EXPECT(pending.wait_for_waiters(1));
  current.reset();
  DOBA_EXPECT(!frame.expired());
  pending.open();
  DOBA_EXPECT(frame.expired());
  DOBA_EXPECT_EQUAL(output->sends.load(), 1);
  DOBA_EXPECT(output->wire.ends_with("\r\n\r\nowned"));
}

// +===========================================================================+
// | [>] reverse completion keeps queued responses and releases  ( test-case ) |
// |     both frames                                                           |
// +---------------------------------------------------------------------------+
DOBA_TEST(
    "reverse completion keeps queued responses and releases both frames") {
  router<request, response> routes;
  manual_event first, second;
  std::weak_ptr<int> first_frame, second_frame;
  routes.add("GET", "/slow",
             [&](const request&, response& res) -> common::task<void> {
               auto lifetime = std::make_shared<int>(1);
               first_frame = lifetime;
               co_await first;
               respond(res, "first");
             });
  routes.add("GET", "/fast",
             [&](const request&, response& res) -> common::task<void> {
               auto lifetime = std::make_shared<int>(2);
               second_frame = lifetime;
               co_await second;
               respond(res, "second");
             });
  fixture current(routes);
  scope_exit resume([&] {
    second.open();
    first.open();
  });
  current.receive(std::string(slow) + std::string(fast));
  DOBA_EXPECT(first.wait_for_waiters(1));
  DOBA_EXPECT(second.wait_for_waiters(1));
  second.open();
  DOBA_EXPECT(second_frame.expired());
  DOBA_EXPECT_EQUAL(current.output->sends.load(), 0);
  first.open();
  DOBA_EXPECT(first_frame.expired());
  DOBA_EXPECT_EQUAL(current.output->sends.load(), 2);
  const auto one = current.output->wire.find("\r\n\r\nfirst");
  const auto two = current.output->wire.find("\r\n\r\nsecond");
  DOBA_EXPECT(one != std::string::npos && two != std::string::npos &&
              one < two);
}

// +===========================================================================+
// | [>] engine serializes concurrent asynchronous sends         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine serializes concurrent asynchronous sends") {
  router<request, response> routes;
  using martianlabs::doba::tests::manual_event;
  manual_event first, second, entered, release, second_ready;
  routes.add("GET", "/first",
             [&first](const request&, response& res) -> common::task<void> {
               co_await first;
               respond(res, "first");
             });
  routes.add("GET", "/second",
             [&](const request&, response& res) -> common::task<void> {
               co_await second;
               respond(res, "second");
               second_ready.open();
             });
  fixture current(routes);
  std::atomic<int> sending{0};
  std::atomic<bool> overlap{false};
  current.value.set_on_send([&](std::string_view head, std::string_view body,
                                std::unique_ptr<common::reader>) {
    if (sending.fetch_add(1) != 0) overlap = true;
    if (body == "first") {
      entered.open();
      release.wait();
    }
    {
      std::lock_guard lock(current.output->mutex);
      current.output->wire.append(head).append(body);
    }
    sending--;
  });
  current.receive(
      "GET /first HTTP/1.1\r\nHost: localhost\r\n\r\n"
      "GET /second HTTP/1.1\r\nHost: localhost\r\n\r\n");
  std::jthread first_thread, second_thread;
  martianlabs::doba::tests::scope_exit unblock([&] {
    release.open();
    first.open();
    second.open();
  });
  first_thread = std::jthread([&]() { first.open(); });
  DOBA_EXPECT(entered.wait_for());
  second_thread = std::jthread([&]() { second.open(); });
  DOBA_EXPECT(second_ready.wait_for());
  release.open();
  first_thread.join();
  second_thread.join();
  DOBA_EXPECT(!overlap.load());
  const auto next = current.output->wire.find("HTTP/1.1", 1);
  DOBA_EXPECT(next != std::string::npos);
  DOBA_EXPECT(current.output->wire.substr(0, next).ends_with("\r\n\r\nfirst"));
  DOBA_EXPECT(current.output->wire.substr(next).ends_with("\r\n\r\nsecond"));
}
