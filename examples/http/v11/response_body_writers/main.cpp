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
// | Build replies with raw and chunked body writers.                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  http_server<> http_server({.ip = "0.0.0.0", .port = "8080"});
  http_server.add_route("GET", "/raw", [](const request&, response& res) {
    res.ok_200();
    // The raw writer stores bytes without adding transfer framing.
    auto writer = body::body_writer::raw();
    if (!writer.write("first part\n") || !writer.write("second part\n")) {
      res.internal_server_error_500();
      return;
    }
    res.add_header("Content-Type", "text/plain; charset=utf-8")
        // Move the writer in; Content-Length comes from bytes_written().
        .set_body(std::move(writer));
    return;
  });
  http_server.add_route("GET", "/binary", [](const request&, response& res) {
    res.ok_200();
    const std::array bytes{std::byte{0x00}, std::byte{0x01}, std::byte{0x02},
                           std::byte{0x03}};
    auto writer = body::body_writer::raw();
    // Use a byte span for binary data.
    if (!writer.write(bytes)) {
      res.internal_server_error_500();
      return;
    }
    res.add_header("Content-Type", "application/octet-stream")
        .set_body(std::move(writer));
    return;
  });
  http_server.add_route("GET", "/chunked", [](const request&, response& res) {
    res.ok_200();
    auto writer = body::body_writer::chunked();
    // Each write adds a chunk; end() adds the final one.
    if (!writer.write("first chunk\n") || !writer.write("second chunk\n") ||
        !writer.end()) {
      res.internal_server_error_500();
      return;
    }
    res.add_header("Content-Type", "text/plain; charset=utf-8")
        .set_body(std::move(writer));
    return;
  });
  http_server.start();
  signaler::wait();
  return 0;
}
