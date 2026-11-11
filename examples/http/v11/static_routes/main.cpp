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
#include "protocol/http/common/method_names.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http;
using martianlabs::doba::http_server;
using namespace martianlabs::doba::protocol::http::v11;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | Give each HTTP method its own handler for the same path.                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  http_server<> http_server({.ip = "0.0.0.0", .port = "8080"});
  http_server.add_route(
      method_names::kGet, "/resources", [](const request&, response& res) {
        res.ok_200();
        res.add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body("resource list");
        return;
      });
  http_server.add_route(
      method_names::kPost, "/resources", [](const request&, response& res) {
        res.created_201();
        res.add_header("Location", "/resources/1").set_body("resource created");
        return;
      });
  http_server.add_route(method_names::kPut, "/resources",
                        [](const request&, response& res) {
                          res.ok_200();
                          res.set_body("resource replaced");
                          return;
                        });
  http_server.add_route(method_names::kDelete, "/resources",
                        [](const request&, response& res) {
                          res.no_content_204();
                          // A 204 response has no body.
                          return;
                        });
  http_server.start();
  signaler::wait();
  return 0;
}
