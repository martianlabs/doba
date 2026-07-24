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

#include "network/environment.h"
#include "protocol/http11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http11;

int main(int argc, char* argv[]) {
  martianlabs::doba::network::startup();
  date_server::get().start();
  server http_server;

  http_server.add_route(
      "GET", "/pipeline",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res) {
        res->ok_200()
            .add_header("Server", "doba.")
            .add_header("Date", date_server::get().current())
            .add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body("ok");
      });

  /*
  pepe
  */

  http_server.add_route(
      "GET", "/pipelineplus/:param/:param_2",
      [](std::shared_ptr<const request> req, std::shared_ptr<response> res,
         int param, std::string param_2) {
        res->ok_200()
            .add_header("Server", "doba.")
            .add_header("Date", date_server::get().current())
            .add_header("Content-Type", "text/plain; charset=utf-8")
            .set_body("ok with param: " + std::to_string(param) +
                      " and param_2: " + param_2);
      });

  /*
  pepe fin
  */

  http_server.start("8080");
  std::cin.get();
  date_server::get().stop();
  martianlabs::doba::network::cleanup();
  return 0;
}
