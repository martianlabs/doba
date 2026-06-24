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
// +===========================================================================+
// |                                                                    expect |
// +===========================================================================+
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
class expect {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv) {
    bool follows_separator = false;
    while (true) {
      std::size_t separator = std::string_view::npos;
      if (!helpers::find_list_separator(sv, separator)) return false;
      const bool has_separator = separator != std::string_view::npos;
      std::string_view expectation =
          has_separator ? sv.substr(0, separator) : sv;
      // OWS after the previous comma belongs to the list separator.
      if (follows_separator) helpers::ows_ltrim(expectation);
      // OWS before the current comma belongs to the list separator.
      if (has_separator) helpers::ows_rtrim(expectation);
      // Empty list elements are ignored by recipients.
      if (!expectation.empty() && !try_to_parse_expectation(expectation)) {
        return false;
      }
      if (!has_separator) return true;
      sv.remove_prefix(separator + 1);
      follows_separator = true;
    }
  }

 private:
  // +=========================================================================+
  // | [>] try_to_parse_expect_parameter                           ( private ) |
  // +=========================================================================+
  static bool try_to_parse_expect_parameter(std::string_view sv,
                                            std::size_t& bytes_used) {
    bytes_used = 0;
    const std::string_view parameter_name = helpers::parse_token(sv);
    if (parameter_name.empty()) return false;
    sv.remove_prefix(parameter_name.size());
    bytes_used += parameter_name.size();
    if (sv.empty() || sv.front() != '=') return false;
    sv.remove_prefix(1);
    bytes_used++;
    if (sv.empty()) return false;
    if (sv.front() == '"') {
      // parameter-value = quoted-string
      const std::string_view parameter_value = helpers::parse_quoted_string(sv);
      if (parameter_value.empty()) return false;
      sv.remove_prefix(parameter_value.size());
      bytes_used += parameter_value.size();
    } else {
      // parameter-value = token
      const std::string_view parameter_value = helpers::parse_token(sv);
      if (parameter_value.empty()) return false;
      sv.remove_prefix(parameter_value.size());
      bytes_used += parameter_value.size();
    }
    return true;
  }
  // +=========================================================================+
  // | [>] try_to_parse_expect_parameters                          ( private ) |
  // +=========================================================================+
  static bool try_to_parse_expect_parameters(std::string_view sv) {
    while (!sv.empty()) {
      // OWS is valid here only when followed by ';'.
      while (!sv.empty() && helpers::is_ows(sv.front())) sv.remove_prefix(1);
      if (sv.empty() || sv.front() != ';') return false;
      sv.remove_prefix(1);
      // OWS following ';'.
      while (!sv.empty() && helpers::is_ows(sv.front())) sv.remove_prefix(1);
      // The optional parameter is absent.
      if (sv.empty()) return true;
      // Empty parameter followed by another parameter repetition.
      if (sv.front() == ';') continue;
      std::size_t bytes_used = 0;
      if (!try_to_parse_expect_parameter(sv, bytes_used)) return false;
      if (bytes_used == 0 || bytes_used > sv.size()) return false;
      sv.remove_prefix(bytes_used);
    }
    return true;
  }
  // +=========================================================================+
  // | [>] try_to_parse_expectation                                ( private ) |
  // +=========================================================================+
  static bool try_to_parse_expectation(std::string_view sv) {
    if (sv.empty()) return false;
    const std::string_view expectation_name = helpers::parse_token(sv);
    if (expectation_name.empty()) return false;
    sv.remove_prefix(expectation_name.size());
    if (sv.empty()) return true;
    if (sv.front() != '=') return false;
    sv.remove_prefix(1);
    if (sv.empty()) return false;
    if (sv.front() == '"') {
      // expectation = quoted-string
      const std::string_view expectation_value =
          helpers::parse_quoted_string(sv);
      if (expectation_value.empty()) return false;
      sv.remove_prefix(expectation_value.size());
    } else {
      // expectation = token
      const std::string_view expectation_value = helpers::parse_token(sv);
      if (expectation_value.empty()) return false;
      sv.remove_prefix(expectation_value.size());
    }
    return try_to_parse_expect_parameters(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
