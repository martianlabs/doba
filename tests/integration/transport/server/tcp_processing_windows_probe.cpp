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

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "../../../helpers/concurrency.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "transport/server/tcp_windows_overlapped_base.h"

namespace {
using namespace std::chrono_literals;
using martianlabs::doba::transport::server::detail::io_type;
using martianlabs::doba::transport::server::detail::overlapped_base;
// Only this executable intercepts native APIs. No production test hooks exist.
std::atomic<unsigned> timer_creations{0}, timer_arms{0};
std::atomic<bool> watch{false}, immediate_seen{false}, timer_seen{false};
std::atomic<bool> timed_out{false};
std::atomic<bool> fail_immediate{false}, fail_timer_post{false};
std::atomic<bool> fail_timer_create{false}, hold_timer{false};
martianlabs::doba::tests::manual_event release_timer;

// +=========================================================================+
// | [>] wait_until                                              ( function ) |
// +-------------------------------------------------------------------------+
bool wait_until(const std::function<bool()>& predicate) {
  const auto end = std::chrono::steady_clock::now() + 5s;
  do {
    if (predicate()) return true;
    std::this_thread::yield();
  } while (std::chrono::steady_clock::now() < end);
  return false;
}

// +=========================================================================+
// | [>] tracked_create                                          ( function ) |
// +-------------------------------------------------------------------------+
PTP_TIMER WINAPI tracked_create(PTP_TIMER_CALLBACK callback, void* state,
                                PTP_CALLBACK_ENVIRON environment) {
  if (fail_timer_create.exchange(false)) return nullptr;
  ++timer_creations;
  return ::CreateThreadpoolTimer(callback, state, environment);
}

// +=========================================================================+
// | [>] tracked_arm                                             ( function ) |
// +-------------------------------------------------------------------------+
void WINAPI tracked_arm(PTP_TIMER timer, PFILETIME due, DWORD period,
                         DWORD window) {
  if (due) ++timer_arms;
  ::SetThreadpoolTimer(timer, due, period, window);
}

// +=========================================================================+
// | [>] tracked_post                                            ( function ) |
// +-------------------------------------------------------------------------+
BOOL WINAPI tracked_post(HANDLE port, DWORD bytes, ULONG_PTR key,
                          LPOVERLAPPED packet) {
  const auto type = packet ? static_cast<overlapped_base*>(packet)->get_type()
                           : io_type::kAccept;
  if ((type == io_type::kProcessing && fail_immediate.exchange(false)) ||
      (type == io_type::kProcessingTimer && fail_timer_post.exchange(false))) {
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
  }
  const auto result = ::PostQueuedCompletionStatus(port, bytes, key, packet);
  const auto error = GetLastError();
  if (watch && result && type == io_type::kProcessing) {
    if (!wait_until([] { return immediate_seen.load(); })) timed_out = true;
  }
  if (watch && result && type == io_type::kProcessingTimer) {
    if (!wait_until([] { return timer_seen.load(); })) timed_out = true;
  }
  SetLastError(error);
  return result;
}

// +=========================================================================+
// | [>] tracked_completion                                      ( function ) |
// +-------------------------------------------------------------------------+
BOOL WINAPI tracked_completion(HANDLE port, LPDWORD bytes, PULONG_PTR key,
                                LPOVERLAPPED* packet, DWORD timeout) {
  const auto result = ::GetQueuedCompletionStatus(port, bytes, key,
                                                 packet, timeout);
  const auto error = GetLastError();
  if (*packet) {
    const auto type = static_cast<overlapped_base*>(*packet)->get_type();
    if (type == io_type::kProcessing) immediate_seen = true;
    if (type == io_type::kProcessingTimer) {
      timer_seen = true;
      if (hold_timer) {
        if (!release_timer.wait_for(5s)) timed_out = true;
      }
    }
  }
  SetLastError(error);
  return result;
}
}  // namespace

#define SetThreadpoolTimer tracked_arm
#define CreateThreadpoolTimer tracked_create
#define PostQueuedCompletionStatus tracked_post
#define GetQueuedCompletionStatus tracked_completion
#include "transport/server/tcp.h"
#undef GetQueuedCompletionStatus
#undef PostQueuedCompletionStatus
#undef CreateThreadpoolTimer
#undef SetThreadpoolTimer

