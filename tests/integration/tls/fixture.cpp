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
#include <functional>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <utility>

#include "protocol/processing_result.h"
#include "https_server.h"
#include "transport/server/tls.h"
#include "protocol/http/common/auth/basic.h"

namespace {
using martianlabs::doba::protocol::processing_result;
namespace tr = martianlabs::doba::transport::server;
namespace common = martianlabs::doba::common;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] echo_engine                                               ( struct ) |
// +---------------------------------------------------------------------------+
// | A deliberately non-HTTP protocol exercises the unchanged TCP contract.    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct echo_engine {
  using policies_type = int;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(martianlabs::doba::protocol::send_delegate value) {
    send = std::move(value);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> value) { close = std::move(value); }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view input) {
    if (input.size() > 4096) throw std::runtime_error("RX bound exceeded");
    pending.append(input);
    for (;;) {
      const auto end = pending.find('\n');
      if (end == std::string::npos) return {};
      const auto line = pending.substr(0, end);
      pending.erase(0, end + 1);
      if (line == "close") { close(); return {}; }
      if (line == "bad") { send({}, {}, source("x")); send("!", {}, {}); }
      else if (line == "source") {
        send("pre:", "body:", source("tail"));
        send({}, {}, {});
      } else if (line == "second-source") {
        send("discard:", {}, source(""));
        send({}, {}, source(""));
      } else if (line == "empty-close") {
        send("pre:", "body:", source("")); close(); return {};
      } else if (line == "inline-exact") {
        send("pre:", std::string(4092, 'e'), {});
      } else if (line == "inline-over") {
        send("pre:", std::string(4093, 'e'), {});
      } else if (line == "sum-over") {
        send({}, std::string(2048, 'e'), {});
        send({}, std::string(2049, 'e'), {});
      } else if (line == "released") {
        auto owner = std::make_shared<const std::string>("tail");
        send({}, {}, std::make_unique<common::reader>(common::reader::shared(
            owner, std::as_bytes(std::span(*owner)))));
        if (owner.use_count() != 1) throw std::runtime_error("Reader retained");
      } else if (line == "failed") {
        common::filesystem_file file;
        std::byte byte{};
        file.read(std::span<std::byte>(&byte, 1));
        send("discard:", {}, std::make_unique<common::reader>(std::move(file)));
      } else if (line == "empty") {
        send("pre:", "body:", source(""));
      } else if (line == "slow") {
        send("pre:", {}, source(std::string(4 * 1024 * 1024, 's')));
      } else if (line == "large") {
        send("pre:", {}, source(std::string(128 * 1024, 's')));
      } else if (line == "exact") send({}, {}, source(std::string(4096, 'e')));
      else if (line == "over") send({}, {}, source(std::string(4097, 'e')));
      else if (line == "source-close") {
        send("pre:", "body:", source("tail")); close(); return {};
      } else send("echo:", line + "\n", {});
    }
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool) { return {}; }

  // +=========================================================================+
  // | [>] source                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  static std::unique_ptr<common::reader> source(std::string text) {
    auto owner = std::make_shared<const std::string>(std::move(text));
    return std::make_unique<common::reader>(common::reader::shared(
        owner, std::as_bytes(std::span(*owner))));
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Incomplete input retained by the protocol between receive blocks.
  std::string pending;
  // Callbacks are used only during the synchronous input call.
  martianlabs::doba::protocol::send_delegate send;
  std::function<void()> close;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] serve                                                    ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename SRty>
