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

#ifndef martianlabs_doba_examples_expect_continue_echo_handler_h
#define martianlabs_doba_examples_expect_continue_echo_handler_h

#include <array>
#include <string>

#include "protocol/http/v11/server.h"

namespace martianlabs::doba::examples {
// +===========================================================================+
// | [>] register_echo_route                                        ( method ) |
// +===========================================================================+
inline void register_echo_route(protocol::http::v11::server<>& http_server) {
  using protocol::http::v11::request;
  using protocol::http::v11::response;
  http_server.add_route(
      "POST", "/echo",
      [](const request& req) {
        response res = response::ok_200();
        if (!req.has_body_reader()) {
          res.set_body("");
          return res;
        }
        std::array<std::byte, 1024> buffer{};
        std::string body;
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
        res.set_body(body);
        return res;
      });
}
}  // namespace martianlabs::doba::examples

#endif
