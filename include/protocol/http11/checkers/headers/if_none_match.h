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

#ifndef martianlabs_doba_protocol_http11_checkers_h_if_none_match_h
#define martianlabs_doba_protocol_http11_checkers_h_if_none_match_h

#include <ranges>
#include <string_view>

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                             if-none-match |
// +===========================================================================+
// | RFC 9110 §13.1.2 If-None-Match                                            |
// +---------------------------------------------------------------------------+
// | The "If-None-Match" header field makes a request conditional on a         |
// | recipient cache or origin server either not having any current            |
// | representation of the target resource, when the field value is "*", or    |
// | having a selected representation whose entity tag does not match any      |
// | entity tag in the supplied list.                                          |
// |                                                                           |
// | A recipient MUST use the weak comparison function when comparing entity   |
// | tags for If-None-Match. Weak entity tags are therefore valid in this      |
// | field.                                                                    |
// |                                                                           |
// | If-None-Match is primarily used with conditional GET requests. When one   |
// | of the listed entity tags matches the selected representation, a GET or   |
// | HEAD request receives 304 (Not Modified) instead of the representation    |
// | data.                                                                     |
// |                                                                           |
// | The "*" value can also be used with an unsafe method, such as PUT, to     |
// | prevent the method from modifying an existing representation when the     |
// | client intends to create a new resource only if none currently exists.    |
// |                                                                           |
// | When the condition evaluates to false, the origin server MUST NOT perform |
// | the requested method. It MUST respond with 304 (Not Modified) for GET or  |
// | HEAD, or 412 (Precondition Failed) for every other method.                |
// |                                                                           |
// | The "*" alternative is not an entity-tag and cannot appear as a member of |
// | an entity-tag list. A value combining "*" with other values is            |
// | syntactically invalid.                                                    |
// |                                                                           |
// | Examples:                                                                 |
// |   If-None-Match: "xyzzy"                                                  |
// |   If-None-Match: W/"xyzzy"                                                |
// |   If-None-Match: "xyzzy", "r2d2xxxx", "c3piozzzz"                         |
// |   If-None-Match: W/"xyzzy", W/"r2d2xxxx", W/"c3piozzzz"                   |
// |   If-None-Match: *                                                        |
// +---------------------------------------------------------------------------+
// | RFC 9110 §13.1.2 If-None-Match (ABNF summary)                             |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | If-None-Match    | "*" / #entity-tag                                      |
// | entity-tag       | [ weak ] opaque-tag                                    |
// | weak             | %s"W/"                                                 |
// | opaque-tag       | DQUOTE *etagc DQUOTE                                   |
// | etagc            | %x21 / %x23-7E / obs-text                              |
// |                  | ; VCHAR except DQUOTE, plus obs-text                   |
// | obs-text         | %x80-FF                                                |
// | DQUOTE           | %x22                                                   |
// | OWS              | *( SP / HTAB )                                         |
// +------------------+--------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | The "#entity-tag" alternative expands as follows:                         |
// |                                                                           |
// | Sender syntax:                                                            |
// |                                                                           |
// |   #entity-tag = [ entity-tag *( OWS "," OWS entity-tag ) ]                |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #entity-tag = [ entity-tag ]                                            |
// |                 *( OWS "," OWS [ entity-tag ] )                           |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "#entity-tag", rather than "1#entity-tag", zero         |
// | non-empty entity-tag elements are permitted by the purely syntactic ABNF. |
// | This applies only to the list alternative; "*" remains a separate         |
// | complete field value.                                                     |
// |                                                                           |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is normalized (no OWS around the value).           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class if_none_match {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // If-None-Match = "*" / #entity-tag
    // The "*" alternative matches the whole field value; it cannot be mixed
    // with, or appear as a member of, the entity-tag list.
    if (sv == "*") return true;
    // Otherwise, If-None-Match is a (possibly empty) list of entity tags.
    return helpers::for_each_list_element(sv, helpers::is_entity_tag);
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
