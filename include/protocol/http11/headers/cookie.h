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

#ifndef martianlabs_doba_protocol_http11_headers_cookie_h
#define martianlabs_doba_protocol_http11_headers_cookie_h

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                    cookie |
// +===========================================================================+
// | RFC 6265 §4.2 Cookie                                                      |
// +---------------------------------------------------------------------------+
// | The "Cookie" header field is sent by a user agent in an HTTP request to   |
// | return stored cookies to the origin server. The field contains cookies    |
// | previously received from Set-Cookie response header fields.               |
// |                                                                           |
// | Each cookie-pair represents one stored cookie. The cookie-pair contains   |
// | the cookie-name and cookie-value received from a Set-Cookie field. Cookie |
// | attributes are not returned in the Cookie header field. In particular,    |
// | the server cannot determine from Cookie alone when a cookie expires, for  |
// | which hosts or paths it is valid, or whether it was set with Secure or    |
// | HttpOnly.                                                                 |
// |                                                                           |
// | The semantics of individual cookies are application-specific. Servers     |
// | SHOULD NOT rely on the serialization order of cookies, especially when    |
// | multiple cookies have the same name but were set with different Path or   |
// | Domain attributes.                                                        |
// |                                                                           |
// | Cookie is not an HTTP list field and does not use the RFC 9110 #rule.     |
// | Multiple cookie-pairs are separated by a semicolon followed by one SP.    |
// | Empty cookie-pairs are not generated by the ABNF below.                   |
// |                                                                           |
// | Examples:                                                                 |
// |   Cookie: SID=31d4d96e407aad42                                            |
// |   Cookie: SID=31d4d96e407aad42; lang=en-US                                |
// |   Cookie: theme=light; sessionToken=abc123                                |
// +---------------------------------------------------------------------------+
// | RFC 6265 §4.2.1 Cookie (ABNF summary)                                     |
// +---------------------------------------------------------------------------+
// +---------------+-----------------------------------------------------------+
// | Field         | Definition                                                |
// +---------------+-----------------------------------------------------------+
// | Cookie        | cookie-string                                             |
// | cookie-string | cookie-pair *( ";" SP cookie-pair )                       |
// | cookie-pair   | cookie-name "=" cookie-value                              |
// | cookie-name   | token                                                     |
// | cookie-value  | *cookie-octet / ( DQUOTE *cookie-octet DQUOTE )           |
// | cookie-octet  | %x21 / %x23-2B / %x2D-3A / %x3C-5B / %x5D-7E              |
// |               | ; US-ASCII excluding CTLs, whitespace, DQUOTE, comma,     |
// |               | ; semicolon, and backslash                                |
// | token         | 1*tchar                                                   |
// | tchar         | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /     |
// |               | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA         |
// | DQUOTE        | %x22                                                      |
// | SP            | %x20                                                      |
// +---------------------------------------------------------------------------+
// | RFC 6265 Cookie header notes                                              |
// +---------------------------------------------------------------------------+
// | Cookie is a request header field. Servers send cookies with Set-Cookie;   |
// | user agents return applicable stored cookies with Cookie.                 |
// |                                                                           |
// | Cookie attributes such as Expires, Max-Age, Domain, Path, Secure,         |
// | HttpOnly, SameSite, and extension attributes belong to Set-Cookie. They   |
// | are not serialized back in the Cookie header field.                       |
// |                                                                           |
// | The separator between cookie-pairs is exactly ";" SP in the RFC 6265      |
// | generation grammar. This differs from many HTTP fields that use comma     |
// | lists and OWS around separators.                                          |
// |                                                                           |
// | A cookie-value may be empty:                                              |
// |                                                                           |
// |   Cookie: lang=                                                           |
// |                                                                           |
// | A quoted cookie-value is syntactically permitted by the ABNF, but the     |
// | surrounding DQUOTE characters are part of the cookie-value syntax, not    |
// | HTTP quoted-string semantics. Backslash escaping is not defined here.     |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class cookie {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    // Cookie = cookie-string = cookie-pair *( "; " cookie-pair ). Cookie is
    // not an RFC 9110 #list: the separator is exactly "; " (semicolon + one
    // SP), with no OWS and no comma, and at least one cookie-pair is required.
    return consume_cookie_string(sv);
  }

 private:
  // +=========================================================================+
  // | [>] consume_cookie_string                                   ( private ) |
  // +=========================================================================+
  // | cookie-string = cookie-pair *( "; " cookie-pair )                       |
  // +-------------------------------------------------------------------------+
  // | Splits sv on the exact 2-octet "; " separator and validates every       |
  // | segment as a cookie-pair. Because the grammar is cookie-pair *( ... ),  |
  // | an empty field value or any empty segment (leading, trailing, or a      |
  // | doubled separator) is rejected via helpers::is_cookie_pair, which       |
  // | requires a non-empty token cookie-name.                                 |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_cookie_string(std::string_view sv) {
    std::size_t start = 0;
    std::size_t i = 0;
    while (i < sv.size()) {
      // The one and only separator is the exact 2-octet sequence "; ".
      if (sv[i] == ';' && i + 1 < sv.size() && sv[i + 1] == ' ') {
        if (!helpers::is_cookie_pair(sv.substr(start, i - start))) return false;
        i += 2;
        start = i;
        continue;
      }
      i++;
    }
    // The final segment (or the only one when there is no separator).
    return helpers::is_cookie_pair(sv.substr(start));
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
