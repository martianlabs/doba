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

#ifndef martianlabs_doba_protocol_http11_helpers_h
#define martianlabs_doba_protocol_http11_helpers_h

#include <cstdint>
#include <algorithm>
#include <string_view>

#include "constants.h"
#include "common/date_server.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// helpers                                                            ( struct )
// -----------------------------------------------------------------------------
// This struct holds for the http 1.1 helper functions.
// -----------------------------------------------------------------------------
// =============================================================================
struct helpers {
  // +-------------------------------------------------------------------------+
  // | RFC 5234 (ABNF Core Rules) — DIGIT                                      |
  // +-------------------------------------------------------------------------+
  // | DIGIT = %x30-39    ; "0" - "9"                                          |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_digit(unsigned char c) noexcept {
    return c >= '0' && c <= '9';
  }
  static constexpr bool is_digit(char c) noexcept {
    return is_digit(static_cast<unsigned char>(c));
  }
  // +-------------------------------------------------------------------------+
  // | RFC 5234 (ABNF Core Rules) — HEXDIG                                     |
  // +-------------------------------------------------------------------------+
  // | HEXDIG = DIGIT / "A" / "B" / "C" / "D" / "E" / "F"                      |
  // |                                                                         |
  // | DIGIT  = %x30-39   ; "0" - "9"                                          |
  // +-------------------------------------------------------------------------+
  // | Note: In HTTP (RFC 9110 / RFC 9112), hexadecimal parsing is             |
  // | case-insensitive. Therefore, implementations typically also accept      |
  // | lowercase "a" - "f" (i.e., %x61-66).                                    |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_hex_digit(unsigned char c) noexcept {
    return is_digit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
  }
  static constexpr bool is_hex_digit(char c) noexcept {
    return is_hex_digit(static_cast<unsigned char>(c));
  }
  // +-------------------------------------------------------------------------+
  // | RFC 5234 (ABNF Core Rules) — ALPHA                                      |
  // +-------------------------------------------------------------------------+
  // | ALPHA = %x41-5A / %x61-7A                                               |
  // |        ; A-Z / a-z                                                      |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_alpha(unsigned char c) noexcept {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
  }
  static constexpr bool is_alpha(char c) noexcept {
    return is_alpha(static_cast<unsigned char>(c));
  }
  // +-------------------------------------------------------------------------+
  // | RFC 9110 §5.6.2 — token                                                 |
  // +-------------------------------------------------------------------------+
  // | token  = 1*tchar                                                        |
  // |                                                                         |
  // | tchar  = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /    |
  // |          "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA                    |
  // +-------------------------------------------------------------------------+
  // | DIGIT  = %x30-39   ; "0" - "9"                                          |
  // | ALPHA  = %x41-5A / %x61-7A   ; A-Z / a-z                                |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_token(unsigned char c) noexcept {
    return is_digit(c) || is_alpha(c) ||
           c == constants::character::kExclamation ||
           c == constants::character::kHash ||
           c == constants::character::kDollar ||
           c == constants::character::kPercent ||
           c == constants::character::kAmpersand ||
           c == constants::character::kApostrophe ||
           c == constants::character::kAsterisk ||
           c == constants::character::kPlus ||
           c == constants::character::kHyphen ||
           c == constants::character::kDot ||
           c == constants::character::kCircumflex ||
           c == constants::character::kUnderscore ||
           c == constants::character::kBackTick ||
           c == constants::character::kVerticalBar ||
           c == constants::character::kTilde;
  }
  static constexpr bool is_token(char c) noexcept {
    return is_token(static_cast<unsigned char>(c));
  }
  // +-------------------------------------------------------------------------+
  // | RFC 3986 §2.3 — unreserved                                              |
  // +-------------------------------------------------------------------------+
  // | unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"                      |
  // +-------------------------------------------------------------------------+
  // | ALPHA      = %x41-5A / %x61-7A   ; A-Z / a-z                            |
  // | DIGIT      = %x30-39             ; 0-9                                  |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_unreserved(unsigned char c) noexcept {
    return is_digit(c) || is_alpha(c) || c == constants::character::kHyphen ||
           c == constants::character::kDot ||
           c == constants::character::kUnderscore ||
           c == constants::character::kTilde;
  }
  static constexpr bool is_unreserved(char c) noexcept {
    return is_unreserved(static_cast<unsigned char>(c));
  }
  // +-------------------------------------------------------------------------+
  // | RFC 3986 §2.2 — sub-delims                                              |
  // +-------------------------------------------------------------------------+
  // | sub-delims = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," /      |
  // |              ";" / "="                                                  |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_sub_delim(unsigned char c) noexcept {
    return c == constants::character::kExclamation ||
           c == constants::character::kDollar ||
           c == constants::character::kAmpersand ||
           c == constants::character::kApostrophe ||
           c == constants::character::kLParenthesis ||
           c == constants::character::kRParenthesis ||
           c == constants::character::kAsterisk ||
           c == constants::character::kPlus ||
           c == constants::character::kComma ||
           c == constants::character::kSemiColon ||
           c == constants::character::kEquals;
  }
  static constexpr bool is_sub_delim(char c) noexcept {
    return is_sub_delim(static_cast<unsigned char>(c));
  }
  // +-------------------------------------------------------------------------+
  // | RFC 3986 §3.3 — pchar                                                   |
  // +-------------------------------------------------------------------------+
  // | pchar = unreserved / pct-encoded / sub-delims / ":" / "@"               |
  // +-------------------------------------------------------------------------+
  // | unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"                     |
  // | pct-encoded = "%" HEXDIG HEXDIG                                         |
  // | sub-delims  = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," /     |
  // |               ";" / "="                                                 |
  // +-------------------------------------------------------------------------+
  // | ALPHA  = %x41-5A / %x61-7A   ; A-Z / a-z                                |
  // | DIGIT  = %x30-39             ; 0-9                                      |
  // | HEXDIG = DIGIT / "A" / "B" / "C" / "D" / "E" / "F"                      |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_pchar(unsigned char c) noexcept {
    return is_unreserved(c) || is_sub_delim(c) ||
           c == constants::character::kColon || c == constants::character::kAt;
  }
  static constexpr bool is_pchar(char c) noexcept {
    return is_pchar(static_cast<unsigned char>(c));
  }
  // +-------------------------------------------------------------------------+
  // | RFC 5234 (ABNF Core Rules) — VCHAR                                      |
  // +-------------------------------------------------------------------------+
  // | VCHAR = %x21-7E                                                         |
  // |        ; visible (printing) characters                                  |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_vchar(unsigned char c) noexcept {
    return c >= constants::character::kExclamation &&
           c <= constants::character::kTilde;
  }
  static constexpr bool is_vchar(char c) noexcept {
    return is_vchar(static_cast<unsigned char>(c));
  }
  // +-------------------------------------------------------------------------+
  // | RFC 9110 §5.6.4 — qdtext                                                |
  // +-------------------------------------------------------------------------+
  // | qdtext = HTAB / SP / "!" / %x23-5B / %x5D-7E / obs-text                 |
  // +-------------------------------------------------------------------------+
  // | HTAB     = %x09                                                         |
  // | SP       = %x20                                                         |
  // | obs-text = %x80-FF                                                      |
  // +-------------------------------------------------------------------------+
  // | Note: DQUOTE (%x22) and "\" (%x5C) are excluded and only allowed        |
  // |       via quoted-pair.                                                  |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_qdtext(unsigned char c) noexcept {
    return c == constants::character::kHTab ||
           c == constants::character::kSpace ||
           c == constants::character::kExclamation ||
           (c >= constants::character::kHash &&
            c <= constants::character::kOpenBracket) ||
           (c >= constants::character::kCloseBracket &&
            c <= constants::character::kTilde) ||
           is_obs_text(c);
  }
  static constexpr bool is_qdtext(char c) noexcept {
    return is_qdtext(static_cast<unsigned char>(c));
  }
  // +-------------------------------------------------------------------------+
  // | RFC 9110 §5.6.4 — obs-text                                              |
  // +-------------------------------------------------------------------------+
  // | obs-text = %x80-FF                                                      |
  // +-------------------------------------------------------------------------+
  // | Note: obs-text represents obsolete text allowed for backward            |
  // |       compatibility with legacy implementations.                        |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_obs_text(unsigned char c) noexcept {
    return c >= constants::character::kObsTextStart &&
           c <= constants::character::kObsTextEnd;
  }
  static constexpr bool is_obs_text(char c) noexcept {
    return is_obs_text(static_cast<unsigned char>(c));
  }
  // +-------------------------------------------------------------------------+
  // | RFC 9110 §5.6.3 — OWS (Optional Whitespace)                             |
  // +-------------------------------------------------------------------------+
  // | OWS = *( SP / HTAB )                                                    |
  // +-------------------------------------------------------------------------+
  // | SP   = %x20                                                             |
  // | HTAB = %x09                                                             |
  // +-------------------------------------------------------------------------+
  // | Note: OWS is allowed only where explicitly defined by the ABNF.         |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_ows(unsigned char c) noexcept {
    return c == constants::character::kSpace ||
           c == constants::character::kHTab;
  }
  static constexpr bool is_ows(char c) noexcept {
    return is_ows(static_cast<unsigned char>(c));
  }
  // +-------------------------------------------------------------------------+
  // | Performs OWS trimming on the left side of the provided string_view.     |
  // +-------------------------------------------------------------------------+
  static constexpr void ows_ltrim(std::string_view& sv) noexcept {
    while (!sv.empty() && helpers::is_ows(sv.front())) {
      sv.remove_prefix(1);
    }
  }
  // +-------------------------------------------------------------------------+
  // | Performs OWS trimming on the right side of the provided string_view.    |
  // +-------------------------------------------------------------------------+
  static constexpr void ows_rtrim(std::string_view& sv) noexcept {
    while (!sv.empty() && helpers::is_ows(sv.back())) {
      sv.remove_suffix(1);
    }
  }
  // +-------------------------------------------------------------------------+
  // | Performs OWS trimming on left & right sides of the provided string_view.|
  // +-------------------------------------------------------------------------+
  static constexpr void ows_trim(std::string_view& sv) noexcept {
    ows_ltrim(sv);
    ows_rtrim(sv);
  }
  // +-------------------------------------------------------------------------+
  // | Converts character to its lower-case version.                           |
  // +-------------------------------------------------------------------------+
  static constexpr unsigned char tolower_ascii(unsigned char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
  }
  static constexpr char tolower_ascii(char c) noexcept {
    return static_cast<char>(tolower_ascii(static_cast<unsigned char>(c)));
  }
};
}  // namespace martianlabs::doba::protocol::http11

#endif
