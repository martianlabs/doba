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

#ifndef martianlabs_doba_protocol_http11_checkers_transfer_encoding_h
#define martianlabs_doba_protocol_http11_checkers_transfer_encoding_h

#include <ranges>
#include <string_view>

#include "protocol/http11/constants.h"
#include "protocol/http11/helpers.h"
#include "protocol/http11/transfer_encodings.h"

namespace martianlabs::doba::protocol::http11::checkers {
// +---------------------+-----------------------------------------------------+
// | quoted-string       | DQUOTE *( qdtext / quoted-pair ) DQUOTE             |
// | quoted-pair         | "\" ( HTAB / SP / VCHAR / obs-text )                |
// +---------------------+-----------------------------------------------------+
static inline bool check_quoted_string(std::string_view& sv, std::size_t& of) {
  bool end_double_quote_found = false;
  while (of < sv.size()) {
    if (sv[of] == '"') {
      end_double_quote_found = true;
      of++;
      break;
    } else if (sv[of] == '\\') {
      if ((of + 1) >= sv.size()) return false;
      if (sv[of + 1] != '\t' && sv[of + 1] != ' ' &&
          !helpers::is_vchar(sv[of + 1]) && !helpers::is_obs_text(sv[of + 1])) {
        return false;
      }
      of += 2;
      continue;
    } else if (!helpers::is_qdtext(sv[of])) {
      return false;
    }
    of++;
  }
  return end_double_quote_found;
}
// +---------------------+-----------------------------------------------------+
// | transfer-parameter  | token BWS "=" BWS ( token / quoted-string )         |
// | token               | 1*tchar                                             |
// | tchar               | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" /     |
// |                     | "-" / "." / "^" / "_" / "`" / "|" / "~" /           |
// |                     | DIGIT / ALPHA                                       |
// | OWS                 | *( SP / HTAB )                                      |
// | RWS                 | 1*( SP / HTAB )                                     |
// | BWS                 | OWS                                                 |
// | quoted-string       | DQUOTE *( qdtext / quoted-pair ) DQUOTE             |
// | quoted-pair         | "\" ( HTAB / SP / VCHAR / obs-text )                |
// +---------------------+-----------------------------------------------------+
static inline bool check_transfer_parameters(std::string_view sv) {
  // an empty parameters list if fully supported!
  if (!sv.size()) return true;
  // let's iterate through all the parameters..
  std::size_t off = 0;
  while (off < sv.size()) {
    // let's skip all OWS data before the semi-colon..
    while (off < sv.size() && helpers::is_ows(sv[off])) off++;
    // let's check that the next character is a semi-colon..
    if (off >= sv.size()) return false;
    if (sv[off++] != ';') return false;
    // let's skip all OWS data after the semi-colon..
    while (off < sv.size() && helpers::is_ows(sv[off])) off++;
    std::size_t beg = off;
    while (off < sv.size() && helpers::is_token(sv[off])) off++;
    if (off == beg) return false;        // empty token!
    if (off >= sv.size()) return false;  // bad parameter!
    while (off < sv.size() && helpers::is_ows(sv[off])) off++;
    if (off >= sv.size()) return false;  // bad parameter!
    if (sv[off++] != '=') return false;
    while (off < sv.size() && helpers::is_ows(sv[off])) off++;
    if (off >= sv.size()) return false;  // bad parameter!
    // let's check for ( token / quoted-string )..
    if (sv[off] == '"') {
      // let's check for ( quoted-string )..
      if (!check_quoted_string(sv, ++off)) {
        return false;
      }
    } else {
      // let's check for ( token )..
      bool valid_token = false;
      while (off < sv.size()) {
        if (!helpers::is_token(sv[off])) break;
        if (!valid_token) valid_token = true;
        off++;
      }
      if (!valid_token) return false;
    }
  }
  return true;
}
// +===========================================================================+
// |                                                         transfer-encoding |
// +===========================================================================+
// | RFC 9112 / RFC 9110 (ABNF) — Transfer-Encoding                            |
// +---------------------------------------------------------------------------+
// | transfer-coding is defined in RFC 9110, Section 10.1.4                    |
// +---------------------------------------------------------------------------+
// | The "#rule" list extension used above (RFC 9110, Section 5.6.1):          |
// |                                                                           |
// |    <n>#<m>element  -> comma-delimited list with OWS around the comma.     |
// |                                                                           |
// | Sender-side expansion examples:                                           |
// |    1#element ........ element *( OWS "," OWS element )                    |
// |    #element ......... [ 1#element ]                                       |
// |    <n>#<m>element ... element <n-1>*<m-1>( OWS "," OWS element )          |
// |                                                                           |
// | Recipient-side acceptance (must parse/ignore empty elements):             |
// |    #element ......... [ element ] *( OWS "," OWS [ element ] )            |
// +---------------------------------------------------------------------------+
// | transfer-coding grammar (RFC 9110, Section 10.1.4 "TE"):                  |
// +---------------------+-----------------------------------------------------+
// | Field               | Definition                                          |
// +---------------------+-----------------------------------------------------+
// | transfer-encoding   | #transfer-coding                                    |
// | transfer-coding     | token *( OWS ";" OWS transfer-parameter )           |
// | transfer-parameter  | token BWS "=" BWS ( token / quoted-string )         |
// | token               | 1*tchar                                             |
// | tchar               | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" /     |
// |                     | "-" / "." / "^" / "_" / "`" / "|" / "~" /           |
// |                     | DIGIT / ALPHA                                       |
// | OWS                 | *( SP / HTAB )                                      |
// | RWS                 | 1*( SP / HTAB )                                     |
// | BWS                 | OWS                                                 |
// | quoted-string       | DQUOTE *( qdtext / quoted-pair ) DQUOTE             |
// | quoted-pair         | "\" ( HTAB / SP / VCHAR / obs-text )                |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
static inline bool transfer_encoding(std::string_view sv) {
  std::size_t values_found = 0;
  for (auto token : sv | std::views::split(',')) {
    std::size_t off = 0;
    // check for empty transfer-coding (not supported!)
    if (token.begin() == token.end()) continue;
    // let's create the value from the found segment..
    std::string_view value(&*token.begin(), std::ranges::distance(token));
    helpers::ows_ltrim(value);
    helpers::ows_rtrim(value);
    if (value.empty()) continue;  // this is an empty coding value..
    // let's check [transfer-coding]..
    bool valid_transfer_coding = false;
    while (off < value.size()) {
      if (helpers::is_ows(value[off])) break;
      if (value[off] == ';') break;
      if (!helpers::is_token(value[off])) return false;
      if (!valid_transfer_coding) valid_transfer_coding = true;
      off++;
    }
    if (!valid_transfer_coding) return false;
    // let's check [transfer-parameter]..
    if (off < value.size()) {
      std::string_view transfer_parameters = value.substr(off);
      if (!check_transfer_parameters(transfer_parameters)) {
        return false;
      }
    }
    values_found++;
  }
  return values_found > 0;
}
}  // namespace martianlabs::doba::protocol::http11::checkers

#endif
