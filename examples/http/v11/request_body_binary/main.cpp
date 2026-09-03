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
#include <utility>

#include "common/console_logger.h"
#include "common/logo.h"
#include "common/signaler.h"
#include "protocol/http/v11/body/writer.h"
#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http::v11;

int main() {
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
        auto writer = body::body_writer::raw();
        for (;;) {
          const auto state = req.get_body_reader()->read(buffer);
          if (state.has_error) {
            res.bad_request_400().set_body("unable to read request body");
            return;
          }
          // Forward only the bytes produced by this read, without conversion.
          if (!writer.write(
                  std::span<const std::byte>(buffer.data(), state.produced))) {
            res.internal_server_error_500();
            return;
          }
          if (state.complete) break;
        }
        res.ok_200()
            .add_header("Content-Type", "application/octet-stream")
            .set_body(std::move(writer));
      });
  http_server.start("8080");
  signaler::wait();
  return 0;
}
