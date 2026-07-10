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

#ifndef martianlabs_doba_protocol_http11_headers_www_authenticate_h
#define martianlabs_doba_protocol_http11_headers_www_authenticate_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                          www-authenticate |
// +===========================================================================+
// | RFC 9110 §11.6.1 WWW-Authenticate                                         |
// +---------------------------------------------------------------------------+
// | The "WWW-Authenticate" response header field indicates the authentication |
// | scheme(s) and parameters applicable to the target resource.               |
// |                                                                           |
// | A server generating a 401 (Unauthorized) response MUST send a             |
// | WWW-Authenticate header field containing at least one challenge           |
// | applicable to the requested resource. A server MAY generate this field in |
// | other response messages to indicate that supplying credentials, or        |
// | different credentials, might affect the response.                         |
// |                                                                           |
// | A proxy forwarding a response MUST NOT modify any WWW-Authenticate header |
// | fields in that response.                                                  |
// |                                                                           |
// | The field value can contain more than one challenge, and each challenge   |
// | can contain a comma-separated list of authentication parameters. The      |
// | field itself can also occur multiple times. Parsers therefore need to     |
// | distinguish commas that separate challenges from commas that separate     |
// | auth-params inside a challenge.                                           |
// |                                                                           |
// | Each challenge starts with an auth-scheme token. The auth-scheme token is |
// | matched case-insensitively. A challenge can then contain either a single  |
// | token68 value or a list of auth-param name/value pairs.                   |
// |                                                                           |
// | Authentication parameter names are matched case-insensitively, and each   |
// | parameter name MUST only occur once per challenge. Parameter values can   |
// | be represented as either token or quoted-string.                          |
// |                                                                           |
// | For the "realm" authentication parameter, senders MUST generate the       |
// | quoted-string syntax. Recipients might need to accept both token and      |
// | quoted-string syntax for interoperability.                                |
// |                                                                           |
// | Some user agents fail to parse unknown authentication schemes. Listing    |
// | widely supported schemes, such as Basic, before unknown schemes can       |
// | improve interoperability.                                                 |
// |                                                                           |
// | Examples:                                                                 |
// |   WWW-Authenticate: Basic realm="simple"                                  |
// |   WWW-Authenticate: Basic realm="simple", Newauth realm="apps",           |
// |                     type=1, title="Login to \"apps\""                     |
// +---------------------------------------------------------------------------+
// | RFC 9110 §11.6.1 WWW-Authenticate (ABNF summary)                          |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | WWW-Authenticate | #challenge                                             |
// | challenge        | auth-scheme [ 1*SP ( token68 / #auth-param ) ]         |
// | auth-scheme      | token                                                  |
// | token68          | 1*( ALPHA / DIGIT / "-" / "." / "_" / "~" / "+" /      |
// |                  | "/" ) *"="                                             |
// | auth-param       | token BWS "=" BWS ( token / quoted-string )            |
// | token            | 1*tchar                                                |
// | tchar            | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /  |
// |                  | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA      |
// | quoted-string    | DQUOTE *( qdtext / quoted-pair ) DQUOTE                |
// | qdtext           | HTAB / SP / "!" / %x23-5B / %x5D-7E / obs-text         |
// | quoted-pair      | "\" ( HTAB / SP / VCHAR / obs-text )                   |
// | BWS              | OWS                                                    |
// | OWS              | *( SP / HTAB )                                         |
// | SP               | %x20                                                   |
// | HTAB             | %x09                                                   |
// | VCHAR            | %x21-7E                                                |
// | obs-text         | %x80-FF                                                |
// | DQUOTE           | %x22                                                   |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax for the outer #challenge rule:                              |
// |                                                                           |
// |   #challenge = [ challenge *( OWS "," OWS challenge ) ]                   |
// |                                                                           |
// | Recipient syntax for the outer #challenge rule:                           |
// |                                                                           |
// |   #challenge = [ challenge ] *( OWS "," OWS [ challenge ] )               |
// |                                                                           |
// | Sender syntax for the inner #auth-param rule:                             |
// |                                                                           |
// |   #auth-param = [ auth-param *( OWS "," OWS auth-param ) ]                |
// |                                                                           |
// | Recipient syntax for the inner #auth-param rule:                          |
// |                                                                           |
// |   #auth-param = [ auth-param ] *( OWS "," OWS [ auth-param ] )            |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the field rule is "#challenge", rather than "1#challenge", zero     |
// | non-empty challenge elements are permitted by the purely syntactic ABNF.  |
// | However, a 401 response semantically requires at least one challenge.     |
// |                                                                           |
// | The grammar is ambiguous around sequences such as comma, whitespace, and  |
// | comma because challenge itself uses list syntax for auth-param. This is   |
// | harmless semantically but important for parser implementation.            |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class www_authenticate {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // WWW-Authenticate = #challenge. The #challenge grammar (a comma-separated
    // list of auth-scheme challenges, each optionally carrying a token68 or a
    // #auth-param list) is shared with Proxy-Authenticate, so validation is
    // delegated to the common helper.
    return helpers::check_challenge_list(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
