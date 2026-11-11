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

#ifndef martianlabs_doba_https_server_h
#define martianlabs_doba_https_server_h

#include <stdexcept>
#include <tuple>
#include <utility>

#include "http_server.h"
#include "transport/tls.h"

namespace martianlabs::doba {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] https_server                                                ( class ) |
// +---------------------------------------------------------------------------+
// | Composes a TLS HTTP/1.1 routing with a protocol engine and transport.     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty = protocol::http::v11::request,
          typename RSty = protocol::http::v11::response,
          typename ROty = protocol::http::router<RQty, RSty>,
          protocol::contracts::engine ENty =
              protocol::http::v11::engine<RQty, RSty, ROty>,
          typename... AUty>
class https_server
    : public http_server<RQty, RSty, ROty, ENty, transport::tls, AUty...> {
  // +=========================================================================+
  // | [>] USINGs                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  using base = http_server<RQty, RSty, ROty, ENty, transport::tls, AUty...>;

 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit https_server(transport::tls_policies configuration = {},
                        typename ENty::policies_type engine_configuration = {})
    requires(sizeof...(AUty) == 0)
      : base(configure(std::move(configuration)),
             std::move(engine_configuration)) {}
  https_server(transport::tls_policies configuration,
               typename ENty::policies_type engine_configuration,
               std::tuple<AUty...> authentication)
      : base(configure(std::move(configuration)),
             std::move(engine_configuration), std::move(authentication)) {}

 private:
  // +=========================================================================+
  // | [>] configure                                               ( private ) |
  // +-------------------------------------------------------------------------+
  // HTTP composition owns ALPN; the generic TLS transport remains agnostic.
  static transport::tls_policies configure(
      transport::tls_policies configuration) {
    if (!configuration.alpn_protocol.empty() &&
        configuration.alpn_protocol != "http/1.1") {
      throw std::invalid_argument("HTTPS requires the http/1.1 ALPN token");
    }
    configuration.alpn_protocol = "http/1.1";
    return configuration;
  }
};
}  // namespace martianlabs::doba

#endif
