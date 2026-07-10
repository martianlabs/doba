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

#ifndef martianlabs_doba_protocol_http11_headers_x_proxy_connection_h
#define martianlabs_doba_protocol_http11_headers_x_proxy_connection_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                          proxy-connection |
// +===========================================================================+
// | Non-standard / legacy Proxy-Connection                                    |
// +---------------------------------------------------------------------------+
// | The "Proxy-Connection" header field is a non-standard, legacy field used  |
// | by some clients when communicating with an explicit HTTP proxy. It was    |
// | intended to provide semantics similar to the HTTP/1.0 "Connection" header |
// | field, mainly for controlling persistence of the client-to-proxy          |
// | connection.                                                               |
// |                                                                           |
// | Proxy-Connection is not defined by RFC 9110 and is not a standardized HTTP|
// | field. A conforming implementation SHOULD prefer the standard             |
// | "Connection" header field.                                                |
// |                                                                           |
// | For compatibility, recipients MAY parse Proxy-Connection using the same   |
// | field-value syntax as "Connection": a comma-separated list of connection  |
// | options, where each option is a token.                                    |
// |                                                                           |
// | Each connection option applies only to the current transport connection   |
// | between adjacent HTTP nodes. It is not end-to-end metadata. Intermediaries|
// | that accept this field for compatibility SHOULD consume it locally and    |
// | SHOULD NOT forward it unchanged to the next inbound or outbound hop.      |
// |                                                                           |
// | Common legacy values include "keep-alive" and "close". These values are   |
// | parsed syntactically as connection-option tokens; their semantics are     |
// | handled separately from ABNF validation.                                  |
// |                                                                           |
// | Examples:                                                                 |
// |   Proxy-Connection: keep-alive                                            |
// |   Proxy-Connection: close                                                 |
// |   Proxy-Connection: keep-alive, Upgrade                                   |
// +---------------------------------------------------------------------------+
// | Compatibility ABNF summary                                                |
// +---------------------------------------------------------------------------+
// +--------------------+------------------------------------------------------+
// | Field              | Definition                                           |
// +--------------------+------------------------------------------------------+
// | Proxy-Connection   | #connection-option                                   |
// | connection-option  | token                                                |
// | token              | 1*tchar                                              |
// | tchar              | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" /      |
// |                    | "-" / "." / "^" / "_" / "`" / "|" / "~" / DIGIT /    |
// |                    | ALPHA                                                |
// | OWS                | *( SP / HTAB )                                       |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #connection-option = [ connection-option *( OWS "," OWS                 |
// |                         connection-option ) ]                             |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #connection-option = [ connection-option ] *( OWS "," OWS               |
// |                         [ connection-option ] )                           |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the compatibility rule is "#connection-option", rather than         |
// | "1#connection-option", zero non-empty connection-option elements are      |
// | permitted by the purely syntactic list ABNF.                              |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class x_proxy_connection {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Proxy-Connection = #connection-option, where connection-option = token.
    // This is the same comma-separated token list as the standard Connection
    // header (RFC 9110 §7.6.1). Since the rule is "#connection-option" (not
    // "1#connection-option"), an empty list and empty list elements are
    // permitted.
    return helpers::for_each_list_element(sv, consume_connection_option);
  }

 private:
  // +=========================================================================+
  // | [>] consume_connection_option                               ( private ) |
  // +=========================================================================+
  static constexpr bool consume_connection_option(std::string_view sv) {
    return helpers::is_token(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
