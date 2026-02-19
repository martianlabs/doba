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

#ifndef martianlabs_doba_protocol_http11_checkers_host_h
#define martianlabs_doba_protocol_http11_checkers_host_h

#include <array>
#include <string_view>

namespace martianlabs::doba::protocol::http11::checkers {
// +----------------+----------------------------------------------------------+
// | uri-host       | IP-literal / IPv4address / reg-name                      |
// +----------------+----------------------------------------------------------+
enum class host_type { kUnknown, kIpLiteral, kIpV4Address, kRegName };
// +----------------+----------------------------------------------------------+
// | h16            | 1*4HEXDIG                                                |
// +----------------+----------------------------------------------------------+
static inline int skip_n_hextets(std::string_view sv) {
  int hextets_found = 0;
  std::size_t off = 0;
  std::size_t hex_digits = 0;
  while (off < sv.size()) {
    if (helpers::is_hex_digit(sv[off])) {
      if (++hex_digits > 4) return -1;
    } else if (sv[off] == constants::character::kColon) {
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
  if (sv.size() && sv.back() == constants::character::kColon) return -1;
  if (hex_digits) hextets_found++;
  return hextets_found;
}
// +----------------+----------------------------------------------------------+
// | dec-octet      | DIGIT                    ; 0-9                           |
// |                | / %x31-39 DIGIT          ; 10-99                         |
// |                | / "1" 2DIGIT             ; 100-199                       |
// |                | / "2" %x30-34 DIGIT      ; 200-249                       |
// |                | / "25" %x30-35           ; 250-255                       |
// +----------------+----------------------------------------------------------+
static inline bool check_dec_octet(std::string_view sv, std::size_t digits,
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
// +--------------+------------------------------------------------------------+
// | IPvFuture    | "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )       |
// +--------------+------------------------------------------------------------+
static inline bool check_ip_v_future(std::string_view sv) {
  if (sv.empty()) return false;
  if (sv.front() != 'v') return false;
  // [1*HEXDIG "."] part..
  std::size_t hex_digits_found = 0;
  std::size_t off = 1;
  while (off < sv.size() && sv[off] != constants::character::kDot) {
    if (!helpers::is_hex_digit(sv[off])) return false;
    hex_digits_found++;
    off++;
  }
  if (off++ == sv.size() || !hex_digits_found) return false;
  // [1*( unreserved / sub-delims / ":" )] part..
  std::size_t subparts_found = 0;
  while (off < sv.size()) {
    if (!helpers::is_unreserved(sv[off]) && !helpers::is_sub_delim(sv[off]) &&
        sv[off] != constants::character::kColon) {
      return false;
    }
    subparts_found++;
    off++;
  }
  return subparts_found > 0;
}
// +----------------+----------------------------------------------------------+
// | IPv4address    | dec-octet "." dec-octet "." dec-octet "." dec-octet      |
// +----------------+----------------------------------------------------------+
static inline bool check_ip_v4_address(std::string_view sv) {
  if (sv.empty()) return false;
  int dec_octets_count = 0;
  std::size_t off = 0;
  std::size_t digits = 0;
  while (off < sv.size()) {
    if (sv[off] == constants::character::kDot) {
      if (!digits || digits > 3) return false;
      if (!check_dec_octet(sv, digits, off)) return false;
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
  if (!check_dec_octet(sv, digits, off)) return false;
  return ++dec_octets_count == 4;
}
// +----------------+----------------------------------------------------------+
// | IPv6address    | 6( h16 ":" ) ls32                                        |
// |                | / "::" 5( h16 ":" ) ls32                                 |
// |                | / [ h16 ] "::" 4( h16 ":" ) ls32                         |
// |                | / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32           |
// |                | / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32           |
// |                | / [ *3( h16 ":" ) h16 ] "::" h16 ":" ls32                |
// |                | / [ *4( h16 ":" ) h16 ] "::" ls32                        |
// |                | / [ *5( h16 ":" ) h16 ] "::" h16                         |
// |                | / [ *6( h16 ":" ) h16 ] "::"                             |
// +----------------+----------------------------------------------------------+
// | ls32           | ( h16 ":" h16 ) / IPv4address                            |
// | h16            | 1*4HEXDIG                                                |
// +----------------+----------------------------------------------------------+
static inline bool check_ip_v6_address(std::string_view sv) {
  // first character must be either ':' or hex-digit..
  std::size_t sep = sv.find("::");
  std::string_view head;
  std::string_view tail;
  if (sep == std::string_view::npos) {
    // +-----------------------------------------------------------------------+
    // | no-compression case!                                                  |
    // +-----------------------------------------------------------------------+
    // | 6( h16 ":" ) ls32                                                     |
    // +-----------------------------------------------------------------------+
    std::size_t dot = sv.find(constants::character::kDot);
    if (dot == std::string_view::npos) return skip_n_hextets(sv) == 8;
    std::size_t col = sv.find_last_of(constants::character::kColon);
    if (col == std::string_view::npos) return false;
    if (dot <= col) return false;
    head = sv.substr(0, col);
    tail = sv.substr(col + 1);
    if (skip_n_hextets(head) != 6) return false;
    if (!check_ip_v4_address(tail)) return false;
    return true;
  } else if (sv.find("::", sep + 1) == std::string_view::npos) {
    // +-----------------------------------------------------------------------+
    // | compression case!                                                     |
    // +-----------------------------------------------------------------------+
    // | "::" 5( h16 ":" ) ls32                                                |
    // | / [ h16 ] "::" 4( h16 ":" ) ls32                                      |
    // | / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32                        |
    // | / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32                        |
    // | / [ *3( h16 ":" ) h16 ] "::" h16 ":" ls32                             |
    // | / [ *4( h16 ":" ) h16 ] "::" ls32                                     |
    // | / [ *5( h16 ":" ) h16 ] "::" h16                                      |
    // | / [ *6( h16 ":" ) h16 ] "::"                                          |
    // +-----------------------------------------------------------------------+
    std::string_view svl = sv.substr(0, sep);
    std::string_view svr = sv.substr(sep + 2);
    int lh = !svl.empty() ? skip_n_hextets(svl) : 0;
    if (lh == -1) return false;
    std::size_t dot = svr.find(constants::character::kDot);
    if (dot == std::string_view::npos) {
      int rh = !svr.empty() ? skip_n_hextets(svr) : 0;
      return rh == -1 ? false : lh + rh <= 7;
    }
    std::size_t col = svr.find_last_of(constants::character::kColon);
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
    if (!check_ip_v4_address(tail)) return false;
    return true;
  }
  return false;
}
// +----------------+----------------------------------------------------------+
// | IP-literal     | "[" ( IPv6address / IPvFuture ) "]"                      |
// +----------------+----------------------------------------------------------+
static inline bool check_ip_literal(std::string_view sv) {
  if (sv.size() < 2) return false;  // at least two characters..
  if (sv.front() != constants::character::kOpenBracket) return false;
  if (sv.back() != constants::character::kCloseBracket) return false;
  sv.remove_prefix(1);
  sv.remove_suffix(1);
  if (sv.empty()) return false;
  if (check_ip_v_future(sv)) return true;
  if (check_ip_v6_address(sv)) return true;
  return false;
}
// +----------------+----------------------------------------------------------+
// | reg-name       | *( unreserved / pct-encoded / sub-delims )               |
// +----------------+----------------------------------------------------------+
static inline bool check_reg_name(std::string_view sv) {
  std::size_t off = 0;
  while (off < sv.size()) {
    if (!helpers::is_unreserved(sv[off])) {
      if (!helpers::is_sub_delim(sv[off])) {
        if (sv[off] != constants::character::kPercent) return false;
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
// +----------------+----------------------------------------------------------+
// | uri-host       | IP-literal / IPv4address / reg-name                      |
// +----------------+----------------------------------------------------------+
static inline host_type check_uri_host(std::string_view sv) {
  // if the provided field-value is empty then we assume it to be regname..
  if (sv.empty()) return host_type::kRegName;
  // check [ip-literal] host..
  if (check_ip_literal(sv)) return host_type::kIpLiteral;
  // check [ip-v4-address] host..
  if (check_ip_v4_address(sv)) return host_type::kIpV4Address;
  // check [reg-name] host..
  if (check_reg_name(sv)) return host_type::kRegName;
  // default [unknown] host type..
  return host_type::kUnknown;
}
// +----------------+----------------------------------------------------------+
// | port           | *DIGIT                                                   |
// +----------------+----------------------------------------------------------+
static inline bool check_port(std::string_view sv) {
  std::size_t off = 0;
  while (off < sv.size()) {
    if (!helpers::is_digit(sv[off])) return false;
    off++;
  }
  return true;
}
// +===========================================================================+
// |                                                                      host |
// +===========================================================================+
// | RFC 9110 §7.2.Host                                                        |
// +---------------------------------------------------------------------------+
// | The "Host" header field in a request provides the host and optional       |
// | port information from the target URI.                                     |
// +---------------------------------------------------------------------------+
// | RFC 3986 §3.2.2 y §3.2.3 (ABNF summary)                                   |
// +---------------------------------------------------------------------------+
// +----------------+----------------------------------------------------------+
// | Field          | Definition                                               |
// +----------------+----------------------------------------------------------+
// | Host           | uri-host [ ":" port ]                                    |
// | uri-host       | IP-literal / IPv4address / reg-name                      |
// | port           | *DIGIT                                                   |
// +----------------+----------------------------------------------------------+
// | IP-literal     | "[" ( IPv6address / IPvFuture ) "]"                      |
// +----------------+----------------------------------------------------------+
// | IPv6address    | 6( h16 ":" ) ls32                                        |
// |                | / "::" 5( h16 ":" ) ls32                                 |
// |                | / [ h16 ] "::" 4( h16 ":" ) ls32                         |
// |                | / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32           |
// |                | / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32           |
// |                | / [ *3( h16 ":" ) h16 ] "::" h16 ":" ls32                |
// |                | / [ *4( h16 ":" ) h16 ] "::" ls32                        |
// |                | / [ *5( h16 ":" ) h16 ] "::" h16                         |
// |                | / [ *6( h16 ":" ) h16 ] "::"                             |
// +----------------+----------------------------------------------------------+
// | IPvFuture      | "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )     |
// +----------------+----------------------------------------------------------+
// | ls32           | ( h16 ":" h16 ) / IPv4address                            |
// | h16            | 1*4HEXDIG                                                |
// +----------------+----------------------------------------------------------+
// | IPv4address    | dec-octet "." dec-octet "." dec-octet "." dec-octet      |
// | dec-octet      | DIGIT                    ; 0-9                           |
// |                | / %x31-39 DIGIT          ; 10-99                         |
// |                | / "1" 2DIGIT             ; 100-199                       |
// |                | / "2" %x30-34 DIGIT      ; 200-249                       |
// |                | / "25" %x30-35           ; 250-255                       |
// +----------------+----------------------------------------------------------+
// | reg-name       | *( unreserved / pct-encoded / sub-delims )               |
// +----------------+----------------------------------------------------------+
// | unreserved     | ALPHA / DIGIT / "-" / "." / "_" / "~"                    |
// | pct-encoded    | "%" HEXDIG HEXDIG                                        |
// | sub-delims     | "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / ","      |
// |                | / ";" / "="                                              |
// +---------------------------------------------------------------------------+
// | IMPORTANT: field-value is supposed to be normalized (no OWS around value).|
// +---------------------------------------------------------------------------+
static inline bool host(std::string_view sv) {
  std::size_t off = 0;
  if (sv.empty()) return true;  // reg-name!
  std::string_view sv_uri_host;
  std::string_view sv_port;
  if (sv.front() == constants::character::kOpenBracket) {
    std::size_t clb = sv.find_first_of(constants::character::kCloseBracket);
    if (clb == std::string_view::npos) return false;  // pre-check!
    sv_uri_host = sv.substr(0, clb + 1);
    sv_port = sv.substr(clb + 1);
  } else {
    sv_uri_host = sv;
    std::size_t col = sv.find_last_of(constants::character::kColon);
    if (col != std::string_view::npos) {
      sv_uri_host = sv.substr(0, col);
      sv_port = sv.substr(col);
    }
  }
  if (check_uri_host(sv_uri_host) == host_type::kUnknown) return false;
  if (sv_port.empty()) return true;
  if (sv_port.front() != constants::character::kColon) return false;
  sv_port.remove_prefix(1);
  return check_port(sv_port);
}
}  // namespace martianlabs::doba::protocol::http11::checkers

#endif
