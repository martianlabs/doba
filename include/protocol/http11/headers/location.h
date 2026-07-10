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

#ifndef martianlabs_doba_protocol_http11_headers_location_h
#define martianlabs_doba_protocol_http11_headers_location_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                  location |
// +===========================================================================+
// | RFC 9110 §10.2.2 Location                                                 |
// +---------------------------------------------------------------------------+
// | The "Location" header field is used in some responses to refer to a       |
// | specific resource in relation to the response. Its field value consists   |
// | of a single URI-reference.                                                |
// |                                                                           |
// | In a 201 (Created) response, Location refers to the primary resource      |
// | created by the request.                                                   |
// |                                                                           |
// | In a 3xx (Redirection) response, Location indicates the preferred target  |
// | URI for automatically redirecting the request.                            |
// |                                                                           |
// | A Location field value can be absolute or relative. If it is relative,    |
// | the final value is computed by resolving it against the target URI of the |
// | request.                                                                  |
// |                                                                           |
// | For 3xx responses, if the Location value does not include a fragment      |
// | component, a user agent MUST process the redirection as if the value      |
// | inherits the fragment component of the original target URI, when such a   |
// | fragment exists.                                                          |
// |                                                                           |
// | Location is not a list-valued field. A comma is data when it appears as   |
// | part of a valid URI-reference; it is not a field-value separator.         |
// |                                                                           |
// | Examples:                                                                 |
// |   Location: https://www.example.com/people/alice                          |
// |   Location: /new-resource                                                 |
// |   Location: ../archive/item.html                                          |
// |   Location: /People.html#tim                                              |
// +---------------------------------------------------------------------------+
// | RFC 9110 §10.2.2 Location (ABNF summary)                                  |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Location         | URI-reference                                          |
// | URI-reference    | URI / relative-ref                                     |
// | URI              | scheme ":" hier-part [ "?" query ] [ "#" fragment ]    |
// | relative-ref     | relative-part [ "?" query ] [ "#" fragment ]           |
// | scheme           | ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )             |
// | hier-part        | "//" authority path-abempty / path-absolute /          |
// |                  | path-rootless / path-empty                             |
// | relative-part    | "//" authority path-abempty / path-absolute /          |
// |                  | path-noscheme / path-empty                             |
// | authority        | [ userinfo "@" ] host [ ":" port ]                     |
// | userinfo         | *( unreserved / pct-encoded / sub-delims / ":" )       |
// | host             | IP-literal / IPv4address / reg-name                    |
// | port             | *DIGIT                                                 |
// | path-abempty     | *( "/" segment )                                       |
// | path-absolute    | "/" [ segment-nz *( "/" segment ) ]                    |
// | path-rootless    | segment-nz *( "/" segment )                            |
// | path-noscheme    | segment-nz-nc *( "/" segment )                         |
// | path-empty       | 0<pchar>                                               |
// | segment          | *pchar                                                 |
// | segment-nz       | 1*pchar                                                |
// | segment-nz-nc    | 1*( unreserved / pct-encoded / sub-delims / "@" )      |
// | query            | *( pchar / "/" / "?" )                                 |
// | fragment         | *( pchar / "/" / "?" )                                 |
// | pchar            | unreserved / pct-encoded / sub-delims / ":" / "@"      |
// | pct-encoded      | "%" HEXDIG HEXDIG                                      |
// | unreserved       | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
// | sub-delims       | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," /  |
// |                  | ";" / "="                                              |
// | OWS              | *( SP / HTAB )                                         |
// +---------------------------------------------------------------------------+
// | RFC 3986 URI-reference notes                                              |
// +---------------------------------------------------------------------------+
// | Location accepts both absolute URI references and relative references.    |
// | Therefore, all of the following are syntactically possible field values:  |
// |                                                                           |
// |   https://example.com/a/b                                                 |
// |   /a/b                                                                    |
// |   a/b                                                                     |
// |   ../a/b                                                                  |
// |   ?q=1                                                                    |
// |   #section                                                                |
// |                                                                           |
// | An empty field value is syntactically permitted by URI-reference because  |
// | relative-ref can derive path-empty with no query and no fragment.         |
// | Whether an empty Location is meaningful is a semantic question, not an    |
// | ABNF question.                                                            |
// |                                                                           |
// | The URI-reference grammar allows userinfo in authority because it is part |
// | of the generic URI syntax. Any policy that rejects userinfo in Location   |
// | is stricter than the generic ABNF and should be treated as semantic or    |
// | security policy rather than as the base field grammar.                    |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class location {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Location = URI-reference. This is the full URI-reference grammar, which
    // (unlike Content-Location) permits an optional "#" fragment component.
    return helpers::check_uri_reference(sv, /*allow_fragment=*/true);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
