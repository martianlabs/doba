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
#include <array>
#include <atomic>
#include <string>
#include <chrono>
#include <cstdio>
#include <stdexcept>
#include <thread>

#include "http_server.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "../../helpers/concurrency.h"
#include "common/task.h"

// +===========================================================================+
// | [>] probe passes                                            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("probe passes") {
  std::puts("probe body");
  DOBA_EXPECT(true);
}

// +===========================================================================+
// | [>] probe assertion fails                                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("probe assertion fails") {
  DOBA_EXPECT(2 + 2 == 5);
}

// +===========================================================================+
// | [>] probe standard exception                                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("probe standard exception") {
  throw std::runtime_error("probe standard error");
}

// +===========================================================================+
// | [>] probe unknown exception                                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("probe unknown exception") {
  throw 7;
}

// +===========================================================================+
// | [>] probe continues after failures                          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("probe continues after failures") {
  std::puts("probe continued");
  DOBA_EXPECT(true);
}

// +===========================================================================+
// | [>] probe blocks                                            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("probe blocks") {
  struct blocked_cleanup {
    ~blocked_cleanup() {
      std::puts("probe blocked cleanup");
      std::fflush(stdout);
      std::this_thread::sleep_for(std::chrono::seconds(30));
    }
  } cleanup;
}

DOBA_TEST("probe failure releases suspended work and joins threads") {
  using namespace martianlabs::doba;
  tests::manual_event pending;
  std::atomic<bool> completed{false};
  tests::integration::tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  http_server<> server({.ip = "127.0.0.1", .port = std::to_string(port)});
  tests::scope_exit release_on_failure([&] { pending.open(); });
  server.add_route("GET", "/", [&](const protocol::http::v11::request&,
                                   protocol::http::v11::response& res)
      -> common::task<void> {
    co_await pending;
    completed = true;
    res.ok_200().set_body("done");
  });
  server.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("GET / HTTP/1.1\r\nHost: a\r\n\r\n"));
  DOBA_EXPECT(pending.wait_for_waiters(1));
  const auto exercise = [&]() {
    std::jthread waiter([&] {
      server.stop();
    });
    tests::scope_exit release([&] { pending.open(); });
    DOBA_EXPECT(false);
  };
  exercise();
  DOBA_EXPECT(completed.load());
  std::puts("probe suspended cleanup complete");
}

// +===========================================================================+
// | [>] probe row context                                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("probe row context") {
  martianlabs::doba::tests::integration::test_helper::set_context(
      "row 7: input=invalid");
  DOBA_EXPECT(false);
}

// +===========================================================================+
// | [>] failure unwinds active transport                        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("probe failure cleans up active transport") {
  using namespace martianlabs::doba;
  std::atomic<std::size_t> connected = 0;
  std::atomic<std::size_t> disconnected = 0;
  tests::integration::tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  const auto exercise = [&]() {
    protocol::http::router<protocol::http::v11::request,
                           protocol::http::v11::response>
        routes;
    auto factory = [&routes]() {
      return protocol::http::v11::engine<protocol::http::v11::request,
                                         protocol::http::v11::response>({},
                                                                        routes);
    };
    transport::server::tcp<decltype(factory()), decltype(factory)> server(
        {.ip = "127.0.0.1", .port = std::to_string(port)}, factory);
    server.set_on_connection([&]() { connected.fetch_add(1); });
    server.set_on_disconnection([&]() { disconnected.fetch_add(1); });
    server.start();
    DOBA_EXPECT(client.connect(port));
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (connected.load() == 0 &&
           std::chrono::steady_clock::now() < deadline) {
      std::this_thread::yield();
    }
    DOBA_EXPECT_EQUAL(connected.load(), 1);
    DOBA_EXPECT(false);
  };
  exercise();
  DOBA_EXPECT_EQUAL(connected.load(), 1);
  DOBA_EXPECT_EQUAL(disconnected.load(), 1);
  std::puts("probe cleanup complete");
}

// +===========================================================================+
// | [>] concurrent assertion diagnostics                        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("probe concurrent assertions") {
  std::atomic<bool> ready{false};
  std::array<std::jthread, 8> workers;
  for (auto& worker : workers) {
    worker = std::jthread([&] {
      while (!ready.load()) std::this_thread::yield();
      for (int i = 0; i < 16; ++i) {
        martianlabs::doba::tests::integration::test_helper::set_context("concurrent worker");
        (void)martianlabs::doba::tests::integration::test_helper::expect(
            false, "worker failure", __FILE__, __LINE__);
      }
    });
  }
  ready = true;
}