namespace {
using namespace martianlabs::doba;
using protocol::processing_result;
using clock_type = processing_result::clock;
using tests::integration::tcpip_client;
namespace tr = transport::server;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] publication_engine                                         ( struct ) |
// +---------------------------------------------------------------------------+
// | Generic connection-turn probe; it contains no coroutine execution.        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct publication_engine {
  // +=========================================================================+
  // | [>] USINGs                                                  ( public  ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( public  ) |
  // +-------------------------------------------------------------------------+
  protocol::send_delegate send;
  std::function<void()> close;
  std::function<processing_result(publication_engine&, std::string_view)> input;
  std::function<processing_result(publication_engine&, bool)> turn;
  clock_type::time_point due{};
  unsigned step{0};
  // +=========================================================================+
  // | [>] set_on_send                                             ( public  ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(protocol::send_delegate value) { send = std::move(value); }
  // +=========================================================================+
  // | [>] set_on_close                                            ( public  ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> value) { close = std::move(value); }
  // +=========================================================================+
  // | [>] on_bytes_received                                       ( public  ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view bytes) {
    return input(*this, bytes);
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                      ( public  ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool idle) { return turn(*this, idle); }
};
}  // namespace

DOBA_TEST("Windows IOCP processing can dequeue before publication returns") {
  immediate_seen = false;
  timer_seen = false;
  timed_out = false;
  watch = true;
  tests::scope_exit unwatch([] { watch = false; });
  auto factory = [] {
    publication_engine engine;
    engine.input = [](auto&, auto) {
      return processing_result{true, {}, false};
    };
    engine.turn = [](auto& current, bool) {
      if (current.step++ == 0) {
        current.due = clock_type::now() + 1ms;
        current.send("A", {}, nullptr);
        return processing_result{false, current.due, false};
      }
      current.send("B", {}, nullptr);
      current.close();
      return processing_result{};
    };
    return engine;
  };
  tcpip_client peer;
  const auto port = peer.find_available_port();
  tr::tcp<publication_engine, decltype(factory)> server(
      {.worker_count = 2, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  server.start();
  DOBA_EXPECT(peer.connect(port));
  DOBA_EXPECT(peer.send_all("x"));
  const auto reply = peer.receive(2);
  DOBA_EXPECT(reply && *reply == "AB");
  DOBA_EXPECT(peer.wait_for_close(3s));
  server.stop();
  DOBA_EXPECT(immediate_seen && timer_seen && !timed_out);
}

DOBA_TEST("Windows keeps deadlines after a failed timer wake publication") {
  fail_timer_post = true;
  auto factory = [] {
    publication_engine engine;
    engine.input = [](auto&, auto) {
      return processing_result{false, clock_type::now() + 1ms, false};
    };
    engine.turn = [](auto& current, bool) {
      current.send("ok", {}, nullptr);
      current.close();
      return processing_result{};
    };
    return engine;
  };
  tcpip_client peer;
  const auto port = peer.find_available_port();
  tr::tcp<publication_engine, decltype(factory)> server(
      {.worker_count = 2, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  server.start();
  DOBA_EXPECT(peer.connect(port));
  DOBA_EXPECT(peer.send_all("x"));
  const auto reply = peer.receive(2);
  DOBA_EXPECT(reply && *reply == "ok");
  DOBA_EXPECT(peer.wait_for_close(3s));
  server.stop();
  DOBA_EXPECT(!fail_timer_post);
}

DOBA_TEST(
    "Windows scheduling allocation and publication failures close safely") {
  for (const bool timer : {false, true}) {
    fail_immediate = !timer;
    fail_timer_create = timer;
    auto factory = [timer] {
      publication_engine engine;
      engine.input = [timer](auto&, auto) {
        return timer
            ? processing_result{false, clock_type::now() + 1h, false}
            : processing_result{true, {}, false};
      };
      engine.turn = [](auto&, bool) { return processing_result{}; };
      return engine;
    };
    tcpip_client peer;
    const auto port = peer.find_available_port();
    tr::tcp<publication_engine, decltype(factory)> server(
        {.worker_count = 2, .ip = "127.0.0.1", .port = std::to_string(port)},
        factory);
    server.start();
    DOBA_EXPECT(peer.connect(port));
    DOBA_EXPECT(peer.send_all("x"));
    DOBA_EXPECT(peer.wait_for_close(3s));
    server.stop();
    DOBA_EXPECT(!fail_immediate && !fail_timer_create);
  }
}

DOBA_TEST("Windows creates no sync timer and one timer for many deadlines") {
  timer_creations = 0;
  timer_arms = 0;
  auto factory = [] {
    publication_engine engine;
    engine.input = [](auto& current, auto bytes) {
      current.send("ok", {}, nullptr);
      return bytes == "s" ? processing_result{}
          : processing_result{false, clock_type::time_point::max(), false};
    };
    engine.turn = [](auto&, bool) { return processing_result{}; };
    return engine;
  };
  tcpip_client probe;
  const auto port = probe.find_available_port();
  tr::tcp<publication_engine, decltype(factory)> server(
      {.worker_count = 4, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  for (unsigned run = 0; run < 2; ++run) {
    server.start();
    DOBA_EXPECT(probe.connect(port));
    DOBA_EXPECT(probe.send_all("s"));
    const auto sync = probe.receive(2);
    DOBA_EXPECT(sync && *sync == "ok");
    DOBA_EXPECT_EQUAL(timer_creations.load(), run);
    std::vector<std::unique_ptr<tcpip_client>> peers;
    for (int i = 0; i < 8; ++i) {
      auto peer = std::make_unique<tcpip_client>();
      DOBA_EXPECT(peer->connect(port));
      DOBA_EXPECT(peer->send_all("a"));
      const auto reply = peer->receive(2);
      DOBA_EXPECT(reply && *reply == "ok");
      peers.push_back(std::move(peer));
    }
    DOBA_EXPECT(wait_until([run] { return timer_creations == run + 1; }));
    server.stop();
    DOBA_EXPECT_EQUAL(timer_arms.load(), run + 1);
    probe.close();
  }
}

DOBA_TEST("Windows timer wake batches select at most 64 exact connections") {
  namespace detail = tr::detail;
  HANDLE port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
  DOBA_EXPECT(port != nullptr);
  tests::scope_exit close_port([&] { CloseHandle(port); });
  detail::windows_processing_timer<publication_engine> timer;
  timer.start(port);
  std::atomic<bool> stopping{false};
  auto factory = [] { return publication_engine{}; };
  const auto previous_arms = timer_arms.load();
  for (unsigned i = 0; i < 65; ++i) {
    auto ctx = std::make_shared<detail::context<publication_engine>>(
        INVALID_SOCKET, 128, 128, factory, stopping, nullptr, nullptr);
    DOBA_EXPECT(timer.replace(ctx, clock_type::now() - 1ms, i + 1));
  }
  std::size_t total = 0;
  // Drain notifications after producer stop as well, keeping embedded storage
  // and the IOCP alive for every packet, including early callback publication.
  tests::scope_exit stop_timer([&] { timer.stop(); });
  for (const auto expected : {64U, 1U}) {
    DWORD bytes = 0;
    ULONG_PTR key = 0;
    LPOVERLAPPED packet = nullptr;
    DOBA_EXPECT(::GetQueuedCompletionStatus(port, &bytes, &key, &packet, 5000));
    DOBA_EXPECT(packet != nullptr);
    const auto batch = timer.take_due();
    std::size_t count = 0;
    for (const auto& [ctx, version] : batch) {
      if (!ctx) continue;
      DOBA_EXPECT(version != 0);
      ++count;
    }
    DOBA_EXPECT_EQUAL(count, expected);
    total += count;
  }
  DOBA_EXPECT_EQUAL(total, 65U);
  // Already-due batches must not wait for another native timer tick.
  DOBA_EXPECT_EQUAL(timer_arms.load(), previous_arms);
}

DOBA_TEST("Windows discards a queued timer wake after its connection closes") {
  timer_seen = false;
  timed_out = false;
  hold_timer = true;
  tests::scope_exit release([] { hold_timer = false; release_timer.open(); });
  std::atomic<unsigned> turns{0}, disconnected{0};
  auto factory = [&] {
    publication_engine engine;
    engine.input = [](auto& current, auto bytes) {
      current.send("ok", {}, nullptr);
      return bytes == "a"
          ? processing_result{false, clock_type::now() + 1ms, false}
          : processing_result{};
    };
    engine.turn = [&](auto&, bool) { ++turns; return processing_result{}; };
    return engine;
  };
  tcpip_client peer;
  const auto port = peer.find_available_port();
  tr::tcp<publication_engine, decltype(factory)> server(
      {.worker_count = 2, .ip = "127.0.0.1", .port = std::to_string(port)},
      factory);
  server.set_on_disconnection([&] { ++disconnected; });
  server.start();
  tests::scope_exit unblock([] { hold_timer = false; release_timer.open(); });
  DOBA_EXPECT(peer.connect(port));
  DOBA_EXPECT(peer.send_all("a"));
  const auto first = peer.receive(2);
  DOBA_EXPECT(first && *first == "ok");
  DOBA_EXPECT(wait_until([] { return timer_seen.load(); }));
  peer.close();
  DOBA_EXPECT(wait_until([&] { return disconnected == 1; }));
  DOBA_EXPECT(peer.connect(port));
  DOBA_EXPECT(peer.send_all("s"));
  const auto next = peer.receive(2);
  DOBA_EXPECT(next && *next == "ok");
  hold_timer = false;
  release_timer.open();
  server.stop();
  DOBA_EXPECT_EQUAL(turns.load(), 0U);
  DOBA_EXPECT(!timed_out);
}
