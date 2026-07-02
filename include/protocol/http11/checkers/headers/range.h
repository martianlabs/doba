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

#ifndef martianlabs_doba_protocol_http11_checkers_h_range_h
#define martianlabs_doba_protocol_http11_checkers_h_range_h

#include <ranges>

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::checkers::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                                     range |
// +===========================================================================+
// | RFC 9110 §14.2 Range                                                      |
// +---------------------------------------------------------------------------+
// | The "Range" request header field modifies GET semantics to request the    |
// | transfer of only one or more subranges of the selected representation     |
// | data instead of the complete representation.                              |
// |                                                                           |
// | Range requests are an OPTIONAL HTTP feature. A server MAY ignore the      |
// | field and process the request as a normal GET request.                    |
// |                                                                           |
// | A server MUST ignore Range when the request method is unrecognized or     |
// | range handling is not defined for that method. RFC 9110 defines range     |
// | handling only for GET.                                                    |
// |                                                                           |
// | An origin server MUST ignore a Range field containing an unknown range    |
// | unit. A proxy MAY discard a Range field containing an unknown range unit. |
// |                                                                           |
// | Range units are case-insensitive. The registered "bytes" unit addresses   |
// | octets using zero-based, inclusive positions. When a content coding is    |
// | applied, byte ranges refer to the encoded byte sequence.                  |
// |                                                                           |
// | An int-range identifies first-pos through last-pos. If last-pos is        |
// | omitted, the range continues through the end of the representation. An    |
// | int-range is invalid when last-pos is present and less than first-pos.    |
// |                                                                           |
// | A suffix-range requests the final suffix-length units. For byte ranges, a |
// | non-zero suffix-length is satisfiable; if it exceeds the representation   |
// | length, the whole representation is selected.                             |
// |                                                                           |
// | Range is evaluated after request preconditions and only when the response |
// | without Range would be 200 (OK). It is ignored when a conditional GET     |
// | would result in 304 (Not Modified).                                       |
// |                                                                           |
// | When the range unit is supported and at least one range is satisfiable,   |
// | the server SHOULD send 206 (Partial Content). When the unit is            |
// | unsupported or the range-set is unsatisfiable, the server SHOULD send 416 |
// | (Range Not Satisfiable).                                                  |
// |                                                                           |
// | A server MAY ignore or reject invalid ranges, more than two overlapping   |
// | ranges, or many small ranges not listed in ascending order, since these   |
// | can indicate a broken client or a denial-of-service attempt.              |
// |                                                                           |
// | Recipients MUST anticipate arbitrarily large decimal numerals and prevent |
// | integer conversion overflow.                                              |
// |                                                                           |
// | Examples:                                                                 |
// |   Range: bytes=0-499                                                      |
// |   Range: bytes=500-999                                                    |
// |   Range: bytes=9500-                                                      |
// |   Range: bytes=-500                                                       |
// |   Range: bytes=0-0,-1                                                     |
// |   Range: bytes= 0-999, 4500-5499, -1000                                   |
// +---------------------------------------------------------------------------+
// | RFC 9110 §§14.1, 14.1.1, 14.1.2 and 14.2 (ABNF summary)                   |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Range            | ranges-specifier                                       |
// | ranges-specifier | range-unit "=" OWS range-set                           |
// | range-unit       | token                                                  |
// | range-set        | 1#range-spec                                           |
// | range-spec       | int-range / suffix-range / other-range                 |
// | int-range        | first-pos "-" [ last-pos ]                             |
// | first-pos        | 1*DIGIT                                                |
// | last-pos         | 1*DIGIT                                                |
// | suffix-range     | "-" suffix-length                                      |
// | suffix-length    | 1*DIGIT                                                |
// | other-range      | 1*( %x21-2B / %x2D-7E )                                |
// |                  | ; 1*(VCHAR excluding comma)                            |
// | token            | 1*tchar                                                |
// | tchar            | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" /  |
// |                  | "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA      |
// | OWS              | *( SP / HTAB )                                         |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   1#range-spec = range-spec *( OWS "," OWS range-spec )                   |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   1#range-spec = [ range-spec ]                                           |
// |                  *( OWS "," OWS [ range-spec ] )                          |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Because range-set uses "1#range-spec", at least one non-empty range-spec  |
// | is required. A value containing only empty elements is invalid.           |
// |                                                                           |
// | RFC 9110 Errata 7306 adds OWS after "=" in ranges-specifier. Therefore,   |
// | values such as "bytes= 0-999, 4500-5499, -1000" are syntactically valid.  |
// |                                                                           |
// | The generic range-spec grammar permits other-range for extension units.   |
// | The "bytes" unit uses only int-range and suffix-range; other-range is not |
// | defined for bytes.                                                        |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class range {
 public:
  static constexpr bool check(std::string_view sv) {
    std::size_t off = 0;
    const std::string_view range_unit = helpers::consume_token(sv);
    if (range_unit.empty()) return false;
    off += range_unit.size();
    // OWS is not allowed before '='.
    if (off >= sv.size() || sv[off++] != '=') return false;
    // Skip OWS after '='.
    while (off < sv.size() && helpers::is_ows(sv[off])) off++;  
    // Now we should be at the start of the range-set.
    return consume_range_set(sv.substr(off));
  }

 private:
  // +=========================================================================+
  // | [>] consume_range_set                                       ( private ) |
  // +=========================================================================+
  static constexpr bool consume_range_set(std::string_view sv) {
    bool at_least_one_range_spec = false;
    std::size_t last = 0;
    for (std::size_t i = 0; i <= sv.size(); i++) {
      if (i < sv.size() && sv[i] != ',') continue;
      std::string_view range_specifier = sv.substr(last, i - last);
      helpers::ows_trim(range_specifier);
      if (!range_specifier.empty()) {
        if (!consume_range_specifier(range_specifier)) return false;
        at_least_one_range_spec = true;
      }
      last = i + 1;
    }
    return at_least_one_range_spec;
  }
  // +=========================================================================+
  // | [>] consume_range_specifier                                 ( private ) |
  // +=========================================================================+
  static constexpr bool consume_range_specifier(std::string_view sv) {
    if (sv.empty()) return false;
    if (consume_int_range(sv)) return true;
    if (consume_suffix_range(sv)) return true;
    return consume_other_range(sv);
  }
  // +=========================================================================+
  // | [>] consume_int_range                                       ( private ) |
  // +=========================================================================+
  static constexpr bool consume_int_range(std::string_view sv) {
    std::size_t off = 0;
    if (off >= sv.size() || !helpers::is_digit(sv[off])) return false;
    do {
      off++;
    } while (off < sv.size() && helpers::is_digit(sv[off]));
    if (off >= sv.size() || sv[off++] != '-') return false;
    while (off < sv.size() && helpers::is_digit(sv[off])) off++;
    return off == sv.size();
  }
  // +=========================================================================+
  // | [>] consume_suffix_range                                    ( private ) |
  // +=========================================================================+
  static constexpr bool consume_suffix_range(std::string_view sv) {
    if (sv.size() < 2 || sv[0] != '-') return false;
    for (std::size_t i = 1; i < sv.size(); i++) {
      if (!helpers::is_digit(sv[i])) return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] consume_other_range                                     ( private ) |
  // +=========================================================================+
  static constexpr bool consume_other_range(std::string_view sv) {
    if (sv.empty()) return false;
    for (const auto c : sv) {
      if (!helpers::is_vchar(c)) return false;
    }
    return true;
  }
};
}  // namespace martianlabs::doba::protocol::http11::checkers::headers

#endif
