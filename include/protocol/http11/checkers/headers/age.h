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

#ifndef martianlabs_doba_protocol_http11_checkers_h_age_h
#define martianlabs_doba_protocol_http11_checkers_h_age_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                       age |
// +===========================================================================+
// | RFC 9111 §5.1 Age                                                         |
// +---------------------------------------------------------------------------+
// | The "Age" response header field conveys the sender's estimate of the time |
// | since the response was generated or successfully validated at the origin  |
// | server. Age values are calculated as specified by the HTTP caching age    |
// | calculation rules.                                                        |
// |                                                                           |
// | The field value is a non-negative integer representing time in seconds.   |
// |                                                                           |
// | Age is a singleton header field. It is not defined as a comma-separated   |
// | list and senders MUST NOT generate list-based Age values.                 |
// |                                                                           |
// | A cache encountering a message with a list-based Age field value SHOULD   |
// | use the first member and discard subsequent members. If the resulting     |
// | value is invalid, the cache SHOULD ignore the field.                      |
// |                                                                           |
// | The presence of an Age header field implies that the response was not     |
// | generated or validated by the origin server for this request. However,    |
// | the absence of an Age header field does not imply that the origin server  |
// | was contacted.                                                            |
// |                                                                           |
// | Examples:                                                                 |
// |   Age: 0                                                                  |
// |   Age: 60                                                                 |
// |   Age: 3600                                                               |
// +---------------------------------------------------------------------------+
// | RFC 9111 §5.1 Age (ABNF summary)                                          |
// +---------------------------------------------------------------------------+
// +---------------+-----------------------------------------------------------+
// | Field         | Definition                                                |
// +---------------+-----------------------------------------------------------+
// | Age           | delta-seconds                                             |
// | delta-seconds | 1*DIGIT                                                   |
// | DIGIT         | %x30-39                                                   |
// +---------------------------------------------------------------------------+
// | RFC 9111 §1.2.2 Delta Seconds                                             |
// +---------------------------------------------------------------------------+
// | The delta-seconds rule specifies a non-negative integer, representing     |
// | time in seconds.                                                          |
// |                                                                           |
// |   delta-seconds = 1*DIGIT                                                 |
// |                                                                           |
// | Recipients converting delta-seconds to binary form ought to use an        |
// | arithmetic type with at least 31 bits of non-negative integer range. If   |
// | the received value is greater than the greatest representable integer,    |
// | or if a later calculation overflows, a cache MUST consider the value to   |
// | be 2147483648 (2^31) or the greatest positive integer it can conveniently |
// | represent.                                                                |
// |                                                                           |
// | This overflow rule is semantic processing. The syntactic ABNF still       |
// | accepts any non-empty sequence of decimal digits.                         |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class age {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Age = delta-seconds = 1*DIGIT.
    // Overflow handling (clamping to 2^31) is semantic processing; the ABNF
    // accepts any non-empty run of decimal digits.
    return helpers::is_digits(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
