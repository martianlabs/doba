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

#ifndef martianlabs_doba_protocol_http11_headers_forwarded_h
#define martianlabs_doba_protocol_http11_headers_forwarded_h

#include <utility>

#include "protocol/http11/connection.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                 forwarded |
// +===========================================================================+
// | RFC 7239 §4 Forwarded                                                     |
// +---------------------------------------------------------------------------+
// | The "Forwarded" header field is an optional request header field used by  |
// | proxies to disclose information that is altered or lost when a request    |
// | is forwarded. It can carry, for example, the original client address, the |
// | proxy-facing interface address, the original Host field value, or the     |
// | protocol used by the client.                                              |
// |                                                                           |
// | Forwarded is only for use in HTTP requests and MUST NOT be used in HTTP   |
// | responses. A proxy MAY append a new forwarded-element to an existing      |
// | Forwarded field value, add a new Forwarded field at the end of the header |
// | block, or remove existing Forwarded fields according to local policy.     |
// |                                                                           |
// | Each forwarded-element represents information added by one proxy. The     |
// | first element represents the first proxy that used this header field, and |
// | each subsequent element represents information added by each subsequent   |
// | proxy in the forwarding chain.                                            |
// |                                                                           |
// | Each forwarded-element is a semicolon-separated list of parameter-value   |
// | pairs. Parameter names are case-insensitive. Each parameter MUST NOT      |
// | occur more than once per forwarded-element.                               |
// |                                                                           |
// | Standard parameters are:                                                  |
// |                                                                           |
// |   by    identifies the user-agent-facing interface of the proxy.          |
// |   for   identifies the node making the request to the proxy.              |
// |   host  carries the Host request header field as received by the proxy.   |
// |   proto identifies the protocol used to make the request.                 |
// |                                                                           |
// | The "by" and "for" values, after quoted-string unescaping, conform to     |
// | the node syntax. A node can be an IPv4 address, a bracketed IPv6 address, |
// | the "unknown" identifier, or an obfuscated identifier.                    |
// |                                                                           |
// | IPv6 addresses and nodes containing a port MUST be represented as         |
// | quoted-string values because ":" and "[]" are not valid token chars.      |
// |                                                                           |
// | The header can contain privacy-sensitive and integrity-sensitive data. It |
// | MUST NOT be blindly trusted unless the forwarding proxies are trusted by  |
// | deployment policy.                                                        |
// |                                                                           |
// | Examples:                                                                 |
// |   Forwarded: for="_gazonk"                                                |
// |   Forwarded: For="[2001:db8:cafe::17]:4711"                               |
// |   Forwarded: for=192.0.2.60;proto=http;by=203.0.113.43                    |
// |   Forwarded: for=192.0.2.43, for=198.51.100.17                            |
// +---------------------------------------------------------------------------+
// | RFC 7239 §4 Forwarded (ABNF summary)                                      |
// +---------------------------------------------------------------------------+
// +-------------------+-------------------------------------------------------+
// | Field             | Definition                                            |
// +-------------------+-------------------------------------------------------+
// | Forwarded         | 1#forwarded-element                                   |
// | forwarded-element | [ forwarded-pair ] *( ";" [ forwarded-pair ] )        |
// | forwarded-pair    | token "=" value                                       |
// | value             | token / quoted-string                                 |
// | token             | 1*tchar                                               |
// | tchar             | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-"   |
// |                   | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA     |
// | quoted-string     | DQUOTE *( qdtext / quoted-pair ) DQUOTE               |
// | qdtext            | HTAB / SP / "!" / %x23-5B / %x5D-7E / obs-text        |
// | quoted-pair       | "\" ( HTAB / SP / VCHAR / obs-text )                  |
// | obs-text          | %x80-FF                                               |
// | OWS               | *( SP / HTAB )                                        |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class forwarded {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv, parsed_forwarded_list& out) {
    // The producer overload validates each forwarded-element exactly as the
    // pure check() does and captures every non-empty element's name=value
    // pairs, in order. It preserves the same 1#forwarded-element minimum.
    const bool result =
        helpers::for_each_list_element(sv, [&out](std::string_view element) {
          parsed_forwarded_element parsed;
          if (!consume_forwarded_element(element, &parsed)) return false;
          out.elements.push_back(std::move(parsed));
          return true;
        });
    return result && !out.elements.empty();
  }
  // +=========================================================================+
  // | [>] interpret                                                ( public ) |
  // +=========================================================================+
  static constexpr verdict interpret(
      const parsed_forwarded_list& forwarded_list, http11::connection&,
      const policies& pol) {
    if (pol.max_forwarding_hops != 0 &&
        forwarded_list.elements.size() > pol.max_forwarding_hops) {
      return verdict::kReject;
    }
    return verdict::kAccept;
  }

 private:
  // +=========================================================================+
  // | [>] consume_forwarded_element                               ( private ) |
  // +=========================================================================+
  // | forwarded-element = [ forwarded-pair ] *( ";" [ forwarded-pair ] )      |
  // | forwarded-pair    = token "=" ( token / quoted-string )                 |
  // +-------------------------------------------------------------------------+
  // | The RFC 7239 §5/§6 node / Host / URI-scheme constraints apply to the    |
  // | value after quoted-string unescaping and are semantic; only the token   |
  // | "=" ( token / quoted-string ) syntax is validated here. When out is     |
  // | non-null, each pair is captured, split at the "=" that                  |
  // | consume_parameter (allow_bws=false) has already validated, so no        |
  // | additional parsing is performed.                                        |
  // +=========================================================================+
  static bool consume_forwarded_element(
      std::string_view sv, parsed_forwarded_element* out = nullptr) {
    return helpers::for_each_forwarded_pair(
        sv, [out](std::string_view rest, std::size_t& bytes) {
          // forwarded-pair = token "=" value with a mandatory "=" and no
          // whitespace around it (RFC 7239 §4 has no BWS).
          if (!helpers::consume_parameter(rest, bytes, /*allow_bws=*/false)) {
            return false;
          }
          if (out) {
            const std::string_view pair = rest.substr(0, bytes);
            const std::size_t eq = pair.find('=');
            out->pairs.push_back({pair.substr(0, eq), pair.substr(eq + 1)});
          }
          return true;
        });
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
