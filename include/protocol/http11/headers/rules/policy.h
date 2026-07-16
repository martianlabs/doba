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


#ifndef martianlabs_doba_protocol_http11_headers_rules_policy_h
#define martianlabs_doba_protocol_http11_headers_rules_policy_h

#include "protocol/http11/context.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers::rules {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] policy                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | Inbound policy limits that span several headers                           |
// +---------------------------------------------------------------------------+
// | Transversal policy rules that enforce limits no single intra-header       |
// | interpreter can, because the quantity being limited is an aggregate of    |
// | several headers. Applied by deserialize() after the single header pass    |
// | has populated the context.                                                |
// |                                                                           |
// | 1. Aggregate forwarding hops: an intermediary chain is described jointly  |
// |    by Via, Forwarded, and X-Forwarded-For. Each intra-header interpreter  |
// |    checks its own element count, but the combined hop count (accumulated  |
// |    into the context) is checked here against policies.max_forwarding_hops |
// |    to bound the total forwarding depth. A limit of 0 means "unlimited".   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class policy {
 public:
  // +=========================================================================+
  // | [>] apply                                                    ( public ) |
  // +=========================================================================+
  static constexpr verdict apply(const context& ctx) {
    if (ctx.policies.max_forwarding_hops != 0 &&
        ctx.forwarding_hops > ctx.policies.max_forwarding_hops) {
      return verdict::kReject;
    }
    return verdict::kAccept;
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers::rules

#endif
