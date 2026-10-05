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
#include "protocol/http/common/method_names.h"
#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http;
using namespace martianlabs::doba::protocol::http::v11;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | This is the entry point of the application. It creates an HTTP server     |
// | that listens on all interfaces (0.0.0.0) and port 8080.                   |
// | It defines a resource handler for the "/resource" endpoint that responds  |
// | to both GET and HEAD requests. The server is started, and the application |
// | waits for termination signals.                                            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  server<> http_server({.ip = "0.0.0.0", .port = "8080"});
  auto resource = [](const request&, response& res) {
    res.ok_200();
    res.set_body("resource representation");
    return;
  };
  http_server.add_route(method_names::kGet, "/resource", resource);
  http_server.add_route(method_names::kHead, "/resource", resource);
  http_server.start();
  signaler::wait();
  return 0;
}
