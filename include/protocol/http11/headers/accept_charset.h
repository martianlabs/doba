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

#ifndef martianlabs_doba_protocol_http11_headers_accept_charset_h
#define martianlabs_doba_protocol_http11_headers_accept_charset_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                            accept-charset |
// +===========================================================================+
// | RFC 9110 §12.5.2 Accept-Charset                                           |
// +---------------------------------------------------------------------------+
// | The "Accept-Charset" header field can be sent by a user agent to indicate |
// | what charsets are preferred for textual response content.                 |
// |                                                                           |
// | Each member of the list identifies either a charset name or the wildcard  |
// | "*" and can include an optional weight parameter to express relative      |
// | preference.                                                               |
// |                                                                           |
// | A sender MUST NOT generate an Accept-Charset field containing a charset   |
// | that it does not understand. A user agent that has no charset preference  |
// | ought not send this field.                                                |
// |                                                                           |
// | A recipient MAY use this field when selecting a representation,           | 
// | but is not required to do so. Absence of Accept-Charset implies           |
// | that the user agent does not express a preference among charsets.         |
// |                                                                           |
// | The field has limited practical use in modern HTTP because                |
// | UTF-8 is widely deployed and sending detailed charset preferences can     |
// | increase fingerprint surface.                                             |
// |                                                                           |
// | Examples:                                                                 |
// |   Accept-Charset: utf-8                                                   |
// |   Accept-Charset: utf-8, iso-8859-1;q=0.5, *;q=0.1                        |
// +---------------------------------------------------------------------------+
// | RFC 9110 §12.5.2 Accept-Charset (ABNF summary)                            |
// +---------------------------------------------------------------------------+
// +----------------+----------------------------------------------------------+
// | Field          | Definition                                               |
// +----------------+----------------------------------------------------------+
// | Accept-Charset | #( ( token / "*" ) [ weight ] )                          |
// | weight         | OWS ";" OWS "q=" qvalue                                  |
// | qvalue         | "0" [ "." 0*3DIGIT ] / "1" [ "." 0*3("0") ]              |
// | token          | 1*tchar                                                  |
// | tchar          | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /    |
// |                | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA        |
// | OWS            | *( SP / HTAB )                                           |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #accept-charset = [ accept-charset                                      |
// |                       *( OWS "," OWS accept-charset ) ]                   |
// |                                                                           |
// |   accept-charset = ( token / "*" ) [ weight ]                             |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #accept-charset = [ accept-charset ]                                    |
// |                       *( OWS "," OWS [ accept-charset ] )                 |
// |                                                                           |
// |   accept-charset = ( token / "*" ) [ weight ]                             |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "#( ... )", rather than "1#( ... )", zero non-empty     |
// | accept-charset elements are permitted by the purely syntactic ABNF.       |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class accept_charset {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    return helpers::for_each_list_element(sv, consume_charset_element);
  }

 private:
  // +=========================================================================+
  // | [>] consume_charset_element                                 ( private ) |
  // +=========================================================================+
  static constexpr bool consume_charset_element(std::string_view sv) {
    // A charset element is ( token / "*" ) followed by an optional weight.
    // The wildcard "*" needs no special handling because it is a tchar, so
    // consume_token accepts it as a single-character token.
    const std::string_view charset = helpers::consume_token(sv);
    if (charset.empty()) return false;
    const std::size_t off = charset.size();
    if (off == sv.size()) return true;
    // Any remaining content must be exactly one weight.
    return helpers::consume_weight(sv.substr(off));
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
