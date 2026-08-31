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

#include "common/console_logger.h"
#include "common/logo.h"
#include "common/signaler.h"
#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http::v11;

int main(int argc, char* argv[]) {
  server http_server;
  // Routes are selected by both the HTTP method and the absolute path.
  http_server.add_route(
      "GET", "/pipeline",
      [](const request& req, response& res) {
        // Response mutators return response&, so they can be chained.
        res.ok_200()
            .add_header("Server", "doba.")
            .add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body("ok");
      });
  http_server.start("8080");
  // start() returns after opening the listener; wait for a shutdown signal.
  signaler::wait();
  return 0;
}
