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

#ifndef martianlabs_doba_protocol_http11_checkers_h_expect_h
#define martianlabs_doba_protocol_http11_checkers_h_expect_h

#include <ranges>

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] expect                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | RFC 9110 §10.1.1 Expect                                                   |
// +---------------------------------------------------------------------------+
// | The "Expect" header field in a request indicates a set of behaviors       |
// | ("expectations") that need to be supported by the server in order to      |
// | properly handle the request.                                              |
// |                                                                           |
// | The field value is a comma-separated list of expectations. Each           |
// | expectation consists of a token and can optionally contain a value and    |
// | zero or more parameters.                                                  |
// |                                                                           |
// | The Expect field value is case-insensitive.                               |
// |                                                                           |
// | The only expectation defined by RFC 9110 is "100-continue", with no       |
// | defined value or parameters.                                              |
// |                                                                           |
// | Recognition of "100-continue", validation of request content, and the     |
// | generation of status code 417 (Expectation Failed) are semantic concerns  |
// | and are outside the scope of ABNF syntax validation.                      |
// |                                                                           |
// | Examples:                                                                 |
// |  Expect: 100-continue                                                     |
// |  Expect: custom=value                                                     |
// |  Expect: custom="quoted value"; parameter=value                           |
// +---------------------------------------------------------------------------+
// | RFC 9110 §10.1.1 Expect (ABNF)                                            |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Expect           | #expectation                                           |
// | expectation      | token [ "=" ( token / quoted-string ) parameters ]     |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 List Extension                                            |
// +---------------------------------------------------------------------------+
// | #expectation     | [ expectation *( OWS "," OWS expectation ) ]           |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.6 Parameters                                                |
// +---------------------------------------------------------------------------+
// | parameters       | *( OWS ";" OWS [ parameter ] )                         |
// | parameter        | parameter-name "=" parameter-value                     |
// | parameter-name   | token                                                  |
// | parameter-value  | token / quoted-string                                  |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.2 Tokens                                                    |
// +---------------------------------------------------------------------------+
// | token            | 1*tchar                                                |
// | tchar            | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" /        |
// |                  | "-" / "." / "^" / "_" / "`" / "|" / "~" / DIGIT /      |
// |                  | ALPHA                                                  |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.4 Quoted Strings                                            |
// +---------------------------------------------------------------------------+
// | quoted-string    | DQUOTE *( qdtext / quoted-pair ) DQUOTE                |
// | qdtext           | HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text        |
// | quoted-pair      | "\" ( HTAB / SP / VCHAR / obs-text )                   |
// | obs-text         | %x80-FF                                                |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.3 Whitespace                                                |
// +---------------------------------------------------------------------------+
// | OWS              | *( SP / HTAB )                                         |
// +---------------------------------------------------------------------------+
// | RFC 5234 Appendix B.1 Core Rules                                          |
// +---------------------------------------------------------------------------+
// | ALPHA            | %x41-5A / %x61-7A                                      |
// | DIGIT            | %x30-39                                                |
// | DQUOTE           | %x22                                                   |
// | HTAB             | %x09                                                   |
// | SP               | %x20                                                   |
// | VCHAR            | %x21-7E                                                |
// +---------------------------------------------------------------------------+
// | IMPORTANT: commas inside quoted-string do not delimit list elements.      |
// |                                                                           |
// | IMPORTANT: the "=" following expectation and the "=" inside parameter do  |
// | not allow surrounding whitespace.                                         |
// |                                                                           |
// | IMPORTANT: parameters permits empty parameter slots after semicolons      |
// | because parameter is enclosed in brackets: [ parameter ].                 |
// |                                                                           |
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class expect {
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
        // We found an expectation, let's parse it.
        std::string_view expectation = sv.substr(last, i++ - last);
        if (follows_separator) helpers::ows_ltrim(expectation);
        helpers::ows_rtrim(expectation);
        if (!expectation.empty() && !consume_expectation(expectation)) {
          return false;
        }
        follows_separator = true;
        last = i;
        continue;
      }
      i++;
    }
    // Check if we ended inside a quoted string, which would be invalid.
    if (inside_string) return false;
    // Last expectation after the last comma (or the only one if no commas).
    std::string_view expectation = sv.substr(last);
    if (follows_separator) helpers::ows_ltrim(expectation);
    if (!expectation.empty() && !consume_expectation(expectation)) {
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] consume_expectation                                     ( private ) |
  // +=========================================================================+
  static constexpr bool consume_expectation(std::string_view sv) {
    std::size_t i = 0;
    const std::string_view token = helpers::consume_token(sv);
    if (token.empty()) return false;
    i += token.size();
    if (i >= sv.size()) return true;
    if (sv[i++] != '=') return false;
    if (i >= sv.size()) return false;
    // The value can be either a token or a quoted-string.
    if (sv[i] == '"') {
      // Parse the quoted-string.
      const std::string_view qs = helpers::consume_quoted_string(sv.substr(i));
      if (qs.empty()) return false;
      i += qs.size();
    } else {
      // Parse the token.
      const std::string_view tk = helpers::consume_token(sv.substr(i));
      if (tk.empty()) return false;
      i += tk.size();
    }
    return consume_expectation_parameters(sv.substr(i));
  }
  // +=========================================================================+
  // | [>] consume_expectation_parameters                          ( private ) |
  // +=========================================================================+
  static constexpr bool consume_expectation_parameters(std::string_view sv) {
    std::size_t i = 0;
    while (i < sv.size()) {
      // OWS before ';'.
      while (i < sv.size() && helpers::is_ows(sv[i])) i++;
      // A ';' is mandatory before every parameter.
      if (i >= sv.size()) return false;
      if (sv[i++] != ';') return false;
      // OWS after ';'.
      while (i < sv.size() && helpers::is_ows(sv[i])) i++;
      // A parameter is not mandatory after every ';'.
      if (i >= sv.size()) return true;
      if (sv[i] == ';') continue;
      std::size_t bytes = 0;
      if (!consume_parameter(sv.substr(i), bytes)) return false;
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
                                          std::size_t& bytes_used) {
    bytes_used = 0;
    std::size_t i = 0;
    const std::string_view name = helpers::consume_token(sv);
    if (name.empty()) return false;
    i += name.size();
    // A '=' is mandatory after every parameter name.
    if (i >= sv.size()) return false;
    if (sv[i++] != '=') return false;
    // A parameter value is mandatory after every '='.
    if (i >= sv.size()) return false;
    // The value can be either a token or a quoted-string.
    if (sv[i] == '"') {
      // Parse the quoted-string.
      const std::string_view qs = helpers::consume_quoted_string(sv.substr(i));
      if (qs.empty()) return false;
      bytes_used = i + qs.size();
      return true;
    }
    // Parse the token.
    const std::string_view tk = helpers::consume_token(sv.substr(i));
    if (tk.empty()) return false;
    bytes_used = i + tk.size();
    return true;
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
