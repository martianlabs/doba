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

#ifndef martianlabs_doba_protocol_http11_headers_content_location_h
#define martianlabs_doba_protocol_http11_headers_content_location_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                          content-location |
// +===========================================================================+
// | RFC 9110 �8.7 Content-Location                                            |
// +---------------------------------------------------------------------------+
// | The "Content-Location" header field references a URI that identifies a    |
// | resource corresponding to the representation enclosed in the message.     |
// |                                                                           |
// | The identified resource is not necessarily the same as the target         |
// | resource. Content-Location is representation metadata and does not        |
// | replace the target URI or alter the semantics of the request or response. |
// |                                                                           |
// | When Content-Location is included in a 2xx response and its resolved URI  |
// | is identical to the target URI, the recipient MAY consider the enclosed   |
// | content to be a current representation of that resource at the message    |
// | origination time.                                                         |
// |                                                                           |
// | When its resolved URI differs from the target URI, the origin server      |
// | claims that the URI identifies another resource corresponding to the      |
// | enclosed representation, such as a more specific negotiated variant.      |
// |                                                                           |
// | In a 201 (Created) response, a Content-Location value identical to the    |
// | Location value indicates that the enclosed content is a current           |
// | representation of the newly created resource.                             |
// |                                                                           |
// | In a request, Content-Location indicates where the user agent originally  |
// | obtained the enclosed representation. An origin server MUST treat this as |
// | transitory request context and MUST NOT use it to alter request           |
// | semantics.                                                                |
// |                                                                           |
// | A partial URI is resolved relative to the target URI.                     |
// |                                                                           |
// | Examples:                                                                 |
// |   Content-Location: /documents/report.en                                  |
// |   Content-Location: https://example.com/documents/report.en               |
// +---------------------------------------------------------------------------+
// | RFC 9110 �8.7 Content-Location (ABNF summary)                             |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Content-Location | absolute-URI / partial-URI                             |
// | absolute-URI     | scheme ":" hier-part [ "?" query ]                     |
// | partial-URI      | relative-part [ "?" query ]                            |
// | scheme           | ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )             |
// | hier-part        | "//" authority path-abempty / path-absolute /          |
// |                  | path-rootless / path-empty                             |
// | relative-part    | "//" authority path-abempty / path-absolute /          |
// |                  | path-noscheme / path-empty                             |
// | authority        | [ userinfo "@" ] host [ ":" port ]                     |
// | path-abempty     | *( "/" segment )                                       |
// | path-absolute    | "/" [ segment-nz *( "/" segment ) ]                    |
// | path-rootless    | segment-nz *( "/" segment )                            |
// | path-noscheme    | segment-nz-nc *( "/" segment )                         |
// | path-empty       | 0<pchar>                                               |
// | segment          | *pchar                                                 |
// | segment-nz       | 1*pchar                                                |
// | segment-nz-nc    | 1*( unreserved / pct-encoded / sub-delims / "@" )      |
// | query            | *( pchar / "/" / "?" )                                 |
// | pchar            | unreserved / pct-encoded / sub-delims / ":" / "@"      |
// | pct-encoded      | "%" HEXDIG HEXDIG                                      |
// | unreserved       | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
// | sub-delims       | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /        |
// |                  | "," / ";" / "="                                        |
// +---------------------------------------------------------------------------+
// | Content-Location contains one URI value. It is not a list-valued field,   |
// | so commas MUST NOT be interpreted as field-value separators.              |
// |                                                                           |
// | Neither absolute-URI nor partial-URI permits a fragment component. An     |
// | unencoded "#" and any following fragment are therefore invalid.           |
// |                                                                           |
// | Because relative-part permits path-empty, the purely syntactic ABNF       |
// | permits an empty value and query-only values such as "?version=2".        |
// |                                                                           |
// | Percent-encoded sequences MUST contain exactly two hexadecimal digits.    |
// |                                                                           |
// | IMPORTANT: field-value is supposed to be normalized (no OWS around        |
// | value).                                                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class content_location {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Content-Location = absolute-URI / partial-URI. This is the URI-reference
    // grammar without a fragment component, so an unencoded '#' is rejected.
    return helpers::check_uri_reference(sv, /*allow_fragment=*/false);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
