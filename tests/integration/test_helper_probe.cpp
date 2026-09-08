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
#include <cstdio>
#include <stdexcept>
#include <thread>

#include "protocol/http/v11/server.h"
#include "tcpip_client.h"
#include "test_helper.h"

// +===========================================================================+
// | [>] probe passes                                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe passes") {
  std::puts("probe body");
  DOBA_EXPECT(true);
}

// +===========================================================================+
// | [>] probe assertion fails                                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe assertion fails") {
  DOBA_EXPECT(2 + 2 == 5);
}

// +===========================================================================+
// | [>] probe standard exception                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe standard exception") {
  throw std::runtime_error("probe standard error");
}

// +===========================================================================+
// | [>] probe unknown exception                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe unknown exception") {
  throw 7;
}

// +===========================================================================+
// | [>] probe continues after failures                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe continues after failures") {
  std::puts("probe continued");
  DOBA_EXPECT(true);
}

// +===========================================================================+
// | [>] probe blocks                                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe blocks") {
  std::this_thread::sleep_for(std::chrono::seconds(30));
}

// +===========================================================================+
// | [>] probe row context                                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe row context") {
  martianlabs::doba::tests::integration::test_helper::set_context(
      "row 7: input=invalid");
  DOBA_EXPECT(false);
}

// +===========================================================================+
// | [>] failure unwinds active transport                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe failure cleans up active transport") {
  using namespace martianlabs::doba;
  std::atomic<std::size_t> connected = 0;
  std::atomic<std::size_t> disconnected = 0;
  tests::integration::tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  const auto exercise = [&]() {
    transport::server::tcpip<
        protocol::http::v11::request, protocol::http::v11::response,
        protocol::http::v11::decoder> server;
    server.set_on_request(
        [](const std::shared_ptr<protocol::http::v11::request>&,
           protocol::http::v11::response&, const std::stop_token&)
            -> std::optional<common::task<protocol::http::v11::response>> {
          return std::nullopt;
        });
    server.set_on_bad_request(
        [](int, std::string_view, protocol::http::v11::response&) {});
    server.set_on_connection([&]() { connected.fetch_add(1); });
    server.set_on_disconnection([&]() { disconnected.fetch_add(1); });
    const std::string port_text = std::to_string(port);
    server.start(port_text.c_str());
    DOBA_EXPECT(client.connect(port));
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::seconds(2);
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
