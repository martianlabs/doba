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
#include <cstdint>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "common/task.h"
#include "http_server.h"
#include "http_test_helper.h"
#include "tcpip_client.h"
#include "test_helper.h"

namespace {
// +---------------------------------------------------------------------------+
// | [>] general                                                    ( usings ) |
// +---------------------------------------------------------------------------+
using martianlabs::doba::http_server;
using martianlabs::doba::common::task;
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::tests::integration::receive_http_response;
using martianlabs::doba::tests::integration::tcpip_client;

// +---------------------------------------------------------------------------+
// | [>] text_response                                            ( function ) |
// +---------------------------------------------------------------------------+
void text_response(response& res, std::string_view text) {
  res.ok_200().set_body(text);
}
}  // namespace

// +===========================================================================+
// | [>] HTTP/1.1 runs asynchronous routes with multiple IOCP    ( test-case ) |
// |     workers                                                               |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP/1.1 runs asynchronous routes with multiple IOCP workers") {
  tcpip_client probe;
  const uint16_t port = probe.find_available_port();
  DOBA_EXPECT(port != 0);
  http_server<> http_server(
      {.worker_count = 2, .ip = "127.0.0.1", .port = std::to_string(port)});
  http_server.add_route("GET", "/wait",
                        [](const request&, response& res) -> task<void> {
                          co_await martianlabs::doba::common::yield();
                          co_await martianlabs::doba::common::sleep_for(
                              std::chrono::milliseconds(1));
                          text_response(res, "ready");
                        });
  http_server.start();
  std::atomic<int> connected{0};
  std::atomic<int> sent{0};
  std::atomic<int> received{0};
  std::atomic<int> server_errors{0};
  std::atomic<int> passed{0};
  std::vector<std::jthread> clients;
  for (int i = 0; i < 32; i++) {
    clients.emplace_back([&]() {
      tcpip_client client;
      if (!client.connect(port)) return;
      connected++;
      if (!client.send_all("GET /wait HTTP/1.1\r\nHost: localhost\r\n\r\n")) {
        return;
      }
      sent++;
      const auto result = receive_http_response(client);
      if (!result) return;
      received++;
      if (result->status.starts_with("HTTP/1.1 500 ")) server_errors++;
      if (result->status.starts_with("HTTP/1.1 200 ") &&
          result->body == "ready") {
        passed++;
      }
    });
  }
  for (auto& client : clients) client.join();
  DOBA_EXPECT_EQUAL(connected.load(), 32);
  DOBA_EXPECT_EQUAL(sent.load(), 32);
  DOBA_EXPECT_EQUAL(received.load(), 32);
  DOBA_EXPECT_EQUAL(server_errors.load(), 0);
  DOBA_EXPECT_EQUAL(passed.load(), 32);
  http_server.stop();
}
