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

#include <filesystem>

#include "protocol/http/v11/server.h"
#include "transport/server/tls.h"

namespace http = martianlabs::doba::protocol::http;
namespace v11 = http::v11;
namespace transport = martianlabs::doba::transport::server;
using router_type = http::router<v11::request, v11::response>;
using engine_type = v11::engine<v11::request, v11::response, router_type>;
using tls_server = v11::server<v11::request, v11::response, router_type,
                               engine_type, transport::tls>;

int main() {
  SSL_CTX* context = SSL_CTX_new(TLS_server_method());
  if (!context) return 1;
  SSL_CTX_free(context);
  const auto fixtures = std::filesystem::path(__FILE__).parent_path() /
      "../../../unit/transport/server/fixtures";
  transport::tls_policies configuration;
  configuration.certificate_file = (fixtures / "server.crt").string();
  configuration.private_key_file = (fixtures / "server.key").string();
  tls_server value(configuration);
  value.add_route("GET", "/", [](const v11::request&) {
    return v11::response::ok_200();
  });
  return 0;
}
