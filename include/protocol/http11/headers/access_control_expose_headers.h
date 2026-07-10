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

#ifndef martianlabs_doba_protocol_http11_headers_aceh_h
#define martianlabs_doba_protocol_http11_headers_aceh_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                             access-control-expose-headers |
// +===========================================================================+
// | Fetch Standard §3.3.3 Access-Control-Expose-Headers                       |
// +---------------------------------------------------------------------------+
// | The "Access-Control-Expose-Headers" response header indicates which       |
// | response headers can be exposed to frontend JavaScript as part of a CORS  |
// | response.                                                                 |
// |                                                                           |
// | By default, only CORS-safelisted response headers are exposed. This header|
// | allows a server to explicitly list additional response header field names |
// | that the user agent may make visible to APIs such as Fetch or             |
// | XMLHttpRequest.                                                           |
// |                                                                           |
// | This field is used in a response to a CORS request that is not a          |
// | CORS-preflight request. It does not authorize request methods or request  |
// | headers; those are controlled by Access-Control-Allow-Methods and         |
// | Access-Control-Allow-Headers in preflight responses.                      |
// |                                                                           |
// | The field value is a comma-separated list of header field names. Header   |
// | field names use HTTP token syntax and are matched case-insensitively.     |
// |                                                                           |
// | The value "*" is syntactically valid because "*" is a valid token         |
// | character. Semantically, "*" acts as a wildcard only for requests without |
// | credentials. For credentialed requests, "*" is treated as the literal     |
// | header field name "*".                                                    |
// |                                                                           |
// | Examples:                                                                 |
// |   Access-Control-Expose-Headers: Content-Length                           |
// |   Access-Control-Expose-Headers: Content-Encoding, X-Request-Id           |
// |   Access-Control-Expose-Headers: *                                        |
// +---------------------------------------------------------------------------+
// | Fetch Standard §3.3.4 HTTP new-header syntax (ABNF summary)               |
// +---------------------------------------------------------------------------+
// +--------------------------------+------------------------------------------+
// | Field                          | Definition                               |
// +--------------------------------+------------------------------------------+
// | Access-Control-Expose-Headers  | #field-name                              |
// | field-name                     | token                                    |
// | token                          | 1*tchar                                  |
// | tchar                          | "!" / "#" / "$" / "%" / "&" / "'" /      |
// |                                | "*" / "+" / "-" / "." / "^" / "_" /      |
// |                                | "`" / "|" / "~" / DIGIT / ALPHA          |
// | OWS                            | *( SP / HTAB )                           |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #field-name = [ field-name *( OWS "," OWS field-name ) ]                |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #field-name = [ field-name ] *( OWS "," OWS [ field-name ] )            |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "#field-name", rather than "1#field-name", zero         |
// | non-empty field-name elements are permitted by the purely syntactic ABNF. |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class access_control_expose_headers {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    return helpers::for_each_list_element(sv, consume_field_name);
  }

 private:
  // +=========================================================================+
  // | [>] consume_field_name                                      ( private ) |
  // +=========================================================================+
  static constexpr bool consume_field_name(std::string_view sv) {
    return helpers::is_token(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
