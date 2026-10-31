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

#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <future>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <system_error>

#include "http_server.h"

using martianlabs::doba::http_server;
using namespace martianlabs::doba::protocol::http::v11;

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] parse_integer                                            ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
bool parse_integer(std::string_view source, std::int64_t& value) {
  if (source.empty()) return false;
  const char* first = source.data();
  const char* last = first + source.size();
  const auto [end, error] = std::from_chars(first, last, value);
  return error == std::errc() && end == last;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] read_query_sum                                           ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
bool read_query_sum(const request& req, std::int64_t& value) {
  const auto a = req.get_query_parameter("a");
  const auto b = req.get_query_parameter("b");
  std::int64_t a_value = 0;
  std::int64_t b_value = 0;
  if (!a || !b || !parse_integer(a->second, a_value) ||
      !parse_integer(b->second, b_value)) {
    return false;
  }
  value = a_value + b_value;
  return true;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] read_body_integer                                        ( function ) |
// +---------------------------------------------------------------------------+
// | Reads decoded bytes for Content-Length and chunked bodies.                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
bool read_body_integer(const request& req, std::int64_t& value) {
  if (!req.has_body_reader()) return false;
  std::array<std::byte, 64> buffer{};
  std::size_t used = 0;
  for (;;) {
    const auto state =
        req.get_body_reader()->read(std::span(buffer).subspan(used));
    if (state.has_error) return false;
    used += state.produced;
    if (state.complete) break;
    if (used == buffer.size()) return false;
  }
  return parse_integer(
      std::string_view(reinterpret_cast<const char*>(buffer.data()), used),
      value);
}
}  // namespace

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | Runs the baseline and pipelined routes on TCP.                            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  std::size_t workers = 1;
  if (const char* configured = std::getenv("WORKERS")) {
    const std::string_view text(configured);
    std::size_t requested = 0;
    const auto [end, error] = std::from_chars(
        text.data(), text.data() + text.size(), requested);
    if (error != std::errc() || end != text.data() + text.size() ||
        requested == 0) {
      std::cerr << "WORKERS must be a positive integer\n";
      return 1;
    }
    workers = requested;
  }
  policies http_configuration;
  http_server<> http_server(
      {.worker_count = workers, .ip = "0.0.0.0", .port = "8080"},
      http_configuration);
  http_server.add_route(
      "GET", "/pipeline",
      [](const request&, response& res) {
        res.ok_200();
        res.add_header("Content-Type", "text/plain").set_body("ok");
      });
  // Parse every baseline value; HttpArena randomizes them to detect shortcuts.
  http_server.add_route(
      "GET", "/baseline11",
      [](const request& req, response& res) {
        res.ok_200();
        std::int64_t value = 0;
        if (!read_query_sum(req, value)) {
          res.bad_request_400();
          res.set_body("invalid request");
          return;
        }
        res.add_header("Content-Type", "text/plain")
            .set_body(std::to_string(value));
      });
  http_server.add_route(
      "POST", "/baseline11",
      [](const request& req, response& res) {
        res.ok_200();
        std::int64_t value = 0;
        std::int64_t body_value = 0;
        if (!read_query_sum(req, value) ||
            !read_body_integer(req, body_value)) {
          res.bad_request_400();
          res.set_body("invalid request");
          return;
        }
        res.add_header("Content-Type", "text/plain")
            .set_body(std::to_string(value + body_value));
      });
  http_server.start();
  std::cerr << "workers: " << workers << "; HTTP port: 8080\n";
  // The benchmark harness owns process shutdown.
  std::promise<void> shutdown;
  shutdown.get_future().wait();
  return 0;
}
