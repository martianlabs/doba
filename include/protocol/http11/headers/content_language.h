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

#ifndef martianlabs_doba_protocol_http11_headers_content_language_h
#define martianlabs_doba_protocol_http11_headers_content_language_h

#include <cstddef>

#include "protocol/http11/helpers.h"

namespace martianlabs::doba::protocol::http11::headers {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// |                                                          content-language |
// +===========================================================================+
// | RFC 9110 §8.5 Content-Language                                            |
// +---------------------------------------------------------------------------+
// | The "Content-Language" header field identifies the natural language(s) of |
// | the intended audience for a representation. It does not necessarily list  |
// | every language that appears within the representation.                    |
// |                                                                           |
// | If Content-Language is absent, the representation is considered suitable  |
// | for all language audiences. This can also mean that the sender does not   |
// | know, or does not consider, the representation to be language-specific.   |
// |                                                                           |
// | Multiple language tags MAY be listed when the representation is intended  |
// | for multiple linguistic audiences.                                        |
// |                                                                           |
// | Content-Language MAY be applied to any media type; it is not limited to   |
// | textual representations.                                                  |
// |                                                                           |
// | Each language tag is case-insensitive and consists of one or more subtags |
// | separated by hyphens. Whitespace is not permitted inside a language tag.  |
// |                                                                           |
// | Unlike Accept-Language, which uses the broader language-range production, |
// | Content-Language uses complete language-tag values defined by BCP 47.     |
// |                                                                           |
// | Examples:                                                                 |
// |   Content-Language: da                                                    |
// |   Content-Language: mi, en                                                |
// |   Content-Language: en-CA                                                 |
// |   Content-Language: zh-Hant-TW                                            |
// +---------------------------------------------------------------------------+
// | RFC 9110 §8.5 and §8.5.1 (ABNF summary)                                   |
// +---------------------------------------------------------------------------+
// +------------------+--------------------------------------------------------+
// | Field            | Definition                                             |
// +------------------+--------------------------------------------------------+
// | Content-Language | #language-tag                                          |
// | language-tag     | <Language-Tag, RFC 5646 §2.1>                          |
// | Language-Tag     | langtag / privateuse / grandfathered                   |
// | langtag          | language [ "-" script ] [ "-" region ]                 |
// |                  | *("-" variant) *("-" extension)                        |
// |                  | [ "-" privateuse ]                                     |
// | language         | 2*3ALPHA [ "-" extlang ] / 4ALPHA / 5*8ALPHA           |
// | extlang          | 3ALPHA *2("-" 3ALPHA)                                  |
// | script           | 4ALPHA                                                 |
// | region           | 2ALPHA / 3DIGIT                                        |
// | variant          | 5*8alphanum / (DIGIT 3alphanum)                        |
// | extension        | singleton 1*("-" (2*8alphanum))                        |
// | singleton        | DIGIT / %x41-57 / %x59-5A / %x61-77                    |
// |                  | / %x79-7A                                              |
// | privateuse       | "x" 1*("-" (1*8alphanum))                              |
// | grandfathered    | irregular / regular                                    |
// | alphanum         | ALPHA / DIGIT                                          |
// | OWS              | *( SP / HTAB )                                         |
// +---------------------------------------------------------------------------+
// | RFC 5646 §2.1 grandfathered tags                                          |
// +---------------------------------------------------------------------------+
// | The grandfathered production is a fixed set of literal tags. A complete   |
// | checker needs to recognize these values in addition to langtag and        |
// | privateuse:                                                               |
// |                                                                           |
// |   irregular: en-GB-oed, i-ami, i-bnn, i-default, i-enochian, i-hak,       |
// |              i-klingon, i-lux, i-mingo, i-navajo, i-pwn, i-tao, i-tay,    |
// |              i-tsu, sgn-BE-FR, sgn-BE-NL, sgn-CH-DE                       |
// |                                                                           |
// |   regular:   art-lojban, cel-gaulish, no-bok, no-nyn, zh-guoyu,           |
// |              zh-hakka, zh-min, zh-min-nan, zh-xiang                       |
// |                                                                           |
// | ABNF string literals are case-insensitive unless explicitly marked.       |
// +---------------------------------------------------------------------------+
// | RFC 9110 §5.6.1 list expansion                                            |
// +---------------------------------------------------------------------------+
// | Sender syntax:                                                            |
// |                                                                           |
// |   #language-tag = [ language-tag                                          |
// |                     *( OWS "," OWS language-tag ) ]                       |
// |                                                                           |
// | Recipient syntax:                                                         |
// |                                                                           |
// |   #language-tag = [ language-tag ]                                        |
// |                   *( OWS "," OWS [ language-tag ] )                       |
// |                                                                           |
// | Senders MUST NOT generate empty list elements. Recipients MUST parse and  |
// | ignore a reasonable number of empty list elements.                        |
// |                                                                           |
// | Since the rule is "#language-tag", rather than "1#language-tag", zero     |
// | non-empty language-tag elements are allowed by the syntactic ABNF.        |
// +---------------------------------------------------------------------------+
// | ABNF conformance only establishes that a tag is well-formed. Determining  |
// | whether its subtags are registered and valid requires the IANA Language   |
// | Subtag Registry and the additional validity rules in RFC 5646.            |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is normalized (outer OWS already removed).         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class content_language {
 public:
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +=========================================================================+
  static constexpr bool check(std::string_view sv) {
    return helpers::for_each_list_element(sv, consume_language_tag);
  }

