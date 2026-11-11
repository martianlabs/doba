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

#include <algorithm>
#include <array>
#include <cstdint>
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
// | Character flags shared by the HTTP parsers.                               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
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
// /////////////////////////////////////////////////////////////////////////////
static constexpr std::array<std::uint16_t, 256> build_char_flags() noexcept {
  std::array<std::uint16_t, 256> t{};
  for (int c = 0; c < 256; ++c) {
    const auto u = static_cast<unsigned char>(c);
    if (u == '!' || u == '#' || u == '$' || u == '%' || u == '&' || u == '\'' ||
        u == '*' || u == '+' || u == '-' || u == '.' || u == '^' || u == '_' ||
        u == '`' || u == '|' || u == '~' || (u >= '0' && u <= '9') ||
        (u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z')) {
      t[c] |= kF_token;
    }
    if (u >= 0x21 && u <= 0x7E) t[c] |= kF_vchar;
    if (u >= 0x80) t[c] |= kF_obs_text;
    if (u == 0x20 || u == 0x09) t[c] |= kF_ows;
    if ((u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z') ||
        (u >= '0' && u <= '9') || u == '-' || u == '.' || u == '_' ||
        u == '~' || u == '!' || u == '$' || u == '&' || u == '\'' || u == '(' ||
        u == ')' || u == '*' || u == '+' || u == ',' || u == ';' || u == '=' ||
        u == ':' || u == '@') {
      t[c] |= kF_pchar;
    }
    if (u == 0x09 || u == 0x20 || u == '!' || (u >= 0x23 && u <= 0x5B) ||
        (u >= 0x5D && u <= 0x7E) || u >= 0x80) {
      t[c] |= kF_qdtext;
    }
    if (u == 0x09 || u == 0x20 || (u >= 0x21 && u <= 0x27) ||
        (u >= 0x2A && u <= 0x5B) || (u >= 0x5D && u <= 0x7E) || u >= 0x80) {
      t[c] |= kF_ctext;
    }
    if (u == 0x21 || (u >= 0x23 && u <= 0x7E) || u >= 0x80) t[c] |= kF_etagc;
    if (u == 0x21 || (u >= 0x23 && u <= 0x2B) || (u >= 0x2D && u <= 0x3A) ||
        (u >= 0x3C && u <= 0x5B) || (u >= 0x5D && u <= 0x7E)) {
      t[c] |= kF_cookie;
    }
    if (u >= 0x20 && u <= 0x7E && u != 0x3B) t[c] |= kF_cookie_av;
    if ((u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z') ||
        (u >= '0' && u <= '9') || u == '!' || u == '#' || u == '$' ||
        u == '%' || u == '&' || u == '\'' || u == '*' || u == '+' || u == '-' ||
        u == '/' || u == '=' || u == '?' || u == '^' || u == '_' || u == '`' ||
        u == '{' || u == '|' || u == '}' || u == '~') {
      t[c] |= kF_atext;
    }
    if ((u >= 0x21 && u <= 0x5A) || (u >= 0x5E && u <= 0x7E) || u >= 0x80) {
      t[c] |= kF_dtext;
    }
    if ((u >= '0' && u <= '9') || (u >= 'A' && u <= 'F') ||
        (u >= 'a' && u <= 'f')) {
      t[c] |= kF_hexdig;
    }
    if ((u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z') ||
        (u >= '0' && u <= '9') || u == '-' || u == '.' || u == '_' ||
        u == '~') {
      t[c] |= kF_unreserv;
    }
  }
  return t;
}
static constexpr auto kCharFlags = build_char_flags();
}  // namespace detail

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] helpers                                                    ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct helpers {
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  enum class host_type { kUnknown, kIpLiteral, kIpV4Address, kRegName };
  // +=========================================================================+
  // | [>] is_digit                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_digit(unsigned char c) noexcept {
    return c >= 0x30 && c <= 0x39;
  }
  static constexpr bool is_digit(char c) noexcept {
    return is_digit(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_digits                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Requires at least one digit. Does not check for overflow.               |
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
  // +-------------------------------------------------------------------------+
  // | Rejects empty input and overflow. Ignore out on failure.                |
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
  // +-------------------------------------------------------------------------+
  // | Accepts both upper and lower case.                                      |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_hex_digit(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_hexdig) != 0;
  }
  static constexpr bool is_hex_digit(char c) noexcept {
    return is_hex_digit(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_alpha                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // | ASCII letters only.                                                     |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_alpha(unsigned char c) noexcept {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
  }
  static constexpr bool is_alpha(char c) noexcept {
    return is_alpha(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_token                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // | The string overload checks the whole value and rejects empty input.     |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_token(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_token) != 0;
  }
  static constexpr bool is_token(char c) noexcept {
    return is_token(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_token                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // | The string overload checks the whole value and rejects empty input.     |
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
  // +-------------------------------------------------------------------------+
  static constexpr bool is_unreserved(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_unreserv) != 0;
  }
  static constexpr bool is_unreserved(char c) noexcept {
    return is_unreserved(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_sub_delim                                             ( public ) |
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
  // +-------------------------------------------------------------------------+
  // | Checks one plain path character. Check %HH sequences separately.        |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_pchar(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_pchar) != 0;
  }
  static constexpr bool is_pchar(char c) noexcept {
    return is_pchar(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_vchar                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_vchar(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_vchar) != 0;
  }
  static constexpr bool is_vchar(char c) noexcept {
    return is_vchar(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_qdtext                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Quotes and backslashes need escaping inside a quoted string.            |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_qdtext(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_qdtext) != 0;
  }
  static constexpr bool is_qdtext(char c) noexcept {
    return is_qdtext(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_obs_text                                              ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_obs_text(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_obs_text) != 0;
  }
  static constexpr bool is_obs_text(char c) noexcept {
    return is_obs_text(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_ctext                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // | Parentheses and backslashes need separate handling inside comments.     |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_ctext(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_ctext) != 0;
  }
  static constexpr bool is_ctext(char c) noexcept {
    return is_ctext(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_atext                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // | Characters allowed in an email atom.                                    |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_atext(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_atext) != 0;
  }
  static constexpr bool is_atext(char c) noexcept {
    return is_atext(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_dtext                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // | Characters allowed inside an email domain literal, without escapes.     |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_dtext(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_dtext) != 0;
  }
  static constexpr bool is_dtext(char c) noexcept {
    return is_dtext(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_etagc                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_etagc(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_etagc) != 0;
  }
  static constexpr bool is_etagc(char c) noexcept {
    return is_etagc(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_cookie_octet                                          ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_cookie_octet(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_cookie) != 0;
  }
  static constexpr bool is_cookie_octet(char c) noexcept {
    return is_cookie_octet(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_cookie_av_octet                                       ( public ) |
  // +-------------------------------------------------------------------------+
  // | Allows spaces and commas, but no controls or semicolons.                |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_cookie_av_octet(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_cookie_av) != 0;
  }
  static constexpr bool is_cookie_av_octet(char c) noexcept {
    return is_cookie_av_octet(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_entity_tag                                            ( public ) |
  // +-------------------------------------------------------------------------+
  // | Checks the whole tag. The optional W/ prefix is case-sensitive.         |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_entity_tag(std::string_view sv) noexcept {
    if (sv.empty()) return false;
    std::size_t off = 0;
    if (sv.size() >= 2 && sv[0] == 'W' && sv[1] == '/') off = 2;
    if (off >= sv.size() || sv[off] != '"') return false;
    off++;
    while (off < sv.size() && sv[off] != '"') {
      if (!is_etagc(sv[off])) return false;
      off++;
    }
    if (off >= sv.size() || sv[off] != '"') return false;
    off++;
    return off == sv.size();
  }
  // +=========================================================================+
  // | [>] is_ows                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  // | Only spaces and tabs count as HTTP whitespace.                          |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_ows(unsigned char c) noexcept {
    return (detail::kCharFlags[c] & detail::kF_ows) != 0;
  }
  static constexpr bool is_ows(char c) noexcept {
    return is_ows(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_qvalue                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Allows 0 to 1, with at most three decimal places.                       |
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
  // +-------------------------------------------------------------------------+
  // | Checks the whole weight. No whitespace around =; q ignores case.        |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_weight(std::string_view sv) noexcept {
    std::size_t i = 0;
    while (i < sv.size() && is_ows(sv[i])) i++;
    if (i >= sv.size() || sv[i++] != ';') return false;
    while (i < sv.size() && is_ows(sv[i])) i++;
    if (i >= sv.size()) return false;
    const std::string_view name = consume_token(sv.substr(i));
    if (name.empty() || !iequals(name, "q")) return false;
    i += name.size();
    if (i >= sv.size() || sv[i++] != '=') return false;
    if (i >= sv.size()) return false;
    const std::string_view qvalue = consume_token(sv.substr(i));
    if (qvalue.empty() || !is_qvalue(qvalue)) return false;
    return i + qvalue.size() == sv.size();
  }
  // +=========================================================================+
  // | [>] tolower_ascii                                            ( public ) |
  // +-------------------------------------------------------------------------+
  // | ASCII only.                                                             |
  // +-------------------------------------------------------------------------+
  static constexpr unsigned char tolower_ascii(unsigned char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
  }
  static constexpr char tolower_ascii(char c) noexcept {
    return static_cast<char>(tolower_ascii(static_cast<unsigned char>(c)));
  }
  // +=========================================================================+
  // | [>] ows_ltrim                                                ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr void ows_ltrim(std::string_view& sv) noexcept {
    while (!sv.empty() && helpers::is_ows(sv.front())) {
      sv.remove_prefix(1);
    }
  }
  // +=========================================================================+
  // | [>] ows_rtrim                                                ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr void ows_rtrim(std::string_view& sv) noexcept {
    while (!sv.empty() && helpers::is_ows(sv.back())) {
      sv.remove_suffix(1);
    }
  }
  // +=========================================================================+
  // | [>] ows_trim                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr void ows_trim(std::string_view& sv) noexcept {
    ows_ltrim(sv);
    ows_rtrim(sv);
  }
  // +=========================================================================+
  // | [>] skip_n_hextets                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Counts colon-separated groups of 1 to 4 hex digits; -1 on failure.      |
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
  // +-------------------------------------------------------------------------+
  // | Checks digits only. An empty port is allowed; no range check here.      |
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
  // +-------------------------------------------------------------------------+
  // | Allows 0 to 255, with no leading zeroes except 0 itself.                |
  // | The positional overload expects digits already checked by the caller.   |
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
  // +-------------------------------------------------------------------------+
  // | Allows 0 to 255, with no leading zeroes except 0 itself.                |
  // | The positional overload expects digits already checked by the caller.   |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_dec_octet(std::string_view sv) noexcept {
    const std::size_t sv_size = sv.size();
    if (sv_size == 0 || sv_size > 3) return false;
    for (const auto c : sv) {
      if (!is_digit(c)) return false;
    }
    if (sv_size == 1) return true;
    if (sv[0] == '0') return false;
    if (sv_size == 2) return true;
    if (sv[0] == '1') return true;            // 100-199
    if (sv[0] == '2') {                       // 200-255
      if (sv[1] < '5') return true;           // 200-249
      if (sv[1] == '5') return sv[2] <= '5';  // 250-255
    }
    return false;
  }
  // +=========================================================================+
  // | [>] is_ip_v_future                                           ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_ip_v_future(std::string_view sv) {
    if (sv.empty()) return false;
    if (sv.front() != 'v' && sv.front() != 'V') return false;
    std::size_t hex_digits_found = 0;
    std::size_t off = 1;
    while (off < sv.size() && sv[off] != '.') {
      if (!helpers::is_hex_digit(sv[off])) return false;
      hex_digits_found++;
      off++;
    }
    if (off++ == sv.size() || !hex_digits_found) return false;
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
  // | [>] is_ip_v6_address                                         ( public ) |
  // +-------------------------------------------------------------------------+
  // | Allows :: compression and an IPv4 tail. RFC 3986, 3.2.2.                |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_ip_v6_address(std::string_view sv) {
    std::size_t sep = sv.find("::");
    std::string_view head;
    std::string_view tail;
    if (sep == std::string_view::npos) {
      // Without ::, the address needs all eight groups.
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
      // The single :: must stand for at least one missing group.
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
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
  // | Requires data before any = padding. Nothing may follow the padding.     |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_token68(std::string_view sv) noexcept {
    if (sv.empty()) return false;
    std::size_t i = 0;
    const std::size_t sv_size = sv.size();
    while (i < sv_size &&
           (is_unreserved(sv[i]) || sv[i] == '+' || sv[i] == '/')) {
      i++;
    }
    if (i == 0) return false;
    while (i < sv_size && sv[i] == '=') i++;
    return i == sv_size;
  }
  // +=========================================================================+
  // | [>] is_base64_char                                           ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_base64_char(char c) noexcept {
    return is_alpha(c) || is_digit(c) || c == '+' || c == '/';
  }
  // +=========================================================================+
  // | [>] check_base64_value                                       ( public ) |
  // +-------------------------------------------------------------------------+
  // | Checks non-empty groups of four, with padding only in the last group.   |
  // | Uses the standard alphabet (+ and /). Does not check decoded length.    |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_base64_value(std::string_view sv) noexcept {
    const std::size_t sv_size = sv.size();
    if (sv_size == 0 || sv_size % 4 != 0) return false;
    const std::size_t last = sv_size - 4;
    for (std::size_t i = 0; i < last; i++) {
      if (!is_base64_char(sv[i])) return false;
    }
    const char a = sv[last];
    const char b = sv[last + 1];
    const char c = sv[last + 2];
    const char d = sv[last + 3];
    if (!is_base64_char(a) || !is_base64_char(b)) return false;
    if (is_base64_char(c)) {
      return is_base64_char(d) || d == '=';
    }
    return c == '=' && d == '=';
  }
  // +=========================================================================+
  // | [>] consume_token                                            ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns the token prefix, or an empty view if none is present.          |
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
  // +-------------------------------------------------------------------------+
  // | Returns the quoted prefix, including quotes; empty on invalid input.    |
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
  // +-------------------------------------------------------------------------+
  // | Returns one parenthesized comment, including nested ones.               |
  // | Escaped parentheses do not affect nesting. Empty means failure.         |
  // +-------------------------------------------------------------------------+
  static constexpr std::string_view consume_comment(
      std::string_view sv) noexcept {
    if (sv.empty() || sv.front() != '(') return {};
    std::size_t i = 1;
    std::size_t depth = 1;
    while (i < sv.size()) {
      const char c = sv[i];
      if (c == '\\') {
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
    return {};
  }
  // +=========================================================================+
  // | [>] consume_cfws                                             ( public ) |
  // +-------------------------------------------------------------------------+
  // | Skips spaces, tabs and comments from i. Consuming nothing is OK.        |
  // | Expects HTTP field values with CR/LF already removed.                   |
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
  // +-------------------------------------------------------------------------+
  // | Consumes an email dot-atom and surrounding whitespace/comments.         |
  // | Leading, trailing and doubled dots are rejected.                        |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_dot_atom(std::string_view sv,
                                         std::size_t& i) noexcept {
    if (!consume_cfws(sv, i)) return false;
    if (i >= sv.size() || !is_atext(sv[i])) return false;
    while (i < sv.size() && is_atext(sv[i])) i++;
    while (i < sv.size() && sv[i] == '.') {
      i++;
      if (i >= sv.size() || !is_atext(sv[i])) return false;
      while (i < sv.size() && is_atext(sv[i])) i++;
    }
    return consume_cfws(sv, i);
  }
  // +=========================================================================+
  // | [>] consume_domain_literal                                   ( public ) |
  // +-------------------------------------------------------------------------+
  // | Consumes a bracketed email domain and surrounding whitespace/comments.  |
  // | Expects HTTP field values with CR/LF already removed.                   |
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
  // +-------------------------------------------------------------------------+
  // | Consumes local-part@domain, including surrounding comments/whitespace.  |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_addr_spec(std::string_view sv,
                                          std::size_t& i) noexcept {
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
    if (i >= sv.size() || sv[i] != '@') return false;
    i++;
    peek = i;
    if (!consume_cfws(sv, peek)) return false;
    if (peek < sv.size() && sv[peek] == '[') {
      return consume_domain_literal(sv, i);
    }
    return consume_dot_atom(sv, i);
  }
  // +=========================================================================+
  // | [>] consume_word                                             ( public ) |
  // +-------------------------------------------------------------------------+
  // | Consumes an email atom or quoted string, plus surrounding comments      |
  // | and whitespace. Expects HTTP field values with CR/LF already removed.   |
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
    if (i >= sv.size() || !is_atext(sv[i])) return false;
    while (i < sv.size() && is_atext(sv[i])) i++;
    return consume_cfws(sv, i);
  }
  // +=========================================================================+
  // | [>] check_mailbox                                            ( public ) |
  // +-------------------------------------------------------------------------+
  // | Checks one mailbox, with an optional display name and angle brackets.   |
  // | Comma-separated mailboxes are not accepted here.                        |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_mailbox(std::string_view sv) noexcept {
    {
      std::size_t i = 0;
      while (i < sv.size() && sv[i] != '<') {
        const std::size_t before = i;
        if (!consume_word(sv, i)) break;
        if (i == before) break;
      }
      if (consume_cfws(sv, i) && i < sv.size() && sv[i] == '<') {
        i++;
        if (consume_addr_spec(sv, i) && i < sv.size() && sv[i] == '>') {
          i++;
          if (consume_cfws(sv, i) && i == sv.size()) return true;
        }
      }
    }
    {
      std::size_t i = 0;
      if (consume_addr_spec(sv, i) && i == sv.size()) return true;
    }
    return false;
  }
  // +=========================================================================+
  // | [>] consume_token_or_quoted_string                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns the token or quoted prefix. Empty means failure.                |
  // +-------------------------------------------------------------------------+
  static constexpr std::string_view consume_token_or_quoted_string(
      std::string_view sv) noexcept {
    if (!sv.empty() && sv.front() == '"') return consume_quoted_string(sv);
    return consume_token(sv);
  }
  // +=========================================================================+
  // | [>] is_cookie_value                                          ( public ) |
  // +-------------------------------------------------------------------------+
  // | Allows an empty value, optionally wrapped in quotes.                    |
  // | Cookie quotes do not enable backslash escapes.                          |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_cookie_value(std::string_view sv) noexcept {
    if (sv.size() >= 2 && sv.front() == '"' && sv.back() == '"') {
      sv.remove_prefix(1);
      sv.remove_suffix(1);
    } else if (!sv.empty() && (sv.front() == '"' || sv.back() == '"')) {
      return false;
    }
    for (const char c : sv) {
      if (!is_cookie_octet(c)) return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] is_cookie_pair                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Requires name=value, with no whitespace around =. Value may be empty.   |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_cookie_pair(std::string_view sv) noexcept {
    const std::string_view name = consume_token(sv);
    if (name.empty()) return false;
    if (name.size() >= sv.size() || sv[name.size()] != '=') return false;
    return is_cookie_value(sv.substr(name.size() + 1));
  }
  // +=========================================================================+
  // | [>] is_cookie_av                                             ( public ) |
  // +-------------------------------------------------------------------------+
  // | Checks allowed attribute bytes, not what the attribute means.           |
  // | For example, Max-Age=abc passes this check. Empty input does not.       |
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
  // +-------------------------------------------------------------------------+
  // | Checks a token with an optional =value. No whitespace around =.         |
  // +-------------------------------------------------------------------------+
  static constexpr bool is_directive(std::string_view sv) noexcept {
    std::size_t bytes_used = 0;
    if (!consume_extension_parameter(sv, bytes_used)) return false;
    return bytes_used == sv.size();
  }
  // +=========================================================================+
  // | [>] consume_parameter                                        ( public ) |
  // +-------------------------------------------------------------------------+
  // | Consumes name=value; allow_bws permits spaces and tabs around =.        |
  // | Reports the consumed byte count. On failure, bytes_used is zero.        |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_parameter(std::string_view sv,
                                          std::size_t& bytes_used,
                                          bool allow_bws) noexcept {
    bytes_used = 0;
    std::size_t i = 0;
    const std::string_view name = consume_token(sv);
    if (name.empty()) return false;
    i += name.size();
    if (allow_bws) {
      while (i < sv.size() && is_ows(sv[i])) i++;
    }
    if (i >= sv.size() || sv[i++] != '=') return false;
    if (allow_bws) {
      while (i < sv.size() && is_ows(sv[i])) i++;
    }
    if (i >= sv.size()) return false;
    const std::string_view value = consume_token_or_quoted_string(sv.substr(i));
    if (value.empty()) return false;
    bytes_used = i + value.size();
    return true;
  }
  // +=========================================================================+
  // | [>] consume_extension_parameter                              ( public ) |
  // +-------------------------------------------------------------------------+
  // | Like a parameter, but the value is optional. No whitespace around =.    |
  // | If = is present, a value must follow. bytes_used is zero on failure.    |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_extension_parameter(
      std::string_view sv, std::size_t& bytes_used) noexcept {
    bytes_used = 0;
    const std::string_view name = consume_token(sv);
    if (name.empty()) return false;
    std::size_t i = name.size();
    if (i >= sv.size() || sv[i] != '=') {
      bytes_used = i;
      return true;
    }
    ++i;
    if (i >= sv.size()) return false;
    const std::string_view value = consume_token_or_quoted_string(sv.substr(i));
    if (value.empty()) return false;
    bytes_used = i + value.size();
    return true;
  }
  // +=========================================================================+
  // | [>] for_each_parameter                                       ( public ) |
  // +-------------------------------------------------------------------------+
  // | Each parameter starts with ;, with optional spaces/tabs around it.      |
  // | require_parameter decides whether empty slots are allowed.              |
  // | consume must report success and a valid, nonzero byte count.            |
  // +-------------------------------------------------------------------------+
  template <typename ParameterConsumer>
  static constexpr bool for_each_parameter(std::string_view sv,
                                           bool require_parameter,
                                           ParameterConsumer&& consume) {
    std::size_t i = 0;
    while (i < sv.size()) {
      while (i < sv.size() && is_ows(sv[i])) i++;
      if (i >= sv.size() || sv[i++] != ';') return false;
      while (i < sv.size() && is_ows(sv[i])) i++;
      if (i >= sv.size()) {
        if (require_parameter) return false;
        return true;
      }
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
  // +-------------------------------------------------------------------------+
  // | Splits one Forwarded element on ;, with no whitespace around it.        |
  // | Empty slots are allowed, and the first pair needs no leading ;.         |
  // | consume must report success and a valid, nonzero byte count.            |
  // +-------------------------------------------------------------------------+
  template <typename PairConsumer>
  static constexpr bool for_each_forwarded_pair(std::string_view sv,
                                                PairConsumer&& consume) {
    std::size_t i = 0;
    bool first = true;
    while (i < sv.size()) {
      if (!first) {
        if (sv[i] != ';') return false;
        ++i;
      }
      first = false;
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
  // +-------------------------------------------------------------------------+
  // | Splits on commas outside quoted strings. Trims spaces/tabs and skips    |
  // | empty slots. Rejects broken quotes, escapes, or a rejected element.     |
  // +-------------------------------------------------------------------------+
  template <typename ElementConsumer>
  static constexpr bool for_each_list_element(
      std::string_view sv, ElementConsumer&& consume_element) {
    bool follows_separator = false;
    std::size_t i = 0;
    std::size_t last = 0;
    bool inside_string = false;
    while (i < sv.size()) {
      if (sv[i] == '"') {
        inside_string = !inside_string;
        i++;
        continue;
      }
      if (sv[i] == '\\') {
        if (!inside_string || i + 1 >= sv.size()) return false;
        i += 2;
        continue;
      }
      if (sv[i] == ',' && !inside_string) {
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
    if (inside_string) return false;
    std::string_view element = sv.substr(last);
    if (follows_separator) ows_ltrim(element);
    if (!element.empty() && !consume_element(element)) return false;
    return true;
  }
  // +=========================================================================+
  // | [>] check_auth_param_list                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Checks comma-separated name=value pairs, allowing whitespace around =.  |
  // | Empty lists and empty slots are allowed.                                |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_auth_param_list(std::string_view sv) noexcept {
    return for_each_list_element(sv, [](std::string_view element) {
      std::size_t bytes_used = 0;
      if (!consume_parameter(element, bytes_used, /*allow_bws=*/true)) {
        return false;
      }
      return bytes_used == element.size();
    });
  }
  // +=========================================================================+
  // | [>] check_credentials                                        ( public ) |
  // +-------------------------------------------------------------------------+
  // | A scheme may stand alone or have token68/auth parameters after it.      |
  // | The separator must be spaces, not tabs. Empty parameter lists are OK.   |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_credentials(std::string_view sv) noexcept {
    const std::string_view scheme = consume_token(sv);
    if (scheme.empty()) return false;
    if (scheme.size() == sv.size()) return true;
    std::size_t i = scheme.size();
    if (sv[i] != ' ') return false;
    while (i < sv.size() && sv[i] == ' ') i++;
    const std::string_view rest = sv.substr(i);
    if (is_token68(rest)) return true;
    return check_auth_param_list(rest);
  }
  // +=========================================================================+
  // | [>] challenge_opens_param_list                               ( public ) |
  // +-------------------------------------------------------------------------+
  // | Expects a trimmed, valid challenge. Reports whether following           |
  // | auth parameters can still belong to it; token68 has no such list.       |
  // +-------------------------------------------------------------------------+
  static constexpr bool challenge_opens_param_list(
      std::string_view element) noexcept {
    const std::string_view scheme = consume_token(element);
    if (scheme.empty() || scheme.size() == element.size()) return false;
    std::size_t i = scheme.size();
    while (i < element.size() && element[i] == ' ') i++;
    return !is_token68(element.substr(i));
  }
  // +=========================================================================+
  // | [>] check_challenge_list                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | A comma can start a challenge or continue its auth parameters.          |
  // | Only a challenge with a parameter list accepts more parameters.         |
  // | Empty lists and empty slots are allowed.                                |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_challenge_list(std::string_view sv) noexcept {
    bool in_param_list = false;
    return for_each_list_element(
        sv, [&in_param_list](std::string_view element) {
          std::size_t bytes_used = 0;
          if (consume_parameter(element, bytes_used, /*allow_bws=*/true) &&
              bytes_used == element.size()) {
            return in_param_list;
          }
          if (!check_credentials(element)) return false;
          in_param_list = challenge_opens_param_list(element);
          return true;
        });
  }
  // +=========================================================================+
  // | [>] consume_product                                          ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns a product prefix with an optional /version; empty on failure.   |
  // | A trailing / without a version is invalid.                              |
  // +-------------------------------------------------------------------------+
  static constexpr std::string_view consume_product(
      std::string_view sv) noexcept {
    const std::string_view name = consume_token(sv);
    if (name.empty()) return {};
    std::size_t i = name.size();
    if (i < sv.size() && sv[i] == '/') {
      const std::string_view version = consume_token(sv.substr(i + 1));
      if (version.empty()) return {};
      i += 1 + version.size();
    }
    return sv.substr(0, i);
  }
  // +=========================================================================+
  // | [>] check_product_list                                       ( public ) |
  // +-------------------------------------------------------------------------+
  // | Starts with a product, then products/comments separated by spaces/tabs. |
  // | No leading or trailing whitespace, and no comma separators.             |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_product_list(std::string_view sv) noexcept {
    const std::string_view first = consume_product(sv);
    if (first.empty()) return false;
    std::size_t i = first.size();
    while (i < sv.size()) {
      const std::size_t rws_start = i;
      while (i < sv.size() && is_ows(sv[i])) i++;
      if (i == rws_start) return false;
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
  // +-------------------------------------------------------------------------+
  // | An empty host counts as a registered name.                              |
  // +-------------------------------------------------------------------------+
  static host_type check_uri_host(std::string_view sv) {
    if (sv.empty()) return host_type::kRegName;
    if (helpers::is_ip_literal(sv)) return host_type::kIpLiteral;
    if (helpers::is_ip_v4_address(sv)) return host_type::kIpV4Address;
    if (helpers::is_reg_name(sv)) return host_type::kRegName;
    return host_type::kUnknown;
  }
  // +=========================================================================+
  // | [>] is_uri_scheme                                            ( public ) |
  // +-------------------------------------------------------------------------+
  // | Checks the scheme name without its trailing colon.                      |
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
  // +-------------------------------------------------------------------------+
  // | Skips scheme: from i when present. Leaves i unchanged on failure.       |
  // +-------------------------------------------------------------------------+
  static constexpr bool consume_uri_scheme(std::string_view sv,
                                           std::size_t& i) noexcept {
    const std::size_t colon_at = sv.find(':');
    if (colon_at == std::string_view::npos) return false;
    if (!is_uri_scheme(sv.substr(0, colon_at))) return false;
    i = colon_at + 1;
    return true;
  }
  // +=========================================================================+
  // | [>] consume_uri_pct_encoded                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Expects sv[i] to be %. Skips the %HH triplet on success.                |
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
  // +-------------------------------------------------------------------------+
  // | Expects valid %HH sequences. Rejects encoded NUL without changing sv.   |
  // | This does not check the URI syntax; the parser must do that first.      |
  // +-------------------------------------------------------------------------+
  static constexpr bool percent_decode_validate(std::string_view sv) noexcept {
    const std::size_t len = sv.size();
    for (std::size_t r = 0; r < len;) {
      if (sv[r] == '%' && r + 2 < len && is_hex_digit(sv[r + 1]) &&
          is_hex_digit(sv[r + 2])) {
        auto hex_val = [](char h) -> unsigned char {
          if (h >= '0' && h <= '9') return static_cast<unsigned char>(h - '0');
          if (h >= 'a' && h <= 'f') {
            return static_cast<unsigned char>(h - 'a' + 10);
          }
          return static_cast<unsigned char>(h - 'A' + 10);
        };
        unsigned char decoded = static_cast<unsigned char>(
            (hex_val(sv[r + 1]) << 4) | hex_val(sv[r + 2]));
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
  // +-------------------------------------------------------------------------+
  // | Writes decoded bytes into the same buffer and shortens the view.        |
  // | The buffer must be writable. Check URI syntax and encoded NUL first.    |
  // | Decodes %2F too: identify path segments before decoding.                |
  // +-------------------------------------------------------------------------+
  static constexpr void percent_decode_in_place(std::string_view& sv) noexcept {
    // No % means there is nothing to rewrite.
    const std::size_t pct_at = sv.find('%');
    if (pct_at == std::string_view::npos) return;
    char* data = const_cast<char*>(sv.data());
    const std::size_t len = sv.size();
    // Leave the prefix before the first % untouched.
    std::size_t r = pct_at, w = pct_at;
    while (r < len) {
      if (data[r] == '%' && r + 2 < len && is_hex_digit(data[r + 1]) &&
          is_hex_digit(data[r + 2])) {
        auto hex_val = [](char h) -> unsigned char {
          if (h >= '0' && h <= '9') return static_cast<unsigned char>(h - '0');
          if (h >= 'a' && h <= 'f') {
            return static_cast<unsigned char>(h - 'a' + 10);
          }
          return static_cast<unsigned char>(h - 'A' + 10);
        };
        unsigned char decoded = static_cast<unsigned char>(
            (hex_val(data[r + 1]) << 4) | hex_val(data[r + 2]));
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
  // +-------------------------------------------------------------------------+
  // | Checks host[:port], keeping brackets around IP literals.                |
  // | Empty hosts and ports are allowed. No port range check here.            |
  // | Output views borrow the input. Ignore them on failure.                  |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_host_port(std::string_view sv,
                                        std::string_view& uri_host,
                                        std::string_view& port,
                                        host_type& type) noexcept {
    std::size_t colon_at;
    if (!split_authority_host_port(sv, uri_host, colon_at)) return false;
    type = uri_host.empty() ? host_type::kRegName : check_uri_host(uri_host);
    if (type == host_type::kUnknown) return false;
    port = {};
    if (colon_at < sv.size()) {
      if (sv[colon_at] != ':') return false;
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
  // +-------------------------------------------------------------------------+
  // | Checks scheme://host[:port], without userinfo, path, query or fragment. |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_serialized_origin(std::string_view sv) noexcept {
    std::size_t i = 0;
    if (!consume_uri_scheme(sv, i)) return false;
    if (i + 1 >= sv.size() || sv[i] != '/' || sv[i + 1] != '/') return false;
    i += 2;
    return check_host_port(sv.substr(i));
  }
  // +=========================================================================+
  // | [>] check_uri_authority                                      ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_uri_authority(std::string_view sv) noexcept {
    std::string_view rest = sv;
    const std::size_t at_pos = sv.find('@');
    if (at_pos != std::string_view::npos) {
      if (!check_uri_userinfo(sv.substr(0, at_pos))) return false;
      rest = sv.substr(at_pos + 1);
    }
    return check_host_port(rest);
  }
  // +=========================================================================+
  // | [>] check_uri_reference                                      ( public ) |
  // +-------------------------------------------------------------------------+
  // | Checks a whole URI reference, with no surrounding whitespace.           |
  // | allow_fragment enables #fragment. Empty relative references are OK.     |
  // +-------------------------------------------------------------------------+
  static constexpr bool check_uri_reference(std::string_view sv,
                                            bool allow_fragment) noexcept {
    if (sv.empty()) return true;
    std::size_t i = 0;
    const bool has_scheme = consume_uri_scheme(sv, i);
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
    const std::size_t path_start = i;
    while (i < sv.size() && sv[i] != '?' && sv[i] != '#') {
      if (sv[i] == '%') {
        if (!consume_uri_pct_encoded(sv, i)) return false;
        continue;
      }
      if (sv[i] != '/' && !is_pchar(sv[i])) return false;
      i++;
    }
    // Without a scheme or authority, the first path segment cannot contain :.
    if (!has_scheme && !has_authority) {
      const std::string_view path = sv.substr(path_start, i - path_start);
      const std::string_view first_segment = path.substr(0, path.find('/'));
      if (first_segment.find(':') != std::string_view::npos) return false;
    }
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
  // +-------------------------------------------------------------------------+
  // | Keeps bracketed IP hosts together. False means a missing closing ].     |
  // | colon_found_at is the first colon, or npos when absent.                 |
  // | For bracketed hosts it points just after ]; check bounds and colon.     |
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
  // +-------------------------------------------------------------------------+
  // | Reads host:port, preserving brackets around IP literals.                |
  // | Returns the host, raw port digits, host type and consumed byte count.   |
  // | An empty port is allowed. The caller checks what follows it.            |
  // +-------------------------------------------------------------------------+
  static constexpr deserialization_status try_to_deserialize_as_authority_form(
      std::string_view sv, std::string_view& uri_host, std::string_view& port,
      host_type& type, std::size_t& bytes_used) noexcept {
    bytes_used = 0;
    if (sv.empty()) return deserialization_status::kMoreBytesNeeded;
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
    // After a bracketed host, the next character might not be a colon.
    if (colon_at >= sv.size()) return deserialization_status::kMoreBytesNeeded;
    if (sv[colon_at] != ':') return deserialization_status::kInvalidSource;
    std::size_t i = colon_at + 1;
    while (i < sv.size() && is_digit(sv[i])) i++;
    uri_host = sv_uri_host;
    port = sv.substr(colon_at + 1, i - (colon_at + 1));
    bytes_used = i;
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // | [>] try_to_deserialize_as_asterisk_form                      ( public ) |
  // +-------------------------------------------------------------------------+
  // | Consumes only *. The caller checks what follows it.                     |
  // +-------------------------------------------------------------------------+
  static constexpr deserialization_status try_to_deserialize_as_asterisk_form(
      std::string_view sv, std::size_t& bytes_used) noexcept {
    if (sv.empty()) return deserialization_status::kMoreBytesNeeded;
    if (sv.front() != '*') return deserialization_status::kInvalidSource;
    bytes_used = 1;
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // | [>] try_to_deserialize_as_origin_form                        ( public ) |
  // +-------------------------------------------------------------------------+
  // | Reads an absolute path and optional query, excluding the ? separator.   |
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
      // Skip the separator: the query helper expects content without it.
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
  // +-------------------------------------------------------------------------+
  // | Uses 80 for http and 443 for https, ignoring case; empty otherwise.     |
  // +-------------------------------------------------------------------------+
  static constexpr std::string_view default_port_for_scheme(
      std::string_view scheme) noexcept {
    if (iequals(scheme, "http")) return "80";
    if (iequals(scheme, "https")) return "443";
    return std::string_view();
  }
  // +=========================================================================+
  // | [>] ports_equivalent                                         ( public ) |
  // +-------------------------------------------------------------------------+
  // | Compares raw port text, using the scheme default for an empty port.     |
  // | Without a known default, only identical text matches.                   |
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
  // | Reads an absolute URI, keeping the scheme text as received.             |
  // | has_authority says whether host, port and type describe an authority.   |
  // | Views borrow the input. bytes_used marks where parsing stopped.         |
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
    if (!is_alpha(sv[i])) return deserialization_status::kInvalidSource;
    i++;
    while (i < sv_size && sv[i] != ':') {
      if (!is_alpha(sv[i]) && !is_digit(sv[i]) && sv[i] != '+' &&
          sv[i] != '-' && sv[i] != '.') {
        return deserialization_status::kInvalidSource;
      }
      i++;
    }
    if (i >= sv_size) return deserialization_status::kMoreBytesNeeded;
    scheme = sv.substr(0, i);  // scheme text, excluding the ':' terminator.
    i++;                       // skip ':'
    std::size_t path_start = i;
    if (i + 1 < sv_size && sv[i] == '/' && sv[i + 1] == '/') {
      i += 2;
      const std::size_t authority_start = i;
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
        if (!is_unreserved(c) && !is_sub_delim(c) && c != ':' && c != '@' &&
            c != '[' && c != ']') {
          break;  // Non-authority char (e.g., space) ends authority.
        }
        i++;
      }
      const std::string_view authority =
          sv.substr(authority_start, i - authority_start);
      const std::size_t at_pos = authority.find('@');
      std::string_view host_port;
      if (at_pos != std::string_view::npos) {
        const std::string_view userinfo = authority.substr(0, at_pos);
        for (std::size_t j = 0; j < userinfo.size();) {
          if (userinfo[j] == '%') {
            // The authority scan already checked these %HH sequences.
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
  // +-------------------------------------------------------------------------+
  // | Reads a path starting with /. Checks %HH sequences without decoding.    |
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
  // +-------------------------------------------------------------------------+
  // | Reads query content without the leading ?. Checks %HH without decoding. |
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
  // +-------------------------------------------------------------------------+
  // | Ignores ASCII letter case.                                              |
  // +-------------------------------------------------------------------------+
  static constexpr bool iequals(std::string_view a, std::string_view b) {
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), [](char ac, char bc) {
             return tolower_ascii(ac) == tolower_ascii(bc);
           });
  }
  // +=========================================================================+
  // | [>] split_query_parameters                                   ( public ) |
  // +-------------------------------------------------------------------------+
  // | Expects query content without the leading ?. Splits pairs on &.         |
  // | The first = separates key and value; no = means an empty value.         |
  // | Skips empty pairs. Leaves %HH sequences and + unchanged.                |
  // | Views borrow the query buffer; keep it alive while using them.          |
  // | out_values needs at least out_keys.size() slots. Drops extra pairs.     |
  // | Returns the number of pairs written.                                    |
  // +-------------------------------------------------------------------------+
  static constexpr std::size_t split_query_parameters(
      std::string_view query, std::span<std::string_view> out_keys,
      std::span<std::string_view> out_values) noexcept {
    const std::size_t max_pairs = out_keys.size();
    std::size_t count = 0;
    std::size_t i = 0;
    const std::size_t size = query.size();
    while (i < size && count < max_pairs) {
      std::size_t pair_end = i;
      while (pair_end < size && query[pair_end] != '&') pair_end++;
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
      i = (pair_end < size) ? pair_end + 1 : pair_end;
    }
    return count;
  }
};
}  // namespace martianlabs::doba::protocol::http

#endif
