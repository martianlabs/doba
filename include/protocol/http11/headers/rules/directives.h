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


#ifndef martianlabs_doba_protocol_http11_headers_rules_directives_h
#define martianlabs_doba_protocol_http11_headers_rules_directives_h

#include "protocol/http11/context.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers::rules {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] directives                                                  ( class ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 §7.6.1 Connection                                                |
// +---------------------------------------------------------------------------+
// | The transversal "connection directives" rules: they relate the Connection |
// | option tokens (the hop-by-hop directives) to other headers, which the     |
// | intra-header Connection interpreter cannot check alone because it never   |
// | sees those other headers. Applied by deserialize() after the single header|
// | pass has populated the context. Named "directives" (not "connection") to  |
// | avoid clashing with the protocol-level connection.h.                      |
// |                                                                           |
// | 1. RFC 9110 §7.8: the "upgrade" connection option is only meaningful      |
// |    alongside an Upgrade header; a client that names it without offering   |
// |    any protocol is rejected.                                              |
// | 2. RFC 9110 §7.6.1: a connection option names a header field that is      |
// |    hop-by-hop for this connection, but control headers with connection-   |
// |    wide semantics (Host, Content-Length, Transfer-Encoding, TE, Trailer,  |
// |    Upgrade, and Connection itself) MUST NOT be nominated; doing so is     |
// |    rejected.                                                              |
// |                                                                           |
// | Connection option comparison is case-insensitive per RFC 9110 §7.6.1.     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class directives {
 public:
  // +=========================================================================+
  // | [>] apply                                                    ( public ) |
  // +=========================================================================+
  static constexpr verdict apply(const context& ctx) {
    for (const std::string_view option : ctx.connection.options) {
      // "upgrade" as a connection option requires a companion Upgrade offer.
      if (helpers::iequals(option, "upgrade")) {
        if (ctx.connection.upgrade_offer.empty()) return verdict::kReject;
        continue;
      }
      // Control headers with connection-wide semantics may not be nominated
      // as hop-by-hop connection options.
      if (helpers::iequals(option, "connection") ||
          helpers::iequals(option, "host") ||
          helpers::iequals(option, "content-length") ||
          helpers::iequals(option, "transfer-encoding") ||
          helpers::iequals(option, "te") ||
          helpers::iequals(option, "trailer") ||
          helpers::iequals(option, "upgrade")) {
        return verdict::kReject;
      }
    }
    return verdict::kAccept;
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers::rules

#endif
