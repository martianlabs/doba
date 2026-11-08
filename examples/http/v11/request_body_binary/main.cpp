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

#include <array>
#include <cstddef>
#include <utility>

#include "common/console_logger.h"
#include "common/signaler.h"
#include "protocol/http/v11/body/writer.h"
#include "http_server.h"

using namespace martianlabs::doba::common;
using martianlabs::doba::http_server;
using namespace martianlabs::doba::protocol::http::v11;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | Echo the POST /echo body as binary, zero bytes included.                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  http_server<> http_server({.ip = "0.0.0.0", .port = "8080"});
  http_server.add_route("POST", "/echo", [](const request& req, response& res) {
    res.ok_200();
    // The reader handles Content-Length and chunking for us.
    if (!req.has_body_reader()) {
      res.bad_request_400();
      res.set_body("request body required");
      return;
    }
    std::array<std::byte, 1024> buffer{};
    auto writer = body::body_writer::raw();
    for (;;) {
      const auto state = req.get_body_reader()->read(buffer);
      if (state.has_error) {
        res.bad_request_400();
        res.set_body("unable to read request body");
        return;
      }
      // Forward just the bytes from this read; don't convert them.
      if (!writer.write(
              std::span<const std::byte>(buffer.data(), state.produced))) {
        res.internal_server_error_500();
        return;
      }
      if (state.complete) break;
    }
    res.add_header("Content-Type", "application/octet-stream")
        .set_body(std::move(writer));
    return;
  });
  http_server.start();
  signaler::wait();
  return 0;
}
