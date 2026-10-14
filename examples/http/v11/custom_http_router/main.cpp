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

#include <string>
#include <string_view>

#include "common/signaler.h"
#include "http_server.h"
#include "protocol/http/common/router_match_result.h"
#include "protocol/http/v11/contracts.h"

namespace {
using namespace martianlabs::doba::protocol::http;
using namespace martianlabs::doba::protocol::http::v11;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] custom_router                                               ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class custom_router : public router<request, response> {
 public:
  // +=========================================================================+
  // | [>] match                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  [[nodiscard]]
  router_match_result<request, response> match(
      std::string_view method, std::string_view path) const {
    if (path == "/alias") path = "/hello";
    return router<request, response>::match(method, path);
  }
  // +=========================================================================+
  // | [>] allowed_methods                                          ( public ) |
  // +-------------------------------------------------------------------------+
  [[nodiscard]]
  std::string allowed_methods(std::string_view path) const {
    if (path == "/alias") path = "/hello";
    return router<request, response>::allowed_methods(path);
  }
};

static_assert(contracts::router<custom_router, request, response>);
}  // namespace

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  martianlabs::doba::http_server<request, response, custom_router> server(
      {.ip = "0.0.0.0", .port = "8080"});
  server.add_route("GET", "/hello", [](const request&, response& res) {
    res.ok_200().set_body("hello");
  });
  server.start();
  martianlabs::doba::common::signaler::wait();
  server.stop();
  return 0;
}
