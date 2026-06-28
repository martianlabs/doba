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

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] helpers                                                    ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct holds for the http 1.1 helper functions.                      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct helpers {
  // +=========================================================================+
  // | [>] is_digit                                                 ( public ) |
  // +=========================================================================+
  // | RFC 5234 (ABNF Core Rules) — DIGIT                                      |
  // +-------------------------------------------------------------------------+
  // | DIGIT = %x30-39    ; "0" - "9"                                          |
  // +-------------------------------------------------------------------------+
  static bool is_digit(unsigned char c) noexcept {
    return c >= 0x30 && c <= 0x39;
  }
  static bool is_digit(char c) noexcept {
    return is_digit(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_hex_digit                                             ( public ) |
  // +=========================================================================+
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
  static bool is_hex_digit(unsigned char c) noexcept {
    return is_digit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
  }
  static bool is_hex_digit(char c) noexcept {
    return is_hex_digit(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_alpha                                                 ( public ) |
  // +=========================================================================+
  // | RFC 5234 (ABNF Core Rules) — ALPHA                                      |
  // +-------------------------------------------------------------------------+
  // | ALPHA = %x41-5A / %x61-7A                                               |
  // |        ; A-Z / a-z                                                      |
  // +-------------------------------------------------------------------------+
  static bool is_alpha(unsigned char c) noexcept {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
  }
  static bool is_alpha(char c) noexcept {
    return is_alpha(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_token                                                 ( public ) |
  // +=========================================================================+
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
  static bool is_token(unsigned char c) noexcept {
    return c == '!' || c == '#' || c == '$' || c == '%' || c == '&' ||
           c == '\'' || c == '*' || c == '+' || c == '-' || c == '.' ||
           c == '^' || c == '_' || c == '`' || c == '|' || c == '~' ||
           is_digit(c) || is_alpha(c);
  }
  static bool is_token(char c) noexcept {
    return is_token(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_unreserved                                            ( public ) |
  // +=========================================================================+
  // | RFC 3986 §2.3 — unreserved                                              |
  // +-------------------------------------------------------------------------+
  // | unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"                      |
  // +-------------------------------------------------------------------------+
  // | ALPHA      = %x41-5A / %x61-7A   ; A-Z / a-z                            |
  // | DIGIT      = %x30-39             ; 0-9                                  |
  // +-------------------------------------------------------------------------+
  static bool is_unreserved(unsigned char c) noexcept {
    return is_alpha(c) || is_digit(c) || c == '-' || c == '.' || c == '_' ||
           c == '~';
  }
  static bool is_unreserved(char c) noexcept {
    return is_unreserved(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_sub_delim                                             ( public ) |
  // +=========================================================================+
  // | RFC 3986 §2.2 — sub-delims                                              |
  // +-------------------------------------------------------------------------+
  // | sub-delims = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," /      |
  // |              ";" / "="                                                  |
  // +-------------------------------------------------------------------------+
  static bool is_sub_delim(unsigned char c) noexcept {
    return c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' ||
           c == ')' || c == '*' || c == '+' || c == ',' || c == ';' || c == '=';
  }
  static bool is_sub_delim(char c) noexcept {
    return is_sub_delim(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_pchar                                                 ( public ) |
  // +=========================================================================+
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
  static bool is_pchar(unsigned char c) noexcept {
    return is_unreserved(c) || is_sub_delim(c) || c == ':' || c == '@';
  }
  static bool is_pchar(char c) noexcept {
    return is_pchar(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_vchar                                                 ( public ) |
  // +=========================================================================+
  // | RFC 5234 (ABNF Core Rules) — VCHAR                                      |
  // +-------------------------------------------------------------------------+
  // | VCHAR = %x21-7E                                                         |
  // |        ; visible (printing) characters                                  |
  // +-------------------------------------------------------------------------+
  static bool is_vchar(unsigned char c) noexcept {
    return c >= 0x21 && c <= 0x7E;
  }
  static bool is_vchar(char c) noexcept {
    return is_vchar(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_qdtext                                                ( public ) |
  // +=========================================================================+
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
  static bool is_qdtext(unsigned char c) noexcept {
    return c == '\t' || c == ' ' || c == '!' || (c >= 0x23 && c <= 0x5B) ||
           (c >= 0x5D && c <= 0x7E) || is_obs_text(c);
  }
  static bool is_qdtext(char c) noexcept {
    return is_qdtext(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_obs_text                                              ( public ) |
  // +=========================================================================+
  // | RFC 9110 §5.6.4 — obs-text                                              |
  // +-------------------------------------------------------------------------+
  // | obs-text = %x80-FF                                                      |
  // +-------------------------------------------------------------------------+
  // | Note: obs-text represents obsolete text allowed for backward            |
  // |       compatibility with legacy implementations.                        |
  // +-------------------------------------------------------------------------+
  static bool is_obs_text(unsigned char c) noexcept {
    return c >= 0x80 && c <= 0xFF;
  }
  static bool is_obs_text(char c) noexcept {
    return is_obs_text(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] is_ows                                                   ( public ) |
  // +=========================================================================+
  // | RFC 9110 §5.6.3 — OWS (Optional Whitespace)                             |
  // +-------------------------------------------------------------------------+
  // | OWS = *( SP / HTAB )                                                    |
  // +-------------------------------------------------------------------------+
  // | SP   = %x20                                                             |
  // | HTAB = %x09                                                             |
  // +-------------------------------------------------------------------------+
  // | Note: OWS is allowed only where explicitly defined by the ABNF.         |
  // +-------------------------------------------------------------------------+
  static bool is_ows(unsigned char c) noexcept { return c == ' ' || c == '\t'; }
  static bool is_ows(char c) noexcept {
    return is_ows(static_cast<unsigned char>(c));
  }
  // +=========================================================================+
  // | [>] tolower_ascii                                            ( public ) |
  // +=========================================================================+
  // | Converts character to its lower-case version.                           |
  // +-------------------------------------------------------------------------+
  static unsigned char tolower_ascii(unsigned char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
  }
  static char tolower_ascii(char c) noexcept {
    return static_cast<char>(tolower_ascii(static_cast<unsigned char>(c)));
  }
  // +=========================================================================+
  // | [>] ows_ltrim                                                ( public ) |
  // +=========================================================================+
  // | Performs OWS trimming on the left side of the provided string_view.     |
  // +-------------------------------------------------------------------------+
  static void ows_ltrim(std::string_view& sv) noexcept {
    while (!sv.empty() && helpers::is_ows(sv.front())) {
      sv.remove_prefix(1);
    }
  }
  // +=========================================================================+
  // | [>] ows_rtrim                                                ( public ) |
  // +=========================================================================+
  // | Performs OWS trimming on the right side of the provided string_view.    |
  // +-------------------------------------------------------------------------+
  static void ows_rtrim(std::string_view& sv) noexcept {
    while (!sv.empty() && helpers::is_ows(sv.back())) {
      sv.remove_suffix(1);
    }
  }
  // +=========================================================================+
  // | [>] ows_trim                                                 ( public ) |
  // +=========================================================================+
  // | Performs OWS trimming on left & right sides of the provided string_view.|
  // +-------------------------------------------------------------------------+
  static void ows_trim(std::string_view& sv) noexcept {
    ows_ltrim(sv);
    ows_rtrim(sv);
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
  static std::string_view consume_token(std::string_view sv) noexcept {
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
  // | [>] consume_authority_form                                   ( public ) |
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
  static deserialization_status consume_authority_form(
      std::string_view sv) noexcept {
    return deserialization_status::kInvalidSource;
  }
  // +=========================================================================+
  // | [>] consume_asterisk_form                                    ( public ) |
  // +=========================================================================+
  // +----------------+--------------------------------------------------------+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | asterisk-form  | "*"                                                    |
  // +----------------+--------------------------------------------------------+
  static deserialization_status consume_asterisk_form(
      std::string_view sv, std::size_t& bytes_used) noexcept {
    const std::size_t sv_size = sv.size();
    if (!sv_size) return deserialization_status::kMoreBytesNeeded;
    if (sv[0] != '*') return deserialization_status::kInvalidSource;
    bytes_used = 1;
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // | [>] consume_origin_form                                      ( public ) |
  // +=========================================================================+
  // | Field          | Definition                                             |
  // +----------------+--------------------------------------------------------+
  // | origin-form    | absolute-path [ "?" query ]                            |
  // +----------------+--------------------------------------------------------+
  static deserialization_status consume_origin_form(
      std::string_view sv, std::string_view& consumed_path,
      std::string_view& consumed_query, std::size_t& bytes_used) noexcept {
    std::size_t i;
    std::string_view tmp_path, tmp_query;
    const std::size_t sv_size = sv.size();
    deserialization_status result = consume_absolute_path(sv, tmp_path, i);
    if (result != deserialization_status::kSucceeded) return result;
    if (i < sv_size && sv[i] == '?') {
      std::size_t bytes = 0;
      result = consume_query(sv.substr(i), tmp_query, bytes);
      if (result != deserialization_status::kSucceeded) return result;
      i += bytes;
    }
    bytes_used = i;
    consumed_path = std::move(tmp_path);
    consumed_query = std::move(tmp_query);
    return deserialization_status::kSucceeded;
  }
  // +=========================================================================+
  // | [>] consume_absolute_form                                    ( public ) |
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
  static deserialization_status consume_absolute_form(
      std::string_view sv) noexcept {
    return deserialization_status::kInvalidSource;
  }
  // +=========================================================================+
  // | [>] consume_absolute_path                                    ( public ) |
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
  static deserialization_status consume_absolute_path(
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
  // | [>] consume_query                                            ( public ) |
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
  static deserialization_status consume_query(
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
  // | [>] parse_quoted_string                                      ( public ) |
  // +=========================================================================+
  // | Consumes a quoted-string from the provided string_view.                 |
  // +-------------------------------------------------------------------------+
  static std::string_view parse_quoted_string(std::string_view sv) noexcept {
    if (sv.empty() || sv.front() != '"') return {};
    std::size_t end_index = 1;
    bool inside_quoted_string = true;
    while (end_index < sv.size() && inside_quoted_string) {
      if (sv[end_index] == '"') {
        inside_quoted_string = false;
      } else if (sv[end_index] == '\\' && end_index + 1 < sv.size()) {
        end_index += 2;  // Skip the escaped character
        continue;
      }
      end_index++;
    }
    if (inside_quoted_string) return {};
    return sv.substr(0, end_index);
  }
  // +=========================================================================+
  // | [>] iequals                                                  ( public ) |
  // +=========================================================================+
  // | Compares two string_views for case-insensitive equality.                |
  // +-------------------------------------------------------------------------+
  static bool iequals(std::string_view a, std::string_view b) {
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), [](char ac, char bc) {
             return tolower_ascii(ac) == tolower_ascii(bc);
           });
  }
};
}  // namespace martianlabs::doba::protocol::http11

#endif
