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

#ifndef martianlabs_doba_protocol_http11_headers_x_forwarded_for_h
#define martianlabs_doba_protocol_http11_headers_x_forwarded_for_h

#include "protocol/http11/connection.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                           x-forwarded-for |
// +===========================================================================+
// | De-facto X-Forwarded-For                                                  |
// +---------------------------------------------------------------------------+
// | The "X-Forwarded-For" header field is a de-facto request header used by   |
// | proxies, reverse proxies, load balancers, and gateways to identify the    |
// | originating client IP address and the forwarding chain seen by the HTTP   |
// | request.                                                                  |
// |                                                                           |
// | Because this field can be supplied directly by a client, its contents MUST|
// | NOT be trusted unless the request was received through a trusted proxy    |
// | chain that sanitizes or overwrites untrusted incoming values.             |
// |                                                                           |
// | Examples:                                                                 |
// |   X-Forwarded-For: 203.0.113.195                                          |
// |   X-Forwarded-For: 203.0.113.195, 70.41.3.18, 150.172.238.178             |
// |   X-Forwarded-For: 2001:db8:85a3::8a2e:370:7334                           |
// |   X-Forwarded-For: unknown, 203.0.113.10                                  |
// +---------------------------------------------------------------------------+
// | Practical de-facto syntax (ABNF summary)                                  |
// +---------------------------------------------------------------------------+
// +-------------------+-------------------------------------------------------+
// | Field             | Definition                                            |
// +-------------------+-------------------------------------------------------+
// | X-Forwarded-For   | #xff-node                                             |
// | xff-node          | IPv4address / IPv6address / IP-literal / "unknown"    |
// +-------------------+-------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class x_forwarded_for {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv, parsed_token_list& out) {
    // The producer overload validates each xff-node exactly as the pure
    // check() does and captures every non-empty node in order.
    return helpers::for_each_list_element(sv, [&out](std::string_view element) {
      if (!consume_xff_node(element)) return false;
      out.elements.push_back(element);
      return true;
    });
  }
  // +=========================================================================+
  // | [>] interpret                                                ( public ) |
  // +=========================================================================+
  static constexpr verdict interpret(const parsed_token_list& token_list,
                                     http11::connection&, const policies& pol) {
    if (pol.max_forwarding_hops != 0 &&
        token_list.elements.size() > pol.max_forwarding_hops) {
      return verdict::kReject;
    }
    return verdict::kAccept;
  }

 private:
  // +=========================================================================+
  // | [>] consume_xff_node                                        ( private ) |
  // +=========================================================================+
  // | xff-node = IPv4address / IPv6address / IP-literal / "unknown"           |
  // +-------------------------------------------------------------------------+
  // | Ports and general reg-name / host names are intentionally excluded from |
  // | this strict profile; IPvFuture is accepted only inside an IP-literal.   |
  // +=========================================================================+
  static constexpr bool consume_xff_node(std::string_view sv) {
    return helpers::is_ip_v4_address(sv) || helpers::is_ip_v6_address(sv) ||
           helpers::is_ip_literal(sv) || sv == "unknown";
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
