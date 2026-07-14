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

#ifndef martianlabs_doba_protocol_http11_headers_authorization_h
#define martianlabs_doba_protocol_http11_headers_authorization_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                             authorization |
// +===========================================================================+
// | RFC 9110 �11.6.2 Authorization                                            |
// +---------------------------------------------------------------------------+
// | The "Authorization" header field allows a user agent to authenticate      |
// | itself with an origin server, usually after receiving a 401               |
// | (Unauthorized) response containing one or more WWW-Authenticate           |
// | challenges.                                                               |
// |                                                                           |
// | A user agent MAY send Authorization preemptively, without first receiving |
// | a challenge, when it already has suitable credentials for the requested   |
// | resource.                                                                 |
// |                                                                           |
// | The field value consists of credentials. Credentials start with an        |
// | authentication scheme name, followed optionally by scheme-specific        |
// | information. The scheme-specific part can be either token68 data or a     |
// | comma-separated list of authentication parameters.                        |
// |                                                                           |
// | The authentication scheme name is a token and is matched                  |
// | case-insensitively. The syntax and semantics of the credentials after the |
// | scheme are defined by the selected authentication scheme.                 |
// |                                                                           |
// | Authorization applies to authentication with the origin server.           |
// | Authentication with an intermediary proxy uses Proxy-Authorization        |
// | instead.                                                                  |
// |                                                                           |
// | A request containing Authorization can affect cache handling. Shared      |
// | caches normally cannot use a cached response to such a request unless     |
// | the response explicitly permits reuse according to HTTP caching rules.    |
// |                                                                           |
// | Examples:                                                                 |
// |   Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==                       |
// |   Authorization: Bearer mF_9.B5f-4.1JqM                                   |
// |   Authorization: Digest username="Mufasa", realm="testrealm@host.com",    |
// |                  nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093",              |
// |                  uri="/dir/index.html", response="e966c9329242554e42c8"   |
// +---------------------------------------------------------------------------+
// | RFC 9110 �11.6.2 Authorization (ABNF summary)                             |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Authorization    | credentials                                            |
// | credentials      | auth-scheme [ 1*SP ( token68 / #auth-param ) ]         |
// | auth-scheme      | token                                                  |
// | auth-param       | token BWS "=" BWS ( token / quoted-string )            |
// | token68          | 1*( ALPHA / DIGIT / "-" / "." / "_" / "~" / "+" /      |
// |                  | "/" ) *"="                                             |
// | token            | 1*tchar                                                |
// | tchar            | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /  |
// |                  | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA      |
// | quoted-string    | DQUOTE *( qdtext / quoted-pair ) DQUOTE                |
// | qdtext           | HTAB / SP / "!" / %x23-5B / %x5D-7E / obs-text         |
// | quoted-pair      | "\" ( HTAB / SP / VCHAR / obs-text )                   |
// | BWS              | OWS                                                    |
// | OWS              | *( SP / HTAB )                                         |
// +---------------------------------------------------------------------------+
// | RFC 9110 �5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #auth-param = [ auth-param *( OWS "," OWS auth-param ) ]                |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #auth-param = [ auth-param ] *( OWS "," OWS [ auth-param ] )            |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "#auth-param", rather than "1#auth-param", zero         |
// | non-empty auth-param elements are permitted by the purely syntactic ABNF. |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class authorization {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Authorization = credentials. The credentials grammar (auth-scheme with
    // an optional token68 or #auth-param argument) is shared with Proxy-
    // Authorization, so validation is delegated to the common helper.
    return helpers::check_credentials(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
