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

#ifndef martianlabs_doba_protocol_http11_headers_acrh_h
#define martianlabs_doba_protocol_http11_headers_acrh_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                            access-control-request-headers |
// +===========================================================================+
// | Fetch Standard �3.3.4 Access-Control-Request-Headers                      |
// +---------------------------------------------------------------------------+
// | The "Access-Control-Request-Headers" header field is used by a user       |
// | agent when issuing a CORS-preflight request. It informs the server which  |
// | HTTP header field names the user agent intends to use in the actual CORS  |
// | request.                                                                  |
// |                                                                           |
// | This field is sent on a preflight OPTIONS request, together with Origin   |
// | and Access-Control-Request-Method, when the actual request would include  |
// | non-safelisted request headers.                                           |
// |                                                                           |
// | The field value is a comma-separated list of HTTP field names. Each field |
// | name follows the generic HTTP field-name grammar, which is a token.       |
// |                                                                           |
// | Field names are case-insensitive. For CORS processing, user agents usually|
// | serialize requested header names in lowercase and sorted order, but that  |
// | is a serialization/processing detail rather than additional ABNF syntax.  |
// |                                                                           |
// | Examples:                                                                 |
// |   Access-Control-Request-Headers: content-type                            |
// |   Access-Control-Request-Headers: authorization, x-requested-with         |
// |   Access-Control-Request-Headers: content-type, x-custom-header           |
// +---------------------------------------------------------------------------+
// | Fetch Standard �3.3.4 HTTP new-header syntax (ABNF summary)               |
// +---------------------------------------------------------------------------+
// +--------------------------------+------------------------------------------+
// | Field                          | Definition                               |
// +--------------------------------+------------------------------------------+
// | Access-Control-Request-Headers | 1#field-name                             |
// | field-name                     | token                                    |
// | token                          | 1*tchar                                  |
// | tchar                          | "!" / "#" / "$" / "%" / "&" / "'" / "*"  |
// |                                | / "+" / "-" / "." / "^" / "_" / "`" /    |
// |                                | "|" / "~" / DIGIT / ALPHA                |
// | OWS                            | *( SP / HTAB )                           |
// +---------------------------------------------------------------------------+
// | RFC 9110 �5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   1#field-name = field-name *( OWS "," OWS field-name )                   |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   1#field-name = [ field-name ] *( OWS "," OWS [ field-name ] )           |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "1#field-name", at least one non-empty field-name is    |
// | required by the field definition. After ignoring empty list elements, a   |
// | recipient MUST reject values with zero non-empty field-name elements.     |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class access_control_request_headers {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    bool at_least_one_field_name = false;
    const bool result = helpers::for_each_list_element(
        sv, [&at_least_one_field_name](std::string_view element) {
          if (!consume_field_name(element)) return false;
          at_least_one_field_name = true;
          return true;
        });
    // 1#field-name requires at least one non-empty field-name.
    return result && at_least_one_field_name;
  }

 private:
  // +=========================================================================+
  // | [>] consume_field_name                                      ( private ) |
  // +=========================================================================+
  // | field-name = token                                                      |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_field_name(std::string_view sv) {
    return helpers::is_token(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
