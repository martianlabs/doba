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

#ifndef martianlabs_doba_protocol_http11_headers_acam_h
#define martianlabs_doba_protocol_http11_headers_acam_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                              access-control-allow-methods |
// +===========================================================================+
// | Fetch Standard §3.3.3 HTTP responses                                      |
// +---------------------------------------------------------------------------+
// | The "Access-Control-Allow-Methods" response header field indicates which  |
// | HTTP methods are supported by the response URL for the purposes of the    |
// | CORS protocol.                                                            |
// |                                                                           |
// | It is sent by a server in response to a CORS preflight request, allowing  |
// | the user agent to determine whether a future CORS request using one of    |
// | the listed methods is permitted.                                          |
// |                                                                           |
// | The "Allow" header field is not relevant for the purposes of the CORS     |
// | protocol; Access-Control-Allow-Methods is the CORS-specific mechanism.    |
// |                                                                           |
// | The field value is a comma-separated list of HTTP method names. Method    |
// | names are case-sensitive. Standardized methods are conventionally written |
// | in uppercase US-ASCII letters, but extension methods are permitted by the |
// | HTTP grammar as tokens.                                                   |
// |                                                                           |
// | A value of "*" has special CORS semantics: for requests without           |
// | credentials, it counts as a wildcard allowing any method. For requests    |
// | whose credentials mode is "include", "*" is not treated as a wildcard and |
// | has to be matched as the literal method name "*".                         |
// |                                                                           |
// | Examples:                                                                 |
// |   Access-Control-Allow-Methods: GET, POST, OPTIONS                        |
// |   Access-Control-Allow-Methods: PUT, DELETE                               |
// |   Access-Control-Allow-Methods: *                                         |
// +---------------------------------------------------------------------------+
// | Fetch Standard §3.3.4 HTTP new-header syntax (ABNF summary)               |
// +---------------------------------------------------------------------------+
// +------------------------------+--------------------------------------------+
// | Field                        | Definition                                 |
// +------------------------------+--------------------------------------------+
// | Access-Control-Allow-Methods | #method                                    |
// | method                       | token                                      |
// | token                        | 1*tchar                                    |
// | tchar                        | "!" / "#" / "$" / "%" / "&" / "'" / "*" /  |
// |                              | "+" / "-" / "." / "^" / "_" / "`" / "|" /  |
// |                              | "~" / DIGIT / ALPHA                        |
// | OWS                          | *( SP / HTAB )                             |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #method = [ method *( OWS "," OWS method ) ]                            |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #method = [ method ] *( OWS "," OWS [ method ] )                        |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "#method", rather than "1#method", zero non-empty       |
// | method elements are permitted by the purely syntactic ABNF.               |
// |                                                                           |
// | IMPORTANT: "*" is syntactically valid as a method token because "*" is a  |
// | tchar. Its wildcard behavior is CORS semantics, not additional ABNF.      |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class access_control_allow_methods {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    return helpers::for_each_list_element(sv, consume_method);
  }

 private:
  // +=========================================================================+
  // | [>] consume_method                                          ( private ) |
  // +=========================================================================+
  static constexpr bool consume_method(std::string_view sv) {
    return helpers::is_token(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
