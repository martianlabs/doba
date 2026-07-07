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

#ifndef martianlabs_doba_protocol_http11_checkers_h_if_modified_since_h
#define martianlabs_doba_protocol_http11_checkers_h_if_modified_since_h

#include <ranges>
#include <string_view>

#include "protocol/http11/checkers/headers/date.h"
#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                         if-modified-since |
// +===========================================================================+
// | RFC 9110 §13.1.3 If-Modified-Since                                        |
// +---------------------------------------------------------------------------+
// | The "If-Modified-Since" header field makes a GET or HEAD request          |
// | conditional on the selected representation having been modified after     |
// | the date supplied in the field value. It avoids transferring the          |
// | representation when it has not changed.                                   |
// |                                                                           |
// | A recipient MUST ignore If-Modified-Since when:                           |
// |                                                                           |
// |   - the request also contains If-None-Match;                              |
// |   - the field value is not a valid HTTP-date;                             |
// |   - the field value contains more than one member;                        |
// |   - the request method is neither GET nor HEAD; or                        |
// |   - no modification date is available for the selected resource.          |
// |                                                                           |
// | The timestamp MUST be interpreted relative to the origin server's clock.  |
// |                                                                           |
// | When a request selects a representation and contains If-Modified-Since    |
// | without If-None-Match, the origin server SHOULD evaluate the condition    |
// | before performing the requested method.                                   |
// |                                                                           |
// | If the selected representation's last modification date is earlier than   |
// | or equal to the supplied date, the condition is false; otherwise, it is   |
// | true.                                                                     |
// |                                                                           |
// | An origin server that evaluates a false condition SHOULD NOT perform the  |
// | requested method and SHOULD instead send a 304 (Not Modified) response    |
// | containing the metadata useful for identifying or updating the cached     |
// | response.                                                                 |
// |                                                                           |
// | Typical uses are validating a cached representation that has no entity    |
// | tag and restricting a traversal to resources modified after a given time. |
// |                                                                           |
// | Example:                                                                  |
// |   If-Modified-Since: Sat, 29 Oct 1994 19:43:31 GMT                        |
// +---------------------------------------------------------------------------+
// | RFC 9110 §13.1.3 and §5.6.7 (ABNF summary)                                |
// +---------------------------------------------------------------------------+
// +---------------------+-----------------------------------------------------+
// | Field               | Definition                                          |
// +---------------------+-----------------------------------------------------+
// | If-Modified-Since   | HTTP-date                                           |
// | HTTP-date           | IMF-fixdate / obs-date                              |
// | IMF-fixdate         | day-name "," SP date1 SP time-of-day SP GMT         |
// | day-name            | %s"Mon" / %s"Tue" / %s"Wed" / %s"Thu" /             |
// |                     | %s"Fri" / %s"Sat" / %s"Sun"                         |
// | date1               | day SP month SP year                                |
// | day                 | 2DIGIT                                              |
// | month               | %s"Jan" / %s"Feb" / %s"Mar" / %s"Apr" /             |
// |                     | %s"May" / %s"Jun" / %s"Jul" / %s"Aug" /             |
// |                     | %s"Sep" / %s"Oct" / %s"Nov" / %s"Dec"               |
// | year                | 4DIGIT                                              |
// | time-of-day         | hour ":" minute ":" second                          |
// | hour                | 2DIGIT                                              |
// | minute              | 2DIGIT                                              |
// | second              | 2DIGIT                                              |
// | GMT                 | %s"GMT"                                             |
// | obs-date            | rfc850-date / asctime-date                          |
// | rfc850-date         | day-name-l "," SP date2 SP time-of-day SP GMT       |
// | date2               | day "-" month "-" 2DIGIT                            |
// | day-name-l          | %s"Monday" / %s"Tuesday" / %s"Wednesday" /          |
// |                     | %s"Thursday" / %s"Friday" / %s"Saturday" /          |
// |                     | %s"Sunday"                                          |
// | asctime-date        | day-name SP date3 SP time-of-day SP year            |
// | date3               | month SP ( 2DIGIT / ( SP 1DIGIT ) )                 |
// | SP                  | %x20                                                |
// | DIGIT               | %x30-39                                             |
// +---------------------+-----------------------------------------------------+
// +---------------------------------------------------------------------------+
// | HTTP-date generation and reception requirements                           |
// +---------------------------------------------------------------------------+
// | Senders MUST generate HTTP-date values using IMF-fixdate. Recipients MUST |
// | accept IMF-fixdate, rfc850-date, and asctime-date.                        |
// |                                                                           |
// | HTTP-date is case-sensitive. A sender MUST NOT generate whitespace beyond |
// | the exact SP characters required by the grammar.                          |
// |                                                                           |
// | For an rfc850-date containing a two-digit year, a recipient MUST map a    |
// | date appearing more than 50 years in the future to the most recent past   |
// | year having the same final two digits.                                    |
// +---------------------------------------------------------------------------+
// | IMPORTANT: If-Modified-Since is not a list-valued field. Exactly one      |
// | HTTP-date is permitted. field-value is supposed to be normalized (no OWS  |
// | around value).                                                            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class if_modified_since {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) { return date::check(sv); }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
