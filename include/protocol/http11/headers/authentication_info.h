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

#ifndef martianlabs_doba_protocol_http11_headers_authentication_info_h
#define martianlabs_doba_protocol_http11_headers_authentication_info_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                       authentication-info |
// +===========================================================================+
// | RFC 9110 §11.6.3 Authentication-Info                                      |
// +---------------------------------------------------------------------------+
// | The "Authentication-Info" response header field allows an HTTP            |
// | authentication scheme to communicate information after the client's       |
// | authentication credentials have been accepted.                            |
// |                                                                           |
// | This information can include a finalization message from the server, such |
// | as server authentication data for schemes that support mutual             |
// | authentication.                                                           |
// |                                                                           |
// | The field value is a comma-separated list of authentication parameters    |
// | expressed as name/value pairs. The generic syntax only defines the        |
// | parameter container; individual parameter names and their semantics are   |
// | defined by the authentication scheme in use.                              |
// |                                                                           |
// | For example, the Digest authentication scheme defines parameters such as  |
// | nextnonce, rspauth, cnonce, nc, and qop. Those names are scheme-specific  |
// | and are not part of the generic Authentication-Info syntax itself.        |
// |                                                                           |
// | Authentication-Info can be used in any HTTP response, independently of    |
// | the request method and response status code. Its semantics are defined by |
// | the authentication scheme indicated by the Authorization header field of  |
// | the corresponding request.                                                |
// |                                                                           |
// | A proxy forwarding a response is not allowed to modify the field value in |
// | any way.                                                                  |
// |                                                                           |
// | Authentication-Info can be sent as a trailer field when the               |
// | authentication scheme explicitly allows this.                             |
// |                                                                           |
// | The auth-param value allows both token and quoted-string forms. Scheme    |
// | definitions need to accept both forms so that recipients can use generic  |
// | authentication parameter parsers.                                         |
// |                                                                           |
// | Examples:                                                                 |
// |   Authentication-Info: nextnonce="47364c23432d2e131a5fb210812c"           |
// |   Authentication-Info: rspauth="ea40f60335c427b5527b84dbabcdfffd"         |
// |   Authentication-Info: qop=auth, cnonce="0a4f113b", nc=00000001           |
// +---------------------------------------------------------------------------+
// | RFC 9110 §11.6.3 Authentication-Info (ABNF summary)                       |
// +---------------------------------------------------------------------------+
// +---------------------+-----------------------------------------------------+
// | Field               | Definition                                          |
// +---------------------+-----------------------------------------------------+
// | Authentication-Info | #auth-param                                         |
// | auth-param          | token BWS "=" BWS ( token / quoted-string )         |
// | token               | 1*tchar                                             |
// | tchar               | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" /     |
// |                     | "-" / "." / "^" / "_" / "`" / "|" / "~" / DIGIT     |
// |                     | / ALPHA                                             |
// | quoted-string       | DQUOTE *( qdtext / quoted-pair ) DQUOTE             |
// | qdtext              | HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text     |
// | quoted-pair         | "\" ( HTAB / SP / VCHAR / obs-text )                |
// | obs-text            | %x80-FF                                             |
// | OWS                 | *( SP / HTAB )                                      |
// | BWS                 | OWS                                                 |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #auth-param = [ auth-param *( OWS "," OWS auth-param ) ]                |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #auth-param = [ auth-param ] *( OWS "," OWS [ auth-param ] )            |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "#auth-param", rather than "1#auth-param", zero         |
// | non-empty auth-param elements are permitted by the purely syntactic ABNF. |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.3 BWS                                                       |
// +---------------------------------------------------------------------------+
// | BWS is "bad" whitespace. A sender MUST NOT generate BWS in messages. A    |
// | recipient MUST parse BWS and remove it before interpreting the protocol   |
// | element. BWS has no semantics.                                            |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class authentication_info {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Authentication-Info = #auth-param. This is a plain comma-separated list
    // of auth-param name/value pairs (no auth-scheme or token68), which is the
    // same #auth-param list validated within credentials and challenges, so
    // validation is delegated to the common helper.
    return helpers::check_auth_param_list(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
