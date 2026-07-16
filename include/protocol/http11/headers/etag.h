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

#ifndef martianlabs_doba_protocol_http11_headers_etag_h
#define martianlabs_doba_protocol_http11_headers_etag_h

#include <ranges>
#include <string_view>

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                      etag |
// +===========================================================================+
// | RFC 9110 �8.8.3 ETag                                                      |
// +---------------------------------------------------------------------------+
// | The "ETag" response header field provides the current entity tag for the  |
// | selected representation, as determined at the conclusion of handling the  |
// | request.                                                                  |
// |                                                                           |
// | An entity tag is an opaque validator used to distinguish representations  |
// | of the same resource, whether they differ because the resource changed    |
// | over time, because content negotiation selected different                 |
// | representations, or both.                                                 |
// |                                                                           |
// | An entity tag consists of an opaque quoted sequence, optionally prefixed  |
// | by a weakness indicator. A tag is strong by default. If it does not       |
// | satisfy the requirements of a strong validator, the origin server MUST    |
// | prefix it with "W/"; that prefix is case-sensitive.                       |
// |                                                                           |
// | A sender MAY place ETag in a trailer section, although sending it as a    |
// | header field is preferable because trailer fields are often ignored.      |
// |                                                                           |
// | Strong comparison requires both entity tags to be strong and their opaque |
// | tags to match character-by-character. Weak comparison only requires their |
// | opaque tags to match, regardless of either weakness indicator.            |
// |                                                                           |
// | Examples:                                                                 |
// |   ETag: "xyzzy"                                                           |
// |   ETag: W/"xyzzy"                                                         |
// |   ETag: ""                                                                |
// +---------------------------------------------------------------------------+
// | RFC 9110 �8.8.3 ETag (ABNF summary)                                       |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | ETag             | entity-tag                                             |
// | entity-tag       | [ weak ] opaque-tag                                    |
// | weak             | %s"W/"                                                 |
// | opaque-tag       | DQUOTE *etagc DQUOTE                                   |
// | etagc            | %x21 / %x23-7E / obs-text                              |
// | DQUOTE           | %x22                                                   |
// | obs-text         | %x80-FF                                                |
// +---------------------------------------------------------------------------+
// | The weakness indicator is exactly uppercase "W/" because %s denotes a     |
// | case-sensitive string.                                                    |
// |                                                                           |
// | The opaque-tag is not a quoted-string. A backslash has no escaping        |
// | function and is simply an allowed etagc octet. For compatibility with     |
// | legacy recipients that might unescape it, servers ought to avoid          |
// | backslashes.                                                              |
// |                                                                           |
// | An empty opaque-tag is valid because the grammar uses *etagc:             |
// |                                                                           |
// |   ETag: ""                                                                |
// |                                                                           |
// | SP, HTAB, DQUOTE, DEL (%x7F), and control characters are not valid etagc. |
// |                                                                           |
// | ETag contains exactly one entity-tag; it is not a list-valued field. A    |
// | comma inside the quoted opaque-tag is an ordinary etagc character, not a  |
// | list separator.                                                           |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around        |
// | value).                                                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class etag {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    return helpers::is_entity_tag(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
