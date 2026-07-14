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

#ifndef martianlabs_doba_protocol_http11_headers_cache_control_h
#define martianlabs_doba_protocol_http11_headers_cache_control_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                             cache-control |
// +===========================================================================+
// | RFC 9111 �5.2 Cache-Control                                               |
// +---------------------------------------------------------------------------+
// | The "Cache-Control" header field carries a comma-separated list of cache  |
// | directives along a request/response chain.                                |
// |                                                                           |
// | Cache directives are unidirectional: a directive received in a request    |
// | does not imply that the same directive is present in, or copied to, the   |
// | corresponding response.                                                   |
// |                                                                           |
// | A proxy, whether or not it implements a cache, MUST forward cache         |
// | directives in forwarded messages because they can apply to every          |
// | recipient along the chain. A directive cannot be targeted at one specific |
// | cache.                                                                    |
// |                                                                           |
// | Directive names are tokens and are compared case-insensitively. A         |
// | directive can have an optional argument represented as either a token or  |
// | a quoted-string.                                                          |
// |                                                                           |
// | For directives defined by RFC 9111, an argument is not allowed unless the |
// | directive definition explicitly provides one. These directive-specific    |
// | restrictions are semantic constraints layered on top of the generic field |
// | grammar.                                                                  |
// |                                                                           |
// | Request directives are advisory: caches MAY implement them. Caches MUST   |
// | obey the response directives defined by RFC 9111.                         |
// |                                                                           |
// | The field is extensible. A cache MUST ignore an unrecognized cache        |
// | directive, while still parsing and forwarding it according to the generic |
// | syntax.                                                                   |
// |                                                                           |
// | Standard request directives:                                              |
// |   max-age, max-stale, min-fresh, no-cache, no-store, no-transform,        |
// |   only-if-cached                                                          |
// |                                                                           |
// | Standard response directives:                                             |
// |   max-age, must-revalidate, must-understand, no-cache, no-store,          |
// |   no-transform, private, proxy-revalidate, public, s-maxage               |
// |                                                                           |
// | Examples:                                                                 |
// |   Cache-Control: no-store                                                 |
// |   Cache-Control: max-age=604800                                           |
// |   Cache-Control: private, max-age=0, must-revalidate                      |
// |   Cache-Control: no-cache="Set-Cookie, Authorization"                     |
// +---------------------------------------------------------------------------+
// | RFC 9111 �5.2 Cache-Control (ABNF summary)                                |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Cache-Control    | #cache-directive                                       |
// | cache-directive  | token [ "=" ( token / quoted-string ) ]                |
// | token            | 1*tchar                                                |
// | tchar            | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /  |
// |                  | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA      |
// | quoted-string    | DQUOTE *( qdtext / quoted-pair ) DQUOTE                |
// | qdtext           | HTAB / SP / %x21 / %x23-5B / %x5D-7E /                 |
// |                  | obs-text                                               |
// | quoted-pair      | "\" ( HTAB / SP / VCHAR / obs-text )                   |
// | obs-text         | %x80-FF                                                |
// | VCHAR            | %x21-7E                                                |
// | OWS              | *( SP / HTAB )                                         |
// +---------------------------------------------------------------------------+
// | RFC 9110 �5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #cache-directive =                                                      |
// |     [ cache-directive *( OWS "," OWS cache-directive ) ]                  |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #cache-directive =                                                      |
// |     [ cache-directive ]                                                   |
// |     *( OWS "," OWS [ cache-directive ] )                                  |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Because the field uses "#cache-directive", rather than                    |
// | "1#cache-directive", zero non-empty directives are permitted by the       |
// | purely syntactic ABNF. Therefore, an empty normalized field-value is      |
// | syntactically valid.                                                      |
// |                                                                           |
// | OWS is allowed around each list comma, but not around the "=" inside a    |
// | cache-directive. Thus "max-age = 60" is not valid generic ABNF.           |
// |                                                                           |
// | A comma inside a quoted-string belongs to the directive argument and is   |
// | not a list separator.                                                     |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized                       |
// | (no OWS around value).                                                    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class cache_control {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    return helpers::for_each_list_element(sv, consume_cache_directive);
  }

 private:
  // +=========================================================================+
  // | [>] consume_cache_directive                                 ( private ) |
  // +=========================================================================+
  static constexpr bool consume_cache_directive(std::string_view sv) {
    // cache-directive = token [ "=" ( token / quoted-string ) ]
    return helpers::is_directive(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
