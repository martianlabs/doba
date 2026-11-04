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
// Copyright 2026 martianLabs
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
#include <cstdint>
#include <future>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "common/reader.h"
#include "common/task.h"
#include "http_server.h"
#include "http_test_helper.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "../../../../helpers/concurrency.h"

namespace {
using namespace martianlabs::doba;
using namespace martianlabs::doba::tests::integration;
using protocol::http::v11::request;
using protocol::http::v11::response;

// +---------------------------------------------------------------------------+
// | [>] payload                                                  ( function ) |
// +---------------------------------------------------------------------------+
std::string payload(std::size_t id) {
  std::string value(id % 10 == 0 ? 512 * 1024 : 65536 + id * 17, '\0');
  std::uint64_t state = 0xd1b54a32d192ed03ULL ^ (id + 1);
  for (auto& byte : value) {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    byte = char(state >> 56);
  }
  return value;
}
}  // namespace

// +===========================================================================+
// | [>] binary readers preserve every response and owner        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP pipelines retain binary readers under default limits") {
  constexpr std::size_t clients = 3, requests = 10;
  std::vector<std::shared_ptr<const std::string>> bodies;
  for (std::size_t id = 0; id < clients * requests; ++id)
    bodies.push_back(std::make_shared<const std::string>(payload(id)));
  for (const std::size_t workers : {1U, 4U}) {
    tcpip_client probe;
    const auto port = probe.find_available_port();
    DOBA_EXPECT(port != 0);
    std::atomic<unsigned> sentinels{0};
    http_server<> server({.worker_count = workers,
        .max_buffered_send_bytes = 8192, .ip = "127.0.0.1",
        .port = std::to_string(port)});
    auto fill = [&](const request& req, response& res, int id) {
      const auto bytes = bodies.at(static_cast<std::size_t>(id));
      res.ok_200().set_body(common::reader::shared(
          bytes, std::as_bytes(std::span(*bytes))), bytes->size());
      res.add_header("X-Probe", req.get_header("X-Probe").second);
    };
    auto asynchronous = [&](const request& req, response& res,
                             int id) -> common::task<> {
      co_await common::yield();
      fill(req, res, id);
    };
    for (const auto method : {"GET", "HEAD"}) {
      server.add_route(method, "/sync/:id", fill);
      server.add_route(method, "/async/:id", asynchronous);
    }
    server.add_route("GET", "/sentinel", [&](const request&, response& res) {
      ++sentinels;
      res.ok_200().set_body("forbidden");
    });
    tests::scope_exit stop([&] { server.stop(); });
    server.start();
    std::vector<std::future<bool>> checks;
    for (std::size_t client = 0; client < clients; ++client) {
      checks.push_back(std::async(std::launch::async, [&, client] {
        tcpip_client peer;
        if (!peer.set_receive_buffer_size(32768) || !peer.connect(port))
          return false;
        std::string batch;
        for (std::size_t n = 0; n < requests; ++n) {
          const auto id = std::to_string(client * requests + n);
          const bool head = n % 5 == 3;
          batch += std::string(head ? "HEAD" : "GET") +
              (n % 2 ? " /async/" : " /sync/") + id +
              " HTTP/1.1\r\nHost: localhost\r\nX-Probe: " + id + "\r\n";
          if (n + 1 == requests) batch += "Connection: close\r\n";
          batch += "\r\n";
        }
        batch += "GET /sentinel HTTP/1.1\r\nHost: localhost\r\n\r\n";
        if (!peer.send_all(batch)) return false;
        for (std::size_t n = 0; n < requests; ++n) {
          const auto id = client * requests + n;
          const bool head = n % 5 == 3;
          const auto reply = receive_http_response(peer, head);
          if (!reply || reply->status != "HTTP/1.1 200 OK" ||
              reply->header("X-Probe") != std::to_string(id) ||
              reply->header("Content-Length") !=
                  std::to_string(bodies[id]->size()) ||
              reply->body != (head ? std::string{} : *bodies[id])) return false;
        }
        // EOF must be clean and immediate after the final exact body.
        const auto extra = peer.receive_until_close(1);
        return extra && extra->empty() && peer.error() != "timeout";
      }));
    }
    for (std::size_t client = 0; client < checks.size(); ++client) {
      test_helper::set_context("workers=" + std::to_string(workers) +
                               "; client=" + std::to_string(client));
      DOBA_EXPECT(checks[client].get());
    }
    server.stop();
    DOBA_EXPECT_EQUAL(sentinels.load(), 0U);
    for (const auto& bytes : bodies) DOBA_EXPECT_EQUAL(bytes.use_count(), 1);
  }
}
