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

#ifndef martianlabs_doba_protocol_http11_checkers_h_if_match_h
#define martianlabs_doba_protocol_http11_checkers_h_if_match_h

#include <ranges>
#include <string_view>

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                  if-match |
// +===========================================================================+
// | RFC 9110 §13.1.1 If-Match                                                 |
// +---------------------------------------------------------------------------+
// | The "If-Match" header field makes the requested method conditional on the |
// | target resource having a current representation that satisfies the        |
// | supplied validator condition.                                             |
// |                                                                           |
// | When the field value is "*", the condition is true if the origin server   |
// | has at least one current representation of the target resource.           |
// |                                                                           |
// | When the field value is a list of entity tags, the condition is true if   |
// | the entity tag of the selected representation strongly matches at least   |
// | one member of that list.                                                  |
// |                                                                           |
// | An origin server MUST use the strong comparison function for If-Match.    |
// | Strong comparison succeeds only when neither entity tag is weak and both  |
// | opaque tags are identical.                                                |
// |                                                                           |
// | Weak entity tags are syntactically allowed by the entity-tag production,  |
// | but they can never satisfy an If-Match comparison because strong          |
// | comparison rejects weak validators.                                       |
// |                                                                           |
// | If-Match is commonly used with state-changing methods such as POST, PUT,  |
// | and DELETE to prevent the "lost update" problem. It can also be used with |
// | other methods involving representation selection or modification.         |
// |                                                                           |
// | An origin server receiving If-Match MUST evaluate the precondition before |
// | performing the requested method. If the condition is false, the server    |
// | MUST NOT perform the method and MAY respond with                          |
// | 412 (Precondition Failed).                                                |
// |                                                                           |
// | If a state-changing request appears to have already been applied to the   |
// | selected representation, the origin server MAY instead respond with a 2xx |
// | (Successful) status code.                                                 |
// |                                                                           |
// | A cache or intermediary MAY ignore If-Match because its interoperability  |
// | requirements apply primarily to origin servers.                           |
// |                                                                           |
// | Examples:                                                                 |
// |                                                                           |
// |   If-Match: "xyzzy"                                                       |
// |   If-Match: "xyzzy", "r2d2xxxx", "c3piozzzz"                              |
// |   If-Match: *                                                             |
// +---------------------------------------------------------------------------+
// | RFC 9110 §13.1.1 If-Match (ABNF summary)                                  |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | If-Match         | "*" / #entity-tag                                      |
// | entity-tag       | [ weak ] opaque-tag                                    |
// | weak             | %s"W/"                                                 |
// | opaque-tag       | DQUOTE *etagc DQUOTE                                   |
// | etagc            | %x21 / %x23-7E / obs-text                              |
// |                  | ; VCHAR except DQUOTE, plus obs-text                   |
// | obs-text         | %x80-FF                                                |
// | DQUOTE           | %x22                                                   |
// | OWS              | *( SP / HTAB )                                         |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #entity-tag = [ entity-tag *( OWS "," OWS entity-tag ) ]                |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #entity-tag = [ entity-tag ] *( OWS "," OWS [ entity-tag ] )            |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "#entity-tag", rather than "1#entity-tag", zero         |
// | non-empty entity-tag elements are permitted by the purely syntactic       |
// | recipient ABNF.                                                           |
// |                                                                           |
// | The "*" alternative is separate from #entity-tag. Therefore, "*" cannot   |
// | appear as a member of an entity-tag list or be combined with any other    |
// | value.                                                                    |
// +---------------------------------------------------------------------------+
// | Strong comparison                                                         |
// +---------------------------------------------------------------------------+
// | Two entity tags strongly match only if neither is weak and their          |
// | opaque-tag values are identical, character for character.                 |
// |                                                                           |
// | Consequently, W/"xyzzy" is a valid entity-tag syntactically, but it does  |
// | not strongly match either "xyzzy" or W/"xyzzy".                           |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class if_match {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // If-Match = "*" / #entity-tag
    // The "*" alternative matches the whole field value; it cannot be mixed
    // with, or appear as a member of, the entity-tag list.
    if (sv == "*") return true;
    // Otherwise, If-Match is a (possibly empty) list of entity tags.
    return helpers::for_each_list_element(sv, helpers::is_entity_tag);
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
