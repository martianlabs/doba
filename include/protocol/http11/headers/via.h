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

#ifndef martianlabs_doba_protocol_http11_headers_via_h
#define martianlabs_doba_protocol_http11_headers_via_h

#include "protocol/http11/connection.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                       via |
// +===========================================================================+
// | RFC 9110 §7.6.3 Via                                                       |
// +---------------------------------------------------------------------------+
// | The "Via" header field indicates the presence of intermediate protocols   |
// | and recipients between the user agent and the server on requests, or      |
// | between the origin server and the client on responses.                    |
// |                                                                           |
// | Examples:                                                                 |
// |   Via: 1.0 fred, 1.1 p.example.net                                        |
// |   Via: HTTP/1.1 proxy.example.com (proxy software)                        |
// +---------------------------------------------------------------------------+
// | RFC 9110 §7.6.3 Via (ABNF summary)                                        |
// +---------------------------------------------------------------------------+
// +-------------------+-------------------------------------------------------+
// | Field             | Definition                                            |
// +-------------------+-------------------------------------------------------+
// | Via               | #( received-protocol RWS received-by [ RWS comment ] )|
// | received-protocol | [ protocol-name "/" ] protocol-version                |
// | received-by       | pseudonym [ ":" port ]                                |
// | pseudonym         | token                                                 |
// | port              | *DIGIT                                                |
// | comment           | "(" *( ctext / quoted-pair / comment ) ")"            |
// +-------------------+-------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class via {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv, parsed_via_list& out) {
    // The producer overload validates each via-member exactly as the pure
    // check() does and captures its received-protocol, received-by, and
    // optional comment for every non-empty element, in order.
    return helpers::for_each_list_element(sv, [&out](std::string_view element) {
      parsed_via_element parsed;
      if (!consume_via_member(element, &parsed)) return false;
      out.elements.push_back(parsed);
      return true;
    });
  }
  // +=========================================================================+
  // | [>] interpret                                                ( public ) |
  // +=========================================================================+
  static constexpr verdict interpret(const parsed_via_list& parsed_list,
                                     http11::connection&,
                                     const policies& policies) {
    if (policies.max_forwarding_hops != 0 &&
        parsed_list.elements.size() > policies.max_forwarding_hops) {
      return verdict::kReject;
    }
    return verdict::kAccept;
  }

 private:
  // +=========================================================================+
  // | [>] consume_via_member                                      ( private ) |
  // +=========================================================================+
  // | via-member = received-protocol RWS received-by [ RWS comment ]          |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_via_member(std::string_view sv,
                                           parsed_via_element* out = nullptr) {
    std::size_t off = 0;
    // received-protocol = [ protocol-name "/" ] protocol-version.
    const std::size_t protocol_start = off;
    if (!consume_received_protocol(sv, off)) return false;
    if (out)
      out->received_protocol = sv.substr(protocol_start, off - protocol_start);
    // RWS = 1*( SP / HTAB ) is mandatory between received-protocol and
    // received-by.
    if (!consume_rws(sv, off)) return false;
    // received-by = pseudonym [ ":" port ].
    const std::size_t received_by_start = off;
    if (!consume_received_by(sv, off)) return false;
    if (out)
      out->received_by = sv.substr(received_by_start, off - received_by_start);
    if (off >= sv.size()) return true;
    // [ RWS comment ] is optional, but when a comment is present it MUST be
    // preceded by RWS.
    if (!consume_rws(sv, off)) return false;
    const std::string_view comment = helpers::consume_comment(sv.substr(off));
    if (comment.empty()) return false;
    if (out) out->comment = comment;
    off += comment.size();
    return off == sv.size();
  }
  // +=========================================================================+
  // | [>] consume_received_protocol                               ( private ) |
  // +=========================================================================+
  static constexpr bool consume_received_protocol(std::string_view sv,
                                                  std::size_t& off) {
    // A leading token is either protocol-version, or protocol-name when it is
    // followed by "/".
    const std::string_view first = helpers::consume_token(sv.substr(off));
    if (first.empty()) return false;
    off += first.size();
    // No "/" means the token was protocol-version; received-protocol is done.
    if (off >= sv.size() || sv[off] != '/') return true;
    // A "/" means the token was protocol-name; protocol-version is mandatory.
    ++off;
    const std::string_view version = helpers::consume_token(sv.substr(off));
    if (version.empty()) return false;
    off += version.size();
    return true;
  }
  // +=========================================================================+
  // | [>] consume_received_by                                     ( private ) |
  // +=========================================================================+
  static constexpr bool consume_received_by(std::string_view sv,
                                            std::size_t& off) {
    // pseudonym is a token (never uri-host / IP-literal); it is mandatory.
    const std::string_view pseudonym = helpers::consume_token(sv.substr(off));
    if (pseudonym.empty()) return false;
    off += pseudonym.size();
    // An optional ":" introduces port = *DIGIT (zero or more digits allowed).
    if (off >= sv.size() || sv[off] != ':') return true;
    ++off;
    while (off < sv.size() && helpers::is_digit(sv[off])) ++off;
    return true;
  }
  // +=========================================================================+
  // | [>] consume_rws                                             ( private ) |
  // +=========================================================================+
  // | RWS = 1*( SP / HTAB ). Consumes at least one whitespace octet, returning|
  // | false when none is present.                                             |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_rws(std::string_view sv, std::size_t& off) {
    const std::size_t rws_start = off;
    while (off < sv.size() && helpers::is_ows(sv[off])) ++off;
    return off != rws_start;
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
