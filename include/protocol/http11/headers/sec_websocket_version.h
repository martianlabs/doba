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

#ifndef martianlabs_doba_protocol_http11_headers_sec_websocket_version_h
#define martianlabs_doba_protocol_http11_headers_sec_websocket_version_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                     sec-websocket-version |
// +===========================================================================+
// | RFC 6455 �11.3.5 Sec-WebSocket-Version                                    |
// +---------------------------------------------------------------------------+
// | The "Sec-WebSocket-Version" header field is used in the WebSocket opening |
// | handshake to indicate the protocol version used by the client.            |
// |                                                                           |
// | A client MUST include this header field in its opening handshake. For RFC |
// | 6455, the value sent by the client MUST be "13".                          |
// |                                                                           |
// | If the server does not understand or does not support the version sent by |
// | the client, it MUST abort the WebSocket opening handshake and send an HTTP|
// | response with status code 426 (Upgrade Required). That response MUST      |
// | include a Sec-WebSocket-Version header field indicating the version, or   |
// | versions, that the server is capable of understanding.                    |
// |                                                                           |
// | The header field can appear multiple times in a server response,          |
// | or contain multiple version values separated by commas.                   |
// | Values can be repeated.                                                   |
// |                                                                           |
// | The version value is a decimal integer in the range 0 through 255, without|
// | leading zeroes except for the single value "0".                           |
// |                                                                           |
// | Examples:                                                                 |
// |   Sec-WebSocket-Version: 13                                               |
// |   Sec-WebSocket-Version: 13, 8, 7                                         |
// |   Sec-WebSocket-Version: 13                                               |
// |   Sec-WebSocket-Version: 8, 7                                             |
// +---------------------------------------------------------------------------+
// | RFC 6455 �11.3.5 Sec-WebSocket-Version (ABNF summary)                     |
// +---------------------------------------------------------------------------+
// +-----------------------+---------------------------------------------------+
// | Field                 | Definition                                        |
// +-----------------------+---------------------------------------------------+
// | Sec-WebSocket-Version | version                                           |
// | version               | DIGIT / ( NZDIGIT DIGIT ) /                       |
// |                       | ( "1" DIGIT DIGIT ) / ( "2" DIGIT DIGIT )         |
// | NZDIGIT               | "1" / "2" / "3" / "4" / "5" / "6" /               |
// |                       | "7" / "8" / "9"                                   |
// | DIGIT                 | "0" / "1" / "2" / "3" / "4" / "5" /               |
// |                       | "6" / "7" / "8" / "9"                             |
// +---------------------------------------------------------------------------+
// | Numeric range constraint                                                  |
// +---------------------------------------------------------------------------+
// | The ABNF shape admits 0..299 syntactically, but RFC 6455 constrains       |
// | version to the range 0..255 and forbids leading zeroes. Therefore, values |
// | such as "256", "299", "013", and "00" are invalid.                        |
// |                                                                           |
// | In RFC 6455-compliant client handshakes, the only valid protocol version  |
// | to send is "13".                                                          |
// +---------------------------------------------------------------------------+
// | HTTP list handling                                                        |
// +---------------------------------------------------------------------------+
// | RFC 6455 permits a server response to carry multiple supported versions   |
// | either as repeated Sec-WebSocket-Version field lines or as comma-separated|
// | values in one field line. After HTTP field-line combination, the effective|
// | field value can therefore be parsed as a comma-separated list of version  |
// | values.                                                                   |
// |                                                                           |
// | Sender syntax:                                                            |
// |                                                                           |
// |   #version = [ version *( OWS "," OWS version ) ]                         |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #version = [ version ] *( OWS "," OWS [ version ] )                     |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class sec_websocket_version {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    return helpers::for_each_list_element(sv, consume_version);
  }

 private:
  // +=========================================================================+
  // | [>] consume_version                                         ( private ) |
  // +=========================================================================+
  // | version = a decimal integer in 0..255 with no leading zeroes (except    |
  // | "0"). Although the raw ABNF shape admits 0..299, RFC 6455 constrains it |
  // | to 0..255, which is exactly a canonical RFC 3986 dec-octet.             |
  // +=========================================================================+
  static constexpr bool consume_version(std::string_view sv) {
    return helpers::is_dec_octet(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
