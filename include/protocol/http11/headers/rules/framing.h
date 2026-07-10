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


#ifndef martianlabs_doba_protocol_http11_headers_rules_framing_h
#define martianlabs_doba_protocol_http11_headers_rules_framing_h

#include "protocol/http11/context.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers::rules {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] framing                                                     ( class ) |
// +---------------------------------------------------------------------------+
// | RFC 9112 §6 Message Body Length                                           |
// +---------------------------------------------------------------------------+
// | Transversal framing rules that span Content-Length and Transfer-Encoding, |
// | which no single intra-header interpreter can decide because they need     |
// | both headers at once. Applied by the coordinator after every intra-header |
// | interpreter has run and the context has been populated.                   |
// |                                                                           |
// | 1. RFC 9112 §6.1: if a Transfer-Encoding is present, the "chunked" coding |
// |    MUST be the final coding; otherwise the message length is undefined    |
// |    and the request is rejected.                                           |
// | 2. RFC 9112 §6.3 (bullet 3): a message MUST NOT contain both a            |
// |    Transfer-Encoding and a Content-Length; a sender that does so is       |
// |    treated as a framing error and the request is rejected.                |
// | 3. RFC 9112 §6.3 (bullet 4): a message MUST NOT contain multiple          |
// |    Content-Length header fields, regardless of whether their values       |
// |    agree; doing so is an unrecoverable framing error (the classic         |
// |    CL.CL request-smuggling vector) and the request is rejected.            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class framing {
 public:
  // +=========================================================================+
  // | [>] apply                                                    ( public ) |
  // +=========================================================================+
  static constexpr verdict apply(const context& ctx) {
    // Multiple Content-Length header fields are always a framing error, even
    // when every occurrence carries the same value (RFC 9112 §6.3 bullet 4).
    if (ctx.multiple_content_length) return verdict::kReject;
    if (ctx.has_transfer_encoding) {
      // A present Transfer-Encoding must delimit the body itself, so a
      // simultaneous Content-Length is ambiguous and rejected.
      if (ctx.has_content_length) return verdict::kReject;
      // "chunked" is only valid as the final transfer-coding; when a chunked
      // coding appears elsewhere in the list the length is undefined.
      const auto& codings = ctx.connection.transfer_codings;
      for (std::size_t i = 0; i < codings.size(); i++) {
        const bool is_last = (i + 1 == codings.size());
        if (helpers::iequals(codings[i], "chunked") && !is_last) {
          return verdict::kReject;
        }
      }
    }
    return verdict::kAccept;
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers::rules

#endif
