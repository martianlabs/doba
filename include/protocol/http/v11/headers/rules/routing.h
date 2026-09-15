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


#ifndef martianlabs_doba_protocol_http_v11_headers_rules_routing_h
#define martianlabs_doba_protocol_http_v11_headers_rules_routing_h

#include "protocol/http/v11/context.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/v11/verdict.h"

namespace martianlabs::doba::protocol::http::v11::headers::rules {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] routing                                                     ( class ) |
// +---------------------------------------------------------------------------+
// | RFC 9112 S3.2 / RFC 9110 S7.2 Host and request-target                     |
// +---------------------------------------------------------------------------+
// | Transversal routing rules that reconcile the request-target with the      |
// | Host header, which the intra-header Host interpreter cannot decide alone  |
// | because it never sees the request line. Applied by deserialize() after    |
// | the single header pass has populated the context.                         |
// |                                                                           |
// | 1. RFC 9112 S3.2: a client MUST send exactly one Host header field in an  |
// |    HTTP/1.1 request; a missing or duplicated Host is rejected.            |
// | 2. RFC 9112 S3.2.2: absolute-form uses the target authority regardless    |
// |    of the received Host value.                                            |
// | 3. Authority-form retains the requirement that its authority match Host.  |
// |                                                                           |
// | Authority-form comparison is case-insensitive on the host and requires    |
// | exact port equality. A non-empty scheme identifies absolute-form.         |
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
    // Only authority-form requires equality with Host.
    if (ctx.has_target_authority) {
      if (!ctx.target_authority.scheme.empty()) return verdict::kAccept;
      if (!helpers::iequals(ctx.target_authority.host, ctx.host.host)) {
        return verdict::kReject;
      }
      // Authority-form has no scheme and requires exact port equality.
      if (!helpers::ports_equivalent(ctx.target_authority.scheme,
                                     ctx.target_authority.port,
                                     ctx.host.port)) {
        return verdict::kReject;
      }
    }
    return verdict::kAccept;
  }
};
}  // namespace martianlabs::doba::protocol::http::v11::headers::rules

#endif
