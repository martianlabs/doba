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

#ifndef martianlabs_doba_protocol_http11_headers_location_h
#define martianlabs_doba_protocol_http11_headers_location_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                  location |
// +===========================================================================+
// | RFC 9110 �10.2.2 Location                                                 |
// +---------------------------------------------------------------------------+
// | The "Location" header field is used in some responses to refer to a       |
// | specific resource in relation to the response. Its field value consists   |
// | of a single URI-reference.                                                |
// |                                                                           |
// | In a 201 (Created) response, Location refers to the primary resource      |
// | created by the request.                                                   |
// |                                                                           |
// | In a 3xx (Redirection) response, Location indicates the preferred target  |
// | URI for automatically redirecting the request.                            |
// |                                                                           |
// | A Location field value can be absolute or relative. If it is relative,    |
// | the final value is computed by resolving it against the target URI of the |
// | request.                                                                  |
// |                                                                           |
// | For 3xx responses, if the Location value does not include a fragment      |
// | component, a user agent MUST process the redirection as if the value      |
// | inherits the fragment component of the original target URI, when such a   |
// | fragment exists.                                                          |
// |                                                                           |
// | Location is not a list-valued field. A comma is data when it appears as   |
// | part of a valid URI-reference; it is not a field-value separator.         |
// |                                                                           |
// | Examples:                                                                 |
// |   Location: https://www.example.com/people/alice                          |
// |   Location: /new-resource                                                 |
// |   Location: ../archive/item.html                                          |
// |   Location: /People.html#tim                                              |
// +---------------------------------------------------------------------------+
// | RFC 9110 �10.2.2 Location (ABNF summary)                                  |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Location         | URI-reference                                          |
// | URI-reference    | URI / relative-ref                                     |
// | URI              | scheme ":" hier-part [ "?" query ] [ "#" fragment ]    |
// | relative-ref     | relative-part [ "?" query ] [ "#" fragment ]           |
// | scheme           | ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )             |
// | hier-part        | "//" authority path-abempty / path-absolute /          |
// |                  | path-rootless / path-empty                             |
// | relative-part    | "//" authority path-abempty / path-absolute /          |
// |                  | path-noscheme / path-empty                             |
// | authority        | [ userinfo "@" ] host [ ":" port ]                     |
// | userinfo         | *( unreserved / pct-encoded / sub-delims / ":" )       |
// | host             | IP-literal / IPv4address / reg-name                    |
// | port             | *DIGIT                                                 |
// | path-abempty     | *( "/" segment )                                       |
// | path-absolute    | "/" [ segment-nz *( "/" segment ) ]                    |
// | path-rootless    | segment-nz *( "/" segment )                            |
// | path-noscheme    | segment-nz-nc *( "/" segment )                         |
// | path-empty       | 0<pchar>                                               |
// | segment          | *pchar                                                 |
// | segment-nz       | 1*pchar                                                |
// | segment-nz-nc    | 1*( unreserved / pct-encoded / sub-delims / "@" )      |
// | query            | *( pchar / "/" / "?" )                                 |
// | fragment         | *( pchar / "/" / "?" )                                 |
// | pchar            | unreserved / pct-encoded / sub-delims / ":" / "@"      |
// | pct-encoded      | "%" HEXDIG HEXDIG                                      |
// | unreserved       | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
// | sub-delims       | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," /  |
// |                  | ";" / "="                                              |
// | OWS              | *( SP / HTAB )                                         |
// +---------------------------------------------------------------------------+
// | RFC 3986 URI-reference notes                                              |
// +---------------------------------------------------------------------------+
// | Location accepts both absolute URI references and relative references.    |
// | Therefore, all of the following are syntactically possible field values:  |
// |                                                                           |
// |   https://example.com/a/b                                                 |
// |   /a/b                                                                    |
// |   a/b                                                                     |
// |   ../a/b                                                                  |
// |   ?q=1                                                                    |
// |   #section                                                                |
// |                                                                           |
// | An empty field value is syntactically permitted by URI-reference because  |
// | relative-ref can derive path-empty with no query and no fragment.         |
// | Whether an empty Location is meaningful is a semantic question, not an    |
// | ABNF question.                                                            |
// |                                                                           |
// | The URI-reference grammar allows userinfo in authority because it is part |
// | of the generic URI syntax. Any policy that rejects userinfo in Location   |
// | is stricter than the generic ABNF and should be treated as semantic or    |
// | security policy rather than as the base field grammar.                    |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class location {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Location = URI-reference. This is the full URI-reference grammar, which
    // (unlike Content-Location) permits an optional "#" fragment component.
    return helpers::check_uri_reference(sv, /*allow_fragment=*/true);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
