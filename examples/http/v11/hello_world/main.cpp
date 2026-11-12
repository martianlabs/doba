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

#include "common/signaler.h"
#include "http_server.h"

using namespace martianlabs::doba::common;
using martianlabs::doba::http_server;
using namespace martianlabs::doba::protocol::http::v11;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | Reply to a GET request.                                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  http_server<> http_server({.ip = "0.0.0.0", .port = "8080"});
  // Match both the HTTP method and the path.
  http_server.add_route("GET", "/hello", [](const request&, response& res) {
    res.ok_200();
    // Response setters can be chained.
    res.add_header("Server", "doba.")
        .add_header("Content-Type", "text/plain; charset=utf-8")
        .set_body("ok");
  });
  http_server.start();
  // start() opens the listener and returns. Wait for a shutdown signal.
  signaler::wait();
  return 0;
}
