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

#ifndef martianlabs_doba_protocol_http11_headers_sec_websocket_accept_h
#define martianlabs_doba_protocol_http11_headers_sec_websocket_accept_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                      sec-websocket-accept |
// +===========================================================================+
// | RFC 6455 �11.3.3 Sec-WebSocket-Accept                                     |
// +---------------------------------------------------------------------------+
// | The "Sec-WebSocket-Accept" header field is used in the WebSocket opening  |
// | handshake. It is sent by the server to prove that it received the         |
// | client's Sec-WebSocket-Key value and accepts the protocol switch.         |
// |                                                                           |
// | The field value is a base64-encoded SHA-1 digest. To construct it, the    |
// | server concatenates the Sec-WebSocket-Key value with the fixed GUID:      |
// |                                                                           |
// |   258EAFA5-E914-47DA-95CA-C5AB0DC85B11                                    |
// |                                                                           |
// | The server then computes SHA-1 over that concatenated string and encodes  |
// | the resulting 160-bit digest using base64.                                |
// |                                                                           |
// | A client MUST validate that the received Sec-WebSocket-Accept value       |
// | exactly matches the value expected from the Sec-WebSocket-Key that it     |
// | sent. If the value is missing or incorrect, the client MUST fail the      |
// | WebSocket connection.                                                     |
// |                                                                           |
// | The Sec-WebSocket-Accept header field MUST NOT appear more than once in   |
// | an HTTP response.                                                         |
// |                                                                           |
// | Example:                                                                  |
// |   Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=                      |
// +---------------------------------------------------------------------------+
// | RFC 6455 �11.3.3 Sec-WebSocket-Accept (ABNF summary)                      |
// +---------------------------------------------------------------------------+
// +-----------------------------+---------------------------------------------+
// | Field                       | Definition                                  |
// +-----------------------------+---------------------------------------------+
// | Sec-WebSocket-Accept        | base64-value-non-empty                      |
// | base64-value-non-empty      | ( 1*base64-data [ base64-padding ] ) /      |
// |                             | base64-padding                              |
// | base64-data                 | 4base64-character                           |
// | base64-padding              | ( 2base64-character "==" ) /                |
// |                             | ( 3base64-character "=" )                   |
// | base64-character            | ALPHA / DIGIT / "+" / "/"                   |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class sec_websocket_accept {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Sec-WebSocket-Accept = base64-value-non-empty. Only the base64-shaped
    // syntax is validated here; matching the digest expected from the client's
    // Sec-WebSocket-Key is a semantic handshake requirement, not part of the
    // ABNF.
    return helpers::check_base64_value(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
