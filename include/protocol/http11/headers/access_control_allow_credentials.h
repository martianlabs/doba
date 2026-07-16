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

#ifndef martianlabs_doba_protocol_http11_headers_acac_h
#define martianlabs_doba_protocol_http11_headers_acac_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                          access-control-allow-credentials |
// +===========================================================================+
// | Fetch �3.3.3 HTTP responses                                               |
// +---------------------------------------------------------------------------+
// | The "Access-Control-Allow-Credentials" response header field indicates    |
// | whether the response can be shared when the request's credentials mode is |
// | "include".                                                                |
// |                                                                           |
// | For a CORS-preflight request, the request's credentials mode is always    |
// | "same-origin", meaning that the preflight request itself excludes         |
// | credentials. However, any subsequent CORS request might use credentials,  |
// | so credential support needs to be indicated in the preflight response as  |
// | well.                                                                     |
// |                                                                           |
// | The only valid field value is the lowercase, case-sensitive literal       |
// | "true". There is no "false" value for this header field; to indicate      |
// | that credentials are not allowed, the field is omitted.                   |
// |                                                                           |
// | When credentials mode is "include", the CORS check succeeds only if the   |
// | Access-Control-Allow-Credentials value is exactly "true". Values such as  |
// | "True", "TRUE", or "false" do not match the required byte sequence.       |
// |                                                                           |
// | Examples:                                                                 |
// |   Access-Control-Allow-Credentials: true                                  |
// +---------------------------------------------------------------------------+
// | Fetch �3.3.4 HTTP new-header syntax (ABNF summary)                        |
// +---------------------------------------------------------------------------+
// +----------------------------------+----------------------------------------+
// | Field                            | Definition                             |
// +----------------------------------+----------------------------------------+
// | Access-Control-Allow-Credentials | %s"true" ; case-sensitive              |
// +----------------------------------+----------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class access_control_allow_credentials {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Access-Control-Allow-Credentials = %s"true". The only valid value is the
    // case-sensitive lowercase literal "true" (%x74 %x72 %x75 %x65); there is
    // no "false" value, and "True"/"TRUE" do not match the required sequence.
    return sv == "true";
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
