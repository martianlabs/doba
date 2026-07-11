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
//        --- martianLabs Anti-AI Usage and Model-Training Addendum ---
//
// TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
//
// Copyright 2025 martianLabs
//
// Except as otherwise stated in this Addendum, this software is licensed
// under the Apache License, Version 2.0 (the "License"); you may not use
// this file except in compliance with the License.
//
// The following additional terms are hereby added to the Apache License for
// the purpose of restricting the use of this software by Artificial
// Intelligence systems, machine learning models, data-scraping bots, and
// automated systems.
//
// 1.  MACHINE LEARNING AND AI RESTRICTIONS
//     1.1. No entity, organization, or individual may use this software,
//          its source code, object code, or any derivative work for the
//          purpose of training, fine-tuning, evaluating, or improving any
//          machine learning model, artificial intelligence system, large
//          language model, or similar automated system.
//     1.2. No automated system may copy, parse, analyze, index, or
//          otherwise process this software for any AI-related purpose.
//     1.3. Use of this software as input, prompt material, reference
//          material, or evaluation data for AI systems is expressly
//          prohibited.
//
// 2.  SCRAPING AND AUTOMATED ACCESS RESTRICTIONS
//     2.1. No automated crawler, training pipeline, or data-extraction
//          system may collect, store, or incorporate any portion of this
//          software in any dataset used for machine learning or AI
//          training.
//     2.2. Any automated access must comply with this License and with
//          applicable copyright law.
//
// 3.  PROHIBITION ON DERIVATIVE DATASETS
//     3.1. You may not create datasets, corpora, embeddings, vector
//          stores, or similar derivative data intended for use by
//          automated systems, AI models, or machine learning algorithms.
//
// 4.  NO WAIVER OF RIGHTS
//     4.1. These restrictions apply in addition to, and do not limit,
//          the rights and protections provided to the copyright holder
//          under the Apache License Version 2.0 and applicable law.
//
// 5.  ACCEPTANCE
//     5.1. Any use of this software constitutes acceptance of both the
//          Apache License Version 2.0 and this Anti-AI Addendum.
//
// You may obtain a copy of the Apache License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.  See the License for the specific language governing
// permissions and limitations under the Apache License Version 2.0.

#ifndef martianlabs_doba_protocol_http11_headers_x_forwarded_for_h
#define martianlabs_doba_protocol_http11_headers_x_forwarded_for_h

#include "protocol/http11/connection.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                           x-forwarded-for |
// +===========================================================================+
// | De-facto X-Forwarded-For                                                  |
// +---------------------------------------------------------------------------+
// | The "X-Forwarded-For" header field is a de-facto request header used by   |
// | proxies, reverse proxies, load balancers, and gateways to identify the    |
// | originating client IP address and the forwarding chain seen by the HTTP   |
// | request.                                                                  |
// |                                                                           |
// | Because this field can be supplied directly by a client, its contents MUST|
// | NOT be trusted unless the request was received through a trusted proxy    |
// | chain that sanitizes or overwrites untrusted incoming values.             |
// |                                                                           |
// | Examples:                                                                 |
// |   X-Forwarded-For: 203.0.113.195                                          |
// |   X-Forwarded-For: 203.0.113.195, 70.41.3.18, 150.172.238.178             |
// |   X-Forwarded-For: 2001:db8:85a3::8a2e:370:7334                           |
// |   X-Forwarded-For: unknown, 203.0.113.10                                  |
// +---------------------------------------------------------------------------+
// | Practical de-facto syntax (ABNF summary)                                  |
// +---------------------------------------------------------------------------+
// +-------------------+-------------------------------------------------------+
// | Field             | Definition                                            |
// +-------------------+-------------------------------------------------------+
// | X-Forwarded-For   | #xff-node                                             |
// | xff-node          | IPv4address / IPv6address / IP-literal / "unknown"    |
// +-------------------+-------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class x_forwarded_for {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv, parsed_token_list& out) {
    // The producer overload validates each xff-node exactly as the pure
    // check() does and captures every non-empty node in order.
    return helpers::for_each_list_element(sv, [&out](std::string_view element) {
      if (!consume_xff_node(element)) return false;
      out.elements.push_back(element);
      return true;
    });
  }
  // +=========================================================================+
  // | [>] interpret                                                ( public ) |
  // +=========================================================================+
  static constexpr verdict interpret(const parsed_token_list& token_list,
                                     http11::connection&, const policies& pol) {
    if (pol.max_forwarding_hops != 0 &&
        token_list.elements.size() > pol.max_forwarding_hops) {
      return verdict::kReject;
    }
    return verdict::kAccept;
  }

 private:
  // +=========================================================================+
  // | [>] consume_xff_node                                        ( private ) |
  // +=========================================================================+
  // | xff-node = IPv4address / IPv6address / IP-literal / "unknown"           |
  // +-------------------------------------------------------------------------+
  // | Ports and general reg-name / host names are intentionally excluded from |
  // | this strict profile; IPvFuture is accepted only inside an IP-literal.   |
  // +=========================================================================+
  static constexpr bool consume_xff_node(std::string_view sv) {
    return helpers::is_ip_v4_address(sv) || helpers::is_ip_v6_address(sv) ||
           helpers::is_ip_literal(sv) || sv == "unknown";
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
