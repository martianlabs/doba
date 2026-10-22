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

#include "common/console_logger.h"
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
// | This is the entry point of the application. It creates an HTTP server     |
// | that listens on all interfaces (0.0.0.0) and port 8080.                   |
// | The server registers two static file servers:                             |
// |   * "/assets" serving files from the directory specified by argv[1],      |
// |   * "/downloads" serving files from the same directory.                   |
// | The server runs until a termination signal is received.                   |
// | The root directory for the static file servers must be provided as        |
// | a command-line argument.                                                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
  if (argc < 2 || argc > 4) {
    std::cerr << "Usage: static_file_server <root> [--no-cache|--metadata-cache] [port]\n";
    return 1;
  }
  const bool direct = argc >= 3 && std::string_view(argv[2]) == "--no-cache";
  const bool metadata = argc >= 3 && std::string_view(argv[2]) == "--metadata-cache";
  const std::string port = argc == 4 ? argv[3] : (argc == 3 && !direct && !metadata ? argv[2] : "8080");
  static_file_policies assets;
  if (!direct) assets.cache = std::make_shared<static_file_cache>();
  assets.precompressed = true;
  assets.monitor_files = !metadata;
  http_server<> http_server({.ip = "0.0.0.0", .port = port});
  http_server.add_controller<static_file_server>("/assets", argv[1], assets)
      .add_controller<static_file_server>("/downloads", argv[1]);
  http_server.start();
  signaler::wait();
  http_server.stop();
  return 0;
}
