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

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "protocol/http/v11/engine.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"

using namespace martianlabs::doba;
using namespace protocol::http::v11;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | A serialized in-memory adapter supplies turns for concurrent handlers.   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  protocol::http::router<request, response> routes;
  int started = 0;
  routes.add("GET", "/wait/:id",
             [&](const request&, response& res, int id) -> common::task<> {
    ++started;
    co_await common::sleep_for(std::chrono::milliseconds(1));
    res.ok_200().set_body(std::to_string(id));
  });
  engine<request, response> protocol({}, routes);
  std::string wire;
  bool closed = false;
  protocol.set_on_close([&] { closed = true; });
  protocol.set_on_send([&](auto head, auto body,
                           std::unique_ptr<common::reader> source) {
    wire.append(head).append(body);
    if (source) source->read_all(wire);
  });
  std::string input;
  for (int i = 1; i <= 3; ++i) {
    input += "GET /wait/" + std::to_string(i) +
             " HTTP/1.1\r\nHost: local\r\n\r\n";
  }
  auto needs = protocol.on_bytes_received(input);
  if (started != 3 || closed) return 1;
  input.assign(input.size(), 'x');  // RX storage can be reused after delivery.
  while (needs.turn_requested || needs.turn_at ||
         needs.turn_after_output_drained) {
    if (!needs.turn_requested && needs.turn_at) {
      std::this_thread::sleep_until(*needs.turn_at);
    }
    needs = protocol.on_processing_turn(true);
  }
  const auto one = wire.find("\r\n\r\n1");
  const auto two = wire.find("\r\n\r\n2");
  const auto three = wire.find("\r\n\r\n3");
  if (closed || one == std::string::npos || two == std::string::npos ||
      three == std::string::npos || !(one < two && two < three)) return 1;
  std::cout << "Concurrent handlers: " << started
            << "; ordered output: 1 2 3\n";
}
