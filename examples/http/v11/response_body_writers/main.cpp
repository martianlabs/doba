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
      "GET", "/raw",
      [](const request&, response& res) {
        // A raw writer stores payload bytes without transfer-coding them.
        auto writer = body::body_writer::raw();
        if (!writer.write("first part\n") || !writer.write("second part\n")) {
          res.internal_server_error_500();
          return;
        }
        res.ok_200()
            .add_header("Content-Type", "text/plain; charset=utf-8")
            // Moving the writer derives Content-Length from bytes_written().
            .set_body(std::move(writer));
      });
  http_server.add_route(
      "GET", "/binary",
      [](const request&, response& res) {
        const std::array bytes{std::byte{0x00}, std::byte{0x01},
                               std::byte{0x02}, std::byte{0x03}};
        auto writer = body::body_writer::raw();
        // write() also accepts byte spans for binary payloads.
        if (!writer.write(bytes)) {
          res.internal_server_error_500();
          return;
        }
        res.ok_200()
            .add_header("Content-Type", "application/octet-stream")
            .set_body(std::move(writer));
      });
  http_server.add_route(
      "GET", "/chunked",
      [](const request&, response& res) {
        auto writer = body::body_writer::chunked();
        // Each write emits one chunk; end() emits the terminating chunk.
        if (!writer.write("first chunk\n") || !writer.write("second chunk\n") ||
            !writer.end()) {
          res.internal_server_error_500();
          return;
        }
        res.ok_200()
            .add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body(std::move(writer));
      });
  http_server.start("8080");
  signaler::wait();
  return 0;
}
