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

#ifndef martianlabs_doba_protocol_http11_checkers_h_vary_h
#define martianlabs_doba_protocol_http11_checkers_h_vary_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                      vary |
// +===========================================================================+
// | RFC 9110 §12.5.5 Vary                                                     |
// +---------------------------------------------------------------------------+
// | The "Vary" response header field describes which parts of the request,    |
// | aside from the method and target URI, might have influenced the origin    |
// | server's process for selecting the response content.                      |
// |                                                                           |
// | A Vary field value consists of either the wildcard member "*" or a list   |
// | of request field names, known as selecting header fields, that might have |
// | influenced representation selection. Selecting fields are not limited to  |
// | those defined by RFC 9110.                                                |
// |                                                                           |
// | A list containing "*" indicates that aspects of the request other than    |
// | explicitly named fields might have influenced selection, including        |
// | information outside the HTTP message syntax, such as the client's network |
// | address. A recipient cannot determine whether such a response is suitable |
// | for a later request without consulting the origin server.                 |
// |                                                                           |
// | A proxy MUST NOT generate "*" in a Vary field value.                      |
// |                                                                           |
// | A list of field names informs caches that the response MUST NOT be reused |
// | for a later request unless the listed request fields match those of the   |
// | original request, or the response has been validated by the origin        |
// | server. Vary therefore expands the cache key used to select a stored      |
// | response.                                                                 |
// |                                                                           |
// | It also informs user agents that the response was subject to content      |
// | negotiation and that different values for the listed fields might select  |
// | a different representation in a subsequent request.                       |
// |                                                                           |
// | An origin server SHOULD generate Vary on a cacheable response when it     |
// | wants that response to be reused selectively according to request fields. |
// | Vary can be omitted when the performance cost imposed on caching is more  |
// | significant than the variation in the selected content.                   |
// |                                                                           |
// | The Authorization field name does not need to be listed because its own   |
// | definition already restricts reuse of a response for another user.        |
// |                                                                           |
// | Examples:                                                                 |
// |   Vary: *                                                                 |
// |   Vary: Accept-Encoding                                                   |
// |   Vary: Accept-Encoding, Accept-Language                                  |
// +---------------------------------------------------------------------------+
// | RFC 9110 §12.5.5 Vary (ABNF summary)                                      |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Vary             | #( "*" / field-name )                                  |
// | field-name       | token                                                  |
// | token            | 1*tchar                                                |
// | tchar            | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /  |
// |                  | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA      |
// | OWS              | *( SP / HTAB )                                         |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   Vary = [ ( "*" / field-name )                                           |
// |            *( OWS "," OWS ( "*" / field-name ) ) ]                        |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   Vary = [ ( "*" / field-name ) ]                                         |
// |          *( OWS "," OWS [ ( "*" / field-name ) ] )                        |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule uses "#", rather than "1#", zero non-empty members are     |
// | permitted by the purely syntactic ABNF. Therefore, an empty normalized    |
// | field value is syntactically valid.                                       |
// |                                                                           |
// | Although "*" is itself syntactically a token, it has special semantics    |
// | when it occurs as a member of Vary. Any list containing "*" has wildcard  |
// | semantics, regardless of the presence of other field-name members.        |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class vary {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Vary = #( "*" / field-name )
    // field-name = token
    //
    // "*" is itself a valid token because "*" belongs to tchar. Therefore,
    // both alternatives can be validated uniformly as token.
    //
    // for_each_list_element is expected to:
    // - accept an empty field-value;
    // - ignore empty list elements;
    // - remove OWS surrounding list separators;
    // - invoke consume_member only for non-empty elements.
    return helpers::for_each_list_element(sv, consume_member);
  }

 private:
  // +=========================================================================+
  // | [>] consume_member                                          ( private ) |
  // +=========================================================================+
  static constexpr bool consume_member(std::string_view sv) {
    // field-name = token
    //
    // consume_token returns the longest valid token prefix. The complete
    // member is valid only when the consumed token spans the entire element.
    const std::string_view token = helpers::consume_token(sv);
    return !token.empty() && token.size() == sv.size();
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
