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

#ifndef martianlabs_doba_protocol_http11_headers_sec_websocket_extensions_h
#define martianlabs_doba_protocol_http11_headers_sec_websocket_extensions_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                 sec-websocket-extensions  |
// +===========================================================================+
// | RFC 6455 �9.1 Sec-WebSocket-Extensions                                    |
// +---------------------------------------------------------------------------+
// | The "Sec-WebSocket-Extensions" header field is used during the WebSocket  |
// | opening handshake to negotiate protocol-level extensions.                 |
// |                                                                           |
// | A client MAY send this field in its opening handshake request to indicate |
// | the extensions it wishes to use. The field value is a comma-separated     |
// | list of extension offers. Each extension offer consists of an extension   |
// | token followed by zero or more semicolon-separated extension parameters.  |
// |                                                                           |
// | A server MAY send this field in its opening handshake response to indicate|
// | the extensions it has accepted. The server MUST NOT include an extension  |
// | unless it was offered by the client. If no extensions are accepted, the   |
// | server MUST omit this field.                                              |
// |                                                                           |
// | The order of extensions is significant. Extensions listed earlier are     |
// | applied before extensions listed later. Parameters are specific to the    |
// | extension token with which they appear.                                   |
// |                                                                           |
// | Extension parameters use HTTP token syntax. A parameter MAY have no value,|
// | or it MAY have a value expressed either as a token or as a quoted-string. |
// | When quoted-string syntax is used for a parameter value, the unescaped    |
// | value is expected to conform to the token syntax defined by HTTP.         |
// |                                                                           |
// | Examples:                                                                 |
// |   Sec-WebSocket-Extensions: permessage-deflate                            |
// |   Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits    |
// |   Sec-WebSocket-Extensions: foo; bar=baz; qux="quux"                      |
// |   Sec-WebSocket-Extensions: foo, bar; baz=qux                             |
// +---------------------------------------------------------------------------+
// | RFC 6455 �9.1 Sec-WebSocket-Extensions (ABNF summary)                     |
// +---------------------------------------------------------------------------+
// +--------------------------+------------------------------------------------+
// | Field                    | Definition                                     |
// +--------------------------+------------------------------------------------+
// | Sec-WebSocket-Extensions | extension-list                                 |
// | extension-list           | 1#extension                                    |
// | extension                | extension-token *( ";" extension-param )       |
// | extension-token          | registered-token                               |
// | registered-token         | token                                          |
// | extension-param          | token [ "=" ( token / quoted-string ) ]        |
// | token                    | 1*tchar                                        |
// | quoted-string            | DQUOTE *( qdtext / quoted-pair ) DQUOTE        |
// | quoted-pair              | "\" ( HTAB / SP / VCHAR / obs-text )           |
// | qdtext                   | HTAB / SP / "!" / %x23-5B / %x5D-7E / obs-text |
// | tchar                    | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+"  |
// |                          | / "-" / "." / "^" / "_" / "`" / "|" / "~"      |
// |                          | / DIGIT / ALPHA                                |
// | OWS                      | *( SP / HTAB )                                 |
// +---------------------------------------------------------------------------+
// | RFC 9110 �5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   1#extension = extension *( OWS "," OWS extension )                      |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   1#extension = [ extension ] *( OWS "," OWS [ extension ] )              |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "1#extension", at least one non-empty extension element |
// | is required by the sender-side ABNF.                                      |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class sec_websocket_extensions {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    bool at_least_one_extension = false;
    const bool result = helpers::for_each_list_element(
        sv, [&at_least_one_extension](std::string_view element) {
          if (!consume_extension(element)) return false;
          at_least_one_extension = true;
          return true;
        });
    // 1#extension requires at least one non-empty extension.
    return result && at_least_one_extension;
  }

 private:
  // +=========================================================================+
  // | [>] consume_extension                                       ( private ) |
  // +=========================================================================+
  // | extension = extension-token *( ";" extension-param )                    |
  // | extension-token = token                                                 |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_extension(std::string_view sv) {
    // A non-empty extension-token (a plain token) starts every extension.
    const std::string_view token = helpers::consume_token(sv);
    if (token.empty()) return false;
    const std::size_t off = token.size();
    if (off >= sv.size()) return true;
    // *( ";" extension-param ) � a ";" is mandatory before each parameter and
    // empty parameter slots are tolerated, matching parameters syntax. Each
    // extension-param is token [ "=" ( token / quoted-string ) ].
    return helpers::for_each_parameter(
        sv.substr(off), /*require_parameter=*/false,
        [](std::string_view rest, std::size_t& bytes) {
          return helpers::consume_extension_parameter(rest, bytes);
        });
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
