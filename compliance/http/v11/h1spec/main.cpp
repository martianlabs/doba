//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _`
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
#include <future>
#include <string>

#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::protocol::http::v11;

int main() {
  server<> http_server({.ip = "0.0.0.0", .port = "8080"});
  // h1spec expects successful routes to echo the decoded request body.
  const auto echo_body = [](const request& req) {
    response res = response::ok_200();
    std::string body;
    if (req.has_body_reader()) {
      std::array<std::byte, 4096> buffer{};
      for (;;) {
        const auto state = req.get_body_reader()->read(buffer);
        if (state.has_error) {
          res = response::bad_request_400();
          return res;
        }
        body.append(reinterpret_cast<const char*>(buffer.data()),
                    state.produced);
        if (state.complete) break;
      }
    }
    res.add_header("Content-Type", "text/plain").set_body(body);
    return res;
  };
  for (const auto method : {"GET", "HEAD", "POST", "PUT", "PATCH",
                            "DELETE", "OPTIONS", "TRACE"}) {
    http_server.add_route(method, "/", echo_body);
  }
  http_server.start();
  // The test harness owns process shutdown; wait after the server starts.
  std::promise<void> shutdown;
  shutdown.get_future().wait();
  return 0;
}