 private:
  // +=========================================================================+
  // | [>] is_alphanum                                             ( private ) |
  // +=========================================================================+
  static constexpr bool is_alphanum(std::uint8_t c) {
    return helpers::is_alpha(c) || helpers::is_digit(c);
  }
  // +=========================================================================+
  // | [>] is_alpha_subtag                                         ( private ) |
  // +=========================================================================+
  static constexpr bool is_alpha_subtag(std::string_view sv,
                                        std::size_t min_size,
                                        std::size_t max_size) {
    if (sv.size() < min_size || sv.size() > max_size) return false;
    for (const char c : sv) {
      if (!helpers::is_alpha(static_cast<std::uint8_t>(c))) {
        return false;
      }
    }
    return true;
  }
  // +=========================================================================+
  // | [>] is_digit_subtag                                         ( private ) |
  // +=========================================================================+
  static constexpr bool is_digit_subtag(std::string_view sv,
                                        std::size_t required_size) {
    if (sv.size() != required_size) return false;
    for (const char c : sv) {
      if (!helpers::is_digit(static_cast<std::uint8_t>(c))) {
        return false;
      }
    }
    return true;
  }
  // +=========================================================================+
  // | [>] is_alphanum_subtag                                      ( private ) |
  // +=========================================================================+
  static constexpr bool is_alphanum_subtag(std::string_view sv,
                                           std::size_t min_size,
                                           std::size_t max_size) {
    if (sv.size() < min_size || sv.size() > max_size) return false;
    for (const char c : sv) {
      if (!is_alphanum(static_cast<std::uint8_t>(c))) {
        return false;
      }
    }
    return true;
  }
  // +=========================================================================+
  // | [>] consume_first_subtag                                    ( private ) |
  // +=========================================================================+
  static constexpr bool consume_first_subtag(std::string_view sv,
                                             std::string_view& subtag,
                                             std::size_t& off) {
    off = 0;
    while (off < sv.size() && is_alphanum(static_cast<std::uint8_t>(sv[off]))) {
      off++;
    }
    if (off == 0) return false;
    if (off < sv.size() && sv[off] != '-') return false;
    subtag = sv.substr(0, off);
    return true;
  }
  // +=========================================================================+
  // | [>] peek_next_subtag                                        ( private ) |
  // +=========================================================================+
  static constexpr bool peek_next_subtag(std::string_view sv, std::size_t off,
                                         std::string_view& subtag,
                                         std::size_t& end) {
    if (off >= sv.size() || sv[off] != '-') return false;
    const std::size_t start = off + 1;
    end = start;
    while (end < sv.size() && is_alphanum(static_cast<std::uint8_t>(sv[end]))) {
      end++;
    }
    // Empty subtags are forbidden.
    if (end == start) return false;
    // A language tag can only contain ALPHA, DIGIT and "-" characters.
    if (end < sv.size() && sv[end] != '-') return false;
    subtag = sv.substr(start, end - start);
    return true;
  }
  // +=========================================================================+
  // | [>] ascii_lower                                             ( private ) |
  // +=========================================================================+
  static constexpr std::uint8_t ascii_lower(std::uint8_t c) {
    if (c >= 'A' && c <= 'Z') {
      return static_cast<std::uint8_t>(c + ('a' - 'A'));
    }
    return c;
  }
  // +=========================================================================+
  // | [>] ascii_iequals                                           ( private ) |
  // +=========================================================================+
  static constexpr bool ascii_iequals(std::string_view lhs,
                                      std::string_view rhs) {
    if (lhs.size() != rhs.size()) return false;
    for (std::size_t i = 0; i < lhs.size(); i++) {
      const std::uint8_t lhs_char =
          ascii_lower(static_cast<std::uint8_t>(lhs[i]));
      const std::uint8_t rhs_char =
          ascii_lower(static_cast<std::uint8_t>(rhs[i]));
      if (lhs_char != rhs_char) return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] is_irregular_grandfathered                              ( private ) |
  // +=========================================================================+
  static constexpr bool is_irregular_grandfathered(std::string_view sv) {
    // The regular grandfathered tags already match the langtag ABNF.
    // Only irregular grandfathered tags need explicit recognition.
    constexpr std::array<std::string_view, 17> tags = {
        "en-GB-oed", "i-ami", "i-bnn",     "i-default", "i-enochian", "i-hak",
        "i-klingon", "i-lux", "i-mingo",   "i-navajo",  "i-pwn",      "i-tao",
        "i-tay",     "i-tsu", "sgn-BE-FR", "sgn-BE-NL", "sgn-CH-DE",
    };
    for (const std::string_view tag : tags) {
      if (ascii_iequals(sv, tag)) return true;
    }
    return false;
  }
  // +=========================================================================+
  // | [>] consume_privateuse                                      ( private ) |
  // +=========================================================================+
  static constexpr bool consume_privateuse(std::string_view sv) {
    // privateuse = "x" 1*("-" (1*8alphanum))
    std::string_view subtag;
    std::size_t off = 0;
    std::size_t end = 0;
    if (!consume_first_subtag(sv, subtag, off)) return false;
    if (subtag.size() != 1 ||
        ascii_lower(static_cast<std::uint8_t>(subtag[0])) != 'x') {
      return false;
    }
    bool has_private_subtag = false;
    while (off < sv.size()) {
      if (!peek_next_subtag(sv, off, subtag, end)) return false;
      if (!is_alphanum_subtag(subtag, 1, 8)) return false;
      off = end;
      has_private_subtag = true;
    }
    return has_private_subtag;
  }
  // +=========================================================================+
  // | [>] consume_langtag                                         ( private ) |
  // +=========================================================================+
  static constexpr bool consume_langtag(std::string_view sv) {
    // -------------------------------------------------------------------------
    // RFC 5646 §2.1:
    // -------------------------------------------------------------------------
    // langtag = language
    //           ["-" script]
    //           ["-" region]
    //           *("-" variant)
    //           *("-" extension)
    //           ["-" privateuse]
    // -------------------------------------------------------------------------
    std::string_view subtag;
    std::size_t off = 0;
    std::size_t end = 0;
    // -------------------------------------------------------------------------
    // language = 2*3ALPHA [ "-" extlang ]
    //          / 4ALPHA
    //          / 5*8ALPHA
    // -------------------------------------------------------------------------
    if (!consume_first_subtag(sv, subtag, off)) return false;
    if (!is_alpha_subtag(subtag, 2, 8)) return false;
    const std::size_t language_size = subtag.size();
    // -------------------------------------------------------------------------
    // extlang = 3ALPHA *2("-" 3ALPHA)
    //
    // An extlang is only allowed after a two-letter or three-letter
    // primary language subtag. The complete production permits up to
    // three consecutive three-letter subtags.
    // -------------------------------------------------------------------------
    if (language_size <= 3) {
      std::size_t extlang_count = 0;
      while (extlang_count < 3 && peek_next_subtag(sv, off, subtag, end) &&
             is_alpha_subtag(subtag, 3, 3)) {
        off = end;
        extlang_count++;
      }
    }
    // -------------------------------------------------------------------------
    // script = 4ALPHA
    // -------------------------------------------------------------------------
    if (peek_next_subtag(sv, off, subtag, end) &&
        is_alpha_subtag(subtag, 4, 4)) {
      off = end;
    }
    // -------------------------------------------------------------------------
    // region = 2ALPHA / 3DIGIT
    // -------------------------------------------------------------------------
    if (peek_next_subtag(sv, off, subtag, end) &&
        (is_alpha_subtag(subtag, 2, 2) || is_digit_subtag(subtag, 3))) {
      off = end;
    }
    // -------------------------------------------------------------------------
    // variant = 5*8alphanum / (DIGIT 3alphanum)
    // -------------------------------------------------------------------------
    while (peek_next_subtag(sv, off, subtag, end)) {
      const bool valid_variant =
          is_alphanum_subtag(subtag, 5, 8) ||
          (subtag.size() == 4 &&
           helpers::is_digit(static_cast<std::uint8_t>(subtag[0])));
      if (!valid_variant) break;
      off = end;
    }
    // -------------------------------------------------------------------------
    // extension = singleton 1*("-" (2*8alphanum))
    //
    // singleton = DIGIT / ALPHA except "x"
    // -------------------------------------------------------------------------
    while (peek_next_subtag(sv, off, subtag, end) && subtag.size() == 1 &&
           ascii_lower(static_cast<std::uint8_t>(subtag[0])) != 'x') {
      // Consume the singleton.
      off = end;
      bool has_extension_subtag = false;
      while (peek_next_subtag(sv, off, subtag, end) &&
             is_alphanum_subtag(subtag, 2, 8)) {
        off = end;
        has_extension_subtag = true;
      }
      if (!has_extension_subtag) return false;
    }
    // -------------------------------------------------------------------------
    // privateuse = "x" 1*("-" (1*8alphanum))
    // -------------------------------------------------------------------------
    if (peek_next_subtag(sv, off, subtag, end) && subtag.size() == 1 &&
        ascii_lower(static_cast<std::uint8_t>(subtag[0])) == 'x') {
      // Consume the "x" singleton.
      off = end;
      bool has_private_subtag = false;
      while (peek_next_subtag(sv, off, subtag, end)) {
        if (!is_alphanum_subtag(subtag, 1, 8)) {
          return false;
        }
        off = end;
        has_private_subtag = true;
      }
      if (!has_private_subtag) return false;
    }
    return off == sv.size();
  }
  // +=========================================================================+
  // | [>] consume_language_tag                                    ( private ) |
  // +=========================================================================+
  static constexpr bool consume_language_tag(std::string_view sv) {
    // -------------------------------------------------------------------------
    // RFC 5646 §2.1:
    // -------------------------------------------------------------------------
    // Language-Tag = langtag / privateuse / grandfathered
    //
    // Regular grandfathered tags match the langtag production. Irregular
    // grandfathered tags require explicit case-insensitive matching.
    // -------------------------------------------------------------------------
    if (sv.empty()) return false;
    return is_irregular_grandfathered(sv) || consume_privateuse(sv) ||
           consume_langtag(sv);
  }
};
}  // namespace martianlabs::doba::protocol::http11::headers

#endif
