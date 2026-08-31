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

#include "common/console_logger.h"
#include "common/logo.h"
#include "common/signaler.h"
#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http::v11;

int main(int argc, char* argv[]) {
  server http_server;
  http_server.add_route(
      "GET", "/text",
      [](const request& req, response& res) {
        const std::string text = "body stored in the response buffer";
        // set_body() copies the value; text need not outlive this handler.
        res.ok_200()
            .add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body(text);
      });
  http_server.add_route(
      "GET", "/binary",
      [](const request& req, response& res) {
        const char bytes[] = {'d', 'o', 'b', 'a', '\0'};
        // An explicit size preserves embedded zero bytes.
        res.ok_200()
            .add_header("Content-Type", "application/octet-stream")
            .set_body(std::string_view(bytes, sizeof(bytes)));
      });
  http_server.add_route(
      "GET", "/integer",
      [](const request& req, response& res) {
        // Arithmetic values use the constrained numeric set_body() overload.
        res.ok_200()
            .add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body(42);
      });
  http_server.add_route(
      "GET", "/floating-point",
      [](const request& req, response& res) {
        // Numeric bodies follow std::to_string formatting.
        res.ok_200()
            .add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body(3.14);
      });
  http_server.start("8080");
  signaler::wait();
  return 0;
}
