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

#ifndef martianlabs_doba_protocol_http11_checkers_h_acma_h
#define martianlabs_doba_protocol_http11_checkers_h_acma_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                    access-control-max-age |
// +===========================================================================+
// | Fetch Standard §3.3.3 HTTP responses                                      |
// +---------------------------------------------------------------------------+
// | The "Access-Control-Max-Age" response header field indicates the number   |
// | of seconds for which the information provided by the                      |
// | Access-Control-Allow-Methods and Access-Control-Allow-Headers headers can |
// | be cached in the CORS-preflight cache.                                    |
// |                                                                           |
// | This header is only relevant to successful CORS-preflight responses. It   |
// | allows a user agent to reuse the preflight result for subsequent matching |
// | cross-origin requests without issuing another preflight request until the |
// | cached entry expires or is removed.                                       |
// |                                                                           |
// | If the header is absent or cannot be parsed by the CORS-preflight fetch   |
// | algorithm, the Fetch Standard uses a default max-age of 5 seconds. User   |
// | agents may impose an implementation-defined upper limit and clamp larger  |
// | values to that limit.                                                     |
// |                                                                           |
// | The field value is a delta-seconds value: a non-negative decimal integer  |
// | representing a duration in seconds. A leading sign is not part of the     |
// | syntax. Therefore values such as "-1" are not valid according to the ABNF |
// | used by the Fetch Standard.                                               |
// |                                                                           |
// | Leading zeroes are syntactically valid. Range handling, overflow handling,|
// | and user-agent cache limits are semantic processing concerns, not part of |
// | this field-value ABNF check.                                              |
// |                                                                           |
// | Examples:                                                                 |
// |   Access-Control-Max-Age: 0                                               |
// |   Access-Control-Max-Age: 600                                             |
// |   Access-Control-Max-Age: 86400                                           |
// +---------------------------------------------------------------------------+
// | Fetch Standard §3.3.4 HTTP new-header syntax (ABNF summary)               |
// +---------------------------------------------------------------------------+
// +------------------------+--------------------------------------------------+
// | Field                  | Definition                                       |
// +------------------------+--------------------------------------------------+
// | Access-Control-Max-Age | delta-seconds                                    |
// | delta-seconds          | 1*DIGIT                                          |
// | DIGIT                  | %x30-39                                          |
// +---------------------------------------------------------------------------+
// | RFC 9111 §1.2.2 Delta Seconds                                             |
// +---------------------------------------------------------------------------+
// | The "delta-seconds" rule specifies a non-negative integer, representing   |
// | time in seconds:                                                          |
// |                                                                           |
// |   delta-seconds = 1*DIGIT                                                 |
// |                                                                           |
// | A recipient parsing a delta-seconds value and converting it to binary form|
// | ought to use an arithmetic type of at least 31 bits of non-negative       |
// | integer range. Overflow handling is outside the pure syntactic ABNF       |
// | validation performed by this header checker.                              |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class access_control_max_age {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Access-Control-Max-Age = delta-seconds = 1*DIGIT. A leading sign is not
    // part of the syntax and leading zeroes are valid; range/overflow handling
    // is semantic and outside this purely syntactic ABNF check.
    return helpers::is_digits(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
