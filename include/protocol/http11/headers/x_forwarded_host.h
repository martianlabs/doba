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

#ifndef martianlabs_doba_protocol_http11_headers_x_forwarded_host_h
#define martianlabs_doba_protocol_http11_headers_x_forwarded_host_h

#include "protocol/http11/connection.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                          x-forwarded-host |
// +===========================================================================+
// | Non-standard / de-facto X-Forwarded-Host                                  |
// +---------------------------------------------------------------------------+
// | The "X-Forwarded-Host" header field is a de-facto request header used by  |
// | proxies, reverse proxies, load balancers, and CDNs to preserve the        |
// | original Host value requested by the client before the request is         |
// | forwarded to an origin server.                                            |
// |                                                                           |
// | The field value identifies a host, optionally followed by a port.         |
// |                                                                           |
// | Examples:                                                                 |
// |   X-Forwarded-Host: example.com                                           |
// |   X-Forwarded-Host: example.com:8443                                      |
// |   X-Forwarded-Host: [2001:db8::1]:443                                     |
// +---------------------------------------------------------------------------+
// | RFC 9110 §7.2 Host / RFC 3986 §3.2.2 host (ABNF summary)                  |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | X-Forwarded-Host | uri-host [ ":" port ]                                  |
// | uri-host         | IP-literal / IPv4address / reg-name                    |
// | port             | *DIGIT                                                 |
// +------------------+--------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class x_forwarded_host {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv, parsed_host_port& out) {
    // The producer overload validates exactly as the pure check() does and, on
    // success, fills the parsed uri-host, port, and host type through the same
    // shared helpers::check_host_port so both paths never diverge.
    return helpers::check_host_port(sv, out.host, out.port, out.type);
  }
  // +=========================================================================+
  // | [>] interpret                                                ( public ) |
  // +=========================================================================+
  static constexpr verdict interpret(const parsed_host_port&,
                                     http11::connection&, const policies&) {
    return verdict::kAccept;
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
