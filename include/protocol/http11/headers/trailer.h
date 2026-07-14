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

#ifndef martianlabs_doba_protocol_http11_headers_trailer_h
#define martianlabs_doba_protocol_http11_headers_trailer_h

#include "protocol/http11/connection.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] trailer                                                     ( class ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 §6.6.2 Trailer                                                   |
// +---------------------------------------------------------------------------+
// | The "Trailer" header field provides a list of field names that the sender |
// | anticipates sending as trailer fields within the same message.            |
// |                                                                           |
// | Example:                                                                  |
// |  Trailer: Example-Checksum, Example-Signature                             |
// +---------------------------------------------------------------------------+
// | RFC 9110 §6.6.2 Trailer and §5.6.1 Lists (ABNF summary)                   |
// +---------------------------------------------------------------------------+
// +--------------------------+------------------------------------------------+
// | Field                    | Definition                                     |
// +--------------------------+------------------------------------------------+
// | Trailer                  | #field-name                                    |
// | field-name               | token                                          |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class trailer {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv, parsed_token_list& out) {
    // The producer overload validates each field-name exactly as the pure
    // check() does and captures every non-empty element in order.
    return helpers::for_each_list_element(sv, [&out](std::string_view element) {
      if (!consume_field_name(element)) return false;
      out.elements.push_back(element);
      return true;
    });
  }
  // +=========================================================================+
  // | [>] interpret                                                ( public ) |
  // +=========================================================================+
  static constexpr verdict interpret(const parsed_token_list& token_list,
                                     http11::connection& conn,
                                     const policies&) {
    for (const std::string_view name : token_list.elements) {
      conn.trailer_names.push_back(name);
    }
    return verdict::kAccept;
  }

 private:
  // +=========================================================================+
  // | [>] consume_field_name                                      ( private ) |
  // +=========================================================================+
  static constexpr bool consume_field_name(std::string_view sv) {
    return helpers::is_token(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
