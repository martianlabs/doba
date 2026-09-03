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
using namespace martianlabs::doba::protocol::http;
using namespace martianlabs::doba::protocol::http::v11;

int main() {
  server http_server;
  http_server.add_route(
      "GET", "/request",
      [](const request& req, response& res) {
        // Target and host syntax are parsed before the handler is called.
        std::string target_form;
        switch (req.get_target()) {
          case target::kOriginForm:
            target_form = "origin-form";
            break;
          case target::kAbsoluteForm:
            target_form = "absolute-form";
            break;
          default:
            target_form = "unknown";
            break;
        }
        std::string host_type;
        switch (req.get_host_type()) {
          case helpers::host_type::kIpLiteral:
            host_type = "IP literal";
            break;
          case helpers::host_type::kIpV4Address:
            host_type = "IPv4 address";
            break;
          case helpers::host_type::kRegName:
            host_type = "registered name";
            break;
          default:
            host_type = "unknown";
            break;
        }
        // Request accessors return views backed by the request object.
        std::string body = "method: ";
        body.append(req.get_method());
        body.append("\npath: ");
        body.append(req.get_absolute_path());
        body.append("\ntarget: ");
        body.append(target_form);
        body.append("\nhost: ");
        body.append(req.get_host());
        body.append("\nport: ");
        body.append(req.get_host_port());
        body.append("\nhost type: ");
        body.append(host_type);
        body.append("\nconnection close: ");
        body.append(req.wants_connection_close() ? "true" : "false");
        res.ok_200()
            .add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body(body);
      });
  http_server.start("8080");
  signaler::wait();
  return 0;
}