void serve(SRty& server) {
  server.start();
  std::cout << "READY\n" << std::flush;
  std::string command;
  while (std::getline(std::cin, command) && command == "restart") {
    server.stop();
    server.start();
    std::cout << "READY\n" << std::flush;
  }
  server.stop();
}
}  // namespace

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int count, char** args) {
  if (count < 5) return 1;
  try {
    tr::tls_policies policies;
    policies.ip = "127.0.0.1";
    policies.port = args[3];
    policies.worker_count = 2;
    policies.certificate_file = args[1];
    policies.private_key_file = args[2];
    if (count > 5) policies.max_buffered_send_bytes = std::stoull(args[5]);
    if (count > 6) policies.network_bio_buffer_capacity = std::stoull(args[6]);
    if (count > 7)
      policies.max_buffered_ciphertext_bytes = std::stoull(args[7]);
    const std::string_view mode(args[4]);
    if (mode == "admission") {
      policies.worker_count = 1;
      policies.max_connections = 1;
    }
    if (mode == "http-auth") {
      namespace http = martianlabs::doba::protocol::http;
      namespace auth = http::auth;
      auth::basic provider("test", [](std::string_view user,
                                      std::string_view password) {
        return user == "alice" && password == "secret";
      });
      martianlabs::doba::https_server<http::v11::request, http::v11::response,
          http::router<http::v11::request, http::v11::response>,
          http::v11::engine<http::v11::request, http::v11::response>,
          auth::basic> server(
              policies, {}, std::tuple{std::move(provider)});
      server.add_route<auth::basic>("GET", "/private",
          [](const http::v11::request&, http::v11::response& response) {
            response.ok_200().set_body("private");
          });
      serve(server);
    } else if (mode.starts_with("http")) {
      if (mode == "http-async") policies.worker_count = 1;
      std::atomic<unsigned> started{0}, finished{0}, live{0};
      std::atomic<bool> released{false};
      policies.alpn_protocol =
          std::string_view(args[4]) == "http-invalid" ? "h2" : "http/1.1";
      martianlabs::doba::https_server<> server(std::move(policies));
      server.add_route("GET", "/hello",
      [](const martianlabs::doba::protocol::http::v11::request&,
             martianlabs::doba::protocol::http::v11::response& response) {
        response.ok_200().set_body("hello");
      });
      if (mode == "http-async") {
        namespace http = martianlabs::doba::protocol::http::v11;
        using namespace std::chrono_literals;
        server.add_route("GET", "/wait/:id",
            [&](const http::request&, http::response& response,
                int id) -> common::task<> {
          ++started;
          co_await common::sleep_for(1ms);
          ++finished;
          response.ok_200().set_body(std::to_string(id));
        });
        server.add_route("GET", "/gated",
            [&](const http::request&, http::response& response)
                -> common::task<> {
          ++started;
          while (!released.load()) co_await common::yield();
          ++finished;
          response.ok_200().set_body("gated");
        });
        server.add_route("GET", "/release",
            [&](const http::request&, http::response& response) {
          released = true;
          response.ok_200().set_body("release");
        });
        server.add_route("GET", "/source",
            [](const http::request&, http::response& response)
                -> common::task<> {
          co_await common::sleep_for(1ms);
          response.ok_200().set_body(std::move(*echo_engine::source(
              std::string(32768, 's'))), 32768);
        });
        server.add_route("GET", "/fail",
            [](const http::request&, http::response&) -> common::task<> {
          co_await common::yield();
          throw std::runtime_error("async handler failed");
        });
        server.add_route("GET", "/sleep",
            [&](const http::request&, http::response&) -> common::task<> {
          ++live;
          auto guard = std::shared_ptr<int>(new int, [&](int* value) {
            delete value;
            --live;
          });
          ++started;
          co_await common::sleep_for(1h);
        });
        server.add_route("GET", "/state",
            [&](const http::request&, http::response& response) {
          response.ok_200().set_body(std::to_string(started.load()) + ":" +
              std::to_string(finished.load()) + ":" +
              std::to_string(live.load()));
        });
      }
      serve(server);
      if (mode == "http-async") {
        std::cout << "LIVE " << live.load() << '\n' << std::flush;
      }
    } else {
      policies.alpn_protocol = "doba-echo";
      auto factory = [] { return echo_engine{}; };
      tr::tls<echo_engine, decltype(factory)> server(policies, factory);
      if (mode == "admission") {
        server.set_on_connection([] {
          std::cout << "ADMITTED\n" << std::flush;
        });
      }
      serve(server);
    }
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
