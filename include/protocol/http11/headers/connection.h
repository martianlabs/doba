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

#ifndef martianlabs_doba_protocol_http11_headers_connection_h
#define martianlabs_doba_protocol_http11_headers_connection_h

#include "protocol/http11/connection.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] connection                                                  ( class ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 §7.6.1 Connection                                                |
// +---------------------------------------------------------------------------+
// | The "Connection" header field allows the sender to list control options   |
// | that are specific to the current connection.                              |
// |                                                                           |
// | Connection options are case-insensitive.                                  |
// |                                                                           |
// | An intermediary MUST process the Connection field before forwarding the   |
// | message. It MUST remove every header or trailer field named by a          |
// | connection option and then remove or replace the Connection field itself. |
// |                                                                           |
// | A sender MUST NOT include a connection option corresponding to a field    |
// | intended for all recipients in the request or response chain.             |
// |                                                                           |
// | The "close" connection option indicates that the sender will close the    |
// | connection after completion of the current request-response exchange.     |
// |                                                                           |
// | Examples:                                                                 |
// |  Connection: close                                                        |
// |  Connection: keep-alive, upgrade                                          |
// +---------------------------------------------------------------------------+
// | RFC 9110 §7.6.1 and §§5.6.1-5.6.3 (ABNF summary)                          |
// +---------------------------------------------------------------------------+
// +-------------------+-------------------------------------------------------+
// | Field             | Definition                                            |
// +-------------------+-------------------------------------------------------+
// | Connection        | #connection-option                                    |
// |                   | equivalent, for recipient parsing, to:                |
// |                   | [ connection-option ]                                 |
// |                   | *( OWS "," OWS [ connection-option ] )                |
// | connection-option | token                                                 |
// | token             | 1*tchar                                               |
// | tchar             | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" /       |
// |                   | "-" / "." / "^" / "_" / "`" / "|" / "~" / DIGIT /     |
// |                   | ALPHA                                                 |
// | OWS               | *( SP / HTAB )                                        |
// | SP                | %x20                                                  |
// | HTAB              | %x09                                                  |
// | DIGIT             | %x30-39                                               |
// | ALPHA             | %x41-5A / %x61-7A                                     |
// +---------------------------------------------------------------------------+
// | The "#" operator defines a comma-separated list.                          |
// |                                                                           |
// | "#connection-option" allows zero or more connection options. A recipient  |
// | MUST parse and ignore a reasonable number of empty list elements.         |
// |                                                                           |
// | Therefore, these forms are valid for recipient-side list parsing:         |
// |  Connection:                                                              |
// |  Connection: ,                                                            |
// |  Connection: close,                                                       |
// |  Connection: , close                                                      |
// |  Connection: close, , upgrade                                             |
// |                                                                           |
// | Semantic processing, including case-insensitive option comparison and     |
// | removal of nominated fields by intermediaries, is separate from ABNF      |
// | validation.                                                               |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class connection {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv, parsed_token_list& out) {
    // The producer overload validates each connection-option exactly as the
    // pure check() does and captures every non-empty element in order.
    return helpers::for_each_list_element(sv, [&out](std::string_view element) {
      if (!consume_connection_option(element)) return false;
      out.elements.push_back(element);
      return true;
    });
  }
  // +=========================================================================+
  // | [>] interpret                                                ( public ) |
  // +=========================================================================+
  static constexpr verdict interpret(const parsed_token_list& token_list,
                                     http11::connection& connection_out,
                                     const policies&) {
    for (const std::string_view option : token_list.elements) {
      connection_out.options.push_back(option);
      // The "close" connection option turns the connection non-persistent.
      if (helpers::iequals(option, "close")) {
        connection_out.close_requested = true;
        connection_out.persistent = false;
      }
    }
    return verdict::kAccept;
  }

 private:
  // +=========================================================================+
  // | [>] consume_connection_option                               ( private ) |
  // +=========================================================================+
  static constexpr bool consume_connection_option(std::string_view sv) {
    return helpers::is_token(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
