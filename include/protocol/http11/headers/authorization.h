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

#ifndef martianlabs_doba_protocol_http11_headers_authorization_h
#define martianlabs_doba_protocol_http11_headers_authorization_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                             authorization |
// +===========================================================================+
// | RFC 9110 §11.6.2 Authorization                                            |
// +---------------------------------------------------------------------------+
// | The "Authorization" header field allows a user agent to authenticate      |
// | itself with an origin server, usually after receiving a 401               |
// | (Unauthorized) response containing one or more WWW-Authenticate           |
// | challenges.                                                               |
// |                                                                           |
// | A user agent MAY send Authorization preemptively, without first receiving |
// | a challenge, when it already has suitable credentials for the requested   |
// | resource.                                                                 |
// |                                                                           |
// | The field value consists of credentials. Credentials start with an        |
// | authentication scheme name, followed optionally by scheme-specific        |
// | information. The scheme-specific part can be either token68 data or a     |
// | comma-separated list of authentication parameters.                        |
// |                                                                           |
// | The authentication scheme name is a token and is matched                  |
// | case-insensitively. The syntax and semantics of the credentials after the |
// | scheme are defined by the selected authentication scheme.                 |
// |                                                                           |
// | Authorization applies to authentication with the origin server.           |
// | Authentication with an intermediary proxy uses Proxy-Authorization        |
// | instead.                                                                  |
// |                                                                           |
// | A request containing Authorization can affect cache handling. Shared      |
// | caches normally cannot use a cached response to such a request unless     |
// | the response explicitly permits reuse according to HTTP caching rules.    |
// |                                                                           |
// | Examples:                                                                 |
// |   Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==                       |
// |   Authorization: Bearer mF_9.B5f-4.1JqM                                   |
// |   Authorization: Digest username="Mufasa", realm="testrealm@host.com",    |
// |                  nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093",              |
// |                  uri="/dir/index.html", response="e966c9329242554e42c8"   |
// +---------------------------------------------------------------------------+
// | RFC 9110 §11.6.2 Authorization (ABNF summary)                             |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Authorization    | credentials                                            |
// | credentials      | auth-scheme [ 1*SP ( token68 / #auth-param ) ]         |
// | auth-scheme      | token                                                  |
// | auth-param       | token BWS "=" BWS ( token / quoted-string )            |
// | token68          | 1*( ALPHA / DIGIT / "-" / "." / "_" / "~" / "+" /      |
// |                  | "/" ) *"="                                             |
// | token            | 1*tchar                                                |
// | tchar            | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /  |
// |                  | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA      |
// | quoted-string    | DQUOTE *( qdtext / quoted-pair ) DQUOTE                |
// | qdtext           | HTAB / SP / "!" / %x23-5B / %x5D-7E / obs-text         |
// | quoted-pair      | "\" ( HTAB / SP / VCHAR / obs-text )                   |
// | BWS              | OWS                                                    |
// | OWS              | *( SP / HTAB )                                         |
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
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class authorization {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Authorization = credentials. The credentials grammar (auth-scheme with
    // an optional token68 or #auth-param argument) is shared with Proxy-
    // Authorization, so validation is delegated to the common helper.
    return helpers::check_credentials(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
