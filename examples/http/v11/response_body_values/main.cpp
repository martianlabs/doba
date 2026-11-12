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

#include <string>
#include <string_view>

#include "common/signaler.h"
#include "http_server.h"

using namespace martianlabs::doba::common;
using martianlabs::doba::http_server;
using namespace martianlabs::doba::protocol::http::v11;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | Set text, binary and numeric response bodies.                             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  http_server<> http_server({.ip = "0.0.0.0", .port = "8080"});
  http_server.add_route("GET", "/text", [](const request&, response& res) {
    res.ok_200();
    const std::string text = "body stored in the response buffer";
    // set_body() copies the text; it may go out of scope after this call.
    res.add_header("Content-Type", "text/plain; charset=utf-8").set_body(text);
    return;
  });
  http_server.add_route("GET", "/binary", [](const request&, response& res) {
    res.ok_200();
    const char bytes[] = {'d', 'o', 'b', 'a', '\0'};
    // Pass the size to keep embedded zero bytes.
    res.add_header("Content-Type", "application/octet-stream")
        .set_body(std::string_view(bytes, sizeof(bytes)));
    return;
  });
  http_server.add_route("GET", "/integer", [](const request&, response& res) {
    res.ok_200();
    res.add_header("Content-Type", "text/plain; charset=utf-8").set_body(42);
    return;
  });
  http_server.add_route(
      "GET", "/floating-point", [](const request&, response& res) {
        res.ok_200();
        // Numbers use std::to_string formatting.
        res.add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body(3.14);
        return;
      });
  http_server.start();
  signaler::wait();
  return 0;
}
