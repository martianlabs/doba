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

#ifndef martianlabs_doba_protocol_http11_headers_user_agent_h
#define martianlabs_doba_protocol_http11_headers_user_agent_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                user-agent |
// +===========================================================================+
// | RFC 9110 §10.1.5 User-Agent                                               |
// +---------------------------------------------------------------------------+
// | The "User-Agent" request header field contains information about the user |
// | agent originating the request. It can be used by servers to identify the  |
// | scope of reported interoperability problems, work around specific user    |
// | agent limitations, or tailor responses to avoid particular issues.        |
// |                                                                           |
// | A user agent SHOULD send a User-Agent header field in each request unless |
// | specifically configured not to do so.                                     |
// |                                                                           |
// | User-Agent values consist of one or more product identifiers, optionally  |
// | followed by comments. Product identifiers SHOULD be listed in decreasing  |
// | order of significance for identifying the user agent software.            |
// |                                                                           |
// | A user agent SHOULD NOT generate needless detail, overly fine-grained     |
// | version information, or identifiers for other implementations, since this |
// | can increase request latency, expose fingerprinting information, and lead |
// | to compatibility ossification.                                            |
// |                                                                           |
// | Examples:                                                                 |
// |   User-Agent: CERN-LineMode/2.15 libwww/2.17b3                            |
// |   User-Agent: curl/8.5.0                                                  |
// |   User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)                   |
// +---------------------------------------------------------------------------+
// | RFC 9110 §10.1.5 User-Agent (ABNF summary)                                |
// +---------------------------------------------------------------------------+
// +-----------------+---------------------------------------------------------+
// | Field           | Definition                                              |
// +-----------------+---------------------------------------------------------+
// | User-Agent      | product *( RWS ( product / comment ) )                  |
// | product         | token [ "/" product-version ]                           |
// | product-version | token                                                   |
// | comment         | "(" *( ctext / quoted-pair / comment ) ")"              |
// | ctext           | HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text      |
// | quoted-pair     | "\" ( HTAB / SP / VCHAR / obs-text )                    |
// | token           | 1*tchar                                                 |
// | tchar           | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /   |
// |                 | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA       |
// | RWS             | 1*( SP / HTAB )                                         |
// | VCHAR           | %x21-7E                                                 |
// | obs-text        | %x80-FF                                                 |
// +---------------------------------------------------------------------------+
// | Notes for syntactic validation                                            |
// +---------------------------------------------------------------------------+
// | User-Agent is not a list-based field: it does not use the RFC 9110 §5.6.1 |
// | "#" list extension. Commas have no structural meaning in this field value |
// | and are only valid where allowed by token/comment syntax.                 |
// |                                                                           |
// | The field value MUST begin with a product. Therefore, an empty value, a   |
// | leading comment, or a value beginning with whitespace before the first    |
// | product is invalid after field-value normalization.                       |
// |                                                                           |
// | Each subsequent product or comment MUST be preceded by RWS. Adjacent      |
// | products/comments without at least one SP or                              |
// | HTAB between them are invalid.                                            |
// |                                                                           |
// | Comments are recursive. A validator that supports comment syntax must     |
// | correctly handle nested comments and quoted-pair escaping inside comments.|
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class user_agent {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // User-Agent = product *( RWS ( product / comment ) ). It is not an RFC
    // 9110 §5.6.1 "#" list: the value MUST begin with a product, items are
    // separated by RWS (not commas), and this is the same product-list shape
    // shared with the Server header field (RFC 9110 §10.2.4).
    return helpers::check_product_list(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
