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

#ifndef martianlabs_doba_protocol_http11_headers_max_forwards_h
#define martianlabs_doba_protocol_http11_headers_max_forwards_h

#include <cstddef>

#include "protocol/http11/connection.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                              max-forwards |
// +===========================================================================+
// | RFC 9110 §7.6.2 Max-Forwards                                              |
// +---------------------------------------------------------------------------+
// | The "Max-Forwards" header field provides a mechanism with the TRACE and   |
// | OPTIONS request methods to limit the number of times that a request is    |
// | forwarded by proxies.                                                     |
// |                                                                           |
// | The Max-Forwards value is a decimal integer indicating the remaining      |
// | number of times that the request message can be forwarded.                |
// |                                                                           |
// | Examples:                                                                 |
// |   Max-Forwards: 0                                                         |
// |   Max-Forwards: 1                                                         |
// |   Max-Forwards: 10                                                        |
// +---------------------------------------------------------------------------+
// | RFC 9110 §7.6.2 Max-Forwards (ABNF summary)                               |
// +---------------------------------------------------------------------------+
// +--------------+------------------------------------------------------------+
// | Field        | Definition                                                 |
// +--------------+------------------------------------------------------------+
// | Max-Forwards | 1*DIGIT                                                    |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class max_forwards {
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
  static constexpr verdict interpret(const std::size_t& max_fwd,
                                     http11::connection&, const policies& pol) {
    if (pol.max_forwarding_hops != 0 && max_fwd > pol.max_forwarding_hops) {
      return verdict::kReject;
    }
    return verdict::kAccept;
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
