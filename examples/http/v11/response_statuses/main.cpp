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
#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http::v11;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | This is the entry point of the application. It creates an HTTP server     |
// | that listens on all interfaces (0.0.0.0) and port 8080.                   |
// | The server registers three routes:                                        |
// |   * "/resources" for POST requests,                                       |
// |   * "/redirect" for GET requests,                                         |
// |   * "/resources/1" for DELETE requests                                    |
// | Each route has a corresponding lambda function that handles the request   |
// | and constructs an appropriate response with different HTTP status codes.  |
// | The server runs until a termination signal is received.                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  server<> http_server({.ip = "0.0.0.0", .port = "8080"});
  http_server.add_route("POST", "/resources",
                        [](const request&, response& res) {
                          res.created_201();
                          res.add_header("Location", "/resources/1");
                          return;
                        });
  http_server.add_route("GET", "/redirect", [](const request&, response& res) {
    res.temporary_redirect_307();
    res.add_header("Location", "/resources/1");
    return;
  });
  http_server.add_route("DELETE", "/resources/1",
                        [](const request&, response& res) {
                          res.no_content_204();
                          return;
                        });
  http_server.start();
  signaler::wait();
  return 0;
}
