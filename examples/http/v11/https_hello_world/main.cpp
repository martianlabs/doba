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

#include <chrono>
#include <string>

#include "common/signaler.h"
#include "https_server.h"

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | Supply a PEM certificate chain and an unencrypted PEM private key.        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int count, char** arguments) {
  if (count < 3 || count > 4) return 1;
  martianlabs::doba::transport::server::tls_policies policies;
  policies.ip = "0.0.0.0";
  policies.port = count == 4 ? arguments[3] : "8443";
  policies.certificate_file = arguments[1];
  policies.private_key_file = arguments[2];
  policies.alpn_protocol = "http/1.1";
  martianlabs::doba::https_server<> server(std::move(policies));
  server.add_route("GET", "/hello",
      [](const martianlabs::doba::protocol::http::v11::request&,
             martianlabs::doba::protocol::http::v11::response& response) {
    response.ok_200().set_body("hello over TLS");
  });
  server.add_route("GET", "/wait/:id",
      [](const martianlabs::doba::protocol::http::v11::request&,
         martianlabs::doba::protocol::http::v11::response& response,
         int id) -> martianlabs::doba::common::task<> {
    co_await martianlabs::doba::common::sleep_for(
        std::chrono::milliseconds(10));
    response.ok_200().set_body(std::to_string(id));
  });
  server.start();
  martianlabs::doba::common::signaler::wait();
  server.stop();
}
