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

#ifndef martianlabs_doba_protocol_http11_checkers_h_te_h
#define martianlabs_doba_protocol_http11_checkers_h_te_h

#include <ranges>

#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] te                                                          ( class ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 §10.1.4 TE                                                       |
// +---------------------------------------------------------------------------+
// | The "TE" request header field describes the client's capabilities with    |
// | regard to transfer codings and trailer sections.                          |
// |                                                                           |
// | A member named "trailers" indicates that the client will not discard      |
// | trailer fields received in the response.                                  |
// |                                                                           |
// | Any other member identifies a transfer coding that the client is able to  |
// | accept. A transfer coding can contain parameters and an optional weight   |
// | that indicates the client's relative preference for that coding.          |
// |                                                                           |
// | A weight of 0 means "not acceptable". If no weight is present, the        |
// | default weight is 1.                                                      |
// |                                                                           |
// | A sender of TE MUST also include "TE" as a connection option in the       |
// | Connection header field, preventing intermediaries from forwarding it.    |
// |                                                                           |
// | Examples:                                                                 |
// |  TE: trailers                                                             |
// |  TE: gzip                                                                 |
// |  TE: deflate; q=0.5, trailers                                             |
// |  Connection: TE                                                           |
// +---------------------------------------------------------------------------+
// | RFC 9110 §10.1.4 TE (ABNF summary)                                        |
// +---------------------------------------------------------------------------+
// +--------------------+------------------------------------------------------+
// | Field              | Definition                                           |
// +--------------------+------------------------------------------------------+
// | TE                 | #t-codings                                           |
// |                    |                                                      |
// | Expanded:          | [ t-codings *( OWS "," OWS t-codings ) ]             |
// |                    |                                                      |
// | t-codings          | "trailers" / ( transfer-coding [ weight ] )          |
// | transfer-coding    | token *( OWS ";" OWS transfer-parameter )            |
// | transfer-parameter | token BWS "=" BWS ( token / quoted-string )          |
// | weight             | OWS ";" OWS "q=" qvalue                              |
// | qvalue             | ( "0" [ "." 0*3DIGIT ] ) /                           |
// |                    | ( "1" [ "." 0*3("0") ] )                             |
// | token              | 1*tchar                                              |
// | tchar              | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" /      |
// |                    | "-" / "." / "^" / "_" / "`" / "|" / "~" / DIGIT /    |
// |                    | ALPHA                                                |
// | quoted-string      | DQUOTE *( qdtext / quoted-pair ) DQUOTE              |
// | qdtext             | HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text      |
// | quoted-pair        | "\" ( HTAB / SP / VCHAR / obs-text )                 |
// | OWS                | *( SP / HTAB )                                       |
// | BWS                | OWS                                                  |
// | obs-text           | %x80-FF                                              |
// | DQUOTE             | %x22                                                 |
// +---------------------------------------------------------------------------+
// | ABNF literal strings are case-insensitive unless explicitly specified     |
// | otherwise. Therefore, "trailers" and "q=" are case-insensitive.           |
// |                                                                           |
// | Because TE uses the #rule, an empty field value is syntactically valid.   |
// |                                                                           |
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class te {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv, parsed_parameter_list& out) {
    // The producer overload validates each t-codings element exactly as the
    // pure check() does and captures every non-empty element ("trailers" or a
    // transfer-coding with its optional parameters/weight) as raw text.
    return helpers::for_each_list_element(
        sv, [&out](std::string_view element) {
          if (!consume_t_codings(element)) return false;
          out.elements.push_back(element);
          return true;
        });
  }
  // +=========================================================================+
  // | [>] consume_t_codings                                       ( private ) |
  // +=========================================================================+
  static constexpr bool consume_t_codings(std::string_view sv) {
    if (sv.size() == 8 && helpers::iequals(sv, "trailers")) return true;
    return consume_transfer_coding(sv);
  }
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
    // is mandatory after every ";" and OWS/BWS is permitted around the "=". The
    // "q" ranking parameter is handled specially: it may appear at most once
    // and, if present, must be the last parameter.
    bool q_found = false;
    return helpers::for_each_parameter(
        sv.substr(off), /*require_parameter=*/true,
        [&q_found](std::string_view rest, std::size_t& bytes) {
          return consume_parameter(rest, bytes, q_found);
        });
  }
  // +=========================================================================+
  // | [>] consume_parameter                                       ( private ) |
  // +=========================================================================+
  static constexpr bool consume_parameter(std::string_view sv,
                                          std::size_t& bytes_used,
                                          bool& q_found) {
    bytes_used = 0;
    std::size_t i = 0;
    const std::string_view name = helpers::consume_token(sv);
    if (name.empty()) return false;
    i += name.size();
    if (helpers::iequals(name, "q")) {
      if (q_found) return false;  // "q" parameter can only appear once.
      if (i >= sv.size()) return false;
      if (sv[i++] != '=') return false;
      if (i >= sv.size()) return false;
      if (!helpers::is_qvalue(sv.substr(i))) return false;
      bytes_used = sv.size();
      q_found = true;
      return true;
    }
    if (q_found) return false;  // "q" parameter must be last if present.
    // Regular transfer-parameter: token or quoted-string, OWS/BWS around '='.
    return helpers::consume_parameter(sv, bytes_used, /*allow_bws=*/true);
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
