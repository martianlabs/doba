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

#ifndef martianlabs_doba_protocol_http11_headers_if_none_match_h
#define martianlabs_doba_protocol_http11_headers_if_none_match_h

#include <ranges>
#include <string_view>

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                             if-none-match |
// +===========================================================================+
// | RFC 9110 �13.1.2 If-None-Match                                            |
// +---------------------------------------------------------------------------+
// | The "If-None-Match" header field makes a request conditional on a         |
// | recipient cache or origin server either not having any current            |
// | representation of the target resource, when the field value is "*", or    |
// | having a selected representation whose entity tag does not match any      |
// | entity tag in the supplied list.                                          |
// |                                                                           |
// | A recipient MUST use the weak comparison function when comparing entity   |
// | tags for If-None-Match. Weak entity tags are therefore valid in this      |
// | field.                                                                    |
// |                                                                           |
// | If-None-Match is primarily used with conditional GET requests. When one   |
// | of the listed entity tags matches the selected representation, a GET or   |
// | HEAD request receives 304 (Not Modified) instead of the representation    |
// | data.                                                                     |
// |                                                                           |
// | The "*" value can also be used with an unsafe method, such as PUT, to     |
// | prevent the method from modifying an existing representation when the     |
// | client intends to create a new resource only if none currently exists.    |
// |                                                                           |
// | When the condition evaluates to false, the origin server MUST NOT perform |
// | the requested method. It MUST respond with 304 (Not Modified) for GET or  |
// | HEAD, or 412 (Precondition Failed) for every other method.                |
// |                                                                           |
// | The "*" alternative is not an entity-tag and cannot appear as a member of |
// | an entity-tag list. A value combining "*" with other values is            |
// | syntactically invalid.                                                    |
// |                                                                           |
// | Examples:                                                                 |
// |   If-None-Match: "xyzzy"                                                  |
// |   If-None-Match: W/"xyzzy"                                                |
// |   If-None-Match: "xyzzy", "r2d2xxxx", "c3piozzzz"                         |
// |   If-None-Match: W/"xyzzy", W/"r2d2xxxx", W/"c3piozzzz"                   |
// |   If-None-Match: *                                                        |
// +---------------------------------------------------------------------------+
// | RFC 9110 �13.1.2 If-None-Match (ABNF summary)                             |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | If-None-Match    | "*" / #entity-tag                                      |
// | entity-tag       | [ weak ] opaque-tag                                    |
// | weak             | %s"W/"                                                 |
// | opaque-tag       | DQUOTE *etagc DQUOTE                                   |
// | etagc            | %x21 / %x23-7E / obs-text                              |
// |                  | ; VCHAR except DQUOTE, plus obs-text                   |
// | obs-text         | %x80-FF                                                |
// | DQUOTE           | %x22                                                   |
// | OWS              | *( SP / HTAB )                                         |
// +------------------+--------------------------------------------------------+
// | RFC 9110 �5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | The "#entity-tag" alternative expands as follows:                         |
// |                                                                           |
// | Sender syntax:                                                            |
// |                                                                           |
// |   #entity-tag = [ entity-tag *( OWS "," OWS entity-tag ) ]                |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #entity-tag = [ entity-tag ]                                            |
// |                 *( OWS "," OWS [ entity-tag ] )                           |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "#entity-tag", rather than "1#entity-tag", zero         |
// | non-empty entity-tag elements are permitted by the purely syntactic ABNF. |
// | This applies only to the list alternative; "*" remains a separate         |
// | complete field value.                                                     |
// |                                                                           |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is normalized (no OWS around the value).           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class if_none_match {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // If-None-Match = "*" / #entity-tag
    // The "*" alternative matches the whole field value; it cannot be mixed
    // with, or appear as a member of, the entity-tag list.
    if (sv == "*") return true;
    // Otherwise, If-None-Match is a (possibly empty) list of entity tags.
    return helpers::for_each_list_element(sv, helpers::is_entity_tag);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
