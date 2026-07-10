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

#ifndef martianlabs_doba_protocol_http11_headers_referer_h
#define martianlabs_doba_protocol_http11_headers_referer_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                   referer |
// +===========================================================================+
// | RFC 9110 §10.1.3 Referer                                                  |
// +---------------------------------------------------------------------------+
// | The "Referer" header field allows a user agent to specify a URI reference |
// | for the resource from which the target URI was obtained. This enables     |
// | servers to generate back-links, perform analytics, optimize caching, or   |
// | reject requests from unwanted sources.                                    |
// |                                                                           |
// | The field name is misspelled as "Referer" for historical reasons. The     |
// | misspelling is part of the HTTP protocol and is the                       |
// | registered field name.                                                    |
// |                                                                           |
// | A user agent MAY send a Referer header field in a request.                |
// | The field value can be either an absolute URI or a partial URI.           |
// | A partial URI is resolved relative to the target URI.                     |
// |                                                                           |
// | A user agent MUST NOT include the fragment component of the URI reference |
// | in the Referer field value. A user agent MUST NOT include the userinfo    |
// | component of a URI in the Referer field value.                            |
// |                                                                           |
// | A user agent MUST NOT send a Referer header field in an unsecured HTTP    |
// | request if the referring resource was obtained using a secure protocol.   |
// |                                                                           |
// | Intermediaries SHOULD NOT modify or delete the Referer header field when  |
// | the field value shares the same scheme and host as the target URI.        |
// |                                                                           |
// | The Referer field can reveal sensitive information. User agents commonly  |
// | apply referrer policies that restrict, trim, or omit the field value.     |
// |                                                                           |
// | Examples:                                                                 |
// |   Referer: https://example.com/index.html                                 |
// |   Referer: /docs/current/page.html                                        |
// |   Referer: https://example.com/search?q=http                              |
// +---------------------------------------------------------------------------+
// | RFC 9110 §10.1.3 Referer (ABNF summary)                                   |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Referer          | absolute-URI / partial-URI                             |
// | absolute-URI     | scheme ":" hier-part [ "?" query ]                     |
// | partial-URI      | relative-part [ "?" query ]                            |
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
// | path-noscheme    | segment-nz-nc *( "/" segment )                         |
// | path-rootless    | segment-nz *( "/" segment )                            |
// | path-empty       | 0<pchar>                                               |
// | segment          | *pchar                                                 |
// | segment-nz       | 1*pchar                                                |
// | segment-nz-nc    | 1*( unreserved / pct-encoded / sub-delims / "@" )      |
// | query            | *( pchar / "/" / "?" )                                 |
// | pchar            | unreserved / pct-encoded / sub-delims / ":" / "@"      |
// | pct-encoded      | "%" HEXDIG HEXDIG                                      |
// | unreserved       | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
// | sub-delims       | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," /  |
// |                  | ";" / "="                                              |
// +---------------------------------------------------------------------------+
// | RFC 9110 §4.2.1 URI references                                            |
// +---------------------------------------------------------------------------+
// | Referer uses absolute-URI or partial-URI. Both forms intentionally        |
// |exclude a fragment component from the ABNF used by HTTP field values.      |
// |                                                                           |
// | Although the generic authority grammar contains userinfo, senders         |
// | MUST NOT generate userinfo in Referer. A parser that performs only ABNF   |
// | validation can accept the syntax first and leave the userinfo             |
// | prohibition to semantic validation, or reject it here as a                |
// | stricter field-specific policy.                                           |
// |                                                                           |
// | Since Referer is not a list-based field, commas have no special separator |
// | meaning. They are valid only where allowed by the URI grammar, such as    |
// | inside sub-delims.                                                        |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class referer {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Referer = absolute-URI / partial-URI. This is the URI-reference grammar
    // without a fragment component, so an unencoded '#' is rejected. The same
    // absolute-URI / partial-URI shape is shared with Content-Location (RFC
    // 9110 §8.7). The RFC 9110 §10.1.3 prohibition on a userinfo component is
    // a semantic sender rule; syntactic validation accepts the ABNF as-is.
    return helpers::check_uri_reference(sv, /*allow_fragment=*/false);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
