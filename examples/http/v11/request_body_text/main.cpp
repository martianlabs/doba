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

#include <array>
#include <cstddef>
#include <string>

#include "common/console_logger.h"
#include "common/logo.h"
#include "common/signaler.h"
#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http::v11;

int main(int argc, char* argv[]) {
  server http_server;
  http_server.add_route(
      "POST", "/echo",
      [](const request& req, response& res) {
        // The reader hides Content-Length and chunked request framing.
        if (!req.has_body_reader()) {
          res.bad_request_400().set_body("request body required");
          return;
        }
        std::array<std::byte, 1024> buffer{};
        std::string text;
        for (;;) {
          // produced is the valid byte count; complete ends the read loop.
          const auto state = req.get_body_reader()->read(buffer);
          if (state.has_error) {
            res.bad_request_400().set_body("unable to read request body");
            return;
          }
          text.append(reinterpret_cast<const char*>(buffer.data()),
                      state.produced);
          if (state.complete) break;
        }
        res.ok_200()
            .add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body(text);
      });
  http_server.start("8080");
  signaler::wait();
  return 0;
}
