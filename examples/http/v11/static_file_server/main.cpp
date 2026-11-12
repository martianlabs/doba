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

#include <iostream>

#include "common/signaler.h"
#include "http_server.h"
#include "protocol/http/v11/static_file_server.h"

using namespace martianlabs::doba::common;
using martianlabs::doba::http_server;
using namespace martianlabs::doba::protocol::http::v11;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | Serve files from the supplied root, with optional caching for /assets.    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
  if (argc < 2 || argc > 4) {
    std::cerr << "Usage: static_file_server <root> [--no-cache] [port]\n";
    return 1;
  }
  const bool direct = argc >= 3 && std::string_view(argv[2]) == "--no-cache";
  const std::string port =
      argc == 4 ? argv[3] : (argc == 3 && !direct ? argv[2] : "8080");
  static_file_policies assets;
  if (!direct) assets.cache = std::make_shared<static_file_cache>();
  assets.precompressed = true;
  http_server<> http_server({.ip = "0.0.0.0", .port = port});
  http_server.add_controller<static_file_server>("/assets", argv[1], assets)
      .add_controller<static_file_server>("/downloads", argv[1]);
  http_server.start();
  signaler::wait();
  http_server.stop();
  return 0;
}
