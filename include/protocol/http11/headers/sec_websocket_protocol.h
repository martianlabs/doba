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

#ifndef martianlabs_doba_protocol_http11_headers_sec_websocket_protocol_h
#define martianlabs_doba_protocol_http11_headers_sec_websocket_protocol_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                    sec-websocket-protocol |
// +===========================================================================+
// | RFC 6455 �11.3.4 Sec-WebSocket-Protocol                                   |
// +---------------------------------------------------------------------------+
// | The "Sec-WebSocket-Protocol" header field is used in the WebSocket        |
// | opening handshake to negotiate the application-level subprotocol layered  |
// | over the WebSocket connection.                                            |
// |                                                                           |
// | A client MAY send this field in its opening handshake to indicate one or  |
// | more subprotocols that are acceptable to the client, listed in preference |
// | order. Each listed subprotocol name is a token.                           |
// |                                                                           |
// | A server MAY send this field in its handshake response to confirm the     |
// | selected subprotocol. The value sent by the server MUST be one of the     |
// | values received from the client's Sec-WebSocket-Protocol field.           |
// |                                                                           |
// | If the server does not agree to any of the client's requested             |
// | subprotocols, or if the client did not send this field, the server MUST   |
// | NOT send Sec-WebSocket-Protocol in its response.                          |
// |                                                                           |
// | The empty string is not equivalent to the null value. Absence of the      |
// | header means that no subprotocol was selected; an empty field-value is    |
// | not a valid subprotocol token.                                            |
// |                                                                           |
// | The Sec-WebSocket-Protocol header field MAY appear multiple times in an   |
// | HTTP request, which is logically equivalent to a single field containing  |
// | all values. It MUST NOT appear more than once in an HTTP response.        |
// |                                                                           |
// | Examples:                                                                 |
// |   Sec-WebSocket-Protocol: chat, superchat                                 |
// |   Sec-WebSocket-Protocol: wamp                                            |
// |   Sec-WebSocket-Protocol: mqtt                                            |
// +---------------------------------------------------------------------------+
// | RFC 6455 �4.3 / �11.3.4 Sec-WebSocket-Protocol (ABNF summary)             |
// +---------------------------------------------------------------------------+
// +-------------------------------+-------------------------------------------+
// | Field                         | Definition                                |
// +-------------------------------+-------------------------------------------+
// | Sec-WebSocket-Protocol-Client | 1#token                                   |
// | Sec-WebSocket-Protocol-Server | token                                     |
// | token                         | 1*tchar                                   |
// | tchar                         | "!" / "#" / "$" / "%" / "&" / "'" /       |
// |                               | "*" / "+" / "-" / "." / "^" / "_" /       |
// |                               | "`" / "|" / "~" / DIGIT / ALPHA           |
// | OWS                           | *( SP / HTAB )                            |
// +---------------------------------------------------------------------------+
// | RFC 9110 �5.6.1 list expansion for client syntax                          |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   1#token = token *( OWS "," OWS token )                                  |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   1#token = *( OWS "," OWS ) token *( OWS "," OWS [ token ] )             |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the client rule is "1#token", at least one non-empty token is       |
// | required by the purely syntactic ABNF.                                    |
// |                                                                           |
// | The server rule is not a list rule: Sec-WebSocket-Protocol-Server is      |
// | exactly one token, with no comma-separated alternatives.                  |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class sec_websocket_protocol {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // In a request this is the client form Sec-WebSocket-Protocol-Client =
    // 1#token. After HTTP field-line combination, repeated fields fold into a
    // single comma-separated list, so the value is validated as a 1#token list
    // (which also subsumes the single-token server form).
    bool at_least_one_token = false;
    const bool result = helpers::for_each_list_element(
        sv, [&at_least_one_token](std::string_view element) {
          if (!consume_token(element)) return false;
          at_least_one_token = true;
          return true;
        });
    // 1#token requires at least one non-empty token.
    return result && at_least_one_token;
  }

 private:
  // +=========================================================================+
  // | [>] consume_token                                           ( private ) |
  // +=========================================================================+
  static constexpr bool consume_token(std::string_view sv) {
    return helpers::is_token(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
