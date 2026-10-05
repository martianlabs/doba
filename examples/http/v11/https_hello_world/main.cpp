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

#include <iostream>

#include "common/signaler.h"
#include "protocol/http/v11/server.h"
#include "transport/server/tls.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http::v11;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | This is the entry point of the application. It creates an HTTPS server    |
// | that listens on all interfaces (0.0.0.0) and port 8443.                   |
// | It defines a resource handler for the "/hello" endpoint that responds to  |
// | GET requests with a simple "hello from doba" message. The server is       |
// | started, and the application waits for termination signals.               |
// | This example requires a certificate and private key to be provided as     |
// | command-line arguments.                                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: https_hello_world <certificate> <private-key>\n";
    return 1;
  }
  martianlabs::doba::transport::server::tls_policies configuration;
  configuration.ip = "0.0.0.0";
  configuration.port = "8443";
  configuration.certificate_file = argv[1];
  configuration.private_key_file = argv[2];
  server<request, response,
         martianlabs::doba::protocol::http::router<request, response>,
         engine<request, response>, martianlabs::doba::transport::server::tls>
      http_server(configuration);
  http_server.add_route("GET", "/hello", [](const request&, response& res) {
    res.ok_200();
    res.set_body("hello from doba");
    return;
  });
  http_server.start();
  signaler::wait();
  http_server.stop();
  return 0;
}
