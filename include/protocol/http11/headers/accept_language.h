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

#ifndef martianlabs_doba_protocol_http11_headers_accept_language_h
#define martianlabs_doba_protocol_http11_headers_accept_language_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                           accept-language |
// +===========================================================================+
// | RFC 9110 §12.5.4 Accept-Language                                          |
// +---------------------------------------------------------------------------+
// | The "Accept-Language" request header field allows a user agent to         |
// | indicate the natural languages preferred for response content.            |
// |                                                                           |
// | Each language range can have an optional quality value ("qvalue") that    |
// | represents the user's relative preference. An omitted weight means q=1,   |
// | while q=0 means that the corresponding range is not acceptable.           |
// |                                                                           |
// | Language ranges and language tags are matched case-insensitively. RFC     |
// | 4647 defines several matching schemes; Basic Filtering is equivalent to   |
// | the matching scheme historically used by HTTP.                            |
// |                                                                           |
// | Some recipients use list order to break ties between ranges having equal  |
// | quality values, but senders cannot rely on that behavior. For maximum     |
// | interoperability, user agents commonly assign distinct quality values and |
// | list ranges in decreasing preference order.                               |
// |                                                                           |
// | A request without Accept-Language expresses no language preference. If no |
// | available representation is acceptable, the origin server can return a    |
// | 406 (Not Acceptable) response or disregard the field.                     |
// |                                                                           |
// | User agents MUST NOT send Accept-Language unless the user can control the |
// | linguistic preference. A detailed language list can also reveal private   |
// | information and increase fingerprinting risk.                             |
// |                                                                           |
// | Examples:                                                                 |
// |   Accept-Language: da                                                     |
// |   Accept-Language: da, en-GB;q=0.8, en;q=0.7                              |
// |   Accept-Language: es-ES, es;q=0.9, *;q=0.1                               |
// +---------------------------------------------------------------------------+
// | RFC 9110 §12.5.4 Accept-Language (ABNF summary)                           |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Accept-Language  | #( language-range [ weight ] )                         |
// | language-range   | (1*8ALPHA *( "-" 1*8alphanum )) / "*"                  |
// | alphanum         | ALPHA / DIGIT                                          |
// | weight           | OWS ";" OWS "q=" qvalue                                |
// | qvalue           | ( "0" [ "." 0*3DIGIT ] ) /                             |
// |                  | ( "1" [ "." 0*3("0") ] )                               |
// | OWS              | *( SP / HTAB )                                         |
// +---------------------------------------------------------------------------+
// | RFC 4647 §2.1 Basic Language Range                                        |
// +---------------------------------------------------------------------------+
// | The first subtag contains 1 to 8 letters. Every subsequent subtag         |
// | contains 1 to 8 letters or digits and is introduced by "-". The wildcard  |
// | "*" is valid only as the complete language-range, not as an internal      |
// | subtag.                                                                   |
// |                                                                           |
// | A basic language range is syntactically valid without being a registered  |
// | or well-formed BCP 47 language tag. Such a range might simply match no    |
// | available language tag. Comparisons are case-insensitive.                 |
// +---------------------------------------------------------------------------+
// | RFC 9110 §12.4.2 Quality Values                                           |
// +---------------------------------------------------------------------------+
// | The qvalue range is 0 through 1. Senders MUST NOT generate more than      |
// | three digits after the decimal point. The ABNF permits "0", "0.",         |
// | "0.123", "1", "1.", and "1.000", but rejects ".5", "0.1234", and          |
// | "1.001". The parameter name "q" is case-insensitive.                      |
// |                                                                           |
// | OWS is permitted before ";" and between ";" and "q", but not around the   |
// | "=" character or between "=" and the qvalue.                              |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #( language-range [ weight ] ) =                                        |
// |       [ ( language-range [ weight ] )                                     |
// |         *( OWS "," OWS ( language-range [ weight ] ) ) ]                  |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #( language-range [ weight ] ) =                                        |
// |       [ ( language-range [ weight ] ) ]                                   |
// |       *( OWS "," OWS [ ( language-range [ weight ] ) ] )                  |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule uses "#", rather than "1#", zero non-empty language-range  |
// | elements are permitted by the purely syntactic ABNF.                      |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class accept_language {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    return helpers::for_each_list_element(sv, consume_language_range_element);
  }

 private:
  // +=========================================================================+
  // | [>] consume_language_range_element                          ( private ) |
  // +=========================================================================+
  static constexpr bool consume_language_range_element(std::string_view sv) {
    std::size_t off = 0;
    const std::string_view range = consume_language_range(sv);
    if (range.empty()) return false;
    off += range.size();
    // If we've consumed the entire string, that's valid.
    if (off >= sv.size()) return true;
    // If there's more content, it must be a weight parameter.
    return helpers::consume_weight(sv.substr(off));
  }
  // +=========================================================================+
  // | [>] consume_language_range                                  ( private ) |
  // +=========================================================================+
  static constexpr std::string_view consume_language_range(
      std::string_view sv) {
    // Special case: "*" as the entire range.
    if (!sv.empty() && sv[0] == '*') return sv.substr(0, 1);
    // Normal case: 1*8ALPHA *( "-" 1*8alphanum )
    std::size_t i = 0;
    // First subtag: 1 to 8 letters.
    std::size_t subtag_start = i;
    while (i < sv.size() && helpers::is_alpha(sv[i]) && i - subtag_start < 8) {
      i++;
    }
    // Must have at least one letter in the first subtag.
    if (i == subtag_start) return std::string_view();
    // Subsequent subtags: "-" followed by 1 to 8 alphanumeric characters.
    while (i < sv.size() && sv[i] == '-') {
      i++;
      subtag_start = i;
      while (i < sv.size() &&
             (helpers::is_alpha(sv[i]) || helpers::is_digit(sv[i])) &&
             i - subtag_start < 8) {
        i++;
      }
      // Must have at least one alphanumeric character after each "-".
      if (i == subtag_start) return std::string_view();
    }
    return sv.substr(0, i);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
