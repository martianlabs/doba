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
  static constexpr bool check(std::string_view sv) {
    bool follows_separator = false;
    std::size_t i = 0;
    std::size_t last = 0;
    bool inside_string = false;
    while (i < sv.size()) {
      // We need to handle quoted strings and escaped characters properly.
      if (sv[i] == '"') {
        inside_string = !inside_string;
        i++;
        continue;
      }
      // Handle escaped characters inside quoted strings.
      if (sv[i] == '\\') {
        if (!inside_string || i + 1 >= sv.size()) return false;
        i += 2;
        continue;
      }
      if (sv[i] == ',' && !inside_string) {
        // We found a transfer coding, let's consume it.
        std::string_view tcs = sv.substr(last, i++ - last);
        if (follows_separator) helpers::ows_ltrim(tcs);
        helpers::ows_rtrim(tcs);
        if (!tcs.empty() && !consume_t_codings(tcs)) return false;
        follows_separator = true;
        last = i;
        continue;
      }
      i++;
    }
    // Check if we ended inside a quoted string, which would be invalid.
    if (inside_string) return false;
    // Last transfer coding after the last comma (or the only one if no commas).
    std::string_view tcs = sv.substr(last);
    if (follows_separator) helpers::ows_ltrim(tcs);
    if (!tcs.empty() && !consume_t_codings(tcs)) return false;
    return true;
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
    return consume_transfer_parameters(sv.substr(off));
  }
  // +=========================================================================+
  // | [>] consume_transfer_parameters                             ( private ) |
  // +=========================================================================+
  static constexpr bool consume_transfer_parameters(std::string_view sv) {
    std::size_t i = 0;
    bool q_found = false;
    while (i < sv.size()) {
      while (i < sv.size() && helpers::is_ows(sv[i])) i++; // OWS before ';'
      // A ';' is mandatory before every transfer-parameter.
      if (i >= sv.size()) return false;
      if (sv[i++] != ';') return false;
      while (i < sv.size() && helpers::is_ows(sv[i])) i++; // OWS after ';'
      // A transfer-parameter is mandatory after every ';'.
      if (i >= sv.size()) return false;
      std::size_t bytes = 0;
      if (!consume_parameter(sv.substr(i), bytes, q_found)) return false;
      if (bytes == 0 || bytes > sv.size() - i) {
        return false;
      }
      i += bytes;
    }
    return true;
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
    } else {
      if (q_found) return false;  // "q" parameter must be last if present.
      while (i < sv.size() && helpers::is_ows(sv[i])) i++; // OWS before '='
      // A '=' is mandatory before every transfer-parameter.
      if (i >= sv.size()) return false;
      if (sv[i++] != '=') return false;
      while (i < sv.size() && helpers::is_ows(sv[i])) i++; // OWS after '='
      // A transfer-parameter value is mandatory after every '='.
      if (i >= sv.size()) return false;
      // The value can be either a token or a quoted-string.
      if (sv[i] == '"') {
        // Consume the quoted-string.
        const std::string_view qs =
            helpers::consume_quoted_string(sv.substr(i));
        if (qs.empty()) return false;
        bytes_used = i + qs.size();
        return true;
      }
      // Consume the token.
      const std::string_view tk = helpers::consume_token(sv.substr(i));
      if (tk.empty()) return false;
      bytes_used = i + tk.size();
    }
    return true;
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
