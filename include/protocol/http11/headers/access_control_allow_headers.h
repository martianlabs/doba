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

#ifndef martianlabs_doba_protocol_http11_headers_acah_h
#define martianlabs_doba_protocol_http11_headers_acah_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                              access-control-allow-headers |
// +===========================================================================+
// | Fetch Standard �3.3.3 Access-Control-Allow-Headers                        |
// +---------------------------------------------------------------------------+
// | The "Access-Control-Allow-Headers" response header field is used in a     |
// | CORS-preflight response to indicate which HTTP header fields are          |
// | supported by the response's URL for the purposes of the CORS protocol.    |
// |                                                                           |
// | It answers the client's Access-Control-Request-Headers field by listing   |
// | the non-safelisted request header names that the actual cross-origin      |
// | request is allowed to use.                                                |
// |                                                                           |
// | Header field names are compared case-insensitively by HTTP. Therefore,    |
// | "Content-Type", "content-type", and "CONTENT-TYPE" denote the same field  |
// | name.                                                                     |
// |                                                                           |
// | The value "*" is syntactically valid because it is a token / field-name.  |
// | In CORS semantics, "*" acts as a wildcard only for requests without       |
// | credentials. For credentialed requests, allowed header names need to be   |
// | listed explicitly.                                                        |
// |                                                                           |
// | Examples:                                                                 |
// |   Access-Control-Allow-Headers: Content-Type                              |
// |   Access-Control-Allow-Headers: X-PINGOTHER, Content-Type                 |
// |   Access-Control-Allow-Headers: *                                         |
// +---------------------------------------------------------------------------+
// | Fetch Standard �3.3.4 HTTP new-header syntax (ABNF summary)               |
// +---------------------------------------------------------------------------+
// +------------------------------+--------------------------------------------+
// | Field                        | Definition                                 |
// +------------------------------+--------------------------------------------+
// | Access-Control-Allow-Headers | #field-name                                |
// | field-name                   | token                                      |
// | token                        | 1*tchar                                    |
// | tchar                        | "!" / "#" / "$" / "%" / "&" / "'" / "*" /  |
// |                              | "+" / "-" / "." / "^" / "_" / "`" / "|" /  |
// |                              | "~" / DIGIT / ALPHA                        |
// | OWS                          | *( SP / HTAB )                             |
// +---------------------------------------------------------------------------+
// | RFC 9110 �5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #field-name = [ field-name *( OWS "," OWS field-name ) ]                |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #field-name = [ field-name ] *( OWS "," OWS [ field-name ] )            |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "#field-name", rather than "1#field-name", zero         |
// | non-empty field-name elements are permitted by the purely syntactic ABNF. |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class access_control_allow_headers {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    return helpers::for_each_list_element(sv, consume_field_name);
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
