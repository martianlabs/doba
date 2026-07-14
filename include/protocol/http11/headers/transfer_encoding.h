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

#ifndef martianlabs_doba_protocol_http11_headers_transfer_encoding_h
#define martianlabs_doba_protocol_http11_headers_transfer_encoding_h

#include "protocol/http11/connection.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] transfer_encoding                                           ( class ) |
// +---------------------------------------------------------------------------+
// | RFC 9112 §6.1 Transfer-Encoding                                           |
// +---------------------------------------------------------------------------+
// | The "Transfer-Encoding" header field lists the transfer coding names      |
// | corresponding to the sequence of transfer codings that have been, or will |
// | be, applied to the content in order to form the message body.             |
// |                                                                           |
// | Transfer coding names are case-insensitive.                               |
// |                                                                           |
// | Example:                                                                  |
// |  Transfer-Encoding: gzip, chunked                                         |
// +---------------------------------------------------------------------------+
// | RFC 9112 §6.1 / RFC 9110 §10.1.4 (ABNF summary)                           |
// +---------------------------------------------------------------------------+
// +--------------------+------------------------------------------------------+
// | Field              | Definition                                           |
// +--------------------+------------------------------------------------------+
// | Transfer-Encoding  | #transfer-coding                                     |
// | transfer-coding    | token *( OWS ";" OWS transfer-parameter )            |
// | transfer-parameter | token BWS "=" BWS ( token / quoted-string )          |
// +--------------------+------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class transfer_encoding {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv, parsed_parameter_list& out) {
    // The producer overload validates each transfer-coding exactly as the pure
    // check() does and captures every non-empty element (its token plus any
    // transfer-parameters) as raw text, in order.
    return helpers::for_each_list_element(sv, [&out](std::string_view element) {
      if (!consume_transfer_coding(element)) return false;
      out.elements.push_back(element);
      return true;
    });
  }
  // +=========================================================================+
  // | [>] interpret                                                ( public ) |
  // +=========================================================================+
  static constexpr verdict interpret(
      const parsed_parameter_list& parameters_list, http11::connection& conn,
      const policies& pol) {
    if (pol.max_transfer_codings != 0 &&
        parameters_list.elements.size() > pol.max_transfer_codings) {
      return verdict::kReject;
    }
    for (const std::string_view element : parameters_list.elements) {
      // Each element is "transfer-coding *( ";" parameter )"; the coding name
      // is the leading token, before any OWS or ";"-separated parameters.
      const std::string_view coding = helpers::consume_token(element);
      conn.transfer_codings.push_back(coding);
    }
    // The final transfer-coding governs message framing (RFC 9112 §6.1).
    conn.chunked = !conn.transfer_codings.empty() &&
                   helpers::iequals(conn.transfer_codings.back(), "chunked");
    if (conn.chunked && !pol.allow_chunked) return verdict::kReject;
    return verdict::kAccept;
  }

 private:
  // +=========================================================================+
  // | [>] consume_transfer_coding                                 ( private ) |
  // +=========================================================================+
  static constexpr bool consume_transfer_coding(std::string_view sv) {
    std::size_t off = 0;
    const std::string_view token = helpers::consume_token(sv);
    if (token.empty()) return false;
    off += token.size();
    if (off >= sv.size()) return true;
    // transfer-parameters = *( OWS ";" OWS transfer-parameter ) — a parameter
    // is mandatory after every ";" and OWS/BWS is permitted around the "=".
    return helpers::for_each_parameter(
        sv.substr(off), /*require_parameter=*/true,
        [](std::string_view rest, std::size_t& bytes) {
          return helpers::consume_parameter(rest, bytes, /*allow_bws=*/true);
        });
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
