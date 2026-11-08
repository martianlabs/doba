//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef martianlabs_doba_protocol_http_helpers_h
#define martianlabs_doba_protocol_http_helpers_h

#include <array>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <span>
#include <string_view>
#include <utility>

#include "protocol/deserialization.h"

namespace martianlabs::doba::protocol::http {
namespace detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] CONSTANTs                                                  ( public ) |
// +---------------------------------------------------------------------------+
// | Shared HTTP parsing helpers.                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | Look up a byte's RFC character classes in one bitmask. The 512-byte table |
// | keeps hot parse loops off long comparison chains.                         |
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
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] build_char_flags                                         ( function ) |
// +---------------------------------------------------------------------------+
// | Build the table at compile time; no startup work needed.                  |
// +---------------------------------------------------------------------------+
static constexpr std::array<std::uint16_t, 256> build_char_flags() noexcept {
  std::array<std::uint16_t, 256> t{};
  for (int c = 0; c < 256; ++c) {
    const auto u = static_cast<unsigned char>(c);
    // token = tchar: !#$%&'*+-.^_`|~ / DIGIT / ALPHA
    if (u == '!' || u == '#' || u == '$' || u == '%' || u == '&' || u == '\'' ||
        u == '*' || u == '+' || u == '-' || u == '.' || u == '^' || u == '_' ||
        u == '`' || u == '|' || u == '~' || (u >= '0' && u <= '9') ||
        (u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z'))
      t[c] |= kF_token;
    // vchar: 0x21-0x7E
    if (u >= 0x21 && u <= 0x7E) t[c] |= kF_vchar;
    // obs_text: 0x80-0xFF
    if (u >= 0x80) t[c] |= kF_obs_text;
    // ows: SP (0x20) / HTAB (0x09)
    if (u == 0x20 || u == 0x09) t[c] |= kF_ows;
    // pchar = unreserved / sub-delims / ":" / "@"
    //   unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
    //   sub-delims = "!" / "$" / "&" / "'" / "(" / ")" /
    //                "*" / "+" / "," / ";" / "="
    if ((u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z') ||
        (u >= '0' && u <= '9') || u == '-' || u == '.' || u == '_' ||
        u == '~' || u == '!' || u == '$' || u == '&' || u == '\'' || u == '(' ||
        u == ')' || u == '*' || u == '+' || u == ',' || u == ';' || u == '=' ||
        u == ':' || u == '@')
      t[c] |= kF_pchar;
    // qdtext = HTAB / SP / "!" / %x23-5B / %x5D-7E / obs-text
    if (u == 0x09 || u == 0x20 || u == '!' || (u >= 0x23 && u <= 0x5B) ||
        (u >= 0x5D && u <= 0x7E) || u >= 0x80)
      t[c] |= kF_qdtext;
    // ctext = HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text
    if (u == 0x09 || u == 0x20 || (u >= 0x21 && u <= 0x27) ||
        (u >= 0x2A && u <= 0x5B) || (u >= 0x5D && u <= 0x7E) || u >= 0x80)
      t[c] |= kF_ctext;
    // etagc = %x21 / %x23-7E / obs-text
    if (u == 0x21 || (u >= 0x23 && u <= 0x7E) || u >= 0x80) t[c] |= kF_etagc;
    // cookie-octet = %x21 / %x23-2B / %x2D-3A / %x3C-5B / %x5D-7E
    if (u == 0x21 || (u >= 0x23 && u <= 0x2B) || (u >= 0x2D && u <= 0x3A) ||
        (u >= 0x3C && u <= 0x5B) || (u >= 0x5D && u <= 0x7E))
      t[c] |= kF_cookie;
    // cookie-av-octet = %x20-7E except %x3B (";")
    if (u >= 0x20 && u <= 0x7E && u != 0x3B) t[c] |= kF_cookie_av;
    // atext = ALPHA / DIGIT / "!" / "#" / "$" / "%" / "&" / "'" /
    //         "*" / "+" / "-" / "/" / "=" / "?" / "^" / "_" /
    //         "`" / "{" / "|" / "}" / "~"
    if ((u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z') ||
        (u >= '0' && u <= '9') || u == '!' || u == '#' || u == '$' ||
        u == '%' || u == '&' || u == '\'' || u == '*' || u == '+' || u == '-' ||
        u == '/' || u == '=' || u == '?' || u == '^' || u == '_' || u == '`' ||
        u == '{' || u == '|' || u == '}' || u == '~')
      t[c] |= kF_atext;
    // dtext = %x21-5A / %x5E-7E / obs-text
    if ((u >= 0x21 && u <= 0x5A) || (u >= 0x5E && u <= 0x7E) || u >= 0x80)
      t[c] |= kF_dtext;
    // hex-digit = DIGIT / "A"-"F" / "a"-"f"
    if ((u >= '0' && u <= '9') || (u >= 'A' && u <= 'F') ||
        (u >= 'a' && u <= 'f'))
      t[c] |= kF_hexdig;
    // unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
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
// | Shared HTTP parsing helpers.                                              |
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
  // | RFC 5234 (ABNF Core Rules) S DIGIT                                      |
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
  // | RFC 9110 S5.6.1 / RFC 9111 S1.2.2 S 1*DIGIT (delta-seconds)             |
  // +-------------------------------------------------------------------------+
  // | 1*DIGIT                                                                 |
  // +-------------------------------------------------------------------------+
  // | Check a nonempty run of digits, nothing more. Overflow belongs to       |
  // | numeric parsing; this check only covers syntax (RFC 9111 S1.2.2, RFC    |
  // | 9110 S8.6).                                                             |
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
  // | RFC 9110 S5.6.1 S 1*DIGIT                                               |
  // +-------------------------------------------------------------------------+
  // | Parse a nonempty run of digits into size_t. Reject other bytes and      |
  // | overflow; don't use out after failure. Content-Length and Max-Forwards  |
  // | share this parser (RFC 9110 S8.6, S7.6.2).                              |
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
  // | RFC 5234 (ABNF Core Rules) S HEXDIG                                     |
  // +-------------------------------------------------------------------------+
  // | HEXDIG = DIGIT / "A" / "B" / "C" / "D" / "E" / "F"                      |
  // |                                                                         |
  // | DIGIT  = %x30-39   ; "0" - "9"                                          |
  // +-------------------------------------------------------------------------+
  // | HTTP accepts hex digits in either case, including a-f (RFC 9110 / RFC   |
  // | 9112).                                                                  |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_hex_digit(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_hexdig) != 0;
  }
  static constexpr bool is_hex_digit(char c) noexcept {
    return is_hex_digit(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] hex_value                                                ( public ) |
  // +=========================================================================+
  // | Read either hex case; return -1 for anything else.                      |
  // +-------------------------------------------------------------------------+
  static constexpr int hex_value(char c) noexcept {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
  }
  // +=========================================================================+
  // | [>] is_alpha                                                 ( public ) |
  // +=========================================================================+
  // | RFC 5234 (ABNF Core Rules) S ALPHA                                      |
  // +-------------------------------------------------------------------------+
  // | ALPHA = %x41-5A / %x61-7A                                               |
  // | ; A-Z / a-z                                                             |
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
  // | RFC 9110 S5.6.2 S token                                                 |
  // +-------------------------------------------------------------------------+
  // | token  = 1*tchar                                                        |
  // |                                                                         |
  // | tchar  = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /    |
  // | "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA                             |
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
  // | RFC 9110 S5.6.2 S token (whole-value predicate)                         |
  // +-------------------------------------------------------------------------+
  // | token = 1*tchar                                                         |
  // +-------------------------------------------------------------------------+
  // | Check one whole, nonempty token. Unlike consume_token(), reject         |
  // | trailing bytes that aren't tchar.                                       |
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
  // | RFC 3986 S2.3 S unreserved                                              |
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
  // | RFC 3986 S2.2 S sub-delims                                              |
  // +-------------------------------------------------------------------------+
  // | sub-delims = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," /      |
  // | ";" / "="                                                               |
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
  // | RFC 3986 S3.3 S pchar                                                   |
  // +-------------------------------------------------------------------------+
  // | pchar = unreserved / pct-encoded / sub-delims / ":" / "@"               |
  // +-------------------------------------------------------------------------+
  // | unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"                     |
  // | pct-encoded = "%" HEXDIG HEXDIG                                         |
  // | sub-delims  = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," /     |
  // | ";" / "="                                                               |
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
  // | RFC 5234 (ABNF Core Rules) S VCHAR                                      |
  // +-------------------------------------------------------------------------+
  // | VCHAR = %x21-7E                                                         |
  // | ; visible (printing) characters                                         |
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
  // | RFC 9110 S5.6.4 S qdtext                                                |
  // +-------------------------------------------------------------------------+
  // | qdtext = HTAB / SP / "!" / %x23-5B / %x5D-7E / obs-text                 |
  // +-------------------------------------------------------------------------+
  // | HTAB     = %x09                                                         |
  // | SP       = %x20                                                         |
  // | obs-text = %x80-FF                                                      |
  // +-------------------------------------------------------------------------+
  // | DQUOTE and backslash need quoted-pair escaping here.                    |
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
  // | RFC 9110 S5.6.4 S obs-text                                              |
  // +-------------------------------------------------------------------------+
  // | obs-text = %x80-FF                                                      |
  // +-------------------------------------------------------------------------+
  // | Allow obs-text for compatibility with older senders.                    |
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
  // | RFC 9110 S5.6.5 S ctext                                                 |
  // +-------------------------------------------------------------------------+
  // | ctext = HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text              |
  // +-------------------------------------------------------------------------+
  // | HTAB     = %x09                                                         |
  // | SP       = %x20                                                         |
  // | obs-text = %x80-FF                                                      |
  // +-------------------------------------------------------------------------+
  // | Parentheses handle nesting; backslash handles quoted-pair escapes. None |
  // | belongs to plain ctext.                                                 |
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
  // | RFC 5322 S3.2.3 S atext                                                 |
  // +-------------------------------------------------------------------------+
  // | atext = ALPHA / DIGIT / "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / |
  // | "-" / "/" / "=" / "?" / "^" / "_" / "`" / "{" / "|" / "}" / "~"         |
  // +-------------------------------------------------------------------------+
  // | The atom bytes used by dot-atom-text and atom, imported by HTTP's From  |
  // | mailbox grammar (RFC 5322 S3.2.3, RFC 9110 S10.1.2).                    |
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
  // | RFC 5322 S3.4.1 S dtext                                                 |
  // +-------------------------------------------------------------------------+
  // | dtext = %d33-90 / %d94-126 / obs-dtext                                  |
  // +-------------------------------------------------------------------------+
  // | Inside a domain-literal, keep brackets as delimiters and backslash as   |
  // | an escape. obs-text covers obs-dtext in this HTTP context.              |
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
  // | RFC 9110 S8.8.3 S etagc                                                 |
  // +-------------------------------------------------------------------------+
  // | etagc = %x21 / %x23-7E / obs-text                                       |
  // | ; VCHAR except DQUOTE, plus obs-text                                    |
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
  // | RFC 6265 S4.1.1 S cookie-octet                                          |
  // +-------------------------------------------------------------------------+
  // | cookie-octet = %x21 / %x23-2B / %x2D-3A / %x3C-5B / %x5D-7E             |
  // | ; US-ASCII characters excluding CTLs, whitespace, DQUOTE,               |
  // | ; comma, semicolon, and backslash                                       |
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
  // | RFC 6265 S4.1.1 S <any CHAR except CTLs or ";">                         |
  // +-------------------------------------------------------------------------+
  // | Allow visible ASCII and spaces, except semicolons. Cookie attribute     |
  // | meaning is checked later; even an Expires date fits this broad syntax   |
  // | (RFC 6265 S4.1.1).                                                      |
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
  // | RFC 9110 S8.8.3 S entity-tag                                            |
  // +-------------------------------------------------------------------------+
  // | entity-tag = [ weak ] opaque-tag                                        |
  // | weak       = %s"W/"                                                     |
  // | opaque-tag = DQUOTE *etagc DQUOTE                                       |
  // +-------------------------------------------------------------------------+
  // | Check one whole entity-tag, weak or strong. The optional W/ prefix is   |
  // | case-sensitive.                                                         |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_entity_tag(std::string_view sv) noexcept {
    if (sv.empty()) return false;
    std::size_t off = 0;
    // Allow the case-sensitive W/ prefix.
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
    // The closing quote must end the value.
    return off == sv.size();
  }
  // +=========================================================================+
  // | [>] is_ows                                                   ( public ) |
  // +=========================================================================+
  // | RFC 9110 S5.6.3 S OWS (Optional Whitespace)                             |
  // +-------------------------------------------------------------------------+
  // | OWS = *( SP / HTAB )                                                    |
  // +-------------------------------------------------------------------------+
  // | SP   = %x20                                                             |
  // | HTAB = %x09                                                             |
  // +-------------------------------------------------------------------------+
  // | Allow OWS only where the grammar says so.                               |
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
  // | | ( "1" [ "." 0*3("0") ] )                                              |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_qvalue(std::string_view sv) noexcept {
    if (sv.empty() || sv.size() > 5) return false;
    if (sv[0] != '0' && sv[0] != '1') return false;
    if (sv.size() == 1) return true;  // A lone 0 or 1 is fine.
    if (sv[1] != '.') return false;
    for (std::size_t i = 2; i < sv.size(); ++i) {
      if (!is_digit(sv[i]) || (sv[0] == '1' && sv[i] != '0')) return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] consume_weight                                           ( public ) |
  // +=========================================================================+
  // | RFC 9110 S12.4.2 S weight                                               |
  // +-------------------------------------------------------------------------+
  // | weight = OWS ";" OWS "q=" qvalue                                        |
  // +-------------------------------------------------------------------------+
  // | Check one complete weight: q ignores case, but no spaces around '='.    |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_weight(std::string_view sv) noexcept {
    std::size_t i = 0;
    // Skip OWS before ';'.
    while (i < sv.size() && is_ows(sv[i])) i++;
    // Require ';' before the weight.
    if (i >= sv.size() || sv[i++] != ';') return false;
    // Skip OWS after ';'.
    while (i < sv.size() && is_ows(sv[i])) i++;
    // Require q, ignoring case.
    if (i >= sv.size()) return false;
    const std::string_view name = consume_token(sv.substr(i));
    if (name.empty() || !iequals(name, "q")) return false;
    i += name.size();
    // No spaces around '='.
    if (i >= sv.size() || sv[i++] != '=') return false;
    if (i >= sv.size()) return false;
    const std::string_view qvalue = consume_token(sv.substr(i));
    if (qvalue.empty() || !is_qvalue(qvalue)) return false;
    // The qvalue must reach the end.
    return i + qvalue.size() == sv.size();
  }
  // +=========================================================================+
  // | [>] tolower_ascii                                            ( public ) |
  // +=========================================================================+
  // | Lowercase an ASCII character.                                           |
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
  // | Trim OWS from the left.                                                 |
  // +-------------------------------------------------------------------------+
  static constexpr void ows_ltrim(std::string_view& sv) noexcept {
    while (!sv.empty() && helpers::is_ows(sv.front())) {
      sv.remove_prefix(1);
    }
  }
  // +=========================================================================+
  // | [>] ows_rtrim                                                ( public ) |
  // +=========================================================================+
  // | Trim OWS from the right.                                                |
  // +-------------------------------------------------------------------------+
  static constexpr void ows_rtrim(std::string_view& sv) noexcept {
    while (!sv.empty() && helpers::is_ows(sv.back())) {
      sv.remove_suffix(1);
    }
  }
  // +=========================================================================+
  // | [>] ows_trim                                                 ( public ) |
  // +=========================================================================+
  // | Trim OWS from both ends.                                                |
  // +-------------------------------------------------------------------------+
  static constexpr void ows_trim(std::string_view& sv) noexcept {
    ows_ltrim(sv);
    ows_rtrim(sv);
  }
  // +=========================================================================+
  // | [>] skip_n_hextets                                           ( public ) |
  // +=========================================================================+
  // +-------------------------------------------------------------------------+
  // | h16            | 1*4HEXDIG                                              |
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
  // | port           | *DIGIT                                                 |
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
  // | dec-octet      | DIGIT                    ; 0-9                         |
  // | | / %x31-39 DIGIT          ; 10-99                                      |
  // | | / "1" 2DIGIT             ; 100-199                                    |
  // | | / "2" %x30-34 DIGIT      ; 200-249                                    |
  // | | / "25" %x30-35           ; 250-255                                    |
  // +-------------------------------------------------------------------------+
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
  // | RFC 3986 S3.2.2 S dec-octet (whole-value predicate)                     |
  // +-------------------------------------------------------------------------+
  // | dec-octet = DIGIT                 ; 0-9                                 |
  // | / %x31-39 DIGIT         ; 10-99                                         |
  // | / "1" 2DIGIT            ; 100-199                                       |
  // | / "2" %x30-34 DIGIT     ; 200-249                                       |
  // | / "25" %x30-35          ; 250-255                                       |
  // +-------------------------------------------------------------------------+
  // | Check one canonical IPv4 octet: 0..255, digits only, no leading zeroes  |
  // | except 0 itself. Sec-WebSocket-Version shares this shape (RFC 3986, RFC |
  // | 6455 S11.3.5).                                                          |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_dec_octet(std::string_view sv) noexcept {
    const std::size_t sv_size = sv.size();
    if (sv_size == 0 || sv_size > 3) return false;
    for (const auto c : sv) {
      if (!is_digit(c)) return false;
    }
    // One digit is always in range.
    if (sv_size == 1) return true;
    // No leading zeroes on multi-digit values.
    if (sv[0] == '0') return false;
    // Two digits starting at 1..9 are always in range.
    if (sv_size == 2) return true;
    // For three digits, cap the value at 255.
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
  // +-------------------------------------------------------------------------+
  // | IPvFuture    | "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )     |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_ip_v_future(std::string_view sv) {
    if (sv.empty()) return false;
    if (sv.front() != 'v' && sv.front() != 'V') return false;
    // Read the hex version and its dot first.
    std::size_t hex_digits_found = 0;
    std::size_t off = 1;
    while (off < sv.size() && sv[off] != '.') {
      if (!helpers::is_hex_digit(sv[off])) return false;
      hex_digits_found++;
      off++;
    }
    if (off++ == sv.size() || !hex_digits_found) return false;
    // Now check the address characters.
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
  // +-------------------------------------------------------------------------+
  // | IPv6address    | 6( h16 ":" ) ls32                                      |
  // | | / "::" 5( h16 ":" ) ls32                                              |
  // | | / [ h16 ] "::" 4( h16 ":" ) ls32                                      |
  // | | / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32                        |
  // | | / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32                        |
  // | | / [ *3( h16 ":" ) h16 ] "::" h16 ":" ls32                             |
  // | | / [ *4( h16 ":" ) h16 ] "::" ls32                                     |
  // | | / [ *5( h16 ":" ) h16 ] "::" h16                                      |
  // | | / [ *6( h16 ":" ) h16 ] "::"                                          |
  // +-------------------------------------------------------------------------+
  // | ls32           | ( h16 ":" h16 ) / IPv4address                          |
  // | h16            | 1*4HEXDIG                                              |
  // +-------------------------------------------------------------------------+
  // +-------------------------------------------------------------------------+
  // | [>] is_ip_v6_address                                         ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_ip_v6_address(std::string_view sv) {
    // Start with ':' or a hex digit.
    std::size_t sep = sv.find("::");
    std::string_view head;
    std::string_view tail;
    if (sep == std::string_view::npos) {
      // +---------------------------------------------------------------------+
      // | No '::' compression here.                                           |
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
      // | Handle '::' compression.                                            |
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
  // +-------------------------------------------------------------------------+
  // | IPv4address    | dec-octet "." dec-octet "." dec-octet "." dec-octet    |
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
  // | IP-literal     | "[" ( IPv6address / IPvFuture ) "]"                    |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_ip_literal(std::string_view sv) {
    if (sv.size() < 2) return false;  // Need room for both brackets.
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
  // +-------------------------------------------------------------------------+
  // | reg-name       | *( unreserved / pct-encoded / sub-delims )             |
  // +-------------------------------------------------------------------------+
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
  // | RFC 9110 S11.2 S token68                                                |
  // +-------------------------------------------------------------------------+
  // | token68 = 1*( ALPHA / DIGIT / "-" / "." / "_" / "~" / "+" / "/" ) *"="  |
  // +-------------------------------------------------------------------------+
  // | Check a whole token68: nonempty data, then optional '=' padding.        |
  // | Nothing else may follow the padding.                                    |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_token68(std::string_view sv) noexcept {
    if (sv.empty()) return false;
    std::size_t i = 0;
    const std::size_t sv_size = sv.size();
    while (i < sv_size &&
           (is_unreserved(sv[i]) || sv[i] == '+' || sv[i] == '/')) {
      i++;
    }
    // Need some data before '=' padding.
    if (i == 0) return false;
    while (i < sv_size && sv[i] == '=') i++;
    // Nothing may follow the padding.
    return i == sv_size;
  }
  // +=========================================================================+
  // | [>] is_base64_char                                           ( public ) |
  // +=========================================================================+
  // | RFC 4648 S4 / RFC 6455 S4.3 S base64-character                          |
  // +-------------------------------------------------------------------------+
  // | base64-character = ALPHA / DIGIT / "+" / "/"                            |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_base64_char(char c) noexcept {
    return is_alpha(c) || is_digit(c) || c == '+' || c == '/';
  }
  // +=========================================================================+
  // | [>] check_base64_value                                       ( public ) |
  // +=========================================================================+
  // | RFC 6455 S4.3 S base64-value-non-empty                                  |
  // +-------------------------------------------------------------------------+
  // | base64-value-non-empty = ( 1*base64-data [ base64-padding ] )           |
  // | / base64-padding                                                        |
  // | base64-data            = 4base64-character                              |
  // | base64-padding         = ( 2base64-character "==" )                     |
  // | / ( 3base64-character "=" )                                             |
  // | base64-character       = ALPHA / DIGIT / "+" / "/"                      |
  // +-------------------------------------------------------------------------+
  // | Check nonempty standard-alphabet base64 in groups of four. Only the     |
  // | last group may contain '=': either xx== or xxx=. Decoded size is        |
  // | checked elsewhere; this is syntax only.                                 |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_base64_value(std::string_view sv) noexcept {
    const std::size_t sv_size = sv.size();
    // Need at least one complete group of four.
    if (sv_size == 0 || sv_size % 4 != 0) return false;
    // All but the last group must be plain base64 data.
    const std::size_t last = sv_size - 4;
    for (std::size_t i = 0; i < last; i++) {
      if (!is_base64_char(sv[i])) return false;
    }
    // The last group is data, xx== or xxx=.
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
  // +-------------------------------------------------------------------------+
  // | method  | token                                                         |
  // | token   | 1*tchar                                                       |
  // | tchar   | "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /   |
  // | | "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA                           |
  // +-------------------------------------------------------------------------+
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
  // | RFC 9110 S5.6.4 S quoted-string                                         |
  // +-------------------------------------------------------------------------+
  // | quoted-string = DQUOTE *( qdtext / quoted-pair ) DQUOTE                 |
  // | quoted-pair   = "\" ( HTAB / SP / VCHAR / obs-text )                    |
  // +-------------------------------------------------------------------------+
  // | Consume a quoted string, checking plain qdtext and quoted-pair escapes. |
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
        end_index += 2;  // Skip the escaped byte.
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
  // | RFC 9110 S5.6.5 S comment                                               |
  // +-------------------------------------------------------------------------+
  // | comment     = "(" *( ctext / quoted-pair / comment ) ")"                |
  // | quoted-pair = "\" ( HTAB / SP / VCHAR / obs-text )                      |
  // +-------------------------------------------------------------------------+
  // | Consume one balanced comment, parentheses included. Track nesting, skip |
  // | quoted-pair escapes, and require ctext elsewhere. Empty output means    |
  // | malformed input; even () is a valid nonempty comment.                   |
  // +-------------------------------------------------------------------------+
  static constexpr std::string_view consume_comment(
      std::string_view sv) noexcept {
    if (sv.empty() || sv.front() != '(') return {};
    std::size_t i = 1;
    std::size_t depth = 1;
    while (i < sv.size()) {
      const char c = sv[i];
      if (c == '\\') {
        // An escape consumes exactly one HTAB, SP, VCHAR or obs-text byte.
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
    // Ran out of bytes before all comments closed.
    return {};
  }
  // +=========================================================================+
  // | [>] consume_cfws                                             ( public ) |
  // +=========================================================================+
  // | RFC 5322 S3.2.2 S CFWS (in the HTTP field-value context)                |
  // +-------------------------------------------------------------------------+
  // | CFWS = (1*([FWS] comment) [FWS]) / FWS                                  |
  // | FWS  = ([*WSP CRLF] 1*WSP) / obs-FWS                                    |
  // | WSP  = SP / HTAB                                                        |
  // +-------------------------------------------------------------------------+
  // | Skip whitespace and nested comments, advancing i. HTTP has already      |
  // | removed CR/LF, so FWS here is just SP/HTAB. Consuming nothing is fine;  |
  // | a malformed comment isn't.                                              |
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
  // | RFC 5322 S3.2.3 S dot-atom (in the HTTP field-value context)            |
  // +-------------------------------------------------------------------------+
  // | dot-atom      = [CFWS] dot-atom-text [CFWS]                             |
  // | dot-atom-text = 1*atext *( "." 1*atext )                                |
  // +-------------------------------------------------------------------------+
  // | Consume a dot-atom with optional CFWS on both sides. Each dot needs     |
  // | nonempty atext on both sides; reject leading, trailing or doubled dots  |
  // | and malformed comments.                                                 |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_dot_atom(std::string_view sv,
                                         std::size_t& i) noexcept {
    if (!consume_cfws(sv, i)) return false;
    // Start with a nonempty atext run.
    if (i >= sv.size() || !is_atext(sv[i])) return false;
    while (i < sv.size() && is_atext(sv[i])) i++;
    // A single dot separates each later run.
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
  // | RFC 5322 S3.4.1 S domain-literal (in the HTTP field-value context)      |
  // +-------------------------------------------------------------------------+
  // | domain-literal = [CFWS] "[" *([FWS] dtext) [FWS] "]" [CFWS]             |
  // +-------------------------------------------------------------------------+
  // | Consume a bracketed domain-literal with optional CFWS around it. Allow  |
  // | dtext and SP/HTAB inside; reject missing brackets, other bytes or       |
  // | malformed comments.                                                     |
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
  // | RFC 5322 S3.4.1 S addr-spec (in the HTTP field-value context)           |
  // +-------------------------------------------------------------------------+
  // | addr-spec  = local-part "@" domain                                      |
  // | local-part = dot-atom / quoted-string / obs-local-part                  |
  // | domain     = dot-atom / domain-literal / obs-domain                     |
  // +-------------------------------------------------------------------------+
  // | Consume local-part@domain and advance i. The local part is a quoted     |
  // | string or dot-atom; the domain is a bracketed literal or dot-atom.      |
  // | Allow CFWS where the grammar permits it; reject incomplete input.       |
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
    if (peek < sv.size() && sv[peek] == '[') {
      return consume_domain_literal(sv, i);
    }
    return consume_dot_atom(sv, i);
  }
  // +=========================================================================+
  // | [>] consume_word                                             ( public ) |
  // +=========================================================================+
  // | RFC 5322 S3.2.5 S word (in the HTTP field-value context)                |
  // +-------------------------------------------------------------------------+
  // | word          = atom / quoted-string                                    |
  // | atom          = [CFWS] 1*atext [CFWS]                                   |
  // | quoted-string = [CFWS] DQUOTE *([FWS] qcontent) [FWS] DQUOTE [CFWS]     |
  // +-------------------------------------------------------------------------+
  // | Consume a quoted string or atom, with optional CFWS around it. Reject   |
  // | malformed comments or a missing word.                                   |
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
  // | RFC 5322 S3.4 S mailbox (imported by RFC 9110 S10.1.2 From)             |
  // +-------------------------------------------------------------------------+
  // | mailbox      = name-addr / addr-spec                                    |
  // | name-addr    = [display-name] angle-addr                                |
  // | angle-addr   = [CFWS] "<" addr-spec ">" [CFWS]                          |
  // | display-name = phrase                                                   |
  // | phrase       = 1*word                                                   |
  // +-------------------------------------------------------------------------+
  // | Check one whole mailbox, not a comma-separated list (RFC 9110 S10.1.2). |
  // | Try an optional display name plus <addr-spec>, then a bare addr-spec.   |
  // | Either form must consume all input.                                     |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_mailbox(std::string_view sv) noexcept {
    // Try an optional display name and <addr-spec> first.
    {
      std::size_t i = 0;
      // No display name is fine: <addr-spec> stands on its own.
      while (i < sv.size() && sv[i] != '<') {
        const std::size_t before = i;
        if (!consume_word(sv, i)) break;
        // Stop if a word consumes nothing; don't loop forever.
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
    // Now try a bare addr-spec.
    {
      std::size_t i = 0;
      if (consume_addr_spec(sv, i) && i == sv.size()) return true;
    }
    return false;
  }
  // +=========================================================================+
  // | [>] consume_token_or_quoted_string                           ( public ) |
  // +=========================================================================+
  // | RFC 9110 S5.6.6 S parameter-value (and equivalent value productions)    |
  // +-------------------------------------------------------------------------+
  // | value = token / quoted-string                                           |
  // +-------------------------------------------------------------------------+
  // | Consume a quoted string if it starts with a quote, otherwise a token.   |
  // | Empty output means neither form matched.                                |
  // +-------------------------------------------------------------------------+
  static constexpr std::string_view consume_token_or_quoted_string(
      std::string_view sv) noexcept {
    if (!sv.empty() && sv.front() == '"') return consume_quoted_string(sv);
    return consume_token(sv);
  }
  // +=========================================================================+
  // | [>] is_cookie_value                                          ( public ) |
  // +=========================================================================+
  // | RFC 6265 S4.1.1 S cookie-value                                          |
  // +-------------------------------------------------------------------------+
  // | cookie-value = *cookie-octet / ( DQUOTE *cookie-octet DQUOTE )          |
  // +-------------------------------------------------------------------------+
  // | Check cookie-octets, optionally wrapped in literal quotes. Empty values |
  // | are fine. These quotes aren't HTTP quoted-string syntax: no backslash   |
  // | escapes.                                                                |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_cookie_value(std::string_view sv) noexcept {
    // Allow a pair of literal quotes around the value.
    if (sv.size() >= 2 && sv.front() == '"' && sv.back() == '"') {
      sv.remove_prefix(1);
      sv.remove_suffix(1);
    } else if (!sv.empty() && (sv.front() == '"' || sv.back() == '"')) {
      // A lone quote isn't a valid value.
      return false;
    }
    // Check cookie-octets after unwrapping; empty is fine.
    for (const char c : sv) {
      if (!is_cookie_octet(c)) return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] is_cookie_pair                                           ( public ) |
  // +=========================================================================+
  // | RFC 6265 S4.1.1 S cookie-pair                                           |
  // +-------------------------------------------------------------------------+
  // | cookie-pair  = cookie-name "=" cookie-value                             |
  // | cookie-name  = token                                                    |
  // | cookie-value = *cookie-octet / ( DQUOTE *cookie-octet DQUOTE )          |
  // +-------------------------------------------------------------------------+
  // | Check one whole cookie-pair: a nonempty token name, '=', then a         |
  // | cookie-value. No spaces around '=' and no trailing bytes.               |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_cookie_pair(std::string_view sv) noexcept {
    // Start with a nonempty token name.
    const std::string_view name = consume_token(sv);
    if (name.empty()) return false;
    // Require '=' right after the name.
    if (name.size() >= sv.size() || sv[name.size()] != '=') return false;
    // Check the rest as a cookie-value; empty is fine.
    return is_cookie_value(sv.substr(name.size() + 1));
  }
  // +=========================================================================+
  // | [>] is_cookie_av                                             ( public ) |
  // +=========================================================================+
  // | RFC 6265 S4.1.1 S cookie-av                                             |
  // +-------------------------------------------------------------------------+
  // | cookie-av    = expires-av / max-age-av / domain-av / path-av /          |
  // | secure-av / httponly-av / extension-av                                  |
  // | extension-av = <any CHAR except CTLs or ";">                            |
  // +-------------------------------------------------------------------------+
  // | Check a nonempty cookie attribute using extension-av's broad syntax.    |
  // | Attribute meaning is handled by the recipient later, so Max-Age=abc is  |
  // | still syntactically valid (RFC 6265 S4.1.1, S5.2).                      |
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
  // | RFC 9111 S5.2 / S5.4 S parameterized directive                          |
  // +-------------------------------------------------------------------------+
  // | directive = token [ "=" ( token / quoted-string ) ]                     |
  // +-------------------------------------------------------------------------+
  // | Check one complete directive: a token, optionally followed by '=' and a |
  // | token or quoted string. No spaces around '=' or trailing bytes. Cache   |
  // | directives and extension pragmas share this shape (RFC 9111 S5.2,       |
  // | S5.4).                                                                  |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_directive(std::string_view sv) noexcept {
    std::size_t bytes_used = 0;
    if (!consume_extension_parameter(sv, bytes_used)) return false;
    return bytes_used == sv.size();
  }
  // +=========================================================================+
  // | [>] consume_parameter                                        ( public ) |
  // +=========================================================================+
  // | RFC 9110 S5.6.6 S parameter                                             |
  // +-------------------------------------------------------------------------+
  // | parameter       = parameter-name "=" parameter-value                    |
  // | parameter-name  = token                                                 |
  // | parameter-value = ( token / quoted-string )                             |
  // +-------------------------------------------------------------------------+
  // | Consume name=value and report bytes_used. The name must be a token, the |
  // | value a nonempty token or quoted string. allow_bws permits OWS around   |
  // | '='; otherwise it doesn't. Failure returns false with bytes_used set to |
  // | 0.                                                                      |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_parameter(std::string_view sv,
                                          std::size_t& bytes_used,
                                          bool allow_bws) noexcept {
    bytes_used = 0;
    std::size_t i = 0;
    const std::string_view name = consume_token(sv);
    if (name.empty()) return false;
    i += name.size();
    // Skip BWS before '=' only when allowed.
    if (allow_bws) {
      while (i < sv.size() && is_ows(sv[i])) i++;
    }
    // Require '=' after the name.
    if (i >= sv.size() || sv[i++] != '=') return false;
    // Skip BWS after '=' only when allowed.
    if (allow_bws) {
      while (i < sv.size() && is_ows(sv[i])) i++;
    }
    // Require a value after '='.
    if (i >= sv.size()) return false;
    const std::string_view value = consume_token_or_quoted_string(sv.substr(i));
    if (value.empty()) return false;
    bytes_used = i + value.size();
    return true;
  }
  // +=========================================================================+
  // | [>] consume_extension_parameter                              ( public ) |
  // +=========================================================================+
  // | RFC 6455 S9.1 S extension-param                                         |
  // +-------------------------------------------------------------------------+
  // | extension-param = token [ "=" ( token / quoted-string ) ]               |
  // +-------------------------------------------------------------------------+
  // | Consume a token name with an optional '=value'. If '=' is present,      |
  // | require a nonempty token or quoted string, with no surrounding spaces.  |
  // | Report bytes_used; failure returns false and 0 bytes.                   |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_extension_parameter(
      std::string_view sv, std::size_t& bytes_used) noexcept {
    bytes_used = 0;
    const std::string_view name = consume_token(sv);
    if (name.empty()) return false;
    std::size_t i = name.size();
    // A bare name is fine; the value is optional.
    if (i >= sv.size() || sv[i] != '=') {
      bytes_used = i;
      return true;
    }
    // Found '=': now a value is required, with no spaces around '='.
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
  // | RFC 9110 S5.6.6 S parameters                                            |
  // +-------------------------------------------------------------------------+
  // | parameters = *( OWS ";" OWS [ parameter ] )                             |
  // +-------------------------------------------------------------------------+
  // | Walk semicolon-delimited parameters and trim OWS. require_parameter     |
  // | rejects empty slots and a trailing ';'; otherwise they're allowed.      |
  // | consume reports bytes_used for each slot. Stop on rejection or an       |
  // | out-of-range byte count.                                                |
  // +-------------------------------------------------------------------------+
  template <typename ParameterConsumer>
  static constexpr bool for_each_parameter(std::string_view sv,
                                           bool require_parameter,
                                           ParameterConsumer&& consume) {
    std::size_t i = 0;
    while (i < sv.size()) {
      // Skip OWS before ';'.
      while (i < sv.size() && is_ows(sv[i])) i++;
      // Require ';' before each parameter.
      if (i >= sv.size() || sv[i++] != ';') return false;
      // Skip OWS after ';'.
      while (i < sv.size() && is_ows(sv[i])) i++;
      if (i >= sv.size()) {
        // Nothing left after ';'.
        if (require_parameter) return false;
        return true;
      }
      // Allow empty slots only when the parameter is optional.
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
  // | RFC 7239 S4 S forwarded-element                                         |
  // +-------------------------------------------------------------------------+
  // | forwarded-element = [ forwarded-pair ] *( ";" [ forwarded-pair ] )      |
  // +-------------------------------------------------------------------------+
  // | Walk a Forwarded element's semicolon-separated pairs. No OWS around     |
  // | ';', and empty slots are fine, even an empty element (RFC 7239).        |
  // | consume reports bytes_used for each nonempty pair; reject failures or   |
  // | out-of-range counts.                                                    |
  // +-------------------------------------------------------------------------+
  template <typename PairConsumer>
  static constexpr bool for_each_forwarded_pair(std::string_view sv,
                                                PairConsumer&& consume) {
    std::size_t i = 0;
    bool first = true;
    while (i < sv.size()) {
      // Require ';' between pairs, with no surrounding OWS.
      if (!first) {
        if (sv[i] != ';') return false;
        ++i;
      }
      first = false;
      // A pair is optional; another ';' or the end is fine.
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
  // | RFC 9110 S5.6.1 S Lists (#rule ABNF extension)                          |
  // +-------------------------------------------------------------------------+
  // | #element = [ element ] *( OWS "," OWS [ element ] )                     |
  // +-------------------------------------------------------------------------+
  // | Walk a comma-separated list, trimming OWS and ignoring empty elements.  |
  // | Quoted commas stay inside their element; reject unfinished quotes or    |
  // | escapes. Stop if consume_element rejects an element (RFC 9110 S5.6.1).  |
  // +-------------------------------------------------------------------------+
  template <typename ElementConsumer>
  static constexpr bool for_each_list_element(
      std::string_view sv, ElementConsumer&& consume_element) {
    bool follows_separator = false;
    std::size_t i = 0;
    std::size_t last = 0;
    bool inside_string = false;
    while (i < sv.size()) {
      // Track quotes and escapes so their commas don't split the list.
      if (sv[i] == '"') {
        inside_string = !inside_string;
        i++;
        continue;
      }
      // Skip the escaped byte inside the quoted string.
      if (sv[i] == '\\') {
        if (!inside_string || i + 1 >= sv.size()) return false;
        i += 2;
        continue;
      }
      if (sv[i] == ',' && !inside_string) {
        // Got an element; pass it to the parser.
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
    // An unfinished quoted string isn't valid.
    if (inside_string) return false;
    // Handle the tail after the last comma, or the only element.
    std::string_view element = sv.substr(last);
    if (follows_separator) ows_ltrim(element);
    if (!element.empty() && !consume_element(element)) return false;
    return true;
  }
  // +=========================================================================+
  // | [>] check_auth_param_list                                    ( public ) |
  // +=========================================================================+
  // | RFC 9110 S11.6.3 / S11.4 S #auth-param                                  |
  // +-------------------------------------------------------------------------+
  // | Authentication-Info = #auth-param                                       |
  // | auth-param          = token BWS "=" BWS ( token / quoted-string )       |
  // +-------------------------------------------------------------------------+
  // | Check a whole #auth-param list. Allow BWS around '='; each nonempty     |
  // | element must be a complete pair. An empty list and empty elements are   |
  // | fine.                                                                   |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_auth_param_list(std::string_view sv) noexcept {
    return for_each_list_element(sv, [](std::string_view element) {
      std::size_t bytes_used = 0;
      if (!consume_parameter(element, bytes_used, /*allow_bws=*/true)) {
        return false;
      }
      // Each auth-param must consume its whole trimmed element.
      return bytes_used == element.size();
    });
  }
  // +=========================================================================+
  // | [>] check_credentials                                        ( public ) |
  // +=========================================================================+
  // | RFC 9110 S11.4 / S11.6.2 S credentials                                  |
  // +-------------------------------------------------------------------------+
  // | credentials = auth-scheme [ 1*SP ( token68 / #auth-param ) ]            |
  // | auth-scheme = token                                                     |
  // | auth-param  = token BWS "=" BWS ( token / quoted-string )               |
  // +-------------------------------------------------------------------------+
  // | Check one complete credentials value. Start with an auth-scheme; a bare |
  // | scheme is fine. Otherwise require SP, never HTAB, then token68 or a     |
  // | #auth-param list. Try token68 first; the list may be empty and ignores  |
  // | empty elements.                                                         |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_credentials(std::string_view sv) noexcept {
    // Start with an auth-scheme token.
    const std::string_view scheme = consume_token(sv);
    if (scheme.empty()) return false;
    // A bare scheme is fine.
    if (scheme.size() == sv.size()) return true;
    // Require SP between scheme and argument, never HTAB.
    std::size_t i = scheme.size();
    if (sv[i] != ' ') return false;
    while (i < sv.size() && sv[i] == ' ') i++;
    const std::string_view rest = sv.substr(i);
    // Try a single token68 argument...
    if (is_token68(rest)) return true;
    // ...then an auth-param list, which may be empty.
    return check_auth_param_list(rest);
  }
  // +=========================================================================+
  // | [>] challenge_opens_param_list                               ( public ) |
  // +=========================================================================+
  // | RFC 9110 S11.6.1 S helper for #challenge parsing                        |
  // +-------------------------------------------------------------------------+
  // | For an already-valid challenge, check whether later auth-params can     |
  // | still belong to it. Only an auth-param argument opens that list; a bare |
  // | scheme or token68 doesn't.                                              |
  // +-------------------------------------------------------------------------+
  static constexpr bool challenge_opens_param_list(
      std::string_view element) noexcept {
    const std::string_view scheme = consume_token(element);
    // A bare scheme has no auth-param list.
    if (scheme.empty() || scheme.size() == element.size()) return false;
    std::size_t i = scheme.size();
    // Require SP between scheme and argument, never HTAB.
    while (i < element.size() && element[i] == ' ') i++;
    // A token68 is one value, not the start of an auth-param list.
    return !is_token68(element.substr(i));
  }
  // +=========================================================================+
  // | [>] check_challenge_list                                     ( public ) |
  // +=========================================================================+
  // | RFC 9110 S11.6.1 / S11.7.2 S #challenge                                 |
  // +-------------------------------------------------------------------------+
  // | WWW-Authenticate  = #challenge                                          |
  // | Proxy-Authenticate = #challenge                                         |
  // | challenge         = auth-scheme [ 1*SP ( token68 / #auth-param ) ]      |
  // | auth-scheme       = token                                               |
  // | auth-param        = token BWS "=" BWS ( token / quoted-string )         |
  // +-------------------------------------------------------------------------+
  // | Check a whole challenge list. Resolve each quoted-string-aware element  |
  // | left to right: auth-params may continue an open list; otherwise require |
  // | a complete new challenge. Reject orphan auth-params. Empty lists and    |
  // | empty elements are fine.                                                |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_challenge_list(std::string_view sv) noexcept {
    // Track whether this challenge still accepts auth-params.
    bool in_param_list = false;
    return for_each_list_element(
        sv, [&in_param_list](std::string_view element) {
          // An auth-param can continue only a challenge that opened a parameter
          // list.
          std::size_t bytes_used = 0;
          if (consume_parameter(element, bytes_used, /*allow_bws=*/true) &&
              bytes_used == element.size()) {
            return in_param_list;
          }
          // Otherwise start a new, complete challenge, possibly opening a new
          // parameter list.
          if (!check_credentials(element)) return false;
          in_param_list = challenge_opens_param_list(element);
          return true;
        });
  }
  // +=========================================================================+
  // | [>] consume_product                                          ( public ) |
  // +=========================================================================+
  // | RFC 9110 S10.1.5 / S10.2.4 S product                                    |
  // +-------------------------------------------------------------------------+
  // | product         = token [ "/" product-version ]                         |
  // | product-version = token                                                 |
  // +-------------------------------------------------------------------------+
  // | Consume a product token with an optional /version. A slash needs a      |
  // | nonempty version; empty output means failure.                           |
  // +-------------------------------------------------------------------------+
  static constexpr std::string_view consume_product(
      std::string_view sv) noexcept {
    const std::string_view name = consume_token(sv);
    if (name.empty()) return {};
    std::size_t i = name.size();
    // A slash needs a nonempty version token.
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
  // | RFC 9110 S10.1.5 / S10.2.4 S product list                               |
  // +-------------------------------------------------------------------------+
  // | product-list = product *( RWS ( product / comment ) )                   |
  // | RWS          = 1*( SP / HTAB )                                          |
  // +-------------------------------------------------------------------------+
  // | Check one whole product list. Start with a product; separate later      |
  // | products or comments with SP/HTAB. Reject leading whitespace/comments   |
  // | and trailing whitespace. Commas aren't separators here.                 |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_product_list(std::string_view sv) noexcept {
    // Start with a product, not whitespace or a comment.
    const std::string_view first = consume_product(sv);
    if (first.empty()) return false;
    std::size_t i = first.size();
    while (i < sv.size()) {
      // Separate later items with SP/HTAB.
      const std::size_t rws_start = i;
      while (i < sv.size() && is_ows(sv[i])) i++;
      if (i == rws_start) return false;
      // Whitespace needs a following product or comment.
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
  // +-------------------------------------------------------------------------+
  // | uri-host       | IP-literal / IPv4address / reg-name                    |
  // +-------------------------------------------------------------------------+
  static host_type check_uri_host(std::string_view sv) {
    // An empty host can still be a reg-name.
    if (sv.empty()) return host_type::kRegName;
    // Try a bracketed IP literal.
    if (helpers::is_ip_literal(sv)) return host_type::kIpLiteral;
    // Try an IPv4 address.
    if (helpers::is_ip_v4_address(sv)) return host_type::kIpV4Address;
    // Try a registered name.
    if (helpers::is_reg_name(sv)) return host_type::kRegName;
    // None matched: host type unknown.
    return host_type::kUnknown;
  }
  // +=========================================================================+
  // | [>] is_uri_scheme                                            ( public ) |
  // +=========================================================================+
  // | RFC 3986 S3.1 S scheme                                                  |
  // +-------------------------------------------------------------------------+
  // | scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )                     |
  // +-------------------------------------------------------------------------+
  // | Check a bare URI scheme, without ':'. Start with a letter, then allow   |
  // | letters, digits, '+', '-' and '.'. Forwarded proto values share this    |
  // | syntax.                                                                 |
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
  // | RFC 3986 S3.1 S scheme                                                  |
  // +-------------------------------------------------------------------------+
  // | scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )                     |
  // +-------------------------------------------------------------------------+
  // | Try a leading scheme and ':'. On success, advance i past ':'; otherwise |
  // | leave i alone and treat the input as relative.                          |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_uri_scheme(std::string_view sv,
                                           std::size_t& i) noexcept {
    // A scheme needs its ':' separator.
    const std::size_t colon_at = sv.find(':');
    if (colon_at == std::string_view::npos) return false;
    if (!is_uri_scheme(sv.substr(0, colon_at))) return false;
    i = colon_at + 1;
    return true;
  }
  // +=========================================================================+
  // | [>] consume_uri_pct_encoded                                  ( public ) |
  // +=========================================================================+
  // | RFC 3986 S2.1 S pct-encoded                                             |
  // +-------------------------------------------------------------------------+
  // | pct-encoded = "%" HEXDIG HEXDIG                                         |
  // +-------------------------------------------------------------------------+
  // | Starting at '%', check the next two hex digits and advance i on         |
  // | success.                                                                |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_uri_pct_encoded(std::string_view sv,
                                                std::size_t& i) noexcept {
    if (i + 2 >= sv.size()) return false;
    if (!is_hex_digit(sv[i + 1]) || !is_hex_digit(sv[i + 2])) return false;
    i += 3;
    return true;
  }
  // +=========================================================================+
  // | [>] percent_decode_validate                                  ( public ) |
  // +=========================================================================+
  // | RFC 3986 S2.1 S pct-encoded (validation)                                |
  // +-------------------------------------------------------------------------+
  // | Check that no %HH triplet decodes to NUL, without changing the bytes.   |
  // | URI syntax must already be valid: every '%' needs two hex digits.       |
  // +=========================================================================+
  static constexpr bool percent_decode_validate(std::string_view sv) noexcept {
    const std::size_t len = sv.size();
    for (std::size_t r = 0; r < len;) {
      if (sv[r] == '%' && r + 2 < len && is_hex_digit(sv[r + 1]) &&
          is_hex_digit(sv[r + 2])) {
        unsigned char decoded = static_cast<unsigned char>(
            (hex_value(sv[r + 1]) << 4) | hex_value(sv[r + 2]));
        if (decoded == 0x00) return false;
        r += 3;
        continue;
      }
      r++;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] percent_decode_in_place                                  ( public ) |
  // +=========================================================================+
  // | RFC 3986 S2.1 S pct-encoded (decoding)                                  |
  // +-------------------------------------------------------------------------+
  // | Decode %HH in the caller's writable buffer and shrink sv in place.      |
  // | Validate syntax and reject decoded NULs with percent_decode_validate    |
  // | first. No allocation or second NUL check here. Decode uniformly;        |
  // | callers must keep track of which slashes were literal separators before |
  // | decoding.                                                               |
  // +=========================================================================+
  static constexpr void percent_decode_in_place(
      std::string_view& sv) noexcept {
    // No '%' means nothing to decode. Skip the scan-and-copy work entirely.
    const std::size_t pct_at = sv.find('%');
    if (pct_at == std::string_view::npos) return;
    char* data = const_cast<char*>(sv.data());
    const std::size_t len = sv.size();
    // Leave bytes before the first '%' alone.
    std::size_t r = pct_at, w = pct_at;
    while (r < len) {
      if (data[r] == '%' && r + 2 < len && is_hex_digit(data[r + 1]) &&
          is_hex_digit(data[r + 2])) {
        unsigned char decoded = static_cast<unsigned char>(
            (hex_value(data[r + 1]) << 4) | hex_value(data[r + 2]));
        data[w++] = static_cast<char>(decoded);
        r += 3;
        continue;
      }
      data[w++] = data[r++];
    }
    sv = std::string_view(data, w);
  }
  // +=========================================================================+
  // | [>] check_uri_userinfo                                       ( public ) |
  // +=========================================================================+
  // | RFC 3986 S3.2.1 S userinfo                                              |
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
  // | RFC 3986 S3.2.2 / S3.2.3 S host [ ":" port ]                            |
  // +-------------------------------------------------------------------------+
  // | host = IP-literal / IPv4address / reg-name                              |
  // | port = *DIGIT                                                           |
  // +-------------------------------------------------------------------------+
  // | Check a whole host with an optional :port, without userinfo. Keep IPv6  |
  // | brackets and colons intact. Empty reg-names and ports are valid here;   |
  // | reject malformed hosts or nondigit ports.                               |
  // +-------------------------------------------------------------------------+
  // | Also return the host view, raw port digits and host type. Don't use     |
  // | those outputs after failure. Both overloads share the parser.           |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_host_port(std::string_view sv,
                                        std::string_view& uri_host,
                                        std::string_view& port,
                                        host_type& type) noexcept {
    std::size_t colon_at;
    if (!split_authority_host_port(sv, uri_host, colon_at)) return false;
    // A reg-name can be empty.
    type = uri_host.empty() ? host_type::kRegName : check_uri_host(uri_host);
    if (type == host_type::kUnknown) return false;
    port = {};
    if (colon_at < sv.size()) {
      if (sv[colon_at] != ':') return false;
      // An empty port is allowed by *DIGIT.
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
  // | RFC 6454 S6.1 / WHATWG Fetch S3.2 S serialized-origin                   |
  // +-------------------------------------------------------------------------+
  // | serialized-origin = scheme "://" host [ ":" port ]                      |
  // +-------------------------------------------------------------------------+
  // | Check one origin: scheme://host[:port], without userinfo, path, query,  |
  // | fragment or trailing slash. Origin and Access-Control-Allow-Origin      |
  // | share this form (RFC 6454, WHATWG Fetch).                               |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_serialized_origin(std::string_view sv) noexcept {
    std::size_t i = 0;
    // Consume the scheme and its ':'.
    if (!consume_uri_scheme(sv, i)) return false;
    // Require '//' after ':'.
    if (i + 1 >= sv.size() || sv[i] != '/' || sv[i + 1] != '/') return false;
    i += 2;
    // Check the rest as host[:port], with no userinfo.
    return check_host_port(sv.substr(i));
  }
  // +=========================================================================+
  // | [>] check_uri_authority                                      ( public ) |
  // +=========================================================================+
  // | RFC 3986 S3.2 S authority                                               |
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
    // Check the rest as host[:port], with no userinfo.
    return check_host_port(rest);
  }
  // +=========================================================================+
  // | [>] check_uri_reference                                      ( public ) |
  // +=========================================================================+
  // | RFC 3986 S4.1 S URI-reference                                           |
  // +-------------------------------------------------------------------------+
  // | URI-reference = URI / relative-ref                                      |
  // | URI           = scheme ":" hier-part [ "?" query ] [ "#" fragment ]     |
  // | relative-ref  = relative-part [ "?" query ] [ "#" fragment ]            |
  // | hier-part     = "//" authority path-abempty / path-absolute /           |
  // | path-rootless / path-empty                                              |
  // | relative-part = "//" authority path-abempty / path-absolute /           |
  // | path-noscheme / path-empty                                              |
  // | query         = *( pchar / "/" / "?" )                                  |
  // | fragment      = *( pchar / "/" / "?" )                                  |
  // +-------------------------------------------------------------------------+
  // | Check the whole normalized value, with no surrounding OWS.              |
  // | allow_fragment permits '#fragment' for Location; otherwise reject it,   |
  // | as Content-Location requires.                                           |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_uri_reference(std::string_view sv,
                                            bool allow_fragment) noexcept {
    // A relative reference can have an empty path.
    if (sv.empty()) return true;
    std::size_t i = 0;
    // Try scheme: first. Without it, the first path segment can't contain ':';
    // with it, a rootless path can.
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
      if (!check_uri_authority(
              sv.substr(authority_start, i - authority_start))) {
        return false;
      }
      has_authority = true;
    }
    // Check path bytes, '/' separators and percent escapes.
    const std::size_t path_start = i;
    while (i < sv.size() && sv[i] != '?' && sv[i] != '#') {
      if (sv[i] == '%') {
        if (!consume_uri_pct_encoded(sv, i)) return false;
        continue;
      }
      if (sv[i] != '/' && !is_pchar(sv[i])) return false;
      i++;
    }
    // Without a scheme or authority, ':' can't appear in the first segment: it
    // would look like a scheme (RFC 3986 S3.3).
    if (!has_scheme && !has_authority) {
      const std::string_view path = sv.substr(path_start, i - path_start);
      const std::string_view first_segment = path.substr(0, path.find('/'));
      if (first_segment.find(':') != std::string_view::npos) return false;
    }
    // Allow an optional ?query.
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
    // Allow #fragment only for URI-reference. The restricted form rejects an
    // unescaped '#'.
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
  // | Split host[:port], keeping bracketed IPv6 colons inside the host.       |
  // +-------------------------------------------------------------------------+
  // | Only an unclosed '[' fails here. Callers decide whether that's invalid  |
  // | input or a reason to wait for more bytes.                               |
  // +-------------------------------------------------------------------------+
  // | Return the host view and separator position. For bracketed hosts,       |
  // | colon_at is just after ']': bounds-check it and confirm ':' before      |
  // | reading a port. Otherwise it's the ':' index, or npos when absent.      |
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
  // +-------------------------------------------------------------------------+
  // | Field          | Definition                                             |
  // +-------------------------------------------------------------------------+
  // | authority-form | uri-host ":" port                                      |
  // | uri-host       | IP-literal / IPv4address / reg-name                    |
  // | port           | *DIGIT                                                 |
  // | IP-literal     | "[" ( IPv6address / IPvFuture ) "]"                    |
  // | IPvFuture      | "v" 1*HEXDIG "."                                       |
  // | | 1*( unreserved / sub-delims / ":" )                                   |
  // | IPv4address    | dec-octet "." dec-octet "." dec-octet "." dec-octet    |
  // | dec-octet      | DIGIT                                                  |
  // | | / %x31-39 DIGIT                                                       |
  // | | / "1" 2DIGIT                                                          |
  // | | / "2" %x30-34 DIGIT                                                   |
  // | | / "25" %x30-35                                                        |
  // | reg-name       | *( unreserved / pct-encoded / sub-delims )             |
  // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
  // | pct-encoded    | "%" HEXDIG HEXDIG                                      |
  // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /        |
  // | | "," / ";" / "="                                                       |
  // +-------------------------------------------------------------------------+
  // +-------------------------------------------------------------------------+
  // | Also return the host view, raw port digits and host type, just like     |
  // | check_host_port. No need to parse them again.                           |
  // +-------------------------------------------------------------------------+
  static constexpr deserialization_status try_to_deserialize_as_authority_form(
      std::string_view sv, std::string_view& uri_host, std::string_view& port,
      host_type& type, std::size_t& bytes_used) noexcept {
    bytes_used = 0;
    if (sv.empty()) return deserialization_status::kMoreBytesNeeded;
    // Only an unbracketed ':' separates host and port; IPv6 keeps its inner
    // colons.
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
    // After ']', confirm the next byte is ':'. The unbracketed path already
    // checked it.
    if (colon_at >= sv.size()) return deserialization_status::kMoreBytesNeeded;
    if (sv[colon_at] != ':') return deserialization_status::kInvalidSource;
    // Consume port digits; let the caller check what follows.
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
  // +-------------------------------------------------------------------------+
  // | Field          | Definition                                             |
  // +-------------------------------------------------------------------------+
  // | asterisk-form  | "*"                                                    |
  // +-------------------------------------------------------------------------+
  static constexpr deserialization_status try_to_deserialize_as_asterisk_form(
      std::string_view sv, std::size_t& bytes_used) noexcept {
    // Consume just '*'; leave the rest of the request line to the caller.
    if (sv.empty()) return deserialization_status::kMoreBytesNeeded;
    if (sv.front() != '*') return deserialization_status::kInvalidSource;
    bytes_used = 1;
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // | [>] try_to_deserialize_as_origin_form                        ( public ) |
  // +=========================================================================+
  // | Field          | Definition                                             |
  // +-------------------------------------------------------------------------+
  // | origin-form    | absolute-path [ "?" query ]                            |
  // +-------------------------------------------------------------------------+
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
      // Skip the query's leading '?'; it's a separator, not query data.
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
  // | Use port 80 for http and 443 for https, ignoring scheme case. Unknown   |
  // | or empty schemes have no default (RFC 9110 S4.2.1, S4.2.2).             |
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
  // | Compare raw ports first, then fill an omitted port with the scheme's    |
  // | default. Without a known default, only identical strings match.         |
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
  // +-------------------------------------------------------------------------+
  // | Field          | Definition                                             |
  // +-------------------------------------------------------------------------+
  // | absolute-form  | absolute-URI                                           |
  // | absolute-URI   | scheme ":" hier-part [ "?" query ]                     |
  // | scheme         | ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )             |
  // | hier-part      | "//" authority path-abempty                            |
  // | | / path-absolute                                                       |
  // | | / path-rootless                                                       |
  // | | / path-empty                                                          |
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
  // | | "," / ";" / "="                                                       |
  // +-------------------------------------------------------------------------+
  // +-------------------------------------------------------------------------+
  // | Return the scheme as received. With an authority, also set              |
  // | has_authority and return host, port and type. Without one, leave        |
  // | has_authority false and don't use those outputs.                        |
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
    scheme = sv.substr(0, i);  // Keep the scheme text, without ':'.
    i++;  // Skip ':'.
    // -------------------------------------------------------------------------
    // hier-part = "//" authority path-abempty
    //           / path-absolute / path-rootless / path-empty
    // -------------------------------------------------------------------------
    std::size_t path_start = i;
    if (i + 1 < sv_size && sv[i] == '/' && sv[i + 1] == '/') {
      // "//" authority path-abempty
      i += 2;
      const std::size_t authority_start = i;
      // Scan the authority up to '/', '?' or a byte it can't contain.
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
          break;  // A non-authority byte, such as a space, ends the scan.
        }
        i++;
      }
      const std::string_view authority =
          sv.substr(authority_start, i - authority_start);
      // Check the authority's optional userinfo, host and port.
      const std::size_t at_pos = authority.find('@');
      std::string_view host_port;
      if (at_pos != std::string_view::npos) {
        const std::string_view userinfo = authority.substr(0, at_pos);
        for (std::size_t j = 0; j < userinfo.size();) {
          if (userinfo[j] == '%') {
            // The authority scan already checked percent escapes.
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
      if (colon_at != std::string_view::npos && colon_at < host_port.size()) {
        if (host_port[colon_at] != ':') {
          return deserialization_status::kInvalidSource;
        }
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
        if (sv[i] != '/' && !is_pchar(sv[i])) break;  // Not a path byte.
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
        if (sv[i] != '/' && !is_pchar(sv[i])) break;  // Not a path byte.
        i++;
      }
    }
    consumed_path = sv.substr(path_start, i - path_start);
    // -------------------------------------------------------------------------
    // [ "?" query ]
    // -------------------------------------------------------------------------
    if (i < sv_size && sv[i] == '?') {
      i++;  // Skip '?'.
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
  // +-------------------------------------------------------------------------+
  // | Field          | Definition                                             |
  // +-------------------------------------------------------------------------+
  // | absolute-path  | 1*( "/" segment )                                      |
  // | segment        | *pchar                                                 |
  // | pchar          | unreserved / pct-encoded / sub-delims / ":" / "@"      |
  // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
  // | pct-encoded    | "%" HEXDIG HEXDIG                                      |
  // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /        |
  // | | "," / ";" / "="                                                       |
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
  // | Field          | Definition                                             |
  // +-------------------------------------------------------------------------+
  // | query          | *( pchar / "/" / "?" )                                 |
  // | pchar          | unreserved / pct-encoded / sub-delims / ":" / "@"      |
  // | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                  |
  // | pct-encoded    | "%" HEXDIG HEXDIG                                      |
  // | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /        |
  // | | "," / ";" / "="                                                       |
  // +-------------------------------------------------------------------------+
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
  // | Compare two string views, ignoring ASCII case.                          |
  // +-------------------------------------------------------------------------+
  static constexpr bool iequals(std::string_view a, std::string_view b) {
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), [](char ac, char bc) {
             return tolower_ascii(ac) == tolower_ascii(bc);
           });
  }
  // +=========================================================================+
  // | [>] split_query_parameters                                   ( public ) |
  // +=========================================================================+
  // | Split a query without its leading '?' on '&', then on the first '='.    |
  // | Skip empty pairs; a name without '=' has an empty value. Return         |
  // | borrowed views, so keep the backing bytes alive. Stop at                |
  // | out_keys.size(); drop excess pairs.                                     |
  // +=========================================================================+
  static constexpr std::size_t split_query_parameters(
      std::string_view query, std::span<std::string_view> out_keys,
      std::span<std::string_view> out_values) noexcept {
    const std::size_t max_pairs = out_keys.size();
    std::size_t count = 0;
    std::size_t i = 0;
    const std::size_t size = query.size();
    while (i < size && count < max_pairs) {
      // Find the next '&' or the end of input.
      std::size_t pair_end = i;
      while (pair_end < size && query[pair_end] != '&') pair_end++;
      // Skip empty pairs from leading, trailing or repeated '&'.
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
      // Move past this pair and its '&', if present.
      i = (pair_end < size) ? pair_end + 1 : pair_end;
    }
    return count;
  }
};
} // namespace martianlabs::doba::protocol::http

#endif
