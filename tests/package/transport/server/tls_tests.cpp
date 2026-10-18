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

#include <cstddef>
#include <filesystem>
#include <functional>

#include "https_server.h"
#include "transport/server/tls.h"

namespace http = martianlabs::doba::protocol::http;
namespace v11 = http::v11;
namespace transport = martianlabs::doba::transport::server;

struct custom_engine {
  struct policies_type {};

  custom_engine(const policies_type&,
                const http::router<v11::request, v11::response>&) {}
  void set_on_send(martianlabs::doba::protocol::send_delegate) {}
  void set_on_close(std::function<void()>) {}
  void on_bytes_received(const char*, std::size_t) {
    return;
  }
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | This is a simple example of how to use the TLS server with an HTTP/1.1    | 
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  SSL_CTX* context = SSL_CTX_new(TLS_server_method());
  if (!context) return 1;
  SSL_CTX_free(context);
  const auto fixtures = std::filesystem::path(__FILE__).parent_path() /
                        "fixtures";
  transport::tls_policies configuration;
  configuration.certificate_file = (fixtures / "server.crt").string();
  configuration.private_key_file = (fixtures / "server.key").string();
  martianlabs::doba::https_server<
      v11::request, v11::response, http::router<v11::request, v11::response>,
      v11::engine<v11::request, v11::response,
                  http::router<v11::request, v11::response>>>
      value(configuration);
  value.add_route("GET", "/", [](const v11::request&, v11::response& res) {
    res.ok_200();
  });
  martianlabs::doba::https_server<v11::request, v11::response,
                                  http::router<v11::request, v11::response>,
                                  custom_engine>
      custom(configuration);
  return 0;
}
