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

#ifndef martianlabs_doba_protocol_http11_headers_x_forwarded_proto_h
#define martianlabs_doba_protocol_http11_headers_x_forwarded_proto_h

#include "protocol/http11/connection.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] x_forwarded_proto                                           ( class ) |
// +---------------------------------------------------------------------------+
// | De-facto X-Forwarded-Proto                                                |
// +---------------------------------------------------------------------------+
// | The "X-Forwarded-Proto" header field is a de-facto request header used by |
// | reverse proxies to disclose the protocol (scheme) used by the client to   |
// | communicate with the proxy (typically "http" or "https").                 |
// |                                                                           |
// | The header carries no hop-by-hop connection state. Whether this forwarded |
// | scheme may be trusted is a transversal concern left to the rules layer.   |
// |                                                                           |
// | Examples:                                                                 |
// |   X-Forwarded-Proto: https                                                |
// |   X-Forwarded-Proto: https, http                                          |
// +---------------------------------------------------------------------------+
// | Non-standard X-Forwarded-Proto (ABNF summary)                             |
// +---------------------------------------------------------------------------+
// +-------------------+-------------------------------------------------------+
// | Field             | Definition                                            |
// +-------------------+-------------------------------------------------------+
// | X-Forwarded-Proto | #scheme                                               |
// | scheme            | ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )            |
// +-------------------+-------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class x_forwarded_proto {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv, parsed_token_list& out) {
    // The producer overload validates each scheme exactly as the pure check()
    // does and captures every non-empty scheme in order.
    return helpers::for_each_list_element(sv, [&out](std::string_view element) {
      if (!consume_scheme(element)) return false;
      out.elements.push_back(element);
      return true;
    });
  }
  // +=========================================================================+
  // | [>] interpret                                                ( public ) |
  // +=========================================================================+
  static constexpr verdict interpret(const parsed_token_list&,
                                     http11::connection&, const policies&) {
    return verdict::kAccept;
  }

 private:
  // +=========================================================================+
  // | [>] consume_scheme                                          ( private ) |
  // +=========================================================================+
  static constexpr bool consume_scheme(std::string_view sv) {
    // Each list element is a bare URI scheme (RFC 3986 §3.1), matching the
    // "proto" parameter value of the RFC 7239 Forwarded header.
    return helpers::is_uri_scheme(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
