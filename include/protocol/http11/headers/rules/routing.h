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


#ifndef martianlabs_doba_protocol_http11_headers_rules_routing_h
#define martianlabs_doba_protocol_http11_headers_rules_routing_h

#include "protocol/http11/context.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers::rules {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] routing                                                     ( class ) |
// +---------------------------------------------------------------------------+
// | RFC 9112 §3.2 / RFC 9110 §7.2 Host and request-target                     |
// +---------------------------------------------------------------------------+
// | Transversal routing rules that reconcile the request-target with the      |
// | Host header, which the intra-header Host interpreter cannot decide alone  |
// | because it never sees the request line. Applied by deserialize() after    |
// | the single header pass has populated the context.                         |
// |                                                                           |
// | 1. RFC 9112 §3.2: a client MUST send exactly one Host header field in an  |
// |    HTTP/1.1 request; a missing or duplicated Host is rejected.            |
// | 2. RFC 9112 §3.2.2 / §3.3: when the request-target carries an authority   |
// |    (absolute-form or authority-form), the server MUST use that authority  |
// |    and it MUST agree with Host; a mismatch is rejected.                   |
// |                                                                           |
// | Host authority comparison is case-insensitive on the host and compares    |
// | ports by effective value: an absolute-form target's scheme default port   |
// | ("80" for http, "443" for https) is treated as equivalent to an omitted   |
// | port, so the two forms reconcile. Authority-form targets carry no scheme  |
// | and fall back to exact port equality.                                     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class routing {
 public:
  // +=========================================================================+
  // | [>] apply                                                    ( public ) |
  // +=========================================================================+
  static constexpr verdict apply(const context& ctx) {
    // Exactly one Host header is mandatory in HTTP/1.1.
    if (!ctx.has_host || ctx.multiple_host) return verdict::kReject;
    // When the request-target carried an authority it must match Host.
    if (ctx.has_target_authority) {
      if (!helpers::iequals(ctx.target_authority.host, ctx.host.host)) {
        return verdict::kReject;
      }
      // Compare ports by effective value: an absolute-form target that omits
      // the port (or spells out the scheme default) still matches a Host that
      // does the opposite. The scheme is empty for authority-form targets, so
      // that path degrades to exact string equality.
      if (!helpers::ports_equivalent(ctx.target_authority.scheme,
                                     ctx.target_authority.port,
                                     ctx.host.port)) {
        return verdict::kReject;
      }
    }
    return verdict::kAccept;
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers::rules

#endif
