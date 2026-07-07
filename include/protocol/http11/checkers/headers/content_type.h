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

#ifndef martianlabs_doba_protocol_http11_checkers_h_content_type_h
#define martianlabs_doba_protocol_http11_checkers_h_content_type_h

#include <ranges>

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                              content-type |
// +===========================================================================+
// | RFC 9110 §8.3 Content-Type                                                |
// +---------------------------------------------------------------------------+
// | The "Content-Type" header field indicates the media type of the           |
// | associated representation. It identifies both the representation's data   |
// | format and how that data is intended to be processed by a recipient,      |
// | after any content codings identified by Content-Encoding are decoded.     |
// |                                                                           |
// | A sender that generates a message containing content SHOULD generate a    |
// | Content-Type header field unless the intended media type is unknown.      |
// |                                                                           |
// | If Content-Type is absent, a recipient MAY assume                         |
// | "application/octet-stream" or examine the representation data to          |
// | determine its type. Content inspection ("MIME sniffing") can produce      |
// | incorrect conclusions and expose additional security risks.               |
// |                                                                           |
// | Content-Type is a singleton field. It is not a comma-separated list.      |
// | Multiple field instances can be combined into an invalid list-like value; |
// | differing recovery strategies can create interoperability and security    |
// | issues.                                                                   |
// |                                                                           |
// | The type and subtype are case-insensitive. Parameter names are also       |
// | case-insensitive, while parameter values can be case-sensitive depending  |
// | on the semantics defined for each parameter.                              |
// |                                                                           |
// | Examples:                                                                 |
// |   Content-Type: text/html                                                 |
// |   Content-Type: text/html; charset=utf-8                                  |
// |   Content-Type: application/json                                          |
// |   Content-Type: multipart/form-data; boundary="example-boundary"          |
// +---------------------------------------------------------------------------+
// | RFC 9110 §8.3 and §8.3.1 Content-Type / Media Type (ABNF summary)         |
// +---------------------------------------------------------------------------+
// +-----------------+---------------------------------------------------------+
// | Field           | Definition                                              |
// +-----------------+---------------------------------------------------------+
// | Content-Type    | media-type                                              |
// | media-type      | type "/" subtype parameters                             |
// | type            | token                                                   |
// | subtype         | token                                                   |
// | parameters      | *( OWS ";" OWS [ parameter ] )                          |
// | parameter       | parameter-name "=" parameter-value                      |
// | parameter-name  | token                                                   |
// | parameter-value | token / quoted-string                                   |
// | token           | 1*tchar                                                 |
// | tchar           | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /   |
// |                 | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA       |
// | quoted-string   | DQUOTE *( qdtext / quoted-pair ) DQUOTE                 |
// | qdtext          | HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text         |
// | quoted-pair     | "\" ( HTAB / SP / VCHAR / obs-text )                    |
// | obs-text        | %x80-FF                                                 |
// | OWS             | *( SP / HTAB )                                          |
// | DQUOTE          | %x22                                                    |
// | VCHAR           | %x21-7E                                                 |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.6 parameter rules                                           |
// +---------------------------------------------------------------------------+
// | Parameters are introduced by a semicolon and consist of a name/value      |
// | pair. Parameter names and unquoted parameter values are tokens. A value   |
// | that is not representable as a token can be carried as a quoted-string.   |
// |                                                                           |
// | No whitespace is allowed around the "=" character, not even BWS:          |
// |                                                                           |
// |   text/html; charset=utf-8       valid                                    |
// |   text/html; charset="utf-8"     valid                                    |
// |   text/html; charset =utf-8      invalid                                  |
// |   text/html; charset= utf-8      invalid                                  |
// |                                                                           |
// | The quoted and unquoted forms are equivalent when the value matches the   |
// | token production. An empty quoted-string is syntactically valid, whereas  |
// | an unquoted parameter value cannot be empty because token is 1*tchar.     |
// |                                                                           |
// | The "[ parameter ]" component is optional in the RFC grammar. Therefore,  |
// | the purely syntactic ABNF permits empty parameter slots, including a      |
// | trailing semicolon or consecutive semicolons:                             |
// |                                                                           |
// |   text/plain;                                                             |
// |   text/plain;;charset=utf-8                                               |
// |                                                                           |
// | This tolerance does not make an empty parameter semantically meaningful.  |
// +---------------------------------------------------------------------------+
// | IMPORTANT: Content-Type is a singleton field, not a "#rule" list.         |
// | A comma is therefore not a media-type separator and is only valid when it |
// | occurs inside quoted-string as qdtext or by means of quoted-pair.         |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class content_type {
 public:
  static constexpr bool check(std::string_view sv) {
    std::size_t off = 0;
    const std::string_view type = helpers::consume_token(sv);
    if (type.empty()) return false;
    off += type.size();
    if (off >= sv.size() || sv[off++] != '/') return false;
    const std::string_view subtype = helpers::consume_token(sv.substr(off));
    if (subtype.empty()) return false;
    off += subtype.size();
    if (off >= sv.size()) return true;
    // parameters = *( OWS ";" OWS [ parameter ] ) — empty slots allowed and no
    // whitespace is permitted around the "=" of a media-type parameter.
    return helpers::for_each_parameter(
        sv.substr(off), /*require_parameter=*/false,
        [](std::string_view rest, std::size_t& bytes) {
          return helpers::consume_parameter(rest, bytes, /*allow_bws=*/false);
        });
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
