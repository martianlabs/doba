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

#ifndef martianlabs_doba_protocol_http11_headers_x_keep_alive_h
#define martianlabs_doba_protocol_http11_headers_x_keep_alive_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////                                                 
// +===========================================================================+
// |                                                                keep-alive |
// +===========================================================================+
// | RFC 2068 �19.7.1.1 The Keep-Alive Header                                  |
// +---------------------------------------------------------------------------+
// | The "Keep-Alive" header field is a historical HTTP/1.0 extension used     |
// | with explicitly negotiated persistent connections. It can carry optional  |
// | connection persistence parameters when the "Keep-Alive" connection token  |
// | has been transmitted in a request or response.                            |
// |                                                                           |
// | The corresponding connection token is carried by the Connection header:   |
// |                                                                           |
// |   Connection: Keep-Alive                                                  |
// |                                                                           |
// | When present, Keep-Alive parameters describe advisory properties of the   |
// | persistent connection, such as an idle timeout or a maximum number of     |
// | requests. HTTP/1.1 does not define any standard Keep-Alive parameters.    |
// |                                                                           |
// | A Keep-Alive header field MUST be ignored if received without the         |
// | corresponding Keep-Alive connection token.                                |
// |                                                                           |
// | HTTP/1.1 uses persistent connections by default. The Keep-Alive mechanism |
// | is therefore only relevant for compatibility with older HTTP/1.0-style    |
// | persistent connection negotiation.                                        |
// |                                                                           |
// | Examples:                                                                 |
// |   Keep-Alive: timeout=5                                                   |
// |   Keep-Alive: timeout=5, max=1000                                         |
// +---------------------------------------------------------------------------+
// | RFC 2068 �19.7.1.1 Keep-Alive Header (ABNF summary)                       |
// +---------------------------------------------------------------------------+
// +-----------------+---------------------------------------------------------+
// | Field           | Definition                                              |
// +-----------------+---------------------------------------------------------+
// | Keep-Alive      | #keepalive-param                                        |
// | keepalive-param | parameter-name BWS "=" BWS parameter-value              |
// | parameter-name  | token                                                   |
// | parameter-value | token / quoted-string                                   |
// | token           | 1*tchar                                                 |
// | tchar           | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /   |
// |                 | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA       |
// | quoted-string   | DQUOTE *( qdtext / quoted-pair ) DQUOTE                 |
// | qdtext          | HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text         |
// | quoted-pair     | "\" ( HTAB / SP / VCHAR / obs-text )                    |
// | obs-text        | %x80-FF                                                 |
// | BWS             | OWS                                                     |
// | OWS             | *( SP / HTAB )                                          |
// +---------------------------------------------------------------------------+
// | RFC 9110 �5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #keepalive-param = [ keepalive-param                                    |
// |                        *( OWS "," OWS keepalive-param ) ]                 |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #keepalive-param = [ keepalive-param ]                                  |
// |                      *( OWS "," OWS [ keepalive-param ] )                 |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "#keepalive-param", rather than "1#keepalive-param",    |
// | zero non-empty keepalive-param elements are permitted by the purely       |
// | syntactic ABNF.                                                           |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class x_keep_alive {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Keep-Alive = #keepalive-param, where keepalive-param = parameter-name BWS
    // "=" BWS parameter-value. This is a plain comma-separated list of name/
    // value pairs whose element grammar (token BWS "=" BWS ( token /
    // quoted-string )) is identical to the #auth-param list, so validation is
    // delegated to the common helper.
    return helpers::check_auth_param_list(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
