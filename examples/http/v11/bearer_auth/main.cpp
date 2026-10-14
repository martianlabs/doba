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

#include <string_view>
#include <tuple>
#include <utility>

#include "common/console_logger.h"
#include "common/signaler.h"
#include "http_server.h"
#include "protocol/http/common/auth/bearer.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http::v11;
namespace doba = martianlabs::doba;
namespace auth = doba::protocol::http::auth;
using routes_type = doba::protocol::http::router<request, response>;
using engine_type = engine<request, response>;
using server_type = doba::http_server<request, response, routes_type,
    engine_type, doba::transport::server::tcp, auth::bearer>;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  auth::bearer provider("demo", [](std::string_view token) {
    return token == "demo-token";
  });
  server_type server({.ip = "127.0.0.1", .port = "8080"}, {},
                     std::tuple{std::move(provider)});
  server.add_route("GET", "/health", [](const request&, response& res) {
    res.ok_200().set_body("ok");
  });
  server.add_route<auth::bearer>("GET", "/private",
      [](const request&, response& res) {
        res.ok_200().set_body("Authenticated with Bearer");
      });
  server.start();
  signaler::wait();
  return 0;
}
