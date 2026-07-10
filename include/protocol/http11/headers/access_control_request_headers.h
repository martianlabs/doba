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

#ifndef martianlabs_doba_protocol_http11_headers_acrh_h
#define martianlabs_doba_protocol_http11_headers_acrh_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                            access-control-request-headers |
// +===========================================================================+
// | Fetch Standard §3.3.4 Access-Control-Request-Headers                      |
// +---------------------------------------------------------------------------+
// | The "Access-Control-Request-Headers" header field is used by a user       |
// | agent when issuing a CORS-preflight request. It informs the server which  |
// | HTTP header field names the user agent intends to use in the actual CORS  |
// | request.                                                                  |
// |                                                                           |
// | This field is sent on a preflight OPTIONS request, together with Origin   |
// | and Access-Control-Request-Method, when the actual request would include  |
// | non-safelisted request headers.                                           |
// |                                                                           |
// | The field value is a comma-separated list of HTTP field names. Each field |
// | name follows the generic HTTP field-name grammar, which is a token.       |
// |                                                                           |
// | Field names are case-insensitive. For CORS processing, user agents usually|
// | serialize requested header names in lowercase and sorted order, but that  |
// | is a serialization/processing detail rather than additional ABNF syntax.  |
// |                                                                           |
// | Examples:                                                                 |
// |   Access-Control-Request-Headers: content-type                            |
// |   Access-Control-Request-Headers: authorization, x-requested-with         |
// |   Access-Control-Request-Headers: content-type, x-custom-header           |
// +---------------------------------------------------------------------------+
// | Fetch Standard §3.3.4 HTTP new-header syntax (ABNF summary)               |
// +---------------------------------------------------------------------------+
// +--------------------------------+------------------------------------------+
// | Field                          | Definition                               |
// +--------------------------------+------------------------------------------+
// | Access-Control-Request-Headers | 1#field-name                             |
// | field-name                     | token                                    |
// | token                          | 1*tchar                                  |
// | tchar                          | "!" / "#" / "$" / "%" / "&" / "'" / "*"  |
// |                                | / "+" / "-" / "." / "^" / "_" / "`" /    |
// |                                | "|" / "~" / DIGIT / ALPHA                |
// | OWS                            | *( SP / HTAB )                           |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   1#field-name = field-name *( OWS "," OWS field-name )                   |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   1#field-name = [ field-name ] *( OWS "," OWS [ field-name ] )           |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "1#field-name", at least one non-empty field-name is    |
// | required by the field definition. After ignoring empty list elements, a   |
// | recipient MUST reject values with zero non-empty field-name elements.     |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class access_control_request_headers {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    bool at_least_one_field_name = false;
    const bool result = helpers::for_each_list_element(
        sv, [&at_least_one_field_name](std::string_view element) {
          if (!consume_field_name(element)) return false;
          at_least_one_field_name = true;
          return true;
        });
    // 1#field-name requires at least one non-empty field-name.
    return result && at_least_one_field_name;
  }

 private:
  // +=========================================================================+
  // | [>] consume_field_name                                      ( private ) |
  // +=========================================================================+
  // | field-name = token                                                      |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_field_name(std::string_view sv) {
    return helpers::is_token(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
