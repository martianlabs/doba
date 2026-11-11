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

#include <charconv>
#include <chrono>
#include <string_view>
#include <system_error>
#include <iostream>
#include <string>

#include "http_server.h"

using namespace martianlabs::doba;
using protocol::http::v11::request;
using protocol::http::v11::response;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | TCP workers resume handlers one turn at a time.                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int count, char** args) {
  if (count > 2) return 1;
  unsigned port = 8080;
  if (count == 2) {
    const std::string_view value(args[1]);
    const auto [end, error] = std::from_chars(
        value.data(), value.data() + value.size(), port);
    if (error != std::errc{} || end != value.data() + value.size() ||
        !port || port > 65535) return 1;
  }
  http_server<> server({.worker_count = 1, .ip = "127.0.0.1",
                        .port = std::to_string(port)});
  server.add_route("GET", "/wait/:id",
                   [](const request&, response& res, int id) -> common::task<> {
    co_await common::sleep_for(std::chrono::milliseconds(10));
    res.ok_200().set_body(std::to_string(id));
  });
  server.add_route("GET", "/sync", [](const request&, response& res) {
    res.ok_200().set_body("sync");
  });
  server.start();
  std::cout << "TCP: /wait/:id and /sync; press Enter to stop.\n";
  std::cin.get();
  server.stop();
}
