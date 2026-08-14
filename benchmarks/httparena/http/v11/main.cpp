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
#include <future>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <system_error>

#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::protocol::http::v11;

namespace {
bool parse_integer(std::string_view source, std::int64_t& value) {
  if (source.empty()) return false;
  const char* first = source.data();
  const char* last = first + source.size();
  const auto [end, error] = std::from_chars(first, last, value);
  return error == std::errc() && end == last;
}

bool read_query_sum(const std::shared_ptr<const request>& req,
                    std::int64_t& value) {
  const auto a = req->get_query_parameter("a");
  const auto b = req->get_query_parameter("b");
  std::int64_t a_value = 0;
  std::int64_t b_value = 0;
  if (!a || !b || !parse_integer(a->second, a_value) ||
      !parse_integer(b->second, b_value)) {
    return false;
  }
  value = a_value + b_value;
  return true;
}

// The body reader exposes decoded bytes for Content-Length and chunked bodies.
bool read_body_integer(const std::shared_ptr<const request>& req,
                       std::int64_t& value) {
  if (!req->has_body_reader()) return false;
  std::array<std::byte, 64> buffer{};
  std::size_t used = 0;
  for (;;) {
    const auto state =
        req->get_body_reader()->read(std::span(buffer).subspan(used));
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

int main(int argc, char* argv[]) {
  server http_server;
  // Parse every baseline value; HttpArena randomizes them to detect shortcuts.
  http_server.add_route(
      "GET", "/baseline11",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res) {
        std::int64_t value = 0;
        if (!read_query_sum(req, value)) {
          res->bad_request_400().set_body("invalid request");
          return;
        }
        res->ok_200()
            .add_header("Content-Type", "text/plain")
            .set_body(std::to_string(value));
      });
  http_server.add_route(
      "POST", "/baseline11",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res) {
        std::int64_t value = 0;
        std::int64_t body_value = 0;
        if (!read_query_sum(req, value) ||
            !read_body_integer(req, body_value)) {
          res->bad_request_400().set_body("invalid request");
          return;
        }
        res->ok_200()
            .add_header("Content-Type", "text/plain")
            .set_body(std::to_string(value + body_value));
      });
  // Keep this handler minimal so the profile isolates pipelining overhead.
  http_server.add_route(
      "GET", "/pipeline",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res) {
        res->ok_200().add_header("Content-Type", "text/plain").set_body("ok");
      });
  http_server.add_route(
      "POST", "/upload",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res) {
        if (!req->has_body_reader()) {
          res->bad_request_400().set_body("request body required");
          return;
        }
        // HttpArena requires counting bytes read, not Content-Length.
        std::array<std::byte, 65536> buffer{};
        std::size_t bytes = 0;
        for (;;) {
          const auto state = req->get_body_reader()->read(buffer);
          if (state.has_error) {
            res->bad_request_400().set_body("unable to read request body");
            return;
          }
          bytes += state.produced;
          if (state.complete) break;
        }
        res->ok_200()
            .add_header("Content-Type", "text/plain")
            .set_body(std::to_string(bytes));
      });
  http_server.start("8080");
  // Docker owns process shutdown; wait after the server starts.
  std::promise<void> shutdown;
  shutdown.get_future().wait();
  return 0;
}
