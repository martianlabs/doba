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

#ifndef martianlabs_doba_protocol_http11_headers_content_length_h
#define martianlabs_doba_protocol_http11_headers_content_length_h

#include <cstddef>

#include "protocol/http11/connection.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] content_length                                              ( class ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 §8.6 Content-Length                                              |
// +---------------------------------------------------------------------------+
// | The "Content-Length" header field indicates the associated                |
// | representation's data length as a decimal non-negative integer number of  |
// | octets.                                                                   |
// |                                                                           |
// | When transferring a representation as content, Content-Length refers      |
// | specifically to the amount of data enclosed so that it can be used to     |
// | delimit message framing.                                                  |
// |                                                                           |
// | In other cases, Content-Length indicates the selected representation's    |
// | current length.                                                           |
// |                                                                           |
// | Any Content-Length field value greater than or equal to zero is valid.    |
// | The grammar does not define a maximum number of decimal digits.           |
// |                                                                           |
// | Example:                                                                  |
// |  Content-Length: 3495                                                     |
// +---------------------------------------------------------------------------+
// | RFC 9110 §8.6 Content-Length (ABNF summary)                               |
// +---------------------------------------------------------------------------+
// +----------------+----------------------------------------------------------+
// | Field          | Definition                                               |
// +----------------+----------------------------------------------------------+
// | Content-Length | 1*DIGIT                                                  |
// | DIGIT          | %x30-39 ; decimal digit "0" through "9"                  |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class content_length {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv, std::size_t& out) {
    // The producer overload validates the same 1*DIGIT grammar and, on
    // success, yields the decimal value through the shared parse_size_t, which
    // rejects a value that would overflow std::size_t.
    return helpers::parse_size_t(sv, out);
  }
  // +=========================================================================+
  // | [>] interpret                                                ( public ) |
  // +=========================================================================+
  static constexpr verdict interpret(const std::size_t& len,
                                     http11::connection&,
                                     const policies& policies) {
    if (policies.max_content_length != 0 && len > policies.max_content_length) {
      return verdict::kReject;
    }
    return verdict::kAccept;
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
