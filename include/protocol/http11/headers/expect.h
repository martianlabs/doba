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
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef martianlabs_doba_protocol_http11_headers_expect_h
#define martianlabs_doba_protocol_http11_headers_expect_h

#include "protocol/http11/connection.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"
#include "protocol/http11/policies.h"
#include "protocol/http11/verdict.h"

namespace martianlabs::doba::protocol::http11::headers {
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
  static bool check(std::string_view sv, parsed_parameter_list& out) {
    // The producer overload validates each expectation exactly as the pure
    // check() does and captures every non-empty element (its token plus any
    // value and parameters) as raw text, in order.
    return helpers::for_each_list_element(sv, [&out](std::string_view element) {
      if (!consume_expectation(element)) return false;
      out.elements.push_back(element);
      return true;
    });
  }
  // +=========================================================================+
  // | [>] interpret                                                ( public ) |
  // +=========================================================================+
  static constexpr verdict interpret(
      const parsed_parameter_list& parameters_list, http11::connection&,
      const policies&) {
    for (const std::string_view expectation : parameters_list.elements) {
      // Each element is an expectation optionally followed by ";"-separated
      // parameters; the leading token is the expectation name.
      const std::string_view name = helpers::consume_token(expectation);
      if (!helpers::iequals(name, "100-continue")) return verdict::kReject;
    }
    return verdict::kAccept;
  }

 private:
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
    const std::string_view value =
        helpers::consume_token_or_quoted_string(sv.substr(i));
    if (value.empty()) return false;
    i += value.size();
    // parameters = *( OWS ";" OWS [ parameter ] ) — empty slots allowed and no
    // whitespace is permitted around the "=" of an expectation parameter.
    return helpers::for_each_parameter(
        sv.substr(i), /*require_parameter=*/false,
        [](std::string_view rest, std::size_t& bytes) {
          return helpers::consume_parameter(rest, bytes, /*allow_bws=*/false);
        });
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
