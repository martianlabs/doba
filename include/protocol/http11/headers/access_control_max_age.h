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

#ifndef martianlabs_doba_protocol_http11_headers_acma_h
#define martianlabs_doba_protocol_http11_headers_acma_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                    access-control-max-age |
// +===========================================================================+
// | Fetch Standard �3.3.3 HTTP responses                                      |
// +---------------------------------------------------------------------------+
// | The "Access-Control-Max-Age" response header field indicates the number   |
// | of seconds for which the information provided by the                      |
// | Access-Control-Allow-Methods and Access-Control-Allow-Headers headers can |
// | be cached in the CORS-preflight cache.                                    |
// |                                                                           |
// | This header is only relevant to successful CORS-preflight responses. It   |
// | allows a user agent to reuse the preflight result for subsequent matching |
// | cross-origin requests without issuing another preflight request until the |
// | cached entry expires or is removed.                                       |
// |                                                                           |
// | If the header is absent or cannot be parsed by the CORS-preflight fetch   |
// | algorithm, the Fetch Standard uses a default max-age of 5 seconds. User   |
// | agents may impose an implementation-defined upper limit and clamp larger  |
// | values to that limit.                                                     |
// |                                                                           |
// | The field value is a delta-seconds value: a non-negative decimal integer  |
// | representing a duration in seconds. A leading sign is not part of the     |
// | syntax. Therefore values such as "-1" are not valid according to the ABNF |
// | used by the Fetch Standard.                                               |
// |                                                                           |
// | Leading zeroes are syntactically valid. Range handling, overflow handling,|
// | and user-agent cache limits are semantic processing concerns, not part of |
// | this field-value ABNF check.                                              |
// |                                                                           |
// | Examples:                                                                 |
// |   Access-Control-Max-Age: 0                                               |
// |   Access-Control-Max-Age: 600                                             |
// |   Access-Control-Max-Age: 86400                                           |
// +---------------------------------------------------------------------------+
// | Fetch Standard �3.3.4 HTTP new-header syntax (ABNF summary)               |
// +---------------------------------------------------------------------------+
// +------------------------+--------------------------------------------------+
// | Field                  | Definition                                       |
// +------------------------+--------------------------------------------------+
// | Access-Control-Max-Age | delta-seconds                                    |
// | delta-seconds          | 1*DIGIT                                          |
// | DIGIT                  | %x30-39                                          |
// +---------------------------------------------------------------------------+
// | RFC 9111 �1.2.2 Delta Seconds                                             |
// +---------------------------------------------------------------------------+
// | The "delta-seconds" rule specifies a non-negative integer, representing   |
// | time in seconds:                                                          |
// |                                                                           |
// |   delta-seconds = 1*DIGIT                                                 |
// |                                                                           |
// | A recipient parsing a delta-seconds value and converting it to binary form|
// | ought to use an arithmetic type of at least 31 bits of non-negative       |
// | integer range. Overflow handling is outside the pure syntactic ABNF       |
// | validation performed by this header checker.                              |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class access_control_max_age {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Access-Control-Max-Age = delta-seconds = 1*DIGIT. A leading sign is not
    // part of the syntax and leading zeroes are valid; range/overflow handling
    // is semantic and outside this purely syntactic ABNF check.
    return helpers::is_digits(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
