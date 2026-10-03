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
#include <string_view>
#include <thread>

#include "test_helper.h"
#include "native_peer.h"
#include "tcpip_client.h"

// +===========================================================================+
// | [>] refused connection diagnostic                           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("client reports refused connections") {
  native_peer peer(false);
  martianlabs::doba::tests::integration::tcpip_client client;
  DOBA_EXPECT(peer.port() != 0);
  const auto started = std::chrono::steady_clock::now();
  DOBA_EXPECT(!client.connect(peer.port()));
  DOBA_EXPECT(std::chrono::steady_clock::now() - started <
              std::chrono::seconds(4));
  DOBA_EXPECT_EQUAL(client.operation(), "connect");
  DOBA_EXPECT_EQUAL(client.error(), "socket");
  DOBA_EXPECT(client.native_error() != 0);
}

// +===========================================================================+
// | [>] silent receive deadline                                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("client bounds silent receive operations") {
  native_peer peer;
  martianlabs::doba::tests::integration::tcpip_client client;
  DOBA_EXPECT(peer.port() != 0);
  DOBA_EXPECT(client.connect(peer.port()));
  DOBA_EXPECT(peer.accept());
  const auto started = std::chrono::steady_clock::now();
  auto result = client.receive(2, std::chrono::milliseconds(200));
  const auto elapsed = std::chrono::steady_clock::now() - started;
  DOBA_EXPECT(!result.has_value());
  DOBA_EXPECT_EQUAL(client.operation(), "receive");
  DOBA_EXPECT_EQUAL(client.error(), "timeout");
  DOBA_EXPECT(elapsed >= std::chrono::milliseconds(150));
  DOBA_EXPECT(elapsed < std::chrono::seconds(2));
}

// +===========================================================================+
// | [>] partial receive deadline                                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("client preserves the deadline after a partial receive") {
  native_peer peer;
  martianlabs::doba::tests::integration::tcpip_client client;
  DOBA_EXPECT(peer.port() != 0);
  DOBA_EXPECT(client.connect(peer.port()));
  DOBA_EXPECT(peer.accept());
  std::atomic<bool> sent = false;
  const auto started = std::chrono::steady_clock::now();
  std::jthread producer([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    sent = peer.send("a");
  });
  auto result = client.receive(2, std::chrono::milliseconds(600));
  const auto elapsed = std::chrono::steady_clock::now() - started;
  producer.join();
  DOBA_EXPECT(sent.load());
  DOBA_EXPECT(!result.has_value());
  DOBA_EXPECT_EQUAL(client.error(), "timeout");
  DOBA_EXPECT(elapsed >= std::chrono::milliseconds(550));
  DOBA_EXPECT(elapsed < std::chrono::milliseconds(850));
}

// +===========================================================================+
// | [>] progress does not extend deadline                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("client bounds receive until close despite progress") {
  native_peer peer;
  martianlabs::doba::tests::integration::tcpip_client client;
  DOBA_EXPECT(peer.port() != 0);
  DOBA_EXPECT(client.connect(peer.port()));
  DOBA_EXPECT(peer.accept());
  std::atomic<std::size_t> sent = 0;
  std::jthread producer([&](std::stop_token stop) {
    for (std::size_t i = 0; i < 20 && !stop.stop_requested(); i++) {
      if (peer.send("a")) sent.fetch_add(1);
      std::this_thread::sleep_for(std::chrono::milliseconds(60));
    }
  });
  const auto started = std::chrono::steady_clock::now();
  auto result = client.receive_until_close(100, std::chrono::milliseconds(250));
  const auto elapsed = std::chrono::steady_clock::now() - started;
  producer.request_stop();
  producer.join();
  DOBA_EXPECT(sent.load() >= 2);
  DOBA_EXPECT(!result.has_value());
  DOBA_EXPECT_EQUAL(client.error(), "timeout");
  DOBA_EXPECT(elapsed >= std::chrono::milliseconds(200));
  DOBA_EXPECT(elapsed < std::chrono::milliseconds(600));
}

// +===========================================================================+
// | [>] eof and reset diagnostics                               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("client distinguishes eof from reset") {
  for (bool reset : {false, true}) {
    native_peer peer;
    martianlabs::doba::tests::integration::tcpip_client client;
    DOBA_EXPECT(peer.port() != 0);
    DOBA_EXPECT(client.connect(peer.port()));
    DOBA_EXPECT(peer.accept());
    DOBA_EXPECT(peer.send("a"));
    peer.close(reset);
    auto result = client.receive(2);
    DOBA_EXPECT(!result.has_value());
    DOBA_EXPECT_EQUAL(client.operation(), "receive");
    DOBA_EXPECT_EQUAL(client.error(), reset ? "socket" : "eof");
    DOBA_EXPECT_EQUAL(client.native_error() != 0, reset);
  }
}
