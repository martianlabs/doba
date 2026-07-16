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

#ifndef martianlabs_doba_protocol_http11_headers_acao_h
#define martianlabs_doba_protocol_http11_headers_acao_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                               access-control-allow-origin |
// +===========================================================================+
// | WHATWG Fetch �3.3.3 Access-Control-Allow-Origin                           |
// +---------------------------------------------------------------------------+
// | The "Access-Control-Allow-Origin" response header field indicates whether |
// | a response can be shared with requesting code from the given origin.      |
// |                                                                           |
// | A server MAY return the literal value of the Origin request header, which |
// | can be "null", or the wildcard value "*".                                 |
// |                                                                           |
// | The wildcard value "*" allows sharing the response with any origin,       |
// | subject to the CORS credentials rules. When credentials mode is "include",|
// | "*" is not sufficient for a successful CORS check; the server has to      |
// | return the serialized requesting origin and also satisfy the credentials  | 
// | requirements.                                                             |
// |                                                                           |
// | The serialized origin has no path, no query, no fragment, and no trailing |
// | slash. It consists only of scheme, host, and optional port.               |
// |                                                                           |
// | The value "null" is case-sensitive and represents an opaque or privacy-   |
// | constrained origin serialization; it is not the same as a missing header. |
// |                                                                           |
// | This field is not a list-valued field. Multiple origins separated by comma|
// | are not valid under the current Fetch ABNF.                               |
// |                                                                           |
// | Examples:                                                                 |
// |   Access-Control-Allow-Origin: *                                          |
// |   Access-Control-Allow-Origin: https://example.com                        |
// |   Access-Control-Allow-Origin: http://localhost:8080                      |
// |   Access-Control-Allow-Origin: null                                       |
// +---------------------------------------------------------------------------+
// | WHATWG Fetch �3.3.4 HTTP new-header syntax                                |
// +---------------------------------------------------------------------------+
// +-----------------------------+---------------------------------------------+
// | Field                       | Definition                                  |
// +-----------------------------+---------------------------------------------+
// | Access-Control-Allow-Origin | origin-or-null / wildcard                   |
// | wildcard                    | "*"                                         |
// | origin-or-null              | serialized-origin / %s"null"                |
// | serialized-origin           | serialized-scheme "://" serialized-host     |
// |                             | [ ":" serialized-port ]                     |
// | serialized-scheme           | lower-alpha *( lower-alphanum / "+" / "-"   |
// |                             | / "." )                                     |
// | serialized-host             | serialized-ipv4 / "[" serialized-ipv6 "]"   |
// |                             | / serialized-domain                         |
// | serialized-port             | 1*5 DIGIT                                   |
// +---------------------------------------------------------------------------+
// | WHATWG Fetch �3.2 Origin header serialization ABNF                        |
// +---------------------------------------------------------------------------+
// +-------------------+-------------------------------------------------------+
// | Field             | Definition                                            |
// +-------------------+-------------------------------------------------------+
// | serialized-ipv4   | dec-octet "." dec-octet "." dec-octet "." dec-octet   |
// | dec-octet         | DIGIT                                                 |
// |                   | / %x31-39 DIGIT                                       |
// |                   | / "1" 2DIGIT                                          |
// |                   | / "2" %x30-34 DIGIT                                   |
// |                   | / "25" %x30-35                                        |
// | serialized-ipv6   | 7( h16 ":" ) h16                                      |
// |                   | / "::" 5( h16 ":" ) h16                               |
// |                   | / [ h16 ] "::" 4( h16 ":" ) h16                       |
// |                   | / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) h16         |
// |                   | / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) h16         |
// |                   | / [ *3( h16 ":" ) h16 ] "::" h16 ":" h16              |
// |                   | / [ *4( h16 ":" ) h16 ] "::" h16                      |
// |                   | / [ *5( h16 ":" ) h16 ] "::"                          |
// | h16               | "0" / ( non-zero-hex 0*3hex )                         |
// | non-zero-hex      | %x31-39 / %x61-66                                     |
// | hex               | %x30-39 / %x61-66                                     |
// | lower-alpha       | %x61-7A                                               |
// | lower-alphanum    | lower-alpha / DIGIT                                   |
// | domain-label      | lower-alphanum / ( lower-alphanum                     |
// |                   | *( lower-alphanum / "-" ) lower-alphanum )            |
// | serialized-domain | *( domain-label "." ) domain-label                    |
// | DIGIT             | %x30-39                                               |
// +---------------------------------------------------------------------------+
// | IMPORTANT: this field is not a #rule list.                                |
// +---------------------------------------------------------------------------+
// | There is no RFC 9110 �5.6.1 list expansion for this header field.         |
// | A comma-separated sequence of origins is not valid syntactic input for    |
// | Access-Control-Allow-Origin.                                              |
// |                                                                           |
// | The only syntactic alternatives are:                                      |
// |                                                                           |
// |   "*"                                                                     |
// |   "null"                                                                  |
// |   serialized-origin                                                       |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class access_control_allow_origin {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Access-Control-Allow-Origin = "*" / "null" / serialized-origin. The
    // wildcard "*" and the "null" literal (%s"null") are case-sensitive octet
    // sequences; this field is not a #rule list, so a single value is required.
    if (sv == "*" || sv == "null") return true;
    return helpers::check_serialized_origin(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
