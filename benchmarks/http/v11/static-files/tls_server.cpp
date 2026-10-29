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
// Copyright 2026 martianLabs
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
#include <string>

#include "common/signaler.h"
#include "https_server.h"
#include "protocol/http/v11/static_file_server.h"

using namespace martianlabs::doba;
using namespace martianlabs::doba::protocol::http::v11;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | Serve the mounted assets through the standard cached TLS controller.     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
  if (argc != 6) {
    std::cerr << "Usage: static_tls <root> <workers> <port> <certificate> <key>\n";
    return 1;
  }
  transport::server::tls_policies configuration;
  configuration.worker_count = std::stoul(argv[2]);
  configuration.ip = "127.0.0.1";
  configuration.port = argv[3];
  configuration.certificate_file = argv[4];
  configuration.private_key_file = argv[5];
  https_server<> server(configuration);
  static_file_policies assets;
  assets.precompressed = true;
  assets.cache = std::make_shared<static_file_cache>();
  server.add_controller<static_file_server>("/static", argv[1], assets);
  server.start();
  common::signaler::wait();
  server.stop();
}
