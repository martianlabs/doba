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

#ifndef martianlabs_doba_protocol_http11_checkers_h_trailer_h
#define martianlabs_doba_protocol_http11_checkers_h_trailer_h

#include <ranges>

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] trailer                                                     ( class ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 §6.6.2 Trailer                                                   |
// +---------------------------------------------------------------------------+
// | The "Trailer" header field provides a list of field names that the sender |
// | anticipates sending as trailer fields within the same message.            |
// |                                                                           |
// | This allows a recipient to prepare for receipt of the indicated metadata  |
// | before it starts processing the message content.                          |
// |                                                                           |
// | A sender that intends to generate one or more trailer fields SHOULD       |
// | generate a Trailer header field in the header section to indicate which   |
// | fields might be present in the trailer section.                           |
// |                                                                           |
// | The presence of a field name in Trailer does not guarantee that the       |
// | corresponding trailer field will actually be sent.                        |
// |                                                                           |
// | A sender MUST NOT generate a trailer field unless the definition of that  |
// | field explicitly permits it to be sent in the trailer section.            |
// |                                                                           |
// | A recipient MUST NOT merge a trailer field into the header section unless |
// | the field definition explicitly permits and defines a safe merge.         |
// |                                                                           |
// | Example:                                                                  |
// |  Trailer: Example-Checksum, Example-Signature                             |
// +---------------------------------------------------------------------------+
// | RFC 9110 §6.6.2 Trailer and §5.6.1 Lists (ABNF summary)                   |
// +---------------------------------------------------------------------------+
// +--------------------------+------------------------------------------------+
// | Field                    | Definition                                     |
// +--------------------------+------------------------------------------------+
// | Trailer                  | #field-name                                    |
// | field-name               | token                                          |
// | token                    | 1*tchar                                        |
// | tchar                    | "!" / "#" / "$" / "%" / "&" / "'" / "*" /      |
// |                          | "+" / "-" / "." / "^" / "_" / "`" / "|" /      |
// |                          | "~" / DIGIT / ALPHA                            |
// | OWS                      | *( SP / HTAB )                                 |
// +--------------------------+------------------------------------------------+
// | Sender-side expansion                                                     |
// +--------------------------+------------------------------------------------+
// | #field-name              | [ field-name                                   |
// |                          |   *( OWS "," OWS field-name ) ]                |
// +--------------------------+------------------------------------------------+
// | Recipient-side expansion                                                  |
// +--------------------------+------------------------------------------------+
// | #field-name              | [ field-name ]                                 |
// |                          | *( OWS "," OWS [ field-name ] )                |
// +---------------------------------------------------------------------------+
// | Sender requirements:                                                      |
// |                                                                           |
// |   * Empty list elements MUST NOT be generated.                            |
// |   * The complete list MAY be empty because "#" has a minimum cardinality  |
// |     of zero.                                                              |
// |                                                                           |
// | Recipient requirements:                                                   |
// |                                                                           |
// |   * A reasonable number of empty list elements MUST be accepted and       |
// |     ignored.                                                              |
// |   * Therefore, values such as "Foo,,Bar" or ",Foo," are valid when        |
// |     parsing received field values.                                        |
// |   * Because Trailer uses "#" rather than "1#", an empty field value or a  |
// |     value containing only empty list elements is syntactically valid.     |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class trailer {
 public:
  static constexpr bool check(std::string_view sv) {
    for (auto token : sv | std::views::split(',')) {
      if (token.begin() == token.end()) continue;
      std::string_view value(&*token.begin(), std::ranges::distance(token));
      helpers::ows_trim(value);
      if (value.empty()) continue;
      for (auto const& c : value) {
        if (!helpers::is_token(c)) return false;
      }
    }
    return true;
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
