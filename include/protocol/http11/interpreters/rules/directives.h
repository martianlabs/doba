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


#ifndef martianlabs_doba_protocol_http11_interpreters_r_directives_h
#define martianlabs_doba_protocol_http11_interpreters_r_directives_h

#include <string_view>

#include "protocol/http11/context.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::interpreters::rules {
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
}  // namespace martianlabs::doba::protocol::http11::interpreters::rules

#endif
