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

#ifndef martianlabs_doba_protocol_http11_headers_allow_h
#define martianlabs_doba_protocol_http11_headers_allow_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                     allow |
// +===========================================================================+
// | RFC 9110 §10.2.1 Allow                                                    |
// +---------------------------------------------------------------------------+
// | The "Allow" header field lists the set of methods advertised as supported |
// | by the target resource. Its purpose is strictly to inform the recipient   |
// | of valid request methods associated with that resource.                   |
// |                                                                           |
// | The actual set of allowed methods is defined by the origin server at the  |
// | time of each request. An origin server MUST generate an Allow header      |
// | field in a 405 (Method Not Allowed) response and MAY generate it in any   |
// | other response.                                                           |
// |                                                                           |
// | An empty Allow field value indicates that the resource allows no methods. |
// | This might occur in a 405 response if the resource has been temporarily   |
// | disabled by configuration.                                                |
// |                                                                           |
// | A proxy MUST NOT modify the Allow header field. It does not need to       |
// | understand all indicated methods in order to handle them according to     |
// | the generic message handling rules.                                       |
// |                                                                           |
// | Method tokens are case-sensitive. By convention, standardized HTTP        |
// | methods are defined using all-uppercase US-ASCII letters.                 |
// |                                                                           |
// | Examples:                                                                 |
// |   Allow: GET, HEAD, PUT                                                   |
// |   Allow: OPTIONS, GET, HEAD, POST, DELETE                                 |
// |   Allow:                                                                  |
// +---------------------------------------------------------------------------+
// | RFC 9110 §10.2.1 Allow (ABNF summary)                                     |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Allow            | #method                                                |
// | method           | token                                                  |
// | token            | 1*tchar                                                |
// | tchar            | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /  |
// |                  | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA      |
// | OWS              | *( SP / HTAB )                                         |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #method = [ method *( OWS "," OWS method ) ]                            |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #method = [ method ] *( OWS "," OWS [ method ] )                        |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "#method", rather than "1#method", zero non-empty       |
// | method elements are permitted by the purely syntactic ABNF. This also     |
// | matches the explicitly defined meaning of an empty Allow field value.     |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class allow {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    return helpers::for_each_list_element(sv, consume_method);
  }

 private:
  // +=========================================================================+
  // | [>] consume_method                                          ( private ) |
  // +=========================================================================+
  static constexpr bool consume_method(std::string_view sv) {
    return helpers::is_token(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
