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

#ifndef martianlabs_doba_protocol_http11_checkers_h_max_forwards_h
#define martianlabs_doba_protocol_http11_checkers_h_max_forwards_h

#include <cstddef>

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                              max-forwards |
// +===========================================================================+
// | RFC 9110 §7.6.2 Max-Forwards                                              |
// +---------------------------------------------------------------------------+
// | The "Max-Forwards" header field provides a mechanism with the TRACE and   |
// | OPTIONS request methods to limit the number of times that a request is    |
// | forwarded by proxies. This is useful when a client is attempting to trace |
// | a request that appears to be failing or looping mid-chain.                |
// |                                                                           |
// | The Max-Forwards value is a decimal integer indicating the remaining      |
// | number of times that the request message can be forwarded.                |
// |                                                                           |
// | Each intermediary that receives a TRACE or OPTIONS request containing a   |
// | Max-Forwards header field MUST check and update its value before          |
// | forwarding the request.                                                   |
// |                                                                           |
// | If the received value is zero (0), the intermediary MUST NOT forward the  |
// | request; instead, it MUST respond as the final recipient.                 |
// |                                                                           |
// | If the received value is greater than zero, the intermediary MUST         |
// | generate an updated Max-Forwards field in the forwarded message. The new  |
// | value is the lesser of:                                                   |
// |                                                                           |
// |   a) the received value decremented by one (1), or                        |
// |   b) the recipient's maximum supported value for Max-Forwards.            |
// |                                                                           |
// | A recipient MAY ignore a Max-Forwards header field received with any      |
// | request method other than TRACE or OPTIONS.                               |
// |                                                                           |
// | Examples:                                                                 |
// |   Max-Forwards: 0                                                         |
// |   Max-Forwards: 1                                                         |
// |   Max-Forwards: 10                                                        |
// +---------------------------------------------------------------------------+
// | RFC 9110 §7.6.2 Max-Forwards (ABNF summary)                               |
// +---------------------------------------------------------------------------+
// +--------------+------------------------------------------------------------+
// | Field        | Definition                                                 |
// +--------------+------------------------------------------------------------+
// | Max-Forwards | 1*DIGIT                                                    |
// | DIGIT        | "0" / "1" / "2" / "3" / "4" / "5" / "6" / "7" / "8" /      |
// |              | "9"                                                        |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class max_forwards {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv, std::size_t& out) {
    // The producer overload validates the same 1*DIGIT grammar and, on
    // success, yields the decimal value through the shared parse_size_t, which
    // rejects a value that would overflow std::size_t.
    return helpers::parse_size_t(sv, out);
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
