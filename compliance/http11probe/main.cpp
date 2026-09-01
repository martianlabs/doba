//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _`
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

#include <array>
#include <cstddef>
#include <future>
#include <string>
#include <string_view>

#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::protocol::http::v11;

int main() {
  server http_server;
  // Root routes cover baseline, response metadata, and method handling probes.
  http_server.add_route(
      "GET", "/",
      [](const request&, response& res) {
        res.ok_200().add_header("Content-Type", "text/plain").set_body("OK");
      });
  http_server.add_route(
      "HEAD", "/",
      [](const request&, response& res) {
        res.ok_200().add_header("Content-Type", "text/plain");
      });
  http_server.add_route(
      "OPTIONS", "/",
      [](const request&, response& res) {
        res.ok_200().add_header("Allow", "GET, HEAD, POST, OPTIONS");
      });
  // Echo decoded bytes so both request body framing modes are exercised.
  http_server.add_route(
      "POST", "/",
      [](const request& req, response& res) {
        std::string body;
        if (req.has_body_reader()) {
          std::array<std::byte, 4096> buffer{};
          for (;;) {
            const auto state = req.get_body_reader()->read(buffer);
            if (state.has_error) {
              res.bad_request_400();
              return;
            }
            body.append(reinterpret_cast<const char*>(buffer.data()),
                        state.produced);
            if (state.complete) break;
          }
        }
        res.ok_200().add_header("Content-Type", "text/plain").set_body(body);
      });
  // Http11Probe uses /echo to inspect received header fields verbatim.
  const auto echo_headers = [](const request& req, response& res) {
    std::string body;
    for (std::size_t i = 0; i < req.get_headers_length(); i++) {
      const auto header = req.get_header(i);
      body.append(header.first);
      body.append(": ");
      body.append(header.second);
      body.push_back('\n');
    }
    res.ok_200().add_header("Content-Type", "text/plain").set_body(body);
  };
  http_server.add_route("GET", "/echo", echo_headers);
  http_server.add_route("POST", "/echo", echo_headers);
  // The parsed-cookie probes query only these four fixed names.
  http_server.add_route(
      "GET", "/cookie",
      [](const request& req, response& res) {
        constexpr std::string_view cookie_names[] = {"foo", "a", "b", "c"};
        std::string body;
        for (const auto name : cookie_names) {
          const auto value = req.get_cookie(name);
          if (!value) continue;
          body.append(name);
          body.push_back('=');
          body.append(*value);
          body.push_back('\n');
        }
        res.ok_200().add_header("Content-Type", "text/plain").set_body(body);
      });
  http_server.start("8080");
  // The test harness owns process shutdown; wait after the server starts.
  std::promise<void> shutdown;
  shutdown.get_future().wait();
  return 0;
}
