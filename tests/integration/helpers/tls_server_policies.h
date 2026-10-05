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

#ifndef martianlabs_doba_tests_integration_tls_server_policies_h
#define martianlabs_doba_tests_integration_tls_server_policies_h

#include <cstdint>
#include <filesystem>
#include <string>

#include "transport/server/tls.h"

namespace martianlabs::doba::tests::integration {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] server_policies                                          ( function ) |
// +---------------------------------------------------------------------------+
// | This function returns a TLS server configuration with the given port and  |
// | a self-signed certificate and private key for testing purposes.           |
// | It sets the IP address to "127.0.0.1" and the worker count to 2.          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
inline martianlabs::doba::transport::server::tls_policies server_policies(
    uint16_t port) {
  martianlabs::doba::transport::server::tls_policies configuration;
  const auto fixtures = std::filesystem::path(__FILE__).parent_path() /
                        "../transport/server/fixtures";
  configuration.certificate_file = (fixtures / "server.crt").string();
  configuration.private_key_file = (fixtures / "server.key").string();
  configuration.ip = "127.0.0.1";
  configuration.port = std::to_string(port);
  configuration.worker_count = 2;
  return configuration;
}
}  // namespace martianlabs::doba::tests::integration

#endif
