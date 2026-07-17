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


#ifndef martianlabs_doba_protocol_http11_context_h
#define martianlabs_doba_protocol_http11_context_h

#include <cstddef>

#include "protocol/http11/connection.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] context                                                    ( struct ) |
// +---------------------------------------------------------------------------+
// | The aggregated view the transversal rules operate on. Where an intra-     |
// | header interpreter sees a single header in isolation, a rule needs to     |
// | reason across several: whether a header was present at all, how two       |
// | headers combine, and the connection/policy state the interpreters already |
// | derived. deserialize() fills this context during its single header pass   |
// | (each modelled header parsed once and interpreted in place) and then      |
// | applies the transversal rules over it.                                    |
// |                                                                           |
// | The connection member is the mutable hop-by-hop state (already populated  |
// | by the intra-header interpreters); policies is the read-only inbound      |
// | configuration. The remaining members are the cross-header signals that    |
// | are NOT hop-by-hop and therefore do not belong on the connection state:   |
// | presence flags and the few parsed values a rule must compare directly.    |
// |                                                                           |
// | Every std::string_view is zero-copy and points back into the request's    |
// | field-value buffer, which MUST outlive this context.                      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct context {
  // Policies coming from the inbound configuration; 
  // they are not derived from the request.
  policies policies;
  // The mutable hop-by-hop connection state derived from the request.
  http11::connection connection;
  // Content-Length and Transfer-Encoding are mutually exclusive; the rules
  // check for their presence and the number of Content-Length headers. The
  // parsed Content-Length value is stored for the rules to reason about it.
  bool has_content_length = false;
  bool multiple_content_length = false;
  std::size_t content_length = 0;
  bool has_transfer_encoding = false;
  // Host and Target-Authority are mutually exclusive; the rules check for their
  // presence and the number of Host headers. The parsed values are stored for
  // the rules to reason about them.
  bool has_host = false;
  bool multiple_host = false;
  parsed_host_port host;
  bool has_target_authority = false;
  parsed_host_port target_authority;
  // The number of forwarding hops across Via / Forwarded / X-Forwarded-For.
  std::size_t forwarding_hops = 0;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
