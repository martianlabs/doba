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

#include "common/console_logger.h"
#include "common/logo.h"
#include "common/signaler.h"
#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http::v11;

int main() {
  server http_server;
  http_server.add_route(
      "GET", "/headers",
      [](const request& req, response& res) {
        std::string body = "user-agent: ";
        // Header-name lookup is case-insensitive.
        if (req.exist_header("User-Agent")) {
          body.append(req.get_header("User-Agent").second);
        } else {
          body.append("not provided");
        }
        body.append("\n\nall headers:\n");
        // Indexed access also allows every received field to be inspected.
        for (std::size_t i = 0; i < req.get_headers_length(); i++) {
          const auto [name, value] = req.get_header(i);
          body.append(name);
          body.append(": ");
          body.append(value);
          body.push_back('\n');
        }
        res.ok_200()
            .add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body(body);
      });
  http_server.start("8080");
  signaler::wait();
  return 0;
}
