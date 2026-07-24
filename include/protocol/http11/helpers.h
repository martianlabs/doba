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
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef martianlabs_doba_protocol_http11_helpers_h
#define martianlabs_doba_protocol_http11_helpers_h

#include <array>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "protocol/deserialization.h"

namespace martianlabs::doba::protocol::http11 {
namespace detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] CONSTANTs                                                  ( public ) |
// +---------------------------------------------------------------------------+
// | This struct holds for the http 1.1 helper functions.                      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | Each bit encodes one RFC character-class predicate for a byte value.      |
// | kCharFlags[c] is a bitmask; is_X(c) reduces to a single table lookup and  |
// | a bitmask test (2 instructions) instead of a chain of comparisons.        |
// | The table is 256 � 2 = 512 bytes � fits in one L1 cache line group and    |
// | is shared across all hot parse loops in helpers, request and checkers.    |
// +---------------------------------------------------------------------------+
static constexpr std::uint16_t kF_token = 0x0001;
static constexpr std::uint16_t kF_vchar = 0x0002;
static constexpr std::uint16_t kF_obs_text = 0x0004;
static constexpr std::uint16_t kF_ows = 0x0008;
static constexpr std::uint16_t kF_pchar = 0x0010;
static constexpr std::uint16_t kF_qdtext = 0x0020;
static constexpr std::uint16_t kF_ctext = 0x0040;
static constexpr std::uint16_t kF_etagc = 0x0080;
static constexpr std::uint16_t kF_cookie = 0x0100;
static constexpr std::uint16_t kF_cookie_av = 0x0200;
static constexpr std::uint16_t kF_atext = 0x0400;
static constexpr std::uint16_t kF_dtext = 0x0800;
static constexpr std::uint16_t kF_hexdig = 0x1000;
static constexpr std::uint16_t kF_unreserv = 0x2000;
// +---------------------------------------------------------------------------+
// | The table is built at compile time, so the compiler can                   |
// | optimize it into a static data segment.                                   |
// +---------------------------------------------------------------------------+
static constexpr std::array<std::uint16_t, 256> build_char_flags() noexcept {
  std::array<std::uint16_t, 256> t{};
  for (int c = 0; c < 256; ++c) {
    const auto u = static_cast<unsigned char>(c);
    // -------------------------------------------------------------------------
    // token = tchar: !#$%&'*+-.^_`|~ / DIGIT / ALPHA
    // -------------------------------------------------------------------------
    if (u == '!' || u == '#' || u == '$' || u == '%' || u == '&' || u == '\'' ||
        u == '*' || u == '+' || u == '-' || u == '.' || u == '^' || u == '_' ||
        u == '`' || u == '|' || u == '~' || (u >= '0' && u <= '9') ||
        (u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z'))
      t[c] |= kF_token;
    // -------------------------------------------------------------------------
    // vchar: 0x21-0x7E
    // -------------------------------------------------------------------------
    if (u >= 0x21 && u <= 0x7E) t[c] |= kF_vchar;
    // -------------------------------------------------------------------------
    // obs_text: 0x80-0xFF
    // -------------------------------------------------------------------------
    if (u >= 0x80) t[c] |= kF_obs_text;
    // -------------------------------------------------------------------------
    // ows: SP (0x20) / HTAB (0x09)
    // -------------------------------------------------------------------------
    if (u == 0x20 || u == 0x09) t[c] |= kF_ows;
    // -------------------------------------------------------------------------
    // pchar = unreserved / sub-delims / ":" / "@"
    //   unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
    //   sub-delims = "!" / "$" / "&" / "'" / "(" / ")" /
    //                "*" / "+" / "," / ";" / "="
    // -------------------------------------------------------------------------
    if ((u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z') ||
        (u >= '0' && u <= '9') || u == '-' || u == '.' || u == '_' ||
        u == '~' || u == '!' || u == '$' || u == '&' || u == '\'' || u == '(' ||
        u == ')' || u == '*' || u == '+' || u == ',' || u == ';' || u == '=' ||
        u == ':' || u == '@')
      t[c] |= kF_pchar;
    // -------------------------------------------------------------------------
    // qdtext = HTAB / SP / "!" / %x23-5B / %x5D-7E / obs-text
    // -------------------------------------------------------------------------
    if (u == 0x09 || u == 0x20 || u == '!' || (u >= 0x23 && u <= 0x5B) ||
        (u >= 0x5D && u <= 0x7E) || u >= 0x80)
      t[c] |= kF_qdtext;
    // -------------------------------------------------------------------------
    // ctext = HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text
    // -------------------------------------------------------------------------
    if (u == 0x09 || u == 0x20 || (u >= 0x21 && u <= 0x27) ||
        (u >= 0x2A && u <= 0x5B) || (u >= 0x5D && u <= 0x7E) || u >= 0x80)
      t[c] |= kF_ctext;
    // -------------------------------------------------------------------------
    // etagc = %x21 / %x23-7E / obs-text
    // -------------------------------------------------------------------------
    if (u == 0x21 || (u >= 0x23 && u <= 0x7E) || u >= 0x80) t[c] |= kF_etagc;
    // -------------------------------------------------------------------------
    // cookie-octet = %x21 / %x23-2B / %x2D-3A / %x3C-5B / %x5D-7E
    // -------------------------------------------------------------------------
    if (u == 0x21 || (u >= 0x23 && u <= 0x2B) || (u >= 0x2D && u <= 0x3A) ||
        (u >= 0x3C && u <= 0x5B) || (u >= 0x5D && u <= 0x7E))
      t[c] |= kF_cookie;
    // -------------------------------------------------------------------------
    // cookie-av-octet = %x20-7E except %x3B (";")
    // -------------------------------------------------------------------------
    if (u >= 0x20 && u <= 0x7E && u != 0x3B) t[c] |= kF_cookie_av;
    // -------------------------------------------------------------------------
    // atext = ALPHA / DIGIT / "!" / "#" / "$" / "%" / "&" / "'" /
    //         "*" / "+" / "-" / "/" / "=" / "?" / "^" / "_" /
    //         "`" / "{" / "|" / "}" / "~"
    // -------------------------------------------------------------------------
    if ((u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z') ||
        (u >= '0' && u <= '9') || u == '!' || u == '#' || u == '$' ||
        u == '%' || u == '&' || u == '\'' || u == '*' || u == '+' || u == '-' ||
        u == '/' || u == '=' || u == '?' || u == '^' || u == '_' || u == '`' ||
        u == '{' || u == '|' || u == '}' || u == '~')
      t[c] |= kF_atext;
    // -------------------------------------------------------------------------
    // dtext = %x21-5A / %x5E-7E / obs-text
    // -------------------------------------------------------------------------
    if ((u >= 0x21 && u <= 0x5A) || (u >= 0x5E && u <= 0x7E) || u >= 0x80)
      t[c] |= kF_dtext;
    // -------------------------------------------------------------------------
    // hex-digit = DIGIT / "A"-"F" / "a"-"f"
    // -------------------------------------------------------------------------
    if ((u >= '0' && u <= '9') || (u >= 'A' && u <= 'F') ||
        (u >= 'a' && u <= 'f'))
      t[c] |= kF_hexdig;
    // -------------------------------------------------------------------------
    // unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
    // -------------------------------------------------------------------------
    if ((u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z') ||
        (u >= '0' && u <= '9') || u == '-' || u == '.' || u == '_' || u == '~')
      t[c] |= kF_unreserv;
  }
  return t;
}
static constexpr auto kCharFlags = build_char_flags();
}  // namespace detail

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] helpers                                                    ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct holds for the http 1.1 helper functions.                      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct helpers {
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  enum class host_type { kUnknown, kIpLiteral, kIpV4Address, kRegName };
  // +=========================================================================+
  // | [>] is_digit                                                 ( public ) |
  // +=========================================================================+
  // | RFC 5234 (ABNF Core Rules) � DIGIT                                      |
  // +-------------------------------------------------------------------------+
  // | DIGIT = %x30-39    ; "0" - "9"                                          |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_digit(unsigned char c) noexcept {
    return c >= 0x30 && c <= 0x39;
  }
  static constexpr bool is_digit(char c) noexcept {
    return is_digit(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_digits                                                ( public ) |
  // +=========================================================================+
  // | RFC 9110 �5.6.1 / RFC 9111 �1.2.2 � 1*DIGIT (delta-seconds)             |
  // +-------------------------------------------------------------------------+
  // | 1*DIGIT                                                                 |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is a non-empty run of decimal digits.    |
  // | This is the syntactic shape shared by delta-seconds (RFC 9111 �1.2.2)   |
  // | and Content-Length (RFC 9110 �8.6). Any overflow handling is semantic   |
  // | processing and does not affect this purely syntactic predicate.         |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_digits(std::string_view sv) noexcept {
    if (sv.empty()) return false;
    for (const auto c : sv) {
      if (!is_digit(c)) return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] parse_size_t                                             ( public ) |
  // +=========================================================================+
  // | RFC 9110 �5.6.1 � 1*DIGIT                                               |
  // +-------------------------------------------------------------------------+
  // | Validates that sv is a non-empty run of decimal digits (exactly like    |
  // | is_digits) and, on success, converts it to a std::size_t in out. This   |
  // | is the producing counterpart of the purely syntactic is_digits and is   |
  // | shared by the Content-Length (RFC 9110 �8.6) and Max-Forwards (RFC 9110 |
  // | �7.6.2) producer overloads. Returns false on a non-1*DIGIT value or     |
  // | when the value would overflow std::size_t, in which case out is left    |
  // | unspecified; the overflow guard is the one place where a syntactically  |
  // | valid number is rejected for representability.                          |
  // +-------------------------------------------------------------------------+
  static constexpr bool parse_size_t(std::string_view sv,
                                     std::size_t& out) noexcept {
    if (!is_digits(sv)) return false;
    constexpr std::size_t max = std::numeric_limits<std::size_t>::max();
    std::size_t value = 0;
    for (const auto c : sv) {
      const std::size_t digit = static_cast<std::size_t>(c - '0');
      if (value > (max - digit) / 10) return false;
      value = value * 10 + digit;
    }
    out = value;
    return true;
  }
  // +=========================================================================+
  // | [>] is_hex_digit                                             ( public ) |
  // +=========================================================================+
  // | RFC 5234 (ABNF Core Rules) � HEXDIG                                     |
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
    return (detail::kCharFlags[c] & detail::kF_hexdig) != 0;
  }
  static constexpr bool is_hex_digit(char c) noexcept {
    return is_hex_digit(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_alpha                                                 ( public ) |
  // +=========================================================================+
  // | RFC 5234 (ABNF Core Rules) � ALPHA                                      |
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
  // +=========================================================================+
  // | [>] is_token                                                 ( public ) |
  // +=========================================================================+
  // | RFC 9110 �5.6.2 � token                                                 |
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
    return (detail::kCharFlags[c] & detail::kF_token) != 0;
  }
  static constexpr bool is_token(char c) noexcept {
    return is_token(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_token                                                 ( public ) |
  // +=========================================================================+
  // | RFC 9110 �5.6.2 � token (whole-value predicate)                         |
  // +-------------------------------------------------------------------------+
  // | token = 1*tchar                                                         |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is exactly one token: non-empty, with    |
  // | every character satisfying tchar. Unlike consume_token(), which         |
  // | greedily consumes only the tchar prefix of sv, this rejects any         |
  // | trailing bytes that are not part of the token.                          |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_token(std::string_view sv) noexcept {
    if (sv.empty()) return false;
    for (const auto c : sv) {
      if (!is_token(c)) return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] is_unreserved                                            ( public ) |
  // +=========================================================================+
  // | RFC 3986 �2.3 � unreserved                                              |
  // +-------------------------------------------------------------------------+
  // | unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"                      |
  // +-------------------------------------------------------------------------+
  // | ALPHA      = %x41-5A / %x61-7A   ; A-Z / a-z                            |
  // | DIGIT      = %x30-39             ; 0-9                                  |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_unreserved(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_unreserv) != 0;
  }
  static constexpr bool is_unreserved(char c) noexcept {
    return is_unreserved(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_sub_delim                                             ( public ) |
  // +=========================================================================+
  // | RFC 3986 �2.2 � sub-delims                                              |
  // +-------------------------------------------------------------------------+
  // | sub-delims = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," /      |
  // |              ";" / "="                                                  |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_sub_delim(unsigned char c) noexcept {
    return c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' ||
           c == ')' || c == '*' || c == '+' || c == ',' || c == ';' || c == '=';
  }
  static constexpr bool is_sub_delim(char c) noexcept {
    return is_sub_delim(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_pchar                                                 ( public ) |
  // +=========================================================================+
  // | RFC 3986 �3.3 � pchar                                                   |
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
    return (detail::kCharFlags[c] & detail::kF_pchar) != 0;
  }
  static constexpr bool is_pchar(char c) noexcept {
    return is_pchar(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_vchar                                                 ( public ) |
  // +=========================================================================+
  // | RFC 5234 (ABNF Core Rules) � VCHAR                                      |
  // +-------------------------------------------------------------------------+
  // | VCHAR = %x21-7E                                                         |
  // |        ; visible (printing) characters                                  |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_vchar(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_vchar) != 0;
  }
  static constexpr bool is_vchar(char c) noexcept {
    return is_vchar(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_qdtext                                                ( public ) |
  // +=========================================================================+
  // | RFC 9110 �5.6.4 � qdtext                                                |
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
    return (detail::kCharFlags[c] & detail::kF_qdtext) != 0;
  }
  static constexpr bool is_qdtext(char c) noexcept {
    return is_qdtext(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_obs_text                                              ( public ) |
  // +=========================================================================+
  // | RFC 9110 �5.6.4 � obs-text                                              |
  // +-------------------------------------------------------------------------+
  // | obs-text = %x80-FF                                                      |
  // +-------------------------------------------------------------------------+
  // | Note: obs-text represents obsolete text allowed for backward            |
  // |       compatibility with legacy implementations.                        |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_obs_text(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_obs_text) != 0;
  }
  static constexpr bool is_obs_text(char c) noexcept {
    return is_obs_text(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_ctext                                                 ( public ) |
  // +=========================================================================+
  // | RFC 9110 �5.6.5 � ctext                                                 |
  // +-------------------------------------------------------------------------+
  // | ctext = HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text              |
  // +-------------------------------------------------------------------------+
  // | HTAB     = %x09                                                         |
  // | SP       = %x20                                                         |
  // | obs-text = %x80-FF                                                      |
  // +-------------------------------------------------------------------------+
  // | Note: "(" (%x28), ")" (%x29), and "\" (%x5C) are excluded; inside a     |
  // |       comment they are only allowed structurally (nested comment) or    |
  // |       via quoted-pair.                                                  |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_ctext(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_ctext) != 0;
  }
  static constexpr bool is_ctext(char c) noexcept {
    return is_ctext(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_atext                                                 ( public ) |
  // +=========================================================================+
  // | RFC 5322 �3.2.3 � atext                                                 |
  // +-------------------------------------------------------------------------+
  // | atext = ALPHA / DIGIT / "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / |
  // |         "-" / "/" / "=" / "?" / "^" / "_" / "`" / "{" / "|" / "}" / "~" |
  // +-------------------------------------------------------------------------+
  // | The atom character set used by dot-atom-text and atom (RFC 5322 �3.2.3),|
  // | imported into HTTP via the From "mailbox" grammar (RFC 9110 �10.1.2).   |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_atext(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_atext) != 0;
  }
  static constexpr bool is_atext(char c) noexcept {
    return is_atext(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_dtext                                                 ( public ) |
  // +=========================================================================+
  // | RFC 5322 �3.4.1 � dtext                                                 |
  // +-------------------------------------------------------------------------+
  // | dtext = %d33-90 / %d94-126 / obs-dtext                                  |
  // +-------------------------------------------------------------------------+
  // | The unquoted character set allowed inside a domain-literal. "[" (%d91), |
  // | "\" (%d92), and "]" (%d93) are excluded; inside a domain-literal they   |
  // | are only allowed structurally (the delimiters) or via quoted-pair.      |
  // | obs-text (%x80-FF) covers the obs-dtext octet allowance in the HTTP     |
  // | parsing context.                                                        |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_dtext(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_dtext) != 0;
  }
  static constexpr bool is_dtext(char c) noexcept {
    return is_dtext(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_etagc                                                 ( public ) |
  // +=========================================================================+
  // | RFC 9110 �8.8.3 � etagc                                                 |
  // +-------------------------------------------------------------------------+
  // | etagc = %x21 / %x23-7E / obs-text                                       |
  // |       ; VCHAR except DQUOTE, plus obs-text                              |
  // +-------------------------------------------------------------------------+
  // | VCHAR    = %x21-7E                                                      |
  // | DQUOTE   = %x22                                                         |
  // | obs-text = %x80-FF                                                      |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_etagc(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_etagc) != 0;
  }
  static constexpr bool is_etagc(char c) noexcept {
    return is_etagc(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_cookie_octet                                          ( public ) |
  // +=========================================================================+
  // | RFC 6265 �4.1.1 � cookie-octet                                          |
  // +-------------------------------------------------------------------------+
  // | cookie-octet = %x21 / %x23-2B / %x2D-3A / %x3C-5B / %x5D-7E             |
  // |              ; US-ASCII characters excluding CTLs, whitespace, DQUOTE,  |
  // |              ; comma, semicolon, and backslash                          |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_cookie_octet(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_cookie) != 0;
  }
  static constexpr bool is_cookie_octet(char c) noexcept {
    return is_cookie_octet(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_cookie_av_octet                                       ( public ) |
  // +=========================================================================+
  // | RFC 6265 �4.1.1 � <any CHAR except CTLs or ";">                         |
  // +-------------------------------------------------------------------------+
  // | Both path-value and extension-av are defined as any CHAR except CTLs or |
  // | a ";". CHAR is %x01-7F and CTLs are %x00-1F / %x7F, so the octet class  |
  // | reduces to %x20-7E excluding ";" (%x3B). This is deliberately broad: it |
  // | admits SP, comma, "=", and DQUOTE, which is why an Expires date such as |
  // | "Wed, 09 Jun 2021 10:18:14 GMT" is a syntactically valid cookie-av.     |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_cookie_av_octet(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_cookie_av) != 0;
  }
  static constexpr bool is_cookie_av_octet(char c) noexcept {
    return is_cookie_av_octet(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_entity_tag                                            ( public ) |
  // +=========================================================================+
  // | RFC 9110 �8.8.3 � entity-tag                                            |
  // +-------------------------------------------------------------------------+
  // | entity-tag = [ weak ] opaque-tag                                        |
  // | weak       = %s"W/"                                                     |
  // | opaque-tag = DQUOTE *etagc DQUOTE                                       |
  // +-------------------------------------------------------------------------+
  // | Validates that sv is exactly one entity-tag (optionally weak),          |
  // | consuming the whole of sv. The "W/" prefix is case-sensitive, as %s     |
  // | denotes a case-sensitive string in the ABNF.                            |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_entity_tag(std::string_view sv) noexcept {
    if (sv.empty()) return false;
    std::size_t off = 0;
    // Optional case-sensitive weak indicator "W/".
    if (sv.size() >= 2 && sv[0] == 'W' && sv[1] == '/') off = 2;
    // opaque-tag = DQUOTE *etagc DQUOTE
    if (off >= sv.size() || sv[off] != '"') return false;
    off++;
    while (off < sv.size() && sv[off] != '"') {
      if (!is_etagc(sv[off])) return false;
      off++;
    }
    if (off >= sv.size() || sv[off] != '"') return false;
    off++;
    // The closing DQUOTE must be the last character of sv.
    return off == sv.size();
  }
  // +=========================================================================+
  // | [>] is_ows                                                   ( public ) |
  // +=========================================================================+
  // | RFC 9110 �5.6.3 � OWS (Optional Whitespace)                             |
  // +-------------------------------------------------------------------------+
  // | OWS = *( SP / HTAB )                                                    |
  // +-------------------------------------------------------------------------+
  // | SP   = %x20                                                             |
  // | HTAB = %x09                                                             |
  // +-------------------------------------------------------------------------+
  // | Note: OWS is allowed only where explicitly defined by the ABNF.         |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_ows(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_ows) != 0;
  }
  static constexpr bool is_ows(char c) noexcept {
    return is_ows(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_qvalue                                                ( public ) |
  // +=========================================================================+
  // | qvalue             | ( "0" [ "." 0*3DIGIT ] ) /                         |
  // |                    | ( "1" [ "." 0*3("0") ] )                           |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_qvalue(std::string_view sv) noexcept {
    if (sv.empty() || sv.size() > 5) return false;
    if (sv[0] != '0' && sv[0] != '1') return false;
    if (sv.size() == 1) return true;  // "0" or "1" (valid ones)
    if (sv[1] != '.') return false;
    for (std::size_t i = 2; i < sv.size(); ++i) {
      if (!is_digit(sv[i]) || (sv[0] == '1' && sv[i] != '0')) return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] consume_weight                                           ( public ) |
  // +=========================================================================+
  // | RFC 9110 �12.4.2 � weight                                               |
  // +-------------------------------------------------------------------------+
  // | weight = OWS ";" OWS "q=" qvalue                                        |
  // +-------------------------------------------------------------------------+
  // | Validates that sv is exactly one weight parameter (the case-insensitive |
  // | "q" name, no whitespace around "="), consuming the whole of sv.         |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_weight(std::string_view sv) noexcept {
    std::size_t i = 0;
    // OWS before ';'.
    while (i < sv.size() && is_ows(sv[i])) i++;
    // A ';' is mandatory before the weight parameter.
    if (i >= sv.size() || sv[i++] != ';') return false;
    // OWS after ';'.
    while (i < sv.size() && is_ows(sv[i])) i++;
    // The case-insensitive "q" parameter is mandatory.
    if (i >= sv.size()) return false;
    const std::string_view name = consume_token(sv.substr(i));
    if (name.empty() || !iequals(name, "q")) return false;
    i += name.size();
    // No whitespace is permitted around '='.
    if (i >= sv.size() || sv[i++] != '=') return false;
    if (i >= sv.size()) return false;
    const std::string_view qvalue = consume_token(sv.substr(i));
    if (qvalue.empty() || !is_qvalue(qvalue)) return false;
    // The qvalue must consume the remainder of sv.
    return i + qvalue.size() == sv.size();
  }
  // +=========================================================================+
  // | [>] tolower_ascii                                            ( public ) |
  // +=========================================================================+
  // | Converts character to its lower-case version.                           |
  // +-------------------------------------------------------------------------+
  static constexpr unsigned char tolower_ascii(unsigned char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
  }
  static constexpr char tolower_ascii(char c) noexcept {
    return static_cast<char>(tolower_ascii(static_cast<unsigned char>(c)));
  }
  // +=========================================================================+
  // | [>] ows_ltrim                                                ( public ) |
  // +=========================================================================+
  // | Performs OWS trimming on the left side of the provided string_view.     |
  // +-------------------------------------------------------------------------+
  static constexpr void ows_ltrim(std::string_view& sv) noexcept {
    while (!sv.empty() && helpers::is_ows(sv.front())) {
      sv.remove_prefix(1);
    }
  }
  // +=========================================================================+
  // | [>] ows_rtrim                                                ( public ) |
  // +=========================================================================+
  // | Performs OWS trimming on the right side of the provided string_view.    |
  // +-------------------------------------------------------------------------+
  static constexpr void ows_rtrim(std::string_view& sv) noexcept {
    while (!sv.empty() && helpers::is_ows(sv.back())) {
      sv.remove_suffix(1);
    }
  }
  // +=========================================================================+
  // | [>] ows_trim                                                 ( public ) |
  // +=========================================================================+
  // | Performs OWS trimming on left & right sides of the provided string_view.|
  // +-------------------------------------------------------------------------+
  static constexpr void ows_trim(std::string_view& sv) noexcept {
    ows_ltrim(sv);
    ows_rtrim(sv);
  }
  // +=========================================================================+
  // | [>] skip_n_hextets                                           ( public ) |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | h16            | 1*4HEXDIG                                              |
  // +----------------+--------------------------------------------------------+
  static constexpr int skip_n_hextets(std::string_view sv) {
    int hextets_found = 0;
    std::size_t off = 0;
    std::size_t hex_digits = 0;
    while (off < sv.size()) {
      if (helpers::is_hex_digit(sv[off])) {
        if (++hex_digits > 4) return -1;
      } else if (sv[off] == ':') {
        if (!hex_digits) {
          return -1;
        }
        hextets_found++;
        hex_digits = 0;
      } else {
        return -1;
      }
      off++;
    }
    if (sv.size() && sv.back() == ':') return -1;
    if (hex_digits) hextets_found++;
    return hextets_found;
  }
  // +=========================================================================+
  // | [>] is_port                                                  ( public ) |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | port           | *DIGIT                                                 |
  // +----------------+--------------------------------------------------------+
  static constexpr bool is_port(std::string_view sv) {
    std::size_t off = 0;
    while (off < sv.size()) {
      if (!helpers::is_digit(sv[off])) return false;
      off++;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] is_dec_octet                                             ( public ) |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | dec-octet      | DIGIT                    ; 0-9                         |
  // |                | / %x31-39 DIGIT          ; 10-99                       |
  // |                | / "1" 2DIGIT             ; 100-199                     |
  // |                | / "2" %x30-34 DIGIT      ; 200-249                     |
  // |                | / "25" %x30-35           ; 250-255                     |
  // +----------------+--------------------------------------------------------+
  static constexpr bool is_dec_octet(std::string_view sv, std::size_t digits,
                                     std::size_t off) {
    if (!digits || digits > 3) return false;
    if (off < digits) return false;
    if (digits == 2) {
      if (sv[off - 2] == '0') return false;
    } else if (digits == 3) {
      if (sv[off - 3] == '2') {
        if (sv[off - 2] == '5') {
          if (sv[off - 1] > '5') return false;
        } else if (sv[off - 2] > '4') {
          return false;
        }
      } else if (sv[off - 3] != '1') {
        return false;
      }
    }
    return true;
  }
  // +=========================================================================+
  // | [>] is_dec_octet                                             ( public ) |
  // +=========================================================================+
  // | RFC 3986 �3.2.2 � dec-octet (whole-value predicate)                     |
  // +-------------------------------------------------------------------------+
  // | dec-octet = DIGIT                 ; 0-9                                 |
  // |           / %x31-39 DIGIT         ; 10-99                               |
  // |           / "1" 2DIGIT            ; 100-199                             |
  // |           / "2" %x30-34 DIGIT     ; 200-249                             |
  // |           / "25" %x30-35          ; 250-255                             |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is exactly one canonical dec-octet: a    |
  // | decimal integer in the range 0..255 with no leading zeroes (except the  |
  // | single value "0"). This is the RFC 3986 IPv4 octet shape and is also    |
  // | the syntactic shape of a Sec-WebSocket-Version value (RFC 6455 �11.3.5).|
  // | Unlike the positional overload, this checks digit-ness of every byte    |
  // | and rejects any value outside 0..255 or with a leading zero.            |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_dec_octet(std::string_view sv) noexcept {
    const std::size_t sv_size = sv.size();
    if (sv_size == 0 || sv_size > 3) return false;
    for (const auto c : sv) {
      if (!is_digit(c)) return false;
    }
    // A single digit (0-9) is always a canonical dec-octet.
    if (sv_size == 1) return true;
    // Multi-digit values MUST NOT have a leading zero.
    if (sv[0] == '0') return false;
    // Two digits with a non-zero leader are 10-99, always in range.
    if (sv_size == 2) return true;
    // Three digits: enforce the 0..255 upper bound.
    if (sv[0] == '1') return true;                     // 100-199
    if (sv[0] == '2') {                                // 200-255
      if (sv[1] < '5') return true;                    // 200-249
      if (sv[1] == '5') return sv[2] <= '5';           // 250-255
    }
    return false;
  }
  // +=========================================================================+
  // | [>] is_ip_v_future                                           ( public ) |
  // +=========================================================================+
  // +--------------+----------------------------------------------------------+
  // | IPvFuture    | "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )     |
  // +--------------+----------------------------------------------------------+
  static constexpr bool is_ip_v_future(std::string_view sv) {
    if (sv.empty()) return false;
    if (sv.front() != 'v' && sv.front() != 'V') return false;
    // [1*HEXDIG "."] part..
    std::size_t hex_digits_found = 0;
    std::size_t off = 1;
    while (off < sv.size() && sv[off] != '.') {
      if (!helpers::is_hex_digit(sv[off])) return false;
      hex_digits_found++;
      off++;
    }
    if (off++ == sv.size() || !hex_digits_found) return false;
    // [1*( unreserved / sub-delims / ":" )] part..
    std::size_t subparts_found = 0;
    while (off < sv.size()) {
      if (!helpers::is_unreserved(sv[off]) && !helpers::is_sub_delim(sv[off]) &&
          sv[off] != ':') {
        return false;
      }
      subparts_found++;
      off++;
    }
    return subparts_found > 0;
  }
  // +=========================================================================+
  // | [>] check_ip_v6_address                                      ( public ) |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | IPv6address    | 6( h16 ":" ) ls32                                      |
  // |                | / "::" 5( h16 ":" ) ls32                               |
  // |                | / [ h16 ] "::" 4( h16 ":" ) ls32                       |
  // |                | / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32         |
  // |                | / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32         |
  // |                | / [ *3( h16 ":" ) h16 ] "::" h16 ":" ls32              |
  // |                | / [ *4( h16 ":" ) h16 ] "::" ls32                      |
  // |                | / [ *5( h16 ":" ) h16 ] "::" h16                       |
  // |                | / [ *6( h16 ":" ) h16 ] "::"                           |
  // +----------------+--------------------------------------------------------+
  // | ls32           | ( h16 ":" h16 ) / IPv4address                          |
  // | h16            | 1*4HEXDIG                                              |
  // +----------------+--------------------------------------------------------+
  static constexpr bool is_ip_v6_address(std::string_view sv) {
    // first character must be either ':' or hex-digit..
    std::size_t sep = sv.find("::");
    std::string_view head;
    std::string_view tail;
    if (sep == std::string_view::npos) {
      // +---------------------------------------------------------------------+
      // | no-compression case!                                                |
      // +---------------------------------------------------------------------+
      // | 6( h16 ":" ) ls32                                                   |
      // +---------------------------------------------------------------------+
      std::size_t dot = sv.find('.');
      if (dot == std::string_view::npos) return skip_n_hextets(sv) == 8;
      std::size_t col = sv.find_last_of(':');
      if (col == std::string_view::npos) return false;
      if (dot <= col) return false;
      head = sv.substr(0, col);
      tail = sv.substr(col + 1);
      if (skip_n_hextets(head) != 6) return false;
      if (!is_ip_v4_address(tail)) return false;
      return true;
    } else if (sv.find("::", sep + 1) == std::string_view::npos) {
      // +---------------------------------------------------------------------+
      // | compression case!                                                   |
      // +---------------------------------------------------------------------+
      // | "::" 5( h16 ":" ) ls32                                              |
      // | / [ h16 ] "::" 4( h16 ":" ) ls32                                    |
      // | / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32                      |
      // | / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32                      |
      // | / [ *3( h16 ":" ) h16 ] "::" h16 ":" ls32                           |
      // | / [ *4( h16 ":" ) h16 ] "::" ls32                                   |
      // | / [ *5( h16 ":" ) h16 ] "::" h16                                    |
      // | / [ *6( h16 ":" ) h16 ] "::"                                        |
      // +---------------------------------------------------------------------+
      std::string_view svl = sv.substr(0, sep);
      std::string_view svr = sv.substr(sep + 2);
      int lh = !svl.empty() ? skip_n_hextets(svl) : 0;
      if (lh == -1) return false;
      std::size_t dot = svr.find('.');
      if (dot == std::string_view::npos) {
        int rh = !svr.empty() ? skip_n_hextets(svr) : 0;
        return rh == -1 ? false : lh + rh <= 7;
      }
      std::size_t col = svr.find_last_of(':');
      if (col != std::string_view::npos) {
        if (dot <= col) return false;
        head = svr.substr(0, col);
        tail = svr.substr(col + 1);
      } else {
        tail = svr;
      }
      int rh = !head.empty() ? skip_n_hextets(head) : 0;
      if (rh == -1) return false;
      if (lh + rh > 5) return false;
      if (tail.empty()) return false;
      if (!is_ip_v4_address(tail)) return false;
      return true;
    }
    return false;
  }
  // +=========================================================================+
  // | [>] is_ip_v4_address                                         ( public ) |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | IPv4address    | dec-octet "." dec-octet "." dec-octet "." dec-octet    |
  // +----------------+--------------------------------------------------------+
  static constexpr bool is_ip_v4_address(std::string_view sv) {
    if (sv.empty()) return false;
    int dec_octets_count = 0;
    std::size_t off = 0;
    std::size_t digits = 0;
    while (off < sv.size()) {
      if (sv[off] == '.') {
        if (!digits || digits > 3) return false;
        if (!is_dec_octet(sv, digits, off)) return false;
        dec_octets_count++;
        digits = 0;
      } else if (helpers::is_digit(sv[off])) {
        digits++;
      } else {
        return false;
      }
      off++;
    }
    if (!digits || digits > 3) return false;
    if (!is_dec_octet(sv, digits, off)) return false;
    return ++dec_octets_count == 4;
  }
  // +=========================================================================+
  // | [>] is_ip_literal                                            ( public ) |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | IP-literal     | "[" ( IPv6address / IPvFuture ) "]"                    |
  // +----------------+--------------------------------------------------------+
  static constexpr bool is_ip_literal(std::string_view sv) {
    if (sv.size() < 2) return false;  // at least two characters..
    if (sv.front() != '[') return false;
    if (sv.back() != ']') return false;
    sv.remove_prefix(1);
    sv.remove_suffix(1);
    if (sv.empty()) return false;
    if (is_ip_v_future(sv)) return true;
    if (is_ip_v6_address(sv)) return true;
    return false;
  }
  // +=========================================================================+
  // | [>] is_reg_name                                              ( public ) |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | reg-name       | *( unreserved / pct-encoded / sub-delims )             |
  // +----------------+--------------------------------------------------------+
  static constexpr bool is_reg_name(std::string_view sv) {
    std::size_t off = 0;
    while (off < sv.size()) {
      if (!helpers::is_unreserved(sv[off])) {
        if (!helpers::is_sub_delim(sv[off])) {
          if (sv[off] != '%') return false;
          if (off + 2 >= sv.size()) return false;
          if (!helpers::is_hex_digit(sv[off + 1])) return false;
          if (!helpers::is_hex_digit(sv[off + 2])) return false;
          off += 2;
        }
      }
      off++;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] is_token68                                               ( public ) |
  // +=========================================================================+
  // | RFC 9110 �11.2 � token68                                                |
  // +-------------------------------------------------------------------------+
  // | token68 = 1*( ALPHA / DIGIT / "-" / "." / "_" / "~" / "+" / "/" ) *"="  |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is exactly one token68: a non-empty run  |
  // | of unreserved characters plus "+"/"/", followed by any number (possibly |
  // | zero) of "=" padding characters. At least one leading (non-"=")         |
  // | character is required, and no other bytes may follow the "=" run.       |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_token68(std::string_view sv) noexcept {
    if (sv.empty()) return false;
    std::size_t i = 0;
    const std::size_t sv_size = sv.size();
    while (i < sv_size &&
           (is_unreserved(sv[i]) || sv[i] == '+' || sv[i] == '/')) {
      i++;
    }
    // At least one non-"=" character is mandatory before the padding.
    if (i == 0) return false;
    while (i < sv_size && sv[i] == '=') i++;
    // No bytes are allowed after the "=" padding run.
    return i == sv_size;
  }
  // +=========================================================================+
  // | [>] is_base64_char                                           ( public ) |
  // +=========================================================================+
  // | RFC 4648 �4 / RFC 6455 �4.3 � base64-character                          |
  // +-------------------------------------------------------------------------+
  // | base64-character = ALPHA / DIGIT / "+" / "/"                            |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_base64_char(char c) noexcept {
    return is_alpha(c) || is_digit(c) || c == '+' || c == '/';
  }
  // +=========================================================================+
  // | [>] check_base64_value                                       ( public ) |
  // +=========================================================================+
  // | RFC 6455 �4.3 � base64-value-non-empty                                  |
  // +-------------------------------------------------------------------------+
  // | base64-value-non-empty = ( 1*base64-data [ base64-padding ] )           |
  // |                          / base64-padding                               |
  // | base64-data            = 4base64-character                              |
  // | base64-padding         = ( 2base64-character "==" )                     |
  // |                          / ( 3base64-character "=" )                    |
  // | base64-character       = ALPHA / DIGIT / "+" / "/"                      |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is a non-empty canonical base64 value:   |
  // | a positive multiple of four characters, where every four-character      |
  // | group except possibly the last is pure base64-character data, and the   |
  // | last group is either pure base64-character data or a single padding     |
  // | group ("??==" or "???="). "=" may appear only in the final group. This  |
  // | is the standard-alphabet base64 shape (no URL-safe "-"/"_") shared by   |
  // | WebSocket handshake fields such as Sec-WebSocket-Key (RFC 6455 �4.1).   |
  // | The decoded byte length is a semantic concern and is not checked here.  |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_base64_value(std::string_view sv) noexcept {
    const std::size_t sv_size = sv.size();
    // base64-value-non-empty is non-empty and always a multiple of four.
    if (sv_size == 0 || sv_size % 4 != 0) return false;
    // Every group but the last must be four pure base64-characters.
    const std::size_t last = sv_size - 4;
    for (std::size_t i = 0; i < last; i++) {
      if (!is_base64_char(sv[i])) return false;
    }
    // The final group is pure data or a "??=="/"???=" padding group.
    const char a = sv[last];
    const char b = sv[last + 1];
    const char c = sv[last + 2];
    const char d = sv[last + 3];
    if (!is_base64_char(a) || !is_base64_char(b)) return false;
    if (is_base64_char(c)) {
      // "????" (data) or "???=" (3char "=") padding group.
      return is_base64_char(d) || d == '=';
    }
    // "??==" (2char "==") padding group.
    return c == '=' && d == '=';
  }
  // +=========================================================================+
  // | [>] consume_token                                            ( public ) |
  // +=========================================================================+
  // +---------+---------------------------------------------------------------+
  // | method  | token                                                         |
  // | token   | 1*tchar                                                       |
  // | tchar   | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /   |
  // |         | "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA                   |
  // +---------+---------------------------------------------------------------+
  static constexpr std::string_view consume_token(
      std::string_view sv) noexcept {
    if (sv.empty()) return std::string_view();
    std::size_t i = 0;
    const std::size_t sv_size = sv.size();
    while (i < sv_size) {
      if (!is_token(sv[i])) break;
      i++;
    }
    return sv.substr(0, i);
  }
  // +=========================================================================+
  // | [>] consume_quoted_string                                    ( public ) |
  // +=========================================================================+
  // | RFC 9110 �5.6.4 � quoted-string                                         |
  // +-------------------------------------------------------------------------+
  // | quoted-string = DQUOTE *( qdtext / quoted-pair ) DQUOTE                 |
  // | quoted-pair   = "\" ( HTAB / SP / VCHAR / obs-text )                    |
  // +-------------------------------------------------------------------------+
  // | Consumes a quoted-string from the provided string_view, validating that |
  // | every unescaped character is qdtext and every escaped character is a    |
  // | valid quoted-pair octet.                                                |
  // +-------------------------------------------------------------------------+
  static constexpr std::string_view consume_quoted_string(
      std::string_view sv) noexcept {
    if (sv.empty() || sv.front() != '"') return {};
    std::size_t end_index = 1;
    bool inside_quoted_string = true;
    while (end_index < sv.size() && inside_quoted_string) {
      if (sv[end_index] == '"') {
        inside_quoted_string = false;
      } else if (sv[end_index] == '\\') {
        if (end_index + 1 >= sv.size()) return {};
        const unsigned char escaped =
            static_cast<unsigned char>(sv[end_index + 1]);
        if (!is_ows(escaped) && !is_vchar(escaped) && !is_obs_text(escaped)) {
          return {};
        }
        end_index += 2;  // Skip the escaped character
        continue;
      } else if (!is_qdtext(sv[end_index])) {
        return {};
      }
      end_index++;
    }
    if (inside_quoted_string) return {};
    return sv.substr(0, end_index);
  }
  // +=========================================================================+
  // | [>] consume_comment                                          ( public ) |
  // +=========================================================================+
  // | RFC 9110 �5.6.5 � comment                                               |
  // +-------------------------------------------------------------------------+
  // | comment     = "(" *( ctext / quoted-pair / comment ) ")"                |
  // | quoted-pair = "\" ( HTAB / SP / VCHAR / obs-text )                      |
  // +-------------------------------------------------------------------------+
  // | Consumes a single comment from the front of sv and returns the consumed |
  // | prefix (including the surrounding parentheses), or an empty string_view |
  // | if sv does not begin with a well-formed comment. Comments nest, so a    |
  // | depth counter tracks unescaped "(" / ")" pairs; a "\" introduces a      |
  // | quoted-pair whose escaped octet (HTAB/SP/VCHAR/obs-text) is skipped and |
  // | never affects nesting. Any other octet must be ctext. An unterminated   |
  // | comment (depth never returns to zero) is rejected. A valid comment is   |
  // | never empty (minimum "()"), so an empty result unambiguously signals    |
  // | failure.                                                                |
  // +-------------------------------------------------------------------------+
  static constexpr std::string_view consume_comment(
      std::string_view sv) noexcept {
    if (sv.empty() || sv.front() != '(') return {};
    std::size_t i = 1;
    std::size_t depth = 1;
    while (i < sv.size()) {
      const char c = sv[i];
      if (c == '\\') {
        // A quoted-pair escapes exactly one following HTAB/SP/VCHAR/obs-text.
        if (i + 1 >= sv.size()) return {};
        const unsigned char escaped = static_cast<unsigned char>(sv[i + 1]);
        if (!is_ows(escaped) && !is_vchar(escaped) && !is_obs_text(escaped)) {
          return {};
        }
        i += 2;
        continue;
      }
      if (c == '(') {
        depth++;
      } else if (c == ')') {
        depth--;
        if (depth == 0) return sv.substr(0, i + 1);
      } else if (!is_ctext(c)) {
        return {};
      }
      i++;
    }
    // Reached the end without closing every open comment.
    return {};
  }
  // +=========================================================================+
  // | [>] consume_cfws                                             ( public ) |
  // +=========================================================================+
  // | RFC 5322 �3.2.2 � CFWS (in the HTTP field-value context)                |
  // +-------------------------------------------------------------------------+
  // | CFWS = (1*([FWS] comment) [FWS]) / FWS                                  |
  // | FWS  = ([*WSP CRLF] 1*WSP) / obs-FWS                                    |
  // | WSP  = SP / HTAB                                                        |
  // +-------------------------------------------------------------------------+
  // | Consumes an optional run of folding whitespace and comments starting at |
  // | sv[i], advancing i past it. Because HTTP field values are already       |
  // | normalized (no CR/LF, no surrounding OWS), FWS collapses to runs of WSP |
  // | (SP / HTAB, i.e. is_ows), so CFWS reduces to any interleaving of WSP    |
  // | and RFC 5322 comments (parsed via consume_comment, which is byte-       |
  // | compatible in this context). CFWS is optional, so consuming nothing is  |
  // | success; the only failure is a "(" that does not begin a well-formed    |
  // | comment, which returns false. On success returns true with i advanced.  |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_cfws(std::string_view sv,
                                     std::size_t& i) noexcept {
    while (i < sv.size()) {
      if (is_ows(sv[i])) {
        i++;
      } else if (sv[i] == '(') {
        const std::string_view comment = consume_comment(sv.substr(i));
        if (comment.empty()) return false;
        i += comment.size();
      } else {
        break;
      }
    }
    return true;
  }
  // +=========================================================================+
  // | [>] consume_dot_atom                                         ( public ) |
  // +=========================================================================+
  // | RFC 5322 �3.2.3 � dot-atom (in the HTTP field-value context)            |
  // +-------------------------------------------------------------------------+
  // | dot-atom      = [CFWS] dot-atom-text [CFWS]                             |
  // | dot-atom-text = 1*atext *( "." 1*atext )                                |
  // +-------------------------------------------------------------------------+
  // | Consumes a single dot-atom starting at sv[i], advancing i past it, and  |
  // | returns true on success. Optional leading CFWS is consumed first (via   |
  // | consume_cfws), then dot-atom-text: one or more atext (is_atext) runs    |
  // | separated by single "." characters. A leading, trailing, or doubled     |
  // | "." is rejected because every "." must be both preceded and followed by |
  // | at least one atext. Optional trailing CFWS is consumed last. Returns    |
  // | false when no atext is present, a "." is misplaced, or a CFWS comment   |
  // | is malformed.                                                           |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_dot_atom(std::string_view sv,
                                         std::size_t& i) noexcept {
    if (!consume_cfws(sv, i)) return false;
    // dot-atom-text: the first atext run is mandatory.
    if (i >= sv.size() || !is_atext(sv[i])) return false;
    while (i < sv.size() && is_atext(sv[i])) i++;
    // Subsequent runs are each introduced by a single ".".
    while (i < sv.size() && sv[i] == '.') {
      i++;
      if (i >= sv.size() || !is_atext(sv[i])) return false;
      while (i < sv.size() && is_atext(sv[i])) i++;
    }
    return consume_cfws(sv, i);
  }
  // +=========================================================================+
  // | [>] consume_domain_literal                                   ( public ) |
  // +=========================================================================+
  // | RFC 5322 �3.4.1 � domain-literal (in the HTTP field-value context)      |
  // +-------------------------------------------------------------------------+
  // | domain-literal = [CFWS] "[" *([FWS] dtext) [FWS] "]" [CFWS]             |
  // +-------------------------------------------------------------------------+
  // | Consumes a single domain-literal starting at sv[i], advancing i past it |
  // | and returning true on success. Optional leading CFWS is consumed first, |
  // | then a mandatory "[", then any run of dtext (is_dtext) interleaved with |
  // | FWS which, in the normalized HTTP field-value context, collapses to WSP |
  // | (is_ows), then a mandatory closing "]", then optional trailing CFWS.    |
  // | Returns false when the brackets are missing/unterminated, an interior   |
  // | octet is neither dtext nor WSP, or a CFWS comment is malformed.         |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_domain_literal(std::string_view sv,
                                               std::size_t& i) noexcept {
    if (!consume_cfws(sv, i)) return false;
    if (i >= sv.size() || sv[i] != '[') return false;
    i++;
    while (i < sv.size() && (is_ows(sv[i]) || is_dtext(sv[i]))) i++;
    if (i >= sv.size() || sv[i] != ']') return false;
    i++;
    return consume_cfws(sv, i);
  }
  // +=========================================================================+
  // | [>] consume_addr_spec                                        ( public ) |
  // +=========================================================================+
  // | RFC 5322 �3.4.1 � addr-spec (in the HTTP field-value context)           |
  // +-------------------------------------------------------------------------+
  // | addr-spec  = local-part "@" domain                                      |
  // | local-part = dot-atom / quoted-string / obs-local-part                  |
  // | domain     = dot-atom / domain-literal / obs-domain                     |
  // +-------------------------------------------------------------------------+
  // | Consumes a single addr-spec starting at sv[i], advancing i past it and  |
  // | returning true on success. The local-part is either a quoted-string     |
  // | (when the first non-CFWS octet is DQUOTE, consumed via                  |
  // | consume_quoted_string with the RFC 5322 optional CFWS consumed around   |
  // | its core) or a dot-atom (via consume_dot_atom). A mandatory "@" then    |
  // | separates it from the domain, which is either a domain-literal (when    |
  // | the first non-CFWS octet is "[") or a dot-atom. Returns false when any  |
  // | required production is missing or malformed.                            |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_addr_spec(std::string_view sv,
                                          std::size_t& i) noexcept {
    // local-part = dot-atom / quoted-string
    std::size_t peek = i;
    if (!consume_cfws(sv, peek)) return false;
    if (peek < sv.size() && sv[peek] == '"') {
      i = peek;
      const std::string_view qs = consume_quoted_string(sv.substr(i));
      if (qs.empty()) return false;
      i += qs.size();
      if (!consume_cfws(sv, i)) return false;
    } else if (!consume_dot_atom(sv, i)) {
      return false;
    }
    // "@" separator.
    if (i >= sv.size() || sv[i] != '@') return false;
    i++;
    // domain = dot-atom / domain-literal
    peek = i;
    if (!consume_cfws(sv, peek)) return false;
    if (peek < sv.size() && sv[peek] == '[') return consume_domain_literal(sv, i);
    return consume_dot_atom(sv, i);
  }
  // +=========================================================================+
  // | [>] consume_word                                             ( public ) |
  // +=========================================================================+
  // | RFC 5322 �3.2.5 � word (in the HTTP field-value context)                |
  // +-------------------------------------------------------------------------+
  // | word          = atom / quoted-string                                    |
  // | atom          = [CFWS] 1*atext [CFWS]                                   |
  // | quoted-string = [CFWS] DQUOTE *([FWS] qcontent) [FWS] DQUOTE [CFWS]     |
  // +-------------------------------------------------------------------------+
  // | Consumes a single word starting at sv[i], advancing i past it and       |
  // | returning true on success. A word is either a quoted-string (when the   |
  // | first non-CFWS octet is DQUOTE, consumed via consume_quoted_string with |
  // | the RFC 5322 optional CFWS consumed around its core) or an atom (one or |
  // | more atext, is_atext, surrounded by optional CFWS). Returns false when  |
  // | neither production matches or a CFWS comment is malformed.              |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_word(std::string_view sv,
                                     std::size_t& i) noexcept {
    if (!consume_cfws(sv, i)) return false;
    if (i < sv.size() && sv[i] == '"') {
      const std::string_view qs = consume_quoted_string(sv.substr(i));
      if (qs.empty()) return false;
      i += qs.size();
      return consume_cfws(sv, i);
    }
    // atom: 1*atext.
    if (i >= sv.size() || !is_atext(sv[i])) return false;
    while (i < sv.size() && is_atext(sv[i])) i++;
    return consume_cfws(sv, i);
  }
  // +=========================================================================+
  // | [>] check_mailbox                                            ( public ) |
  // +=========================================================================+
  // | RFC 5322 �3.4 � mailbox (imported by RFC 9110 �10.1.2 From)             |
  // +-------------------------------------------------------------------------+
  // | mailbox      = name-addr / addr-spec                                    |
  // | name-addr    = [display-name] angle-addr                                |
  // | angle-addr   = [CFWS] "<" addr-spec ">" [CFWS]                          |
  // | display-name = phrase                                                   |
  // | phrase       = 1*word                                                   |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is exactly one mailbox. This is the      |
  // | single-mailbox value form imported by the From header (RFC 9110         |
  // | �10.1.2); From is not a list field, so no comma-separated mailboxes are |
  // | accepted. Two forms are tried, each of which must consume the entire    |
  // | input: (1) name-addr, an optional display-name (zero or more words via  |
  // | consume_word) followed by an angle-addr ([CFWS] "<" addr-spec ">"       |
  // | [CFWS]); (2) a bare addr-spec (via consume_addr_spec). Returns true iff |
  // | one form matches the whole input.                                       |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_mailbox(std::string_view sv) noexcept {
    // Form 1: name-addr = [display-name] angle-addr.
    {
      std::size_t i = 0;
      // display-name = phrase = 1*word, but it is optional in name-addr, so
      // zero words are allowed here (that reduces name-addr to angle-addr).
      while (i < sv.size() && sv[i] != '<') {
        const std::size_t before = i;
        if (!consume_word(sv, i)) break;
        // A word must make progress; otherwise stop to avoid an infinite loop.
        if (i == before) break;
      }
      // angle-addr = [CFWS] "<" addr-spec ">" [CFWS].
      if (consume_cfws(sv, i) && i < sv.size() && sv[i] == '<') {
        i++;
        if (consume_addr_spec(sv, i) && i < sv.size() && sv[i] == '>') {
          i++;
          if (consume_cfws(sv, i) && i == sv.size()) return true;
        }
      }
    }
    // Form 2: bare addr-spec.
    {
      std::size_t i = 0;
      if (consume_addr_spec(sv, i) && i == sv.size()) return true;
    }
    return false;
  }
  // +=========================================================================+
  // | [>] consume_token_or_quoted_string                           ( public ) |
  // +=========================================================================+
  // | RFC 9110 �5.6.6 � parameter-value (and equivalent value productions)    |
  // +-------------------------------------------------------------------------+
  // | value = token / quoted-string                                           |
  // +-------------------------------------------------------------------------+
  // | Consumes a quoted-string (via consume_quoted_string) when sv starts     |
  // | with DQUOTE, or a token (via consume_token) otherwise. Returns an       |
  // | empty string_view when neither production matches (e.g., an empty       |
  // | token or an unterminated/invalid quoted-string).                        |
  // +-------------------------------------------------------------------------+
  static constexpr std::string_view consume_token_or_quoted_string(
      std::string_view sv) noexcept {
    if (!sv.empty() && sv.front() == '"') return consume_quoted_string(sv);
    return consume_token(sv);
  }
  // +=========================================================================+
  // | [>] is_cookie_value                                          ( public ) |
  // +=========================================================================+
  // | RFC 6265 �4.1.1 � cookie-value                                          |
  // +-------------------------------------------------------------------------+
  // | cookie-value = *cookie-octet / ( DQUOTE *cookie-octet DQUOTE )          |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is exactly one cookie-value, the shared  |
  // | value form of the Cookie (RFC 6265 �4.2.1) and Set-Cookie (RFC 6265     |
  // | �4.1.1) cookie-pair. The value is either a (possibly empty) run of      |
  // | cookie-octets, or the same run wrapped in a pair of DQUOTE delimiters.  |
  // | The surrounding DQUOTEs are literal cookie-value syntax rather than an  |
  // | HTTP quoted-string, so no backslash escaping is recognised and the      |
  // | inner characters are restricted to cookie-octets (DQUOTE excluded).     |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_cookie_value(std::string_view sv) noexcept {
    // Optionally the value is wrapped in a pair of literal DQUOTE delimiters.
    if (sv.size() >= 2 && sv.front() == '"' && sv.back() == '"') {
      sv.remove_prefix(1);
      sv.remove_suffix(1);
    } else if (!sv.empty() && (sv.front() == '"' || sv.back() == '"')) {
      // A single, unbalanced DQUOTE is not a valid cookie-value.
      return false;
    }
    // The (unwrapped) value must be a run of cookie-octets; it may be empty.
    for (const char c : sv) {
      if (!is_cookie_octet(c)) return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] is_cookie_pair                                           ( public ) |
  // +=========================================================================+
  // | RFC 6265 �4.1.1 � cookie-pair                                           |
  // +-------------------------------------------------------------------------+
  // | cookie-pair  = cookie-name "=" cookie-value                             |
  // | cookie-name  = token                                                    |
  // | cookie-value = *cookie-octet / ( DQUOTE *cookie-octet DQUOTE )          |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is exactly one cookie-pair, the shared   |
  // | pair form of the Cookie (RFC 6265 �4.2.1) and Set-Cookie (RFC 6265      |
  // | �4.1.1) fields. A non-empty token cookie-name and a mandatory "=" (with |
  // | no surrounding whitespace) are required, and the remainder must be a    |
  // | valid cookie-value (which may be empty). Any trailing bytes after the   |
  // | cookie-value cause rejection.                                           |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_cookie_pair(std::string_view sv) noexcept {
    // A non-empty token cookie-name starts the pair.
    const std::string_view name = consume_token(sv);
    if (name.empty()) return false;
    // A "=" is mandatory after the name, with no surrounding whitespace.
    if (name.size() >= sv.size() || sv[name.size()] != '=') return false;
    // The remainder (possibly empty) must be a valid cookie-value.
    return is_cookie_value(sv.substr(name.size() + 1));
  }
  // +=========================================================================+
  // | [>] is_cookie_av                                             ( public ) |
  // +=========================================================================+
  // | RFC 6265 �4.1.1 � cookie-av                                             |
  // +-------------------------------------------------------------------------+
  // | cookie-av    = expires-av / max-age-av / domain-av / path-av /          |
  // |                secure-av / httponly-av / extension-av                   |
  // | extension-av = <any CHAR except CTLs or ";">                            |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is exactly one cookie-av of a Set-Cookie |
  // | field (RFC 6265 �4.1.1). Every named alternative (Expires, Max-Age,     |
  // | Domain, Path, Secure, HttpOnly) is subsumed by extension-av, which is   |
  // | the catch-all "any CHAR except CTLs or ';'"; at the syntactic ABNF      |
  // | level a cookie-av is therefore just a non-empty run of cookie-av-       |
  // | octets. Per-attribute semantics (RFC 6265 �5.2) are applied by the      |
  // | recipient after parsing and are intentionally outside this check, so a  |
  // | value such as "Max-Age=abc" remains a valid extension-av. Every cookie- |
  // | av alternative is non-empty, so an empty sv is rejected.                |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_cookie_av(std::string_view sv) noexcept {
    if (sv.empty()) return false;
    for (const char c : sv) {
      if (!is_cookie_av_octet(c)) return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] is_directive                                             ( public ) |
  // +=========================================================================+
  // | RFC 9111 �5.2 / �5.4 � parameterized directive                          |
  // +-------------------------------------------------------------------------+
  // | directive = token [ "=" ( token / quoted-string ) ]                     |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is exactly one parameterized directive:  |
  // | a non-empty token optionally followed by "=" and a token or quoted-     |
  // | string argument, with no surrounding whitespace around "=". This is the |
  // | shared shape of cache-directive (RFC 9111 �5.2) and extension-pragma    |
  // | (RFC 9111 �5.4); "no-cache" is itself a token and is therefore covered  |
  // | by the token alternative. Since directive and extension-param share the |
  // | same grammar, validation delegates to consume_extension_parameter and   |
  // | then requires that the whole sv was consumed. Any trailing bytes after  |
  // | the directive cause rejection.                                          |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_directive(std::string_view sv) noexcept {
    std::size_t bytes_used = 0;
    if (!consume_extension_parameter(sv, bytes_used)) return false;
    return bytes_used == sv.size();
  }
  // +=========================================================================+
  // | [>] consume_parameter                                        ( public ) |
  // +=========================================================================+
  // | RFC 9110 �5.6.6 � parameter                                             |
  // +-------------------------------------------------------------------------+
  // | parameter       = parameter-name "=" parameter-value                    |
  // | parameter-name  = token                                                 |
  // | parameter-value = ( token / quoted-string )                             |
  // +-------------------------------------------------------------------------+
  // | Consumes a single parameter starting at the beginning of sv, reporting  |
  // | the number of bytes consumed via bytes_used. A non-empty token name, a  |
  // | mandatory "=", and a non-empty token-or-quoted-string value are all     |
  // | required. When allow_bws is true, optional whitespace (OWS/BWS) is      |
  // | tolerated around the "=", as permitted for transfer-parameter and t-    |
  // | ranking parameters; when false, no whitespace is allowed around "=".    |
  // | bytes_used is set to 0 and false is returned on any failure.            |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_parameter(std::string_view sv,
                                          std::size_t& bytes_used,
                                          bool allow_bws) noexcept {
    bytes_used = 0;
    std::size_t i = 0;
    const std::string_view name = consume_token(sv);
    if (name.empty()) return false;
    i += name.size();
    // Optional BWS before "=" (only when allowed).
    if (allow_bws) {
      while (i < sv.size() && is_ows(sv[i])) i++;
    }
    // A "=" is mandatory after the parameter name.
    if (i >= sv.size() || sv[i++] != '=') return false;
    // Optional BWS after "=" (only when allowed).
    if (allow_bws) {
      while (i < sv.size() && is_ows(sv[i])) i++;
    }
    // A parameter value is mandatory after "=".
    if (i >= sv.size()) return false;
    const std::string_view value = consume_token_or_quoted_string(sv.substr(i));
    if (value.empty()) return false;
    bytes_used = i + value.size();
    return true;
  }
  // +=========================================================================+
  // | [>] consume_extension_parameter                              ( public ) |
  // +=========================================================================+
  // | RFC 6455 �9.1 � extension-param                                         |
  // +-------------------------------------------------------------------------+
  // | extension-param = token [ "=" ( token / quoted-string ) ]               |
  // +-------------------------------------------------------------------------+
  // | Consumes a single extension parameter starting at the beginning of sv,  |
  // | reporting the number of bytes consumed via bytes_used. A non-empty      |
  // | token name is mandatory, but unlike consume_parameter() the value is    |
  // | optional: when a "=" follows the name (with no surrounding whitespace)  |
  // | a non-empty token-or-quoted-string value is required, otherwise the     |
  // | parameter is just the bare name. This is the shared shape of extension- |
  // | param (RFC 6455 �9.1) and directive (RFC 9111 �5.2 / �5.4). bytes_used  |
  // | is set to 0 and false is returned on any failure.                       |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_extension_parameter(
      std::string_view sv, std::size_t& bytes_used) noexcept {
    bytes_used = 0;
    const std::string_view name = consume_token(sv);
    if (name.empty()) return false;
    std::size_t i = name.size();
    // The value is optional: a bare parameter name is a complete parameter.
    if (i >= sv.size() || sv[i] != '=') {
      bytes_used = i;
      return true;
    }
    // A "=" is present, so a value is mandatory (no whitespace around "=").
    ++i;
    if (i >= sv.size()) return false;
    const std::string_view value = consume_token_or_quoted_string(sv.substr(i));
    if (value.empty()) return false;
    bytes_used = i + value.size();
    return true;
  }
  // +=========================================================================+
  // | [>] for_each_parameter                                       ( public ) |
  // +=========================================================================+
  // | RFC 9110 �5.6.6 � parameters                                            |
  // +-------------------------------------------------------------------------+
  // | parameters = *( OWS ";" OWS [ parameter ] )                             |
  // +-------------------------------------------------------------------------+
  // | Iterates the ";"-delimited parameters that follow a value (media-type,  |
  // | media-range, transfer-coding, t-codings, expectation, ...), trimming    |
  // | OWS around each ";". A ";" is mandatory before every parameter. When    |
  // | require_parameter is false, empty parameter slots and a trailing ";"    |
  // | are tolerated (as in media-type/media-range/expectation parameters);    |
  // | when true, a parameter is mandatory after every ";" (as in transfer-    |
  // | parameters and t-codings parameters). For every parameter slot the      |
  // | callable consume(remaining, bytes_used) is invoked with the sub-view    |
  // | starting at the parameter; it must return true and set bytes_used to    |
  // | the number of bytes it consumed. The whole sequence is rejected as soon |
  // | as consume rejects a parameter or reports an out-of-range bytes_used.   |
  // +-------------------------------------------------------------------------+
  template <typename ParameterConsumer>
  static constexpr bool for_each_parameter(std::string_view sv,
                                           bool require_parameter,
                                           ParameterConsumer&& consume) {
    std::size_t i = 0;
    while (i < sv.size()) {
      // OWS before ";".
      while (i < sv.size() && is_ows(sv[i])) i++;
      // A ";" is mandatory before every parameter.
      if (i >= sv.size() || sv[i++] != ';') return false;
      // OWS after ";".
      while (i < sv.size() && is_ows(sv[i])) i++;
      if (i >= sv.size()) {
        // Nothing follows the ";".
        if (require_parameter) return false;
        return true;
      }
      // An empty parameter slot is only allowed when not required.
      if (sv[i] == ';') {
        if (require_parameter) return false;
        continue;
      }
      std::size_t bytes = 0;
      if (!consume(sv.substr(i), bytes)) return false;
      if (bytes == 0 || bytes > sv.size() - i) return false;
      i += bytes;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] for_each_forwarded_pair                                  ( public ) |
  // +=========================================================================+
  // | RFC 7239 �4 � forwarded-element                                         |
  // +-------------------------------------------------------------------------+
  // | forwarded-element = [ forwarded-pair ] *( ";" [ forwarded-pair ] )      |
  // +-------------------------------------------------------------------------+
  // | Iterates the ";"-delimited pairs of a single forwarded-element. Unlike  |
  // | for_each_parameter (RFC 9110 �5.6.6 parameters), no OWS is permitted    |
  // | around ";" and the first pair is not preceded by ";", matching the      |
  // | RFC 7239 grammar exactly. Every pair slot is optional, so empty slots   |
  // | (a leading ";", consecutive ";;", or a trailing ";") are tolerated and  |
  // | an entirely empty sv is accepted. For every non-empty pair slot the     |
  // | callable consume(remaining, bytes_used) is invoked with the sub-view    |
  // | starting at the pair; it must return true and set bytes_used to the     |
  // | number of bytes it consumed. The whole element is rejected as soon as   |
  // | consume rejects a pair or reports an out-of-range bytes_used.           |
  // +-------------------------------------------------------------------------+
  template <typename PairConsumer>
  static constexpr bool for_each_forwarded_pair(std::string_view sv,
                                                PairConsumer&& consume) {
    std::size_t i = 0;
    bool first = true;
    while (i < sv.size()) {
      // A ";" separates pairs and is mandatory before every pair except the
      // first; no OWS is permitted around it.
      if (!first) {
        if (sv[i] != ';') return false;
        ++i;
      }
      first = false;
      // Each pair slot is optional, so a ";" or the end of sv may follow.
      if (i >= sv.size() || sv[i] == ';') continue;
      std::size_t bytes = 0;
      if (!consume(sv.substr(i), bytes)) return false;
      if (bytes == 0 || bytes > sv.size() - i) return false;
      i += bytes;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] for_each_list_element                                    ( public ) |
  // +=========================================================================+
  // | RFC 9110 �5.6.1 � Lists (#rule ABNF extension)                          |
  // +-------------------------------------------------------------------------+
  // | #element = [ element ] *( OWS "," OWS [ element ] )                     |
  // +-------------------------------------------------------------------------+
  // | Iterates the comma-separated elements of a list-based field value,      |
  // | trimming OWS around each element and skipping empty list elements, as   |
  // | recipients are required to do (RFC 9110 �5.6.1). A comma occurring      |
  // | inside a quoted-string (RFC 9110 �5.6.4) is treated as part of that     |
  // | quoted-string rather than as a list separator; an unterminated quoted-  |
  // | string or a dangling escape at the end of sv is rejected.               |
  // | For every non-empty trimmed element, invokes consume_element(element).  |
  // | The whole list is rejected as soon as one element is                    |
  // | rejected by consume_element. |                                          |
  // +-------------------------------------------------------------------------+
  template <typename ElementConsumer>
  static constexpr bool for_each_list_element(
      std::string_view sv, ElementConsumer&& consume_element) {
    bool follows_separator = false;
    std::size_t i = 0;
    std::size_t last = 0;
    bool inside_string = false;
    while (i < sv.size()) {
      // We need to handle quoted strings and escaped characters properly.
      if (sv[i] == '"') {
        inside_string = !inside_string;
        i++;
        continue;
      }
      // Handle escaped characters inside quoted strings.
      if (sv[i] == '\\') {
        if (!inside_string || i + 1 >= sv.size()) return false;
        i += 2;
        continue;
      }
      if (sv[i] == ',' && !inside_string) {
        // We found an element, let's parse it.
        std::string_view element = sv.substr(last, i++ - last);
        if (follows_separator) ows_ltrim(element);
        ows_rtrim(element);
        if (!element.empty() && !consume_element(element)) return false;
        follows_separator = true;
        last = i;
        continue;
      }
      i++;
    }
    // Check if we ended inside a quoted string, which would be invalid.
    if (inside_string) return false;
    // Last element after the last comma (or the only one if no commas).
    std::string_view element = sv.substr(last);
    if (follows_separator) ows_ltrim(element);
    if (!element.empty() && !consume_element(element)) return false;
    return true;
  }
  // +=========================================================================+
  // | [>] check_auth_param_list                                    ( public ) |
  // +=========================================================================+
  // | RFC 9110 �11.6.3 / �11.4 � #auth-param                                  |
  // +-------------------------------------------------------------------------+
  // | Authentication-Info = #auth-param                                       |
  // | auth-param          = token BWS "=" BWS ( token / quoted-string )       |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is a "#auth-param" list: a comma-        |
  // | separated sequence of auth-param name/value pairs. This is the shared   |
  // | shape of the Authentication-Info (RFC 9110 �11.6.3) header field value  |
  // | and of the "#auth-param" argument of a single credentials/challenge     |
  // | (RFC 9110 �11.4 / �11.6.1). Every non-empty list element must be a      |
  // | complete auth-param, with BWS tolerated around "=". Since the rule is   |
  // | "#auth-param" (not "1#auth-param"), an empty list is permitted and      |
  // | empty list elements are ignored.                                        |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_auth_param_list(std::string_view sv) noexcept {
    return for_each_list_element(sv, [](std::string_view element) {
      std::size_t bytes_used = 0;
      if (!consume_parameter(element, bytes_used, /*allow_bws=*/true)) {
        return false;
      }
      // The auth-param must consume the whole trimmed list element.
      return bytes_used == element.size();
    });
  }
  // +=========================================================================+
  // | [>] check_credentials                                        ( public ) |
  // +=========================================================================+
  // | RFC 9110 �11.4 / �11.6.2 � credentials                                  |
  // +-------------------------------------------------------------------------+
  // | credentials = auth-scheme [ 1*SP ( token68 / #auth-param ) ]            |
  // | auth-scheme = token                                                     |
  // | auth-param  = token BWS "=" BWS ( token / quoted-string )               |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is exactly one credentials production,   |
  // | the shared shape of the Authorization (RFC 9110 �11.6.2) and Proxy-     |
  // | Authorization (RFC 9110 �11.7.1) header field values. A mandatory       |
  // | auth-scheme token is consumed first; when nothing follows, the optional |
  // | group is absent and the credentials consist of the bare scheme.         |
  // | Otherwise exactly one or more SP (%x20, never HTAB) separates the       |
  // | scheme from its argument, and the remainder must be either a single     |
  // | token68 or a "#auth-param" list. The token68 alternative is tried       |
  // | first so that a bare token argument (a valid token68 but not a valid    |
  // | auth-param) is accepted; otherwise the remainder is validated as a      |
  // | comma-separated auth-param list via for_each_list_element, where every  |
  // | non-empty element must be a complete auth-param (BWS around "=" is      |
  // | tolerated). Since the rule is "#auth-param" (not "1#auth-param"), an    |
  // | empty list is permitted, and empty list elements are ignored.           |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_credentials(std::string_view sv) noexcept {
    // A mandatory auth-scheme token starts the credentials.
    const std::string_view scheme = consume_token(sv);
    if (scheme.empty()) return false;
    // When nothing follows the scheme, the optional group is absent.
    if (scheme.size() == sv.size()) return true;
    // At least one SP (%x20 only, never HTAB) must separate scheme and arg.
    std::size_t i = scheme.size();
    if (sv[i] != ' ') return false;
    while (i < sv.size() && sv[i] == ' ') i++;
    const std::string_view rest = sv.substr(i);
    // The argument is either a single token68 ...
    if (is_token68(rest)) return true;
    // ... or a "#auth-param" list (empty list permitted).
    return check_auth_param_list(rest);
  }
  // +=========================================================================+
  // | [>] challenge_opens_param_list                               ( public ) |
  // +=========================================================================+
  // | RFC 9110 �11.6.1 � helper for #challenge parsing                        |
  // +-------------------------------------------------------------------------+
  // | Given a single, already-trimmed challenge element that is known to be a |
  // | valid challenge (i.e. check_credentials(element) is true), reports      |
  // | whether that challenge opens a "#auth-param" list, meaning that later   |
  // | comma-separated auth-param elements may still belong to it. This is the |
  // | case only when the challenge has a "1*SP" argument region whose content |
  // | is an auth-param rather than a token68; a scheme-only challenge or a    |
  // | token68 challenge does not accept trailing auth-params.                 |
  // +-------------------------------------------------------------------------+
  static constexpr bool challenge_opens_param_list(
      std::string_view element) noexcept {
    const std::string_view scheme = consume_token(element);
    // A scheme-only challenge has no argument region and no auth-param list.
    if (scheme.empty() || scheme.size() == element.size()) return false;
    std::size_t i = scheme.size();
    // The scheme is separated from its argument by 1*SP (%x20 only).
    while (i < element.size() && element[i] == ' ') i++;
    // A token68 argument is a single value and opens no auth-param list.
    return !is_token68(element.substr(i));
  }
  // +=========================================================================+
  // | [>] check_challenge_list                                     ( public ) |
  // +=========================================================================+
  // | RFC 9110 �11.6.1 / �11.7.2 � #challenge                                 |
  // +-------------------------------------------------------------------------+
  // | WWW-Authenticate  = #challenge                                          |
  // | Proxy-Authenticate = #challenge                                         |
  // | challenge         = auth-scheme [ 1*SP ( token68 / #auth-param ) ]      |
  // | auth-scheme       = token                                               |
  // | auth-param        = token BWS "=" BWS ( token / quoted-string )         |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is a "#challenge" list, the shared shape |
  // | of the WWW-Authenticate (RFC 9110 �11.6.1) and Proxy-Authenticate (RFC  |
  // | 9110 �11.7.2) header field values. The list is comma-separated, but so  |
  // | is the inner "#auth-param" list of each challenge, which makes commas   |
  // | ambiguous; the field is therefore scanned left-to-right, resolving each |
  // | top-level (quoted-string-aware) element greedily. Every non-empty       |
  // | element is either an auth-param that continues the current challenge or |
  // | the start of a new challenge. An element that parses fully as an        |
  // | auth-param ("token [BWS] = value") continues the current challenge, but |
  // | only when that challenge previously opened a "#auth-param" list; a bare |
  // | "token = value" is otherwise not a valid challenge and is rejected.     |
  // | Any other element must itself be a complete challenge (validated with   |
  // | check_credentials), which begins a new challenge and may reopen an      |
  // | auth-param list. Since the rule is "#challenge" (not "1#challenge"), an |
  // | empty list is permitted and empty list elements are ignored.            |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_challenge_list(std::string_view sv) noexcept {
    // in_param_list tracks whether the current challenge still accepts
    // trailing auth-param elements (i.e. it opened a "#auth-param" list).
    bool in_param_list = false;
    return for_each_list_element(sv, [&in_param_list](std::string_view element) {
      // An element that fully parses as an auth-param continues the current
      // challenge, but only when that challenge opened an auth-param list.
      std::size_t bytes_used = 0;
      if (consume_parameter(element, bytes_used, /*allow_bws=*/true) &&
          bytes_used == element.size()) {
        return in_param_list;
      }
      // Otherwise the element must itself be a complete challenge, which
      // starts a new challenge and may (re)open an auth-param list.
      if (!check_credentials(element)) return false;
      in_param_list = challenge_opens_param_list(element);
      return true;
    });
  }
  // +=========================================================================+
  // | [>] consume_product                                          ( public ) |
  // +=========================================================================+
  // | RFC 9110 �10.1.5 / �10.2.4 � product                                    |
  // +-------------------------------------------------------------------------+
  // | product         = token [ "/" product-version ]                         |
  // | product-version = token                                                 |
  // +-------------------------------------------------------------------------+
  // | Consumes a single product from the front of sv and returns the consumed |
  // | prefix, or an empty string_view if sv does not begin with a product. A  |
  // | product is a non-empty token, optionally followed by a "/" and a non-   |
  // | empty product-version token; a trailing "/" with no version is invalid. |
  // | A valid product is never empty, so an empty result signals failure.     |
  // +-------------------------------------------------------------------------+
  static constexpr std::string_view consume_product(
      std::string_view sv) noexcept {
    const std::string_view name = consume_token(sv);
    if (name.empty()) return {};
    std::size_t i = name.size();
    // An optional "/" must be followed by a non-empty product-version token.
    if (i < sv.size() && sv[i] == '/') {
      const std::string_view version = consume_token(sv.substr(i + 1));
      if (version.empty()) return {};
      i += 1 + version.size();
    }
    return sv.substr(0, i);
  }
  // +=========================================================================+
  // | [>] check_product_list                                       ( public ) |
  // +=========================================================================+
  // | RFC 9110 �10.1.5 / �10.2.4 � product list                               |
  // +-------------------------------------------------------------------------+
  // | product-list = product *( RWS ( product / comment ) )                   |
  // | RWS          = 1*( SP / HTAB )                                          |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is exactly one product list, the shared  |
  // | shape of the User-Agent (RFC 9110 �10.1.5) and Server (RFC 9110         |
  // | �10.2.4) header field values. The value MUST begin with a product, so   |
  // | an empty value, a leading comment, or leading whitespace is rejected.   |
  // | Each subsequent product or comment MUST be preceded by RWS (at least    |
  // | one SP/HTAB); adjacent items with no RWS, and trailing RWS with no      |
  // | following item, are rejected. This is not an RFC 9110 �5.6.1 "#" list:  |
  // | commas carry no structural meaning and are only valid inside token or   |
  // | comment syntax.                                                         |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_product_list(std::string_view sv) noexcept {
    // The list MUST begin with a product (no leading comment or whitespace).
    const std::string_view first = consume_product(sv);
    if (first.empty()) return false;
    std::size_t i = first.size();
    while (i < sv.size()) {
      // Every subsequent item MUST be preceded by RWS (1*( SP / HTAB )).
      const std::size_t rws_start = i;
      while (i < sv.size() && is_ows(sv[i])) i++;
      if (i == rws_start) return false;
      // RWS must be followed by a product or comment, never left trailing.
      if (i >= sv.size()) return false;
      const std::string_view rest = sv.substr(i);
      if (rest.front() == '(') {
        const std::string_view comment = consume_comment(rest);
        if (comment.empty()) return false;
        i += comment.size();
      } else {
        const std::string_view product = consume_product(rest);
        if (product.empty()) return false;
        i += product.size();
      }
    }
    return true;
  }
  // +=========================================================================+
  // | [>] check_uri_host                                           ( public ) |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | uri-host       | IP-literal / IPv4address / reg-name                    |
  // +----------------+--------------------------------------------------------+
  static host_type check_uri_host(std::string_view sv) {
    // if the provided field-value is empty then we assume it to be regname..
    if (sv.empty()) return host_type::kRegName;
    // check [ip-literal] host..
    if (helpers::is_ip_literal(sv)) return host_type::kIpLiteral;
    // check [ip-v4-address] host..
    if (helpers::is_ip_v4_address(sv)) return host_type::kIpV4Address;
    // check [reg-name] host..
    if (helpers::is_reg_name(sv)) return host_type::kRegName;
    // default [unknown] host type..
    return host_type::kUnknown;
  }
  // +=========================================================================+
  // | [>] is_uri_scheme                                            ( public ) |
  // +=========================================================================+
  // | RFC 3986 �3.1 � scheme                                                  |
  // +-------------------------------------------------------------------------+
  // | scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )                     |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is a bare URI scheme name (no trailing   |
  // | ":"). This is the shared shape of a URI scheme (RFC 3986 �3.1), of the  |
  // | "proto" parameter value of the Forwarded header (RFC 7239 �5.4), and of |
  // | an X-Forwarded-Proto list element. Returns false on an empty value, a   |
  // | non-ALPHA first character, or any subsequent character outside          |
  // | ALPHA / DIGIT / "+" / "-" / ".".                                        |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_uri_scheme(std::string_view sv) noexcept {
    if (sv.empty() || !is_alpha(sv[0])) return false;
    for (std::size_t i = 1; i < sv.size(); i++) {
      if (!is_alpha(sv[i]) && !is_digit(sv[i]) && sv[i] != '+' &&
          sv[i] != '-' && sv[i] != '.') {
        return false;
      }
    }
    return true;
  }
  // +=========================================================================+
  // | [>] consume_uri_scheme                                       ( public ) |
  // +=========================================================================+
  // | RFC 3986 �3.1 � scheme                                                  |
  // +-------------------------------------------------------------------------+
  // | scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )                     |
  // +-------------------------------------------------------------------------+
  // | Detects an optional leading "scheme ':'" prefix. On success, advances   |
  // | 'i' past the ':' and returns true; otherwise leaves 'i' untouched and   |
  // | returns false (the value is then treated as a relative reference). The  |
  // | scheme portion is validated with is_uri_scheme.                         |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_uri_scheme(std::string_view sv,
                                           std::size_t& i) noexcept {
    // A URI scheme is delimited by the mandatory ":" separator.
    const std::size_t colon_at = sv.find(':');
    if (colon_at == std::string_view::npos) return false;
    if (!is_uri_scheme(sv.substr(0, colon_at))) return false;
    i = colon_at + 1;
    return true;
  }
  // +=========================================================================+
  // | [>] consume_uri_pct_encoded                                  ( public ) |
  // +=========================================================================+
  // | RFC 3986 �2.1 � pct-encoded                                             |
  // +-------------------------------------------------------------------------+
  // | pct-encoded = "%" HEXDIG HEXDIG                                         |
  // +-------------------------------------------------------------------------+
  // | Assumes sv[i] == '%'. On success, advances 'i' past the two HEXDIGs     |
  // | and returns true; otherwise returns false.                              |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_uri_pct_encoded(std::string_view sv,
                                                std::size_t& i) noexcept {
    if (i + 2 >= sv.size()) return false;
    if (!is_hex_digit(sv[i + 1]) || !is_hex_digit(sv[i + 2])) return false;
    i += 3;
    return true;
  }
  // +=========================================================================+
  // | [>] check_uri_userinfo                                       ( public ) |
  // +=========================================================================+
  // | RFC 3986 �3.2.1 � userinfo                                              |
  // +-------------------------------------------------------------------------+
  // | userinfo = *( unreserved / pct-encoded / sub-delims / ":" )             |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_uri_userinfo(std::string_view sv) noexcept {
    std::size_t i = 0;
    while (i < sv.size()) {
      if (sv[i] == '%') {
        if (!consume_uri_pct_encoded(sv, i)) return false;
        continue;
      }
      if (!is_unreserved(sv[i]) && !is_sub_delim(sv[i]) && sv[i] != ':') {
        return false;
      }
      i++;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] check_host_port                                          ( public ) |
  // +=========================================================================+
  // | RFC 3986 �3.2.2 / �3.2.3 � host [ ":" port ]                            |
  // +-------------------------------------------------------------------------+
  // | host = IP-literal / IPv4address / reg-name                              |
  // | port = *DIGIT                                                           |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is a uri-host optionally followed by a   |
  // | ":" port. This is the userinfo-free portion of an authority, shared by  |
  // | the Host header (RFC 9110 �7.2), an authority-form request-target, and  |
  // | the serialized-origin of the Origin header (RFC 6454 �7). An IP-literal |
  // | host keeps its bracketed colons intact (via split_authority_host_port), |
  // | and both an empty reg-name host and an empty port are syntactically     |
  // | valid because each uses the "*" repetition operator. Returns false only |
  // | on an unterminated IP-literal, an unknown host form, a missing ":"      |
  // | before a port, or a non-DIGIT port.                                     |
  // +-------------------------------------------------------------------------+
  // | The producer overload additionally exposes the parsed pieces on         |
  // | success: uri_host receives the uri-host substring (bracketed colons of  |
  // | an IP-literal kept intact), port receives the raw *DIGIT run after the  |
  // | ":" (empty when no port is present), and type classifies the host. On   |
  // | failure the out-parameters are left unspecified. Both overloads share   |
  // | the same parsing so the syntactic and producing paths never diverge.    |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_host_port(std::string_view sv,
                                        std::string_view& uri_host,
                                        std::string_view& port,
                                        host_type& type) noexcept {
    std::size_t colon_at;
    if (!split_authority_host_port(sv, uri_host, colon_at)) return false;
    // host can be empty because reg-name = *( ... ).
    type = uri_host.empty() ? host_type::kRegName : check_uri_host(uri_host);
    if (type == host_type::kUnknown) return false;
    port = {};
    if (colon_at < sv.size()) {
      if (sv[colon_at] != ':') return false;
      // port = *DIGIT, so an empty port is syntactically valid.
      port = sv.substr(colon_at + 1);
      if (!is_port(port)) return false;
    }
    return true;
  }
  static constexpr bool check_host_port(std::string_view sv) noexcept {
    std::string_view uri_host;
    std::string_view port;
    host_type type;
    return check_host_port(sv, uri_host, port, type);
  }
  // +=========================================================================+
  // | [>] check_serialized_origin                                  ( public ) |
  // +=========================================================================+
  // | RFC 6454 �6.1 / WHATWG Fetch �3.2 � serialized-origin                   |
  // +-------------------------------------------------------------------------+
  // | serialized-origin = scheme "://" host [ ":" port ]                      |
  // +-------------------------------------------------------------------------+
  // | Validates that sv, as a whole, is a single serialized origin: a URI     |
  // | scheme, the literal "://", and a userinfo-free "host [ ":" port ]". A   |
  // | serialized origin has no path, query, fragment, trailing slash, or      |
  // | userinfo. This is the shared shape of an Origin list element (RFC 6454  |
  // | �7) and of the non-"*"/"null" alternative of Access-Control-Allow-      |
  // | Origin (WHATWG Fetch �3.3.4). Returns false on a missing/invalid        |
  // | scheme, a scheme not followed by "//", or an invalid host/port.         |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_serialized_origin(std::string_view sv) noexcept {
    std::size_t i = 0;
    // consume_uri_scheme validates "scheme :" and advances past the colon.
    if (!consume_uri_scheme(sv, i)) return false;
    // The scheme's colon must be followed by "//".
    if (i + 1 >= sv.size() || sv[i] != '/' || sv[i + 1] != '/') return false;
    i += 2;
    // The remainder is a userinfo-free "host [ ":" port ]".
    return check_host_port(sv.substr(i));
  }
  // +=========================================================================+
  // | [>] check_uri_authority                                      ( public ) |
  // +=========================================================================+
  // | RFC 3986 �3.2 � authority                                               |
  // +-------------------------------------------------------------------------+
  // | authority = [ userinfo "@" ] host [ ":" port ]                          |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_uri_authority(std::string_view sv) noexcept {
    std::string_view rest = sv;
    const std::size_t at_pos = sv.find('@');
    if (at_pos != std::string_view::npos) {
      if (!check_uri_userinfo(sv.substr(0, at_pos))) return false;
      rest = sv.substr(at_pos + 1);
    }
    // The remainder is the userinfo-free "host [ ":" port ]" portion.
    return check_host_port(rest);
  }
  // +=========================================================================+
  // | [>] check_uri_reference                                      ( public ) |
  // +=========================================================================+
  // | RFC 3986 �4.1 � URI-reference                                           |
  // +-------------------------------------------------------------------------+
  // | URI-reference = URI / relative-ref                                      |
  // | URI           = scheme ":" hier-part [ "?" query ] [ "#" fragment ]     |
  // | relative-ref  = relative-part [ "?" query ] [ "#" fragment ]            |
  // | hier-part     = "//" authority path-abempty / path-absolute /           |
  // |                 path-rootless / path-empty                              |
  // | relative-part = "//" authority path-abempty / path-absolute /           |
  // |                 path-noscheme / path-empty                              |
  // | query         = *( pchar / "/" / "?" )                                  |
  // | fragment      = *( pchar / "/" / "?" )                                  |
  // +-------------------------------------------------------------------------+
  // | Validates a whole, normalized field value (no surrounding OWS). When    |
  // | 'allow_fragment' is false the grammar is restricted to                  |
  // | "absolute-URI / partial-URI" (used by Content-Location), so an          |
  // | unencoded "#" is rejected. When 'allow_fragment' is true the optional   |
  // | "#" fragment component is accepted (used by Location).                  |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_uri_reference(std::string_view sv,
                                            bool allow_fragment) noexcept {
    // An empty value is syntactically permitted by relative-ref (path-empty).
    if (sv.empty()) return true;
    std::size_t i = 0;
    // Consume "scheme ':'" if present. When it is not, we are dealing with a
    // relative reference, which forbids ':' in the first (schemeless) path
    // segment. When it is, the hier-part allows ':' in path-rootless without
    // any ambiguity.
    const bool has_scheme = consume_uri_scheme(sv, i);
    // hier-part / relative-part:
    //   "//" authority path-abempty
    //   / path-absolute
    //   / path-rootless (hier-part only) or path-noscheme (relative-part only)
    //   / path-empty
    bool has_authority = false;
    if (i + 1 < sv.size() && sv[i] == '/' && sv[i + 1] == '/') {
      i += 2;
      const std::size_t authority_start = i;
      while (i < sv.size() && sv[i] != '/' && sv[i] != '?' && sv[i] != '#') {
        i++;
      }
      if (!check_uri_authority(sv.substr(authority_start, i - authority_start))) {
        return false;
      }
      has_authority = true;
    }
    // Consume the path, validating "/" separators, pchar, and pct-encoded.
    const std::size_t path_start = i;
    while (i < sv.size() && sv[i] != '?' && sv[i] != '#') {
      if (sv[i] == '%') {
        if (!consume_uri_pct_encoded(sv, i)) return false;
        continue;
      }
      if (sv[i] != '/' && !is_pchar(sv[i])) return false;
      i++;
    }
    // RFC 3986 �3.3: when neither a scheme nor an authority is present, the
    // first path segment MUST NOT contain a colon (path-noscheme); otherwise
    // it would be indistinguishable from a scheme-prefixed reference.
    if (!has_scheme && !has_authority) {
      const std::string_view path = sv.substr(path_start, i - path_start);
      const std::string_view first_segment = path.substr(0, path.find('/'));
      if (first_segment.find(':') != std::string_view::npos) return false;
    }
    // Optional query: "?" query, where query = *( pchar / "/" / "?" ).
    if (i < sv.size() && sv[i] == '?') {
      i++;
      while (i < sv.size() && sv[i] != '#') {
        if (sv[i] == '%') {
          if (!consume_uri_pct_encoded(sv, i)) return false;
          continue;
        }
        if (sv[i] != '/' && sv[i] != '?' && !is_pchar(sv[i])) return false;
        i++;
      }
    }
    // Optional fragment: "#" fragment, where fragment = *( pchar / "/" / "?" ).
    // Only URI-reference (allow_fragment) accepts it; for the restricted
    // "absolute-URI / partial-URI" grammar an unencoded "#" is invalid, so the
    // loops above stop at it and the final length check below fails.
    if (allow_fragment && i < sv.size() && sv[i] == '#') {
      i++;
      while (i < sv.size()) {
        if (sv[i] == '%') {
          if (!consume_uri_pct_encoded(sv, i)) return false;
          continue;
        }
        if (sv[i] != '/' && sv[i] != '?' && !is_pchar(sv[i])) return false;
        i++;
      }
    }
    return i == sv.size();
  }
  // +=========================================================================+
  // | [>] split_authority_host_port                                ( public ) |
  // +=========================================================================+
  // | Splits a "uri-host [ ':' port ]" sequence into its uri-host substring   |
  // | and the position of the uri-host/port separator ':', recognizing        |
  // | IP-literal hosts (e.g. "[::1]") so that colons nested inside "[...]"    |
  // | are never mistaken for that separator.                                  |
  // +-------------------------------------------------------------------------+
  // | Returns false only when sv begins with an unterminated IP-literal (an   |
  // | opening "[" with no matching "]"); callers decide whether that means    |
  // | "invalid" (whole-value checks) or "more bytes needed" (streaming        |
  // | parsers).                                                               |
  // +-------------------------------------------------------------------------+
  // | On success, uri_host receives the uri-host substring. colon_at          |
  // | receives:                                                               |
  // |  - for a non-IP-literal uri-host, the index of ':' within sv, or        |
  // |    std::string_view::npos if sv contains no ':' at all;                 |
  // |  - for an IP-literal uri-host, the index right past the closing "]",    |
  // |    which callers MUST bounds-check (it may equal sv.size()) and         |
  // |    compare against sv[colon_at] themselves before treating what         |
  // |    follows as a port, since that position may hold a character other    |
  // |    than ':'.                                                            |
  // +-------------------------------------------------------------------------+
  static constexpr bool split_authority_host_port(
      std::string_view sv, std::string_view& decoded_uri_host,
      std::size_t& colon_found_at) noexcept {
    if (!sv.empty() && sv.front() == '[') {
      const std::size_t clb = sv.find_first_of(']');
      if (clb == std::string_view::npos) return false;
      decoded_uri_host = sv.substr(0, clb + 1);
      colon_found_at = clb + 1;
      return true;
    }
    colon_found_at = sv.find_first_of(':');
    decoded_uri_host = (colon_found_at == std::string_view::npos)
                           ? sv
                           : sv.substr(0, colon_found_at);
    return true;
  }
  // +=========================================================================+
  // | [>] try_to_deserialize_as_authority_form                     ( public ) |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | authority-form | uri-host ":" port                                      |
  // | uri-host       | IP-literal / IPv4address / reg-name                    |
  // | port           | *DIGIT                                                 |
  // | IP-literal     | "[" ( IPv6address / IPvFuture ) "]"                    |
  // | IPvFuture      | "v" 1*HEXDIG "."                                       |
  // |                | 1*( unreserved / sub-delims / ":" )                    |
  // | IPv4address    | dec-octet "." dec-octet "." dec-octet "." dec-octet    |
  // | dec-octet      | DIGIT                                                  |
  // |                | / %x31-39 DIGIT                                        |
  // |                | / "1" 2DIGIT                                           |
  // |                | / "2" %x30-34 DIGIT                                    |
  // |                | / "25" %x30-35                                         |
  // | reg-name       | *( unreserved / pct-encoded / sub-delims )             |
  // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
  // | pct-encoded    | "%" HEXDIG HEXDIG                                      |
  // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /        |
  // |                | "," / ";" / "="                                        |
  // +----------------+--------------------------------------------------------+
  // +-------------------------------------------------------------------------+
  // | On success, uri_host, port, and type additionally expose the parsed     |
  // | uri-host substring, the raw port digits (empty when none follow the     |
  // | ":"), and the host classification, mirroring what check_host_port       |
  // | exposes, so the caller never has to re-parse them.                      |
  // +-------------------------------------------------------------------------+
  static constexpr deserialization_status try_to_deserialize_as_authority_form(
      std::string_view sv, std::string_view& uri_host, std::string_view& port,
      host_type& type, std::size_t& bytes_used) noexcept {
    bytes_used = 0;
    if (sv.empty()) return deserialization_status::kMoreBytesNeeded;
    // uri-host may be an IP-literal (e.g., "[::1]"), whose embedded colons
    // must not be mistaken for the uri-host/port separator. Since reg-name
    // and IPv4address characters never include ":", the first unbracketed
    // ":" reliably marks the uri-host/port separator.
    std::string_view sv_uri_host;
    std::size_t colon_at;
    if (!split_authority_host_port(sv, sv_uri_host, colon_at)) {
      return deserialization_status::kMoreBytesNeeded;
    }
    if (colon_at == std::string_view::npos) {
      return deserialization_status::kMoreBytesNeeded;
    }
    type = check_uri_host(sv_uri_host);
    if (type == host_type::kUnknown) {
      return deserialization_status::kInvalidSource;
    }
    // For the IP-literal branch, colon_at was not confirmed to hold ':' yet;
    // for the non-bracket branch, split_authority_host_port already
    // guarantees it (colon_at can only be npos there, already handled above).
    if (colon_at >= sv.size()) return deserialization_status::kMoreBytesNeeded;
    if (sv[colon_at] != ':') return deserialization_status::kInvalidSource;
    // port = *DIGIT (self-delimiting: greedily consume digits and let the
    // caller decide whether what follows is a valid request-target
    // terminator).
    std::size_t i = colon_at + 1;
    while (i < sv.size() && is_digit(sv[i])) i++;
    uri_host = sv_uri_host;
    port = sv.substr(colon_at + 1, i - (colon_at + 1));
    bytes_used = i;
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // | [>] try_to_deserialize_as_asterisk_form                      ( public ) |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | asterisk-form  | "*"                                                    |
  // +----------------+--------------------------------------------------------+
  static constexpr deserialization_status try_to_deserialize_as_asterisk_form(
      std::string_view sv, std::size_t& bytes_used) noexcept {
    // asterisk-form only ever occupies a single octet within request-target;
    // trailing content (e.g., " HTTP/1.1") is left for the caller to parse.
    if (sv.empty()) return deserialization_status::kMoreBytesNeeded;
    if (sv.front() != '*') return deserialization_status::kInvalidSource;
    bytes_used = 1;
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // | [>] try_to_deserialize_as_origin_form                        ( public ) |
  // +=========================================================================+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | origin-form    | absolute-path [ "?" query ]                            |
  // +----------------+--------------------------------------------------------+
  static constexpr deserialization_status try_to_deserialize_as_origin_form(
      std::string_view sv, std::string_view& consumed_path,
      std::string_view& consumed_query, std::size_t& bytes_used) noexcept {
    std::size_t i;
    std::string_view tmp_path, tmp_query;
    const std::size_t sv_size = sv.size();
    deserialization_status result =
        try_to_deserialize_as_absolute_path(sv, tmp_path, i);
    if (result != deserialization_status::kSucceeded) return result;
    if (i < sv_size && sv[i] == '?') {
      // The "?" is a literal separator external to the query production
      // (query = *( pchar / "/" / "?" )), so it must be skipped before
      // delegating to try_to_deserialize_as_query; otherwise it would be
      // consumed as if it were part of the query content itself.
      std::size_t bytes = 0;
      result = try_to_deserialize_as_query(sv.substr(i + 1), tmp_query, bytes);
      if (result != deserialization_status::kSucceeded) return result;
      i += 1 + bytes;
    }
    bytes_used = i;
    consumed_path = std::move(tmp_path);
    consumed_query = std::move(tmp_query);
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // | [>] default_port_for_scheme                                  ( public ) |
  // +=========================================================================+
  // | RFC 9110 �4.2.1 / �4.2.2 � returns the default port a scheme normalizes |
  // | to when the authority omits one ("80" for "http", "443" for "https",    |
  // | matched case-insensitively). Any other (or empty) scheme has no known   |
  // | default and yields an empty view.                                       |
  // +=========================================================================+
  static constexpr std::string_view default_port_for_scheme(
      std::string_view scheme) noexcept {
    if (iequals(scheme, "http")) return "80";
    if (iequals(scheme, "https")) return "443";
    return std::string_view();
  }
  // +=========================================================================+
  // | [>] ports_equivalent                                         ( public ) |
  // +=========================================================================+
  // | True when two port views denote the same effective port for "scheme".   |
  // | Equal raw digit runs are always equivalent. Otherwise, an omitted (empty|
  // | ) port is normalized to the scheme default before comparing, so e.g.    |
  // | "http" with "" and "80" compare equal while "" and "8080" do not. When  |
  // | the scheme has no known default, only exact string equality qualifies.  |
  // +=========================================================================+
  static constexpr bool ports_equivalent(std::string_view scheme,
                                         std::string_view a,
                                         std::string_view b) noexcept {
    if (a == b) return true;
    const std::string_view def = default_port_for_scheme(scheme);
    if (def.empty()) return false;
    const std::string_view lhs = a.empty() ? def : a;
    const std::string_view rhs = b.empty() ? def : b;
    return lhs == rhs;
  }
  // +=========================================================================+
  // | [>] try_to_deserialize_as_absolute_form                      ( public ) |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | absolute-form  | absolute-URI                                           |
  // | absolute-URI   | scheme ":" hier-part [ "?" query ]                     |
  // | scheme         | ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )             |
  // | hier-part      | "//" authority path-abempty                            |
  // |                | / path-absolute                                        |
  // |                | / path-rootless                                        |
  // |                | / path-empty                                           |
  // | authority      | [ userinfo "@" ] host [ ":" port ]                     |
  // | userinfo       | *( unreserved / pct-encoded / sub-delims / ":" )       |
  // | host           | IP-literal / IPv4address / reg-name                    |
  // | port           | *DIGIT                                                 |
  // | path-abempty   | *( "/" segment )                                       |
  // | path-absolute  | "/" [ segment-nz *( "/" segment ) ]                    |
  // | path-rootless  | segment-nz *( "/" segment )                            |
  // | path-empty     | 0<pchar>                                               |
  // | segment        | *pchar                                                 |
  // | segment-nz     | 1*pchar                                                |
  // | pchar          | unreserved / pct-encoded / sub-delims / ":" / "@"      |
  // | query          | *( pchar / "/" / "?" )                                 |
  // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
  // | pct-encoded    | "%" HEXDIG HEXDIG                                      |
  // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /        |
  // |                | "," / ";" / "="                                        |
  // +----------------+--------------------------------------------------------+
  // +-------------------------------------------------------------------------+
  // | When the "//" authority path-abempty branch is taken, has_authority is  |
  // | set and uri_host, port, and type additionally expose the parsed         |
  // | authority pieces (mirroring check_host_port), so the caller never has   |
  // | to re-parse them; otherwise has_authority is left false and uri_host /  |
  // | port / type are left unspecified. "scheme" always exposes the parsed    |
  // | scheme substring (lower/upper-case as received), enabling default-port  |
  // | normalization of the authority against the scheme.                      |
  // +-------------------------------------------------------------------------+
  static constexpr deserialization_status try_to_deserialize_as_absolute_form(
      std::string_view sv, std::string_view& consumed_path,
      std::string_view& consumed_query, bool& has_authority,
      std::string_view& uri_host, std::string_view& port, host_type& type,
      std::string_view& scheme, std::size_t& bytes_used) noexcept {
    bytes_used = 0;
    consumed_path = std::string_view();
    consumed_query = std::string_view();
    has_authority = false;
    uri_host = std::string_view();
    port = std::string_view();
    type = host_type::kUnknown;
    scheme = std::string_view();
    if (sv.empty()) return deserialization_status::kMoreBytesNeeded;
    const std::size_t sv_size = sv.size();
    std::size_t i = 0;
    // -------------------------------------------------------------------------
    // scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
    // -------------------------------------------------------------------------
    if (!is_alpha(sv[i])) return deserialization_status::kInvalidSource;
    i++;
    while (i < sv_size && sv[i] != ':') {
      if (!is_alpha(sv[i]) && !is_digit(sv[i]) &&
          sv[i] != '+' && sv[i] != '-' && sv[i] != '.') {
        return deserialization_status::kInvalidSource;
      }
      i++;
    }
    if (i >= sv_size) return deserialization_status::kMoreBytesNeeded;
    scheme = sv.substr(0, i);  // scheme text, excluding the ':' terminator.
    i++;  // skip ':'
    // -------------------------------------------------------------------------
    // hier-part = "//" authority path-abempty
    //           / path-absolute / path-rootless / path-empty
    // -------------------------------------------------------------------------
    std::size_t path_start = i;
    if (i + 1 < sv_size && sv[i] == '/' && sv[i + 1] == '/') {
      // "//" authority path-abempty
      i += 2;
      const std::size_t authority_start = i;
      // Scan authority: stop at '/', '?', or any non-authority character.
      while (i < sv_size) {
        const char c = sv[i];
        if (c == '/' || c == '?') break;
        if (c == '%') {
          if (i + 2 >= sv_size) return deserialization_status::kMoreBytesNeeded;
          if (!is_hex_digit(sv[i + 1]) || !is_hex_digit(sv[i + 2])) {
            return deserialization_status::kInvalidSource;
          }
          i += 3;
          continue;
        }
        // Valid authority chars: unreserved / sub-delims / : / @ / [ / ]
        if (!is_unreserved(c) && !is_sub_delim(c) &&
            c != ':' && c != '@' && c != '[' && c != ']') {
          break;  // Non-authority char (e.g., space) ends authority.
        }
        i++;
      }
      const std::string_view authority =
          sv.substr(authority_start, i - authority_start);
      // Validate authority: [ userinfo "@" ] host [ ":" port ]
      const std::size_t at_pos = authority.find('@');
      std::string_view host_port;
      if (at_pos != std::string_view::npos) {
        const std::string_view userinfo = authority.substr(0, at_pos);
        for (std::size_t j = 0; j < userinfo.size();) {
          if (userinfo[j] == '%') {
            // pct-encoded already validated during authority scan.
            j += 3;
            continue;
          }
          if (!is_unreserved(userinfo[j]) && !is_sub_delim(userinfo[j]) &&
              userinfo[j] != ':') {
            return deserialization_status::kInvalidSource;
          }
          j++;
        }
        host_port = authority.substr(at_pos + 1);
      } else {
        host_port = authority;
      }
      std::size_t colon_at;
      if (!split_authority_host_port(host_port, uri_host, colon_at)) {
        return deserialization_status::kMoreBytesNeeded;
      }
      type = check_uri_host(uri_host);
      if (type == host_type::kUnknown) {
        return deserialization_status::kInvalidSource;
      }
      if (colon_at != std::string_view::npos && colon_at < host_port.size() &&
          host_port[colon_at] == ':') {
        port = host_port.substr(colon_at + 1);
        for (std::size_t p = 0; p < port.size(); p++) {
          if (!is_digit(port[p])) {
            return deserialization_status::kInvalidSource;
          }
        }
      }
      has_authority = true;
      path_start = i;
      // path-abempty = *( "/" segment )
      while (i < sv_size && sv[i] != '?') {
        if (sv[i] == '%') {
          if (i + 2 >= sv_size) return deserialization_status::kMoreBytesNeeded;
          if (!is_hex_digit(sv[i + 1]) || !is_hex_digit(sv[i + 2])) {
            return deserialization_status::kInvalidSource;
          }
          i += 3;
          continue;
        }
        if (sv[i] != '/' && !is_pchar(sv[i])) break;  // Non-path char.
        i++;
      }
    } else {
      // path-absolute / path-rootless / path-empty (no authority).
      while (i < sv_size && sv[i] != '?') {
        if (sv[i] == '%') {
          if (i + 2 >= sv_size) return deserialization_status::kMoreBytesNeeded;
          if (!is_hex_digit(sv[i + 1]) || !is_hex_digit(sv[i + 2])) {
            return deserialization_status::kInvalidSource;
          }
          i += 3;
          continue;
        }
        if (sv[i] != '/' && !is_pchar(sv[i])) break;  // Non-path char.
        i++;
      }
    }
    consumed_path = sv.substr(path_start, i - path_start);
    // -------------------------------------------------------------------------
    // [ "?" query ]
    // -------------------------------------------------------------------------
    if (i < sv_size && sv[i] == '?') {
      i++;  // skip '?'
      std::size_t query_bytes = 0;
      std::string_view tmp_query;
      const deserialization_status result =
          try_to_deserialize_as_query(sv.substr(i), tmp_query, query_bytes);
      if (result != deserialization_status::kSucceeded) return result;
      consumed_query = tmp_query;
      i += query_bytes;
    }
    bytes_used = i;
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // | [>] try_to_deserialize_as_absolute_path                      ( public ) |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | absolute-path  | 1*( "/" segment )                                      |
  // | segment        | *pchar                                                 |
  // | pchar          | unreserved / pct-encoded / sub-delims / ":" / "@"      |
  // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
  // | pct-encoded    | "%" HEXDIG HEXDIG                                      |
  // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /        |
  // |                | "," / ";" / "="                                        |
  // +----------------+--------------------------------------------------------+
  static constexpr deserialization_status try_to_deserialize_as_absolute_path(
      std::string_view sv, std::string_view& consumed_path,
      std::size_t& bytes_used) noexcept {
    bytes_used = 0;
    if (sv.empty()) return deserialization_status::kMoreBytesNeeded;
    if (sv.front() != '/') return deserialization_status::kInvalidSource;
    const std::size_t sv_size = sv.size();
    std::size_t i = 1;
    while (i < sv_size) {
      if (sv[i] == '%') {
        if (i + 2 >= sv_size) return deserialization_status::kMoreBytesNeeded;
        if (!helpers::is_hex_digit(sv[i + 1]) ||
            !helpers::is_hex_digit(sv[i + 2])) {
          return deserialization_status::kInvalidSource;
        }
        i += 3;
        continue;
      }
      if (sv[i] != '/') {
        if (!helpers::is_pchar(sv[i])) break;
      }
      i++;
    }
    bytes_used = i;
    consumed_path = sv.substr(0, i);
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // | [>] try_to_deserialize_as_query                              ( public ) |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | query          | *( pchar / "/" / "?" )                                 |
  // | pchar          | unreserved / pct-encoded / sub-delims / ":" / "@"      |
  // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
  // | pct-encoded    | "%" HEXDIG HEXDIG                                      |
  // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /        |
  // |                | "," / ";" / "="                                        |
  // +----------------+--------------------------------------------------------+
  static constexpr deserialization_status try_to_deserialize_as_query(
      std::string_view sv, std::string_view& consumed_query,
      std::size_t& bytes_used) noexcept {
    bytes_used = 0;
    std::size_t i = 0;
    const std::size_t sv_size = sv.size();
    while (i < sv_size) {
      if (sv[i] == '%') {
        if (i + 2 >= sv_size) return deserialization_status::kMoreBytesNeeded;
        if (!helpers::is_hex_digit(sv[i + 1]) ||
            !helpers::is_hex_digit(sv[i + 2])) {
          return deserialization_status::kInvalidSource;
        }
        i += 3;
        continue;
      }
      if (!helpers::is_pchar(sv[i]) && sv[i] != '/' && sv[i] != '?') break;
      i++;
    }
    bytes_used = i;
    consumed_query = sv.substr(0, i);
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // | [>] iequals                                                  ( public ) |
  // +=========================================================================+
  // | Compares two string_views for case-insensitive equality.                |
  // +-------------------------------------------------------------------------+
  static constexpr bool iequals(std::string_view a, std::string_view b) {
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), [](char ac, char bc) {
             return tolower_ascii(ac) == tolower_ascii(bc);
           });
  }
  // +=========================================================================+
  // | [>] get_parameters                                          ( public ) |
  // +-------------------------------------------------------------------------+
  // | Matches route path segments against a pattern. A colon-prefixed pattern |
  // | segment is a parameter; its name and matching path segment are stored  |
  // | as views in parameters and must not outlive pattern or path. The vector |
  // | is cleared before matching.                                             |
  // +=========================================================================+
  [[nodiscard]]
  static bool get_parameters(
      std::string_view pattern, std::string_view path,
      std::vector<std::pair<std::string_view, std::string_view>>& parameters) {
    parameters.clear();
    std::size_t pattern_pos = 0;
    std::size_t path_pos = 0;
    while (pattern_pos < pattern.size() && path_pos < path.size()) {
      if (pattern[pattern_pos] == '/') pattern_pos++;
      if (path[path_pos] == '/') path_pos++;
      const auto pattern_end = pattern.find('/', pattern_pos);
      const auto path_end = path.find('/', path_pos);
      const auto pattern_segment =
          pattern.substr(pattern_pos, pattern_end == std::string_view::npos
                                          ? pattern.size() - pattern_pos
                                          : pattern_end - pattern_pos);
      const auto path_segment = path.substr(
          path_pos, path_end == std::string_view::npos ? path.size() - path_pos
                                                        : path_end - path_pos);
      if (pattern_segment.empty() || path_segment.empty()) return false;
      if (pattern_segment.front() == ':') {
        auto name = pattern_segment.substr(1);
        if (!name.empty() && name.back() == ':') name.remove_suffix(1);
        if (name.empty()) return false;
        parameters.emplace_back(name, path_segment);
      } else if (pattern_segment != path_segment) {
        return false;
      }
      if (pattern_end == std::string_view::npos ||
          path_end == std::string_view::npos) {
        return pattern_end == path_end;
      }
      pattern_pos = pattern_end;
      path_pos = path_end;
    }
    return pattern_pos == pattern.size() && path_pos == path.size();
  }
  // +=========================================================================+
  // | [>] split_query_parameters                                   ( public ) |
  // +=========================================================================+
  // | Splits a raw query string (the query component already stripped of its  |
  // | leading '?') into key/value pairs following the application/x-www-form- |
  // | urlencoded convention:                                                  |
  // |   query      = pair *( "&" pair )                                       |
  // |   pair       = key [ "=" value ]                                        |
  // |   key/value  = *( any char except "&" ; value also except first "=" )   |
  // | Rules: pairs are separated by '&'; the first '=' inside a pair separates|
  // | key from value; a pair without '=' yields an empty value; empty pairs   |
  // | (e.g. produced by "a&&b" or a leading/trailing '&') are skipped. Every  |
  // | emitted view is zero-copy over 'query', so the caller must keep the     |
  // | backing buffer alive (or copy the spans, as request.h does). Writes at  |
  // | most out_keys.size() entries into the caller-provided spans and returns  |
  // | the number of pairs written (excess pairs are dropped, matching the      |
  // | fixed-capacity policy already used for headers).                        |
  // +=========================================================================+
  static constexpr std::size_t split_query_parameters(
      std::string_view query, std::span<std::string_view> out_keys,
      std::span<std::string_view> out_values) noexcept {
    const std::size_t max_pairs = out_keys.size();
    std::size_t count = 0;
    std::size_t i = 0;
    const std::size_t size = query.size();
    while (i < size && count < max_pairs) {
      // Find the end of the current pair (delimited by '&' or end-of-input).
      std::size_t pair_end = i;
      while (pair_end < size && query[pair_end] != '&') pair_end++;
      // Skip empty pairs ("&&", leading/trailing '&').
      if (pair_end != i) {
        std::string_view pair = query.substr(i, pair_end - i);
        std::size_t eq = pair.find('=');
        if (eq == std::string_view::npos) {
          out_keys[count] = pair;
          out_values[count] = std::string_view{};
        } else {
          out_keys[count] = pair.substr(0, eq);
          out_values[count] = pair.substr(eq + 1);
        }
        count++;
      }
      // Advance past the pair and its '&' delimiter (if any).
      i = (pair_end < size) ? pair_end + 1 : pair_end;
    }
    return count;
  }
};
}  // namespace martianlabs::doba::protocol::http11

#endif
