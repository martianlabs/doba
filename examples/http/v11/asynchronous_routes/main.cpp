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

#include <chrono>
#include <thread>

#include "common/console_logger.h"
#include "common/execution_policy.h"
#include "common/logo.h"
#include "common/signaler.h"
#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http::v11;

int main(int argc, char* argv[]) {
  server http_server;
  // Routes use synchronous execution unless a policy is supplied.
  http_server.add_route(
      "GET", "/synchronous",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res) {
        res->ok_200().set_body("synchronous response");
      });
  http_server.add_route(
      "GET", "/asynchronous",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res) {
        // This delay represents work that should not block synchronous routing.
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        res->ok_200().set_body("asynchronous response");
      },
      // Asynchronous handlers are queued and send their response when done.
      execution_policy::kAsynchronous);
  http_server.start("8080");
  signaler::wait();
  return 0;
}
