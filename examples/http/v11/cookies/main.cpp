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

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | This is the entry point of the application. It creates an HTTP server     |
// | that listens on all interfaces (0.0.0.0) and port 8080.                   |
// | The server has a single route "/cookies" that responds to GET requests.   |
// | When a request is made to this route, the server checks for a "session"   |
// | cookie and returns its value in the response body. If the cookie is not   |
// | provided, it indicates that in the response. Additionally, the server     |
// | lists all cookies present in the request and sets two cookies in the      |
// | response: "session" and "theme". The server runs until it receives a      |
// | termination signal, at which point it stops gracefully.                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  server<> http_server({.ip = "0.0.0.0", .port = "8080"});
  http_server.add_route(
      "GET", "/cookies", [](const request& req, response& res) {
        res.ok_200();
        std::string body = "session: ";
        // Cookie lookup parses the Cookie field on demand.
        const auto session = req.get_cookie("session");
        body.append(session ? *session : "not provided");
        body.append("\n\nall cookies:\n");
        for (const auto& [name, value] : req.get_cookies()) {
          body.append(name);
          body.append(" = ");
          body.append(value);
          body.push_back('\n');
        }
        res.add_header("Content-Type", "text/plain; charset=utf-8")
            // Keep each Set-Cookie value as a separate header field.
            .add_header("Set-Cookie", "session=doba; Path=/; HttpOnly")
            .add_header("Set-Cookie", "theme=dark; Path=/; SameSite=Lax")
            .set_body(body);
        return;
      });
  http_server.start();
  signaler::wait();
  return 0;
}
