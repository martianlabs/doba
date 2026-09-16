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
#include <memory>
#include <stop_token>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "protocol/http/v11/server.h"
#include "http_test_helper.h"
#include "tcpip_client.h"
#include "test_helper.h"

namespace {
using namespace martianlabs::doba::protocol::http::v11;
using martianlabs::doba::common::task;
using martianlabs::doba::tests::integration::tcpip_client;
using martianlabs::doba::tests::integration::receive_http_response;
using martianlabs::doba::tests::integration::http_test_signal;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] socket_controller                                           ( class ) |
// +---------------------------------------------------------------------------+
// | Controller implementation.                                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class socket_controller {
 public:
  // +=========================================================================+
  // | [>] METHODs                                                  ( public ) |
  // +=========================================================================+
  socket_controller(std::string prefix, http_test_signal& signal,
                    std::atomic<bool>& cancelled)
      : prefix_(std::move(prefix)), signal_(signal), cancelled_(cancelled) {}
  template <typename Rty>
  void register_routes(Rty& routes) {
    routes.add("GET", prefix_ + "/count", &socket_controller::count);
    routes.add("GET", prefix_ + "/async/:id", &socket_controller::delayed);
    routes.add("GET", prefix_ + "/fail", &socket_controller::fail);
  }
  response count(const request&) {
    auto result = response::ok_200();
    result.set_body(++count_);
    return result;
  }
  task<response> delayed(std::shared_ptr<const request>,
                         std::stop_token token, int id) {
    co_await signal_;
    cancelled_ = token.stop_requested();
    auto result = response::ok_200();
    result.set_body(id);
    co_return result;
  }
  response fail(const request&) { throw std::runtime_error("controller"); }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::string prefix_;
  http_test_signal& signal_;
  std::atomic<bool>& cancelled_;
  std::atomic<unsigned int> count_{0};
};
}  // namespace

// +===========================================================================+
// | [>] HTTP controller async ordering                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP controllers preserve asynchronous response ordering") {
  http_test_signal signal;
  std::atomic<bool> cancelled{false};
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> value;
  value.add_controller<socket_controller>("/a", signal, cancelled);
  value.add_controller<socket_controller>("/b", signal, cancelled);
  value.start(std::to_string(port).c_str());
  DOBA_EXPECT(client.connect(port));
  const bool sent = client.send_all(
      "GET /a/async/42 HTTP/1.1\r\nHost: a\r\n\r\n"
      "GET /a/count HTTP/1.1\r\nHost: a\r\n\r\n"
      "GET /b/count HTTP/1.1\r\nHost: a\r\n\r\n");
  const bool waiting = signal.wait();
  const bool premature = client.has_data(std::chrono::milliseconds(50));
  signal.resume();
  DOBA_EXPECT(sent);
  DOBA_EXPECT(waiting);
  DOBA_EXPECT(!premature);
  for (std::string_view body : {"42", "1", "1"}) {
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->body, body);
  }
  DOBA_EXPECT(!cancelled);
  value.stop();
}

// +===========================================================================+
// | [>] HTTP controller cancellation                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP suspended controllers receive shutdown cancellation") {
  http_test_signal signal;
  std::atomic<bool> cancelled{false};
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> value;
  value.add_controller<socket_controller>("/a", signal, cancelled);
  value.start(std::to_string(port).c_str());
  DOBA_EXPECT(client.connect(port));
  const bool sent = client.send_all(
      "GET /a/async/1 HTTP/1.1\r\nHost: a\r\n\r\n");
  const bool waiting = signal.wait();
  client.close();
  value.stop();
  signal.resume();
  DOBA_EXPECT(sent);
  DOBA_EXPECT(waiting);
  DOBA_EXPECT(cancelled.load());
}

// +===========================================================================+
// | [>] HTTP controller concurrency and errors                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP controller state supports concurrent requests and failures") {
  http_test_signal signal;
  std::atomic<bool> cancelled{false};
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> value;
  value.add_controller<socket_controller>("/a", signal, cancelled);
  value.start(std::to_string(port).c_str());
  std::atomic<unsigned int> total{0};
  {
    std::vector<std::jthread> clients;
    for (unsigned int i = 0; i < 4; i++) {
      clients.emplace_back([&]() {
        tcpip_client connection;
        if (!connection.connect(port) || !connection.send_all(
                "GET /a/count HTTP/1.1\r\nHost: a\r\n\r\n")) return;
        auto result = receive_http_response(connection);
        if (result) total += static_cast<unsigned int>(std::stoi(result->body));
      });
    }
  }
  DOBA_EXPECT_EQUAL(total.load(), 10);
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /a/fail HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto error = receive_http_response(client);
  DOBA_EXPECT(error.has_value());
  DOBA_EXPECT_EQUAL(error->status, "HTTP/1.1 500 Internal Server Error");
  client.close();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /a/count HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto next = receive_http_response(client);
  DOBA_EXPECT(next.has_value());
  DOBA_EXPECT_EQUAL(next->body, "5");
  value.stop();
}
