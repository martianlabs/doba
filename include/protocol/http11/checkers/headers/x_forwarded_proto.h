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

#ifndef martianlabs_doba_protocol_http11_checkers_h_x_forwarded_proto_h
#define martianlabs_doba_protocol_http11_checkers_h_x_forwarded_proto_h

#include "protocol/http11/helpers.h"
#include "protocol/http11/parsed_types.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                         x-forwarded-proto |
// +===========================================================================+
// | Non-standard X-Forwarded-Proto                                            |
// +---------------------------------------------------------------------------+
// | The "X-Forwarded-Proto" header field is a de-facto request header used by |
// | proxies and load balancers to disclose the protocol scheme originally used|
// | by the client when connecting to the first trusted proxy.                 |
// |                                                                           |
// | This is commonly needed when the connection between the client and the    |
// | proxy uses HTTPS, but the connection from the proxy to the origin server  |
// | uses plain HTTP. The origin server can then generate redirects, absolute  |
// | URLs, and security-sensitive decisions according to the original client   |
// | facing scheme.                                                            |
// |                                                                           |
// | Typical values are "http" and "https". However, syntactically, the value  |
// | is treated as a URI scheme name, not as a fixed enumeration.              |
// |                                                                           |
// | X-Forwarded-Proto is not standardized by an RFC. The standardized         |
// | replacement is the "proto" parameter of the RFC 7239 "Forwarded" header.  |
// | RFC 7239 requires the "proto" value to conform to the URI scheme syntax   |
// | defined by RFC 3986.                                                      |
// |                                                                           |
// | A recipient can encounter multiple values due to multiple field lines,    |
// | field-line combination, or intermediary-specific forwarding behavior. For |
// | syntactic validation, this checker treats the field value as a comma      |
// | separated list of URI schemes. Semantic interpretation and trust policy   |
// | are outside the ABNF layer.                                               |
// |                                                                           |
// | This field MUST NOT be trusted unless it was added or sanitized by a      |
// | trusted proxy. Clients can spoof this header when directly reaching the   |
// | origin server.                                                            |
// |                                                                           |
// | Examples:                                                                 |
// |   X-Forwarded-Proto: https                                                |
// |   X-Forwarded-Proto: http                                                 |
// |   X-Forwarded-Proto: https, http                                          |
// +---------------------------------------------------------------------------+
// | Non-standard X-Forwarded-Proto (ABNF summary)                             |
// +---------------------------------------------------------------------------+
// +-------------------+-------------------------------------------------------+
// | Field             | Definition                                            |
// +-------------------+-------------------------------------------------------+
// | X-Forwarded-Proto | #scheme                                               |
// | scheme            | ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )            |
// | ALPHA             | %x41-5A / %x61-7A                                     |
// | DIGIT             | %x30-39                                               |
// | OWS               | *( SP / HTAB )                                        |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #scheme = [ scheme *( OWS "," OWS scheme ) ]                            |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #scheme = [ scheme ] *( OWS "," OWS [ scheme ] )                        |
// |                                                                           |
// | Senders SHOULD NOT generate empty list elements. Recipients parsing this  |
// | field as a list can ignore a reasonable number of empty list elements.    |
// |                                                                           |
// | Since the rule is "#scheme", rather than "1#scheme", zero non-empty       |
// | scheme elements are permitted by the purely syntactic list expansion.     |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class x_forwarded_proto {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static bool check(std::string_view sv, parsed_token_list& out) {
    // The producer overload validates each scheme exactly as the pure check()
    // does and captures every non-empty scheme in order.
    return helpers::for_each_list_element(
        sv, [&out](std::string_view element) {
          if (!consume_scheme(element)) return false;
          out.elements.push_back(element);
          return true;
        });
  }

 private:
  // +=========================================================================+
  // | [>] consume_scheme                                          ( private ) |
  // +=========================================================================+
  static constexpr bool consume_scheme(std::string_view sv) {
    // Each list element is a bare URI scheme (RFC 3986 §3.1), matching the
    // "proto" parameter value of the RFC 7239 Forwarded header.
    return helpers::is_uri_scheme(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
