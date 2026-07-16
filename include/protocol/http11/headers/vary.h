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

#ifndef martianlabs_doba_protocol_http11_headers_vary_h
#define martianlabs_doba_protocol_http11_headers_vary_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                      vary |
// +===========================================================================+
// | RFC 9110 �12.5.5 Vary                                                     |
// +---------------------------------------------------------------------------+
// | The "Vary" response header field describes which parts of the request,    |
// | aside from the method and target URI, might have influenced the origin    |
// | server's process for selecting the response content.                      |
// |                                                                           |
// | A Vary field value consists of either the wildcard member "*" or a list   |
// | of request field names, known as selecting header fields, that might have |
// | influenced representation selection. Selecting fields are not limited to  |
// | those defined by RFC 9110.                                                |
// |                                                                           |
// | A list containing "*" indicates that aspects of the request other than    |
// | explicitly named fields might have influenced selection, including        |
// | information outside the HTTP message syntax, such as the client's network |
// | address. A recipient cannot determine whether such a response is suitable |
// | for a later request without consulting the origin server.                 |
// |                                                                           |
// | A proxy MUST NOT generate "*" in a Vary field value.                      |
// |                                                                           |
// | A list of field names informs caches that the response MUST NOT be reused |
// | for a later request unless the listed request fields match those of the   |
// | original request, or the response has been validated by the origin        |
// | server. Vary therefore expands the cache key used to select a stored      |
// | response.                                                                 |
// |                                                                           |
// | It also informs user agents that the response was subject to content      |
// | negotiation and that different values for the listed fields might select  |
// | a different representation in a subsequent request.                       |
// |                                                                           |
// | An origin server SHOULD generate Vary on a cacheable response when it     |
// | wants that response to be reused selectively according to request fields. |
// | Vary can be omitted when the performance cost imposed on caching is more  |
// | significant than the variation in the selected content.                   |
// |                                                                           |
// | The Authorization field name does not need to be listed because its own   |
// | definition already restricts reuse of a response for another user.        |
// |                                                                           |
// | Examples:                                                                 |
// |   Vary: *                                                                 |
// |   Vary: Accept-Encoding                                                   |
// |   Vary: Accept-Encoding, Accept-Language                                  |
// +---------------------------------------------------------------------------+
// | RFC 9110 �12.5.5 Vary (ABNF summary)                                      |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Vary             | #( "*" / field-name )                                  |
// | field-name       | token                                                  |
// | token            | 1*tchar                                                |
// | tchar            | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /  |
// |                  | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA      |
// | OWS              | *( SP / HTAB )                                         |
// +---------------------------------------------------------------------------+
// | RFC 9110 �5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   Vary = [ ( "*" / field-name )                                           |
// |            *( OWS "," OWS ( "*" / field-name ) ) ]                        |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   Vary = [ ( "*" / field-name ) ]                                         |
// |          *( OWS "," OWS [ ( "*" / field-name ) ] )                        |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule uses "#", rather than "1#", zero non-empty members are     |
// | permitted by the purely syntactic ABNF. Therefore, an empty normalized    |
// | field value is syntactically valid.                                       |
// |                                                                           |
// | Although "*" is itself syntactically a token, it has special semantics    |
// | when it occurs as a member of Vary. Any list containing "*" has wildcard  |
// | semantics, regardless of the presence of other field-name members.        |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class vary {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Vary = #( "*" / field-name )
    // field-name = token
    //
    // "*" is itself a valid token because "*" belongs to tchar. Therefore,
    // both alternatives can be validated uniformly as token.
    //
    // for_each_list_element is expected to:
    // - accept an empty field-value;
    // - ignore empty list elements;
    // - remove OWS surrounding list separators;
    // - invoke consume_member only for non-empty elements.
    return helpers::for_each_list_element(sv, consume_member);
  }

 private:
  // +=========================================================================+
  // | [>] consume_member                                          ( private ) |
  // +=========================================================================+
  static constexpr bool consume_member(std::string_view sv) {
    // field-name = token
    //
    // consume_token returns the longest valid token prefix. The complete
    // member is valid only when the consumed token spans the entire element.
    const std::string_view token = helpers::consume_token(sv);
    return !token.empty() && token.size() == sv.size();
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
