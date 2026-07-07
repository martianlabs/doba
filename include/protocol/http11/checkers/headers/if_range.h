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

#ifndef martianlabs_doba_protocol_http11_checkers_h_if_range_h
#define martianlabs_doba_protocol_http11_checkers_h_if_range_h

#include <ranges>
#include <string_view>

#include "protocol/http11/checkers/headers/date.h"
#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                  if-range |
// +===========================================================================+
// | RFC 9110 §13.1.5 If-Range                                                 |
// +---------------------------------------------------------------------------+
// | The "If-Range" header field provides a conditional mechanism specifically |
// | for requests that also contain a Range header field. It allows a client   |
// | holding a partial representation to request the remaining range(s) while  |
// | ensuring that those ranges belong to the same representation.             |
// |                                                                           |
// | Informally, its meaning is:                                               |
// |                                                                           |
// |   If the selected representation is unchanged, send the requested range;  |
// |   otherwise, ignore Range and send the entire current representation.     |
// |                                                                           |
// | Unlike If-Match or If-Unmodified-Since, a failed If-Range condition does  |
// | not result in a 412 (Precondition Failed) response. Instead, the Range    |
// | header field is ignored, normally resulting in a 200 (OK) response with   |
// | the complete selected representation.                                     |
// |                                                                           |
// | If the condition evaluates to true, the recipient SHOULD process the      |
// | Range header field as requested. If the range is applicable and           |
// | satisfiable, this normally results in a 206 (Partial Content) response.   |
// |                                                                           |
// | A client MUST NOT generate If-Range in a request that does not contain a  |
// | Range header field. A server MUST ignore If-Range when Range is absent or |
// | when the target resource does not support range requests.                 |
// |                                                                           |
// | The validator can be either:                                              |
// |                                                                           |
// |   * An entity tag corresponding to the previously received representation.|
// |   * The Last-Modified date of that representation.                        |
// |                                                                           |
// | A client MUST NOT generate a weak entity tag in If-Range. Although the    |
// | generic entity-tag ABNF syntactically admits the "W/" prefix, a weak tag  |
// | is forbidden here by a semantic MUST NOT requirement and cannot satisfy   |
// | the required strong comparison.                                           |
// |                                                                           |
// | A client MUST NOT generate an HTTP-date validator unless it has no entity |
// | tag for the corresponding representation and the date qualifies as a      |
// | strong validator according to RFC 9110 §8.8.2.2.                          |
// |                                                                           |
// | Entity-tag evaluation uses the strong comparison function. The condition  |
// | is true only when the supplied entity tag and the current ETag are both   |
// | strong and their opaque tags match character-for-character.               |
// |                                                                           |
// | HTTP-date evaluation uses exact equality, not an earlier-than-or-equal    |
// | comparison. The condition is true only when the date                      |
// | is a strong validator and exactly matches the selected                    |
// | representation's Last-Modified value.                                     |
// |                                                                           |
// | Examples:                                                                 |
// |                                                                           |
// |   If-Range: "67ab43"                                                      |
// |   If-Range: Sat, 29 Oct 1994 19:43:31 GMT                                 |
// |                                                                           |
// | Syntactically admitted by entity-tag but forbidden for a sender:          |
// |                                                                           |
// |   If-Range: W/"67ab43"                                                    |
// +---------------------------------------------------------------------------+
// | RFC 9110 §13.1.5 If-Range (ABNF summary)                                  |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | If-Range         | entity-tag / HTTP-date                                 |
// +------------------+--------------------------------------------------------+
// | entity-tag       | [ weak ] opaque-tag                                    |
// | weak             | %s"W/"                                                 |
// | opaque-tag       | DQUOTE *etagc DQUOTE                                   |
// | etagc            | %x21 / %x23-7E / obs-text                              |
// |                  | ; VCHAR except DQUOTE, plus obs-text                   |
// | obs-text         | %x80-FF                                                |
// +------------------+--------------------------------------------------------+
// | HTTP-date        | IMF-fixdate / obs-date                                 |
// | IMF-fixdate      | day-name "," SP date1 SP time-of-day SP GMT            |
// | day-name         | %s"Mon" / %s"Tue" / %s"Wed" / %s"Thu" / %s"Fri" /      |
// |                  | %s"Sat" / %s"Sun"                                      |
// | date1            | day SP month SP year                                   |
// | day              | 2DIGIT                                                 |
// | month            | %s"Jan" / %s"Feb" / %s"Mar" / %s"Apr" / %s"May" /      |
// |                  | %s"Jun" / %s"Jul" / %s"Aug" / %s"Sep" / %s"Oct" /      |
// |                  | %s"Nov" / %s"Dec"                                      |
// | year             | 4DIGIT                                                 |
// | GMT              | %s"GMT"                                                |
// | time-of-day      | hour ":" minute ":" second                             |
// | hour             | 2DIGIT                                                 |
// | minute           | 2DIGIT                                                 |
// | second           | 2DIGIT                                                 |
// +------------------+--------------------------------------------------------+
// | obs-date         | rfc850-date / asctime-date                             |
// | rfc850-date      | day-name-l "," SP date2 SP time-of-day SP GMT          |
// | date2            | day "-" month "-" 2DIGIT                               |
// | day-name-l       | %s"Monday" / %s"Tuesday" / %s"Wednesday" /             |
// |                  | %s"Thursday" / %s"Friday" / %s"Saturday" /             |
// |                  | %s"Sunday"                                             |
// | asctime-date     | day-name SP date3 SP time-of-day SP year               |
// | date3            | month SP ( 2DIGIT / ( SP 1DIGIT ) )                    |
// +------------------+--------------------------------------------------------+
// | DQUOTE           | %x22                                                   |
// | SP               | %x20                                                   |
// | DIGIT            | %x30-39                                                |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.7 HTTP-date requirements                                    |
// +---------------------------------------------------------------------------+
// | A recipient parsing HTTP-date MUST accept all three date formats:         |
// |                                                                           |
// |   * IMF-fixdate                                                           |
// |   * rfc850-date                                                           |
// |   * asctime-date                                                          |
// |                                                                           |
// | A sender MUST generate HTTP-date using IMF-fixdate. The obsolete formats  |
// | exist only for compatibility with older implementations. HTTP-date is     |
// | case-sensitive and does not allow whitespace beyond the SP characters     |
// | explicitly present in its grammar.                                        |
// +---------------------------------------------------------------------------+
// | If-Range is not a list-based field. Its value contains exactly one        |
// | entity-tag or one HTTP-date; the "#rule" list expansion does not apply.   |
// | A comma can nevertheless occur as a required part of an HTTP-date.        |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class if_range {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // If-Range = entity-tag / HTTP-date
    // Try entity-tag first (starts with optional "W/" followed by DQUOTE).
    if (helpers::is_entity_tag(sv)) return true;
    // Otherwise, try HTTP-date (reuses the date checker).
    return date::check(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
