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

#ifndef martianlabs_doba_protocol_http11_headers_pragma_h
#define martianlabs_doba_protocol_http11_headers_pragma_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                    pragma |
// +===========================================================================+
// | RFC 9111 §5.4 Pragma                                                      |
// +---------------------------------------------------------------------------+
// | The "Pragma" request header field was originally defined for HTTP/1.0     |
// | caches, allowing clients to send a "no-cache" request before              |
// | Cache-Control existed.                                                    |
// |                                                                           |
// | In HTTP/1.1 and later, Cache-Control provides the standard mechanism for  |
// | cache directives. Support for Cache-Control is now widespread; therefore  |
// | RFC 9111 deprecates Pragma.                                               |
// |                                                                           |
// | The only pragma directive with standardized cache-related meaning is      |
// | "no-cache" in requests. Historically, when Cache-Control was not present, |
// | caches treated:                                                           |
// |                                                                           |
// |   Pragma: no-cache                                                        |
// |                                                                           |
// | as equivalent to:                                                         |
// |                                                                           |
// |   Cache-Control: no-cache                                                 |
// |                                                                           |
// | The meaning of "Pragma: no-cache" in responses was never specified, so it |
// | does not provide a reliable replacement for "Cache-Control: no-cache" in  |
// | response messages.                                                        |
// |                                                                           |
// | Pragma was also defined in HTTP/1.0 as an extensible field for            |
// | implementation-specific directives, but such extensions are               |
// | deprecated for interoperability.                                          |
// |                                                                           |
// | Examples:                                                                 |
// |   Pragma: no-cache                                                        |
// |   Pragma: no-cache, foo=bar                                               |
// |   Pragma: foo="bar"                                                       |
// +---------------------------------------------------------------------------+
// | RFC 9111 §5.4 Pragma / RFC 7234 §5.4 historical ABNF summary              |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Pragma           | 1#pragma-directive                                     |
// | pragma-directive | "no-cache" / extension-pragma                          |
// | extension-pragma | token [ "=" ( token / quoted-string ) ]                |
// | token            | 1*tchar                                                |
// | tchar            | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /  |
// |                  | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA      |
// | quoted-string    | DQUOTE *( qdtext / quoted-pair ) DQUOTE                |
// | qdtext           | HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text        |
// | quoted-pair      | "\" ( HTAB / SP / VCHAR / obs-text )                   |
// | obs-text         | %x80-FF                                                |
// | OWS              | *( SP / HTAB )                                         |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   1#pragma-directive = pragma-directive                                   |
// |                      *( OWS "," OWS pragma-directive )                    |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   1#pragma-directive = [ pragma-directive ]                               |
// |                      *( OWS "," OWS [ pragma-directive ] )                |
// |                                                                           |
// | Empty list elements do not contribute to the element count.               |
// | Since the rule is "1#pragma-directive", at least one non-empty            |
// | pragma-directive is required after ignoring empty list elements.          |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class pragma {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    bool at_least_one_directive = false;
    const bool result = helpers::for_each_list_element(
        sv, [&at_least_one_directive](std::string_view element) {
          if (!consume_pragma_directive(element)) return false;
          at_least_one_directive = true;
          return true;
        });
    // 1#pragma-directive requires at least one valid pragma-directive.
    return result && at_least_one_directive;
  }

 private:
  // +=========================================================================+
  // | [>] consume_pragma_directive                                ( private ) |
  // +=========================================================================+
  // | pragma-directive = "no-cache" / extension-pragma                        |
  // | extension-pragma = token [ "=" ( token / quoted-string ) ]              |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_pragma_directive(std::string_view sv) {
    // "no-cache" is itself a token, so it is subsumed by extension-pragma;
    // the whole production reduces to token [ "=" ( token / quoted-string ) ].
    return helpers::is_directive(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
