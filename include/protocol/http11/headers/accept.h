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

#ifndef martianlabs_doba_protocol_http11_headers_accept_h
#define martianlabs_doba_protocol_http11_headers_accept_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                    accept |
// +===========================================================================+
// | RFC 9110 �12.5.1 Accept                                                   |
// +---------------------------------------------------------------------------+
// | The "Accept" header field allows a user agent to specify its preferences  |
// | regarding the media types of the response representation. It can be used  |
// | to restrict the response to a set of desired media types or to express    |
// | relative preferences between them.                                        |
// |                                                                           |
// | When sent by a server in a response, Accept indicates which content types |
// | are preferred in the content of a subsequent request to the same          |
// | resource.                                                                 |
// |                                                                           |
// | Media ranges can use wildcards:                                           |
// |                                                                           |
// |   * "*" / "*" matches every media type.                                   |
// |   * "type/*" matches every subtype of the specified type.                 |
// |   * "type/subtype" matches one specific media type.                       |
// |                                                                           |
// | A media range can contain media type parameters, such as "charset" or     |
// | "format", followed by an optional quality value ("q") representing its    |
// | relative preference.                                                      |
// |                                                                           |
// | Quality values range from 0 to 1:                                         |
// |                                                                           |
// |   * q=1 is the highest preference and is the default when omitted.        |
// |   * q=0 indicates that the media range is not acceptable.                 |
// |   * A maximum of three decimal digits can be generated.                   |
// |                                                                           |
// | More specific media ranges take precedence over less specific ranges.     |
// | For example, "text/plain" takes precedence over "text/*", which takes     |
// | precedence over "*/*". Media parameters make a range more specific.       |
// |                                                                           |
// | Senders using a weight SHOULD place the "q" parameter after all media     |
// | range parameters. Recipients SHOULD interpret any parameter named "q" as  |
// | the weight, regardless of its position. Parameter names, including "q",   |
// | are compared case-insensitively.                                          |
// |                                                                           |
// | Unlike earlier specifications, RFC 9110 does not define accept extension  |
// | parameters after the weight. The former "accept-params" and "accept-ext"  |
// | grammar has been removed.                                                 |
// |                                                                           |
// | Examples:                                                                 |
// |   Accept: text/html                                                       |
// |   Accept: image/*                                                         |
// |   Accept: */*                                                             |
// |   Accept: text/html, application/xhtml+xml                                |
// |   Accept: text/plain;format=flowed;q=0.8, text/*;q=0.5, */*;q=0.1         |
// +---------------------------------------------------------------------------+
// | RFC 9110 �12.5.1 Accept (ABNF summary)                                    |
// +---------------------------------------------------------------------------+
// +-----------------+---------------------------------------------------------+
// | Field           | Definition                                              |
// +-----------------+---------------------------------------------------------+
// | Accept          | #( media-range [ weight ] )                             |
// | media-range     | ( "*/*" / ( type "/" "*" ) /                            |
// |                 |   ( type "/" subtype ) ) parameters                     |
// | type            | token                                                   |
// | subtype         | token                                                   |
// | parameters      | *( OWS ";" OWS [ parameter ] )                          |
// | parameter       | parameter-name "=" parameter-value                      |
// | parameter-name  | token                                                   |
// | parameter-value | token / quoted-string                                   |
// | weight          | OWS ";" OWS "q=" qvalue                                 |
// | qvalue          | ( "0" [ "." 0*3DIGIT ] ) /                              |
// |                 | ( "1" [ "." 0*3("0") ] )                                |
// | token           | 1*tchar                                                 |
// | tchar           | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /   |
// |                 | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA       |
// | quoted-string   | DQUOTE *( qdtext / quoted-pair ) DQUOTE                 |
// | qdtext          | HTAB / SP / "!" / %x23-5B / %x5D-7E / obs-text          |
// | quoted-pair     | "\" ( HTAB / SP / VCHAR / obs-text )                    |
// | obs-text        | %x80-FF                                                 |
// | OWS             | *( SP / HTAB )                                          |
// +---------------------------------------------------------------------------+
// | Parameter syntax                                                          |
// +---------------------------------------------------------------------------+
// | OWS is permitted around the semicolon introducing a parameter, but no     |
// | whitespace is permitted around the "=" character inside the parameter:    |
// |                                                                           |
// |   text/html ; charset=utf-8        valid                                  |
// |   text/html;charset="utf-8"        valid                                  |
// |   text/html;charset = utf-8        invalid                                |
// |                                                                           |
// | The generic "parameters" rule permits an empty parameter after a          |
// | semicolon because "parameter" is optional. Thus, values such as           |
// | "text/html;" are accepted by the recipient-side ABNF.                     |
// +---------------------------------------------------------------------------+
// | RFC 9110 �5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | In this field, the list element is:                                       |
// |                                                                           |
// |   element = media-range [ weight ]                                        |
// |                                                                           |
// | Sender syntax:                                                            |
// |                                                                           |
// |   #element = [ element *( OWS "," OWS element ) ]                         |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #element = [ element ] *( OWS "," OWS [ element ] )                     |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "#element", rather than "1#element", zero non-empty     |
// | media-range elements are permitted by the purely syntactic ABNF.          |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class accept {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    return helpers::for_each_list_element(sv, consume_media_range_element);
  }

 private:
  // +=========================================================================+
  // | [>] consume_media_range_element                             ( private ) |
  // +=========================================================================+
  static constexpr bool consume_media_range_element(std::string_view sv) {
    std::size_t off = 0;
    const std::string_view type = helpers::consume_token(sv);
    if (type.empty()) return false;
    off += type.size();
    if (off >= sv.size() || sv[off++] != '/') return false;
    const std::string_view subtype = helpers::consume_token(sv.substr(off));
    if (subtype.empty()) return false;
    // "*/*" rule: a wildcard type requires a wildcard subtype.
    if (type == "*" && subtype != "*") return false;
    off += subtype.size();
    if (off >= sv.size()) return true;
    // parameters = *( OWS ";" OWS [ parameter ] ) � empty slots allowed and no
    // whitespace is permitted around the "=" of a media-range parameter. The
    // "q" weight parameter is handled specially and may appear at most once.
    bool q_found = false;
    return helpers::for_each_parameter(
        sv.substr(off), /*require_parameter=*/false,
        [&q_found](std::string_view rest, std::size_t& bytes) {
          return consume_parameter(rest, bytes, q_found);
        });
  }
  // +=========================================================================+
  // | [>] consume_parameter                                       ( private ) |
  // +=========================================================================+
  static constexpr bool consume_parameter(std::string_view sv,
                                          std::size_t& bytes_used,
                                          bool& q_found) {
    bytes_used = 0;
    const std::string_view name = helpers::consume_token(sv);
    if (name.empty()) return false;
    if (helpers::iequals(name, "q")) {
      std::size_t i = name.size();
      // No whitespace is allowed around '='.
      if (i >= sv.size() || sv[i++] != '=') return false;
      if (i >= sv.size()) return false;
      // Only one weight parameter is permitted.
      if (q_found) return false;
      // qvalue is a subset of token. Consume only the qvalue candidate,
      // not the complete remainder of the parameter sequence.
      const std::string_view qvalue = helpers::consume_token(sv.substr(i));
      if (qvalue.empty() || !helpers::is_qvalue(qvalue)) return false;
      bytes_used = i + qvalue.size();
      q_found = true;
      return true;
    }
    // Regular parameter value: token or quoted-string, no whitespace around '='.
    return helpers::consume_parameter(sv, bytes_used, /*allow_bws=*/false);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
