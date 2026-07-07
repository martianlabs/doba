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
  // The hop-by-hop transport state derived by the intra-header interpreters.
  http11::connection& connection;
  // The read-only inbound policy configuration.
  const policies& policies;
  // Whether a Content-Length header was present, and its parsed value. When
  // more than one Content-Length header field is seen, multiple_content_length
  // is set and framing::apply rejects the message regardless of whether the
  // values agree (RFC 9112 §6.3 bullet 4), mirroring how multiple_host is
  // handled below.
  bool has_content_length = false;
  bool multiple_content_length = false;
  std::size_t content_length = 0;
  // Whether a Transfer-Encoding header was present (its codings, including
  // whether the final one is chunked, live on the connection state).
  bool has_transfer_encoding = false;
  // Whether a Host header was present and whether more than one was seen.
  bool has_host = false;
  bool multiple_host = false;
  // The parsed Host value (valid only when has_host is true).
  parsed_host_port host;
  // The request-target authority, when the target carried one (absolute-form
  // or authority-form); used to reconcile against Host.
  bool has_target_authority = false;
  parsed_host_port target_authority;
  // The aggregate number of forwarding hops seen across Via, Forwarded, and
  // X-Forwarded-For. Each intra-header interpreter checks its own list against
  // the limit; the policy rule checks their sum, which no single header sees.
  std::size_t forwarding_hops = 0;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
