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

#ifndef martianlabs_doba_protocol_http11_checkers_h_transfer_encoding_h
#define martianlabs_doba_protocol_http11_checkers_h_transfer_encoding_h

#include <ranges>
#include <string_view>

#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] transfer_encoding                                           ( class ) |
// +---------------------------------------------------------------------------+
// | RFC 9112 §6.1 Transfer-Encoding                                           |
// +---------------------------------------------------------------------------+
// | The "Transfer-Encoding" header field lists the transfer coding names      |
// | corresponding to the sequence of transfer codings that have been, or will |
// | be, applied to the content in order to form the message body.             |
// |                                                                           |
// | Transfer codings are a property of the HTTP message rather than a property|
// | of the selected representation. They are primarily used to delimit        |
// | dynamically generated content and to distinguish transformations applied  |
// | during message transfer from representation content codings.              |
// |                                                                           |
// | Transfer codings are listed in the order in which they were applied.      |
// | Consequently, a recipient decodes them in the reverse order.              |
// |                                                                           |
// | A recipient MUST be able to parse the "chunked" transfer coding because   |
// | it has a fundamental role in HTTP/1.1 message framing when the content    |
// | length is not known in advance.                                           |
// |                                                                           |
// | Transfer coding names are case-insensitive.                               |
// |                                                                           |
// | Example:                                                                  |
// |  Transfer-Encoding: gzip, chunked                                         |
// |                                                                           |
// | Example with transfer parameters:                                         |
// |  Transfer-Encoding: custom; level=5, chunked                              |
// +---------------------------------------------------------------------------+
// | RFC 9112 §6.1 / RFC 9110 §10.1.4 (ABNF summary)                           |
// +---------------------------------------------------------------------------+
// +--------------------+------------------------------------------------------+
// | Field              | Definition                                           |
// +--------------------+------------------------------------------------------+
// | Transfer-Encoding  | #transfer-coding                                     |
// | transfer-coding    | token *( OWS ";" OWS transfer-parameter )            |
// | transfer-parameter | token BWS "=" BWS ( token / quoted-string )          |
// | token              | 1*tchar                                              |
// | tchar              | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" /      |
// |                    | "-" / "." / "^" / "_" / "`" / "|" / "~" /            |
// |                    | DIGIT / ALPHA                                        |
// | quoted-string      | DQUOTE *( qdtext / quoted-pair ) DQUOTE              |
// | qdtext             | HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text      |
// | quoted-pair        | "\" ( HTAB / SP / VCHAR / obs-text )                 |
// | OWS                | *( SP / HTAB )                                       |
// | BWS                | OWS                                                  |
// | VCHAR              | %x21-7E                                              |
// | obs-text           | %x80-FF                                              |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 List-Based Fields                                         |
// +---------------------------------------------------------------------------+
// | The "#rule" extension defines a comma-delimited list whose elements can   |
// | be surrounded by optional whitespace:                                     |
// |                                                                           |
// |  1#element = element *( OWS "," OWS element )                             |
// |  #element  = [ 1#element ]                                                |
// |                                                                           |
// | For recipient-side parsing, empty list elements MUST be parsed and        |
// | ignored. The recipient-side expansion is therefore equivalent to:         |
// |                                                                           |
// |  #element = [ element ] *( OWS "," OWS [ element ] )                      |
// |                                                                           |
// | Commas occurring inside a quoted-string are part of that quoted-string    |
// | and MUST NOT be interpreted as list separators.                           |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class transfer_encoding {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv, parsed_parameter_list& out) {
    // The producer overload validates each transfer-coding exactly as the pure
    // check() does and captures every non-empty element (its token plus any
    // transfer-parameters) as raw text, in order.
    return helpers::for_each_list_element(
        sv, [&out](std::string_view element) {
          if (!consume_transfer_coding(element)) return false;
          out.elements.push_back(element);
          return true;
        });
  }

 private:
  // +=========================================================================+
  // | [>] consume_transfer_coding                                 ( private ) |
  // +=========================================================================+
  static constexpr bool consume_transfer_coding(std::string_view sv) {
    std::size_t off = 0;
    const std::string_view token = helpers::consume_token(sv);
    if (token.empty()) return false;
    off += token.size();
    if (off >= sv.size()) return true;
    // transfer-parameters = *( OWS ";" OWS transfer-parameter ) — a parameter
    // is mandatory after every ";" and OWS/BWS is permitted around the "=".
    return helpers::for_each_parameter(
        sv.substr(off), /*require_parameter=*/true,
        [](std::string_view rest, std::size_t& bytes) {
          return helpers::consume_parameter(rest, bytes, /*allow_bws=*/true);
        });
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
