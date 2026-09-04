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
#include <utility>

#include "common/byte_storage.h"
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
      "GET", "/large",
      [](const request&, response& res) {
        // Storage spills to a temporary file after this in-memory threshold.
        byte_storage_options options{.spill_threshold = 1024,
                                     .spill_dir = {}};
        auto writer = body::body_writer::raw(options);
        const std::string block(1024, 'x');
        for (int i = 0; i < 8; i++) {
          if (!writer.write(block)) {
            res.internal_server_error_500();
            return;
          }
        }
        res.ok_200()
            .add_header("Content-Type", "application/octet-stream")
            // The response adopts the writer and its storage.
            .set_body(std::move(writer));
      });
  http_server.start("8080");
  signaler::wait();
  return 0;
}
