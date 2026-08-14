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

int main(int argc, char* argv[]) {
  server http_server;
  // Status helpers select the status line before headers and body are added.
  http_server.add_route(
      "POST", "/resources",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res) {
        res->created_201()
            .add_header("Location", "/resources/1")
            .set_body("resource created");
      });
  http_server.add_route(
      "GET", "/redirect",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res) {
        res->temporary_redirect_307().add_header("Location", "/json");
      });
  http_server.add_route(
      "GET", "/json",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res) {
        res->ok_200()
            .add_header("Content-Type", "application/json")
            .add_header("Access-Control-Allow-Origin", "*")
            .add_header("Cache-Control", "no-store")
            .set_body("{\"framework\":\"doba\"}");
      });
  http_server.add_route(
      "GET", "/html",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res) {
        res->ok_200()
            .add_header("Content-Type", "text/html; charset=utf-8")
            .set_body("<h1>doba</h1>");
      });
  http_server.add_route(
      "GET", "/headers",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res) {
        // add appends, set replaces, and remove erases a response header.
        res->ok_200()
            .add_header("X-Example", "first")
            .set_header("X-Example", "replaced")
            .add_header("X-Remove", "value")
            .remove_header("X-Remove");
        const auto header = res->get_header("X-Example");
        std::string body = header.second;
        body.append("\nheaders: ");
        body.append(std::to_string(res->get_headers_length()));
        res->set_body(body);
      });
  http_server.add_route(
      "DELETE", "/resources",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res) {
        // 204 responses do not carry content or payload framing headers.
        res->no_content_204();
      });
  http_server.start("8080");
  signaler::wait();
  return 0;
}
