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


#ifndef martianlabs_doba_protocol_http11_interpreters_h_upgrade_h
#define martianlabs_doba_protocol_http11_interpreters_h_upgrade_h

#include <string_view>

#include "protocol/http11/connection.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::interpreters::headers {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] upgrade                                                     ( class ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 §7.8 Upgrade                                                     |
// +---------------------------------------------------------------------------+
// | Intra-header semantic interpreter for the Upgrade header. It consumes the |
// | parsed_token_list produced by the syntactic checker (each element being a |
// | protocol-name with an optional "/" protocol-version) and records the      |
// | ordered set of offered protocols on the connection state.                 |
// |                                                                           |
// | Policy is enforced here because it is a per-header switch: when a client  |
// | offers an upgrade but policies.allow_upgrade is false the request is      |
// | rejected. Requiring the companion "Connection: upgrade" option is a       |
// | transversal concern left to the rules layer.                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class upgrade {
 public:
  // +=========================================================================+
  // | [>] interpret                                                ( public ) |
  // +=========================================================================+
  static constexpr verdict interpret(const parsed_token_list& token_list,
                                     http11::connection& conn,
                                     const policies& pol) {
    for (const std::string_view protocol : token_list.elements) {
      conn.upgrade_offer.push_back(protocol);
    }
    if (!conn.upgrade_offer.empty() && !pol.allow_upgrade) {
      return verdict::kReject;
    }
    return verdict::kAccept;
  }
};
}  // namespace martianlabs::doba::protocol::http11::interpreters::headers

#endif
