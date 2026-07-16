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

#ifndef martianlabs_doba_protocol_http11_headers_upgrade_h
#define martianlabs_doba_protocol_http11_headers_upgrade_h

#include "protocol/http11/connection.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                   upgrade |
// +===========================================================================+
// | RFC 9110 §7.8 Upgrade                                                     |
// +---------------------------------------------------------------------------+
// | The "Upgrade" header field provides a mechanism for transitioning from    |
// | HTTP/1.1 to another protocol on the same connection.                      |
// |                                                                           |
// | A client MAY send a list of protocols in a request, ordered by descending |
// | preference, to invite the server to switch protocols before sending the   |
// | final response.                                                           |
// |                                                                           |
// | A sender of Upgrade MUST also send "Upgrade" as a connection option:      |
// |                                                                           |
// |   Connection: Upgrade                                                     |
// |                                                                           |
// | Examples:                                                                 |
// |   Upgrade: websocket                                                      |
// |   Upgrade: websocket, IRC/6.9, RTA/x11                                    |
// +---------------------------------------------------------------------------+
// | RFC 9110 §7.8 Upgrade (ABNF summary)                                      |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Upgrade          | #protocol                                              |
// | protocol         | protocol-name [ "/" protocol-version ]                 |
// | protocol-name    | token                                                  |
// | protocol-version | token                                                  |
// +------------------+--------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class upgrade {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv, parsed_token_list& out) {
    // The producer overload validates each protocol exactly as the pure
    // check() does and captures every non-empty element (protocol-name with
    // its optional "/" protocol-version) in order.
    return helpers::for_each_list_element(sv, [&out](std::string_view element) {
      if (!consume_protocol(element)) return false;
      out.elements.push_back(element);
      return true;
    });
  }
  // +=========================================================================+
  // | [>] interpret                                                ( public ) |
  // +=========================================================================+
  static constexpr verdict interpret(const parsed_token_list& token_list,
                                     http11::connection& conn,
                                     const policies& pol) {
    for (const std::string_view protocol : token_list.elements) {
      conn.upgrade_offer.push_back(protocol);
    }
    if (!conn.upgrade_offer.empty() && !pol.allow_upgrade) {
      return verdict::kReject;
    }
    return verdict::kAccept;
  }

 private:
  // +=========================================================================+
  // | [>] consume_protocol                                        ( private ) |
  // +=========================================================================+
  static constexpr bool consume_protocol(std::string_view sv) {
    std::size_t off = 0;
    const std::string_view name = helpers::consume_token(sv);
    if (name.empty()) return false;
    off += name.size();
    if (off >= sv.size()) return true;
    if (sv[off++] != '/') return false;
    const std::string_view version = helpers::consume_token(sv.substr(off));
    return !version.empty() && off + version.size() == sv.size();
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
