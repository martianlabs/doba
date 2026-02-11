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
// +===========================================================================+
// |                                                                  [ host ] |
// +---------------------------------------------------------------------------+
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
static auto host_fn = [](std::string_view sv) -> bool {
  enum class type { kUnknown, kIpLiteral, kIpV4Address, kRegName };
  // +----------------+--------------------------------------------------------+
  // | h16            | 1*4HEXDIG                                              |
  // +----------------+--------------------------------------------------------+
  static auto skip_n_hextets = [](std::string_view sv, std::size_t n) -> int {
    int hextets_count = 0;
    std::size_t off = 0;
    std::size_t hex_digits = 0;
    while (off < sv.size() && hextets_count < n) {
      if (helpers::is_hex_digit(sv[off])) {
        hex_digits++;
      } else if (sv[off] == constants::character::kColon) {
        if (!hex_digits) return false;
        hextets_count++;
        hex_digits = 0;
      } else {
        return -1;
      }
      off++;
    }
    if (hex_digits) hextets_count++;
    return hextets_count == n ? off : -1;
  };
  // +----------------+--------------------------------------------------------+
  // | IPv4address    | dec-octet "." dec-octet "." dec-octet "." dec-octet    |
  // | dec-octet      | DIGIT                    ; 0-9                         |
  // |                | / %x31-39 DIGIT          ; 10-99                       |
  // |                | / "1" 2DIGIT             ; 100-199                     |
  // |                | / "2" %x30-34 DIGIT      ; 200-249                     |
  // |                | / "25" %x30-35           ; 250-255                     |
  // +----------------+--------------------------------------------------------+
  static auto check_ip_v4_address = [](std::string_view sv) -> bool {
    int dec_octets_count = 0;
    std::size_t off = 0;
    std::size_t digits = 0;
    while (off < sv.size()) {
      if (helpers::is_digit(sv[off])) {
        digits++;
      } else if (sv[off] == constants::character::kDot) {
        if (!digits) return false;
        dec_octets_count++;
        digits = 0;
      } else {
        return false;
      }
      off++;
    }
    if (digits) dec_octets_count++;
    return dec_octets_count == 4;
  };
  // +--------------+----------------------------------------------------------+
  // | IPvFuture    | "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )     |
  // +--------------+----------------------------------------------------------+
  static auto check_ip_v_future = [](std::string_view sv) -> bool {
    // 1*HEXDIG "." part..
    bool hex_digit_valid = false;
    std::size_t off = 0;
    while (off < sv.size()) {
      if (sv[off] == constants::character::kDot) {
        off++;
        break;
      }
      if (!helpers::is_hex_digit(sv[off])) return false;
      if (!hex_digit_valid) hex_digit_valid = true;
      off++;
    }
    if (off >= sv.size() || !hex_digit_valid) return false;
    // 1*( unreserved / sub-delims / ":" ) part..
    bool valid = false;
    while (off < sv.size()) {
      if (!helpers::is_unreserved(sv[off]) && !helpers::is_sub_delim(sv[off]) &&
          sv[off] != constants::character::kColon) {
        return false;
      }
      if (!valid) valid = true;
      off++;
    }
    return valid;
  };
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
  static auto check_ip_v6_address = [](std::string_view sv) -> bool {
    // first character must be either ':' or hex-digit..
    std::size_t sep = sv.find("::");
    if (sep == std::string_view::npos) {
      // +---------------------------------------------------------------------+
      // | no-compression case!                                                |
      // +---------------------------------------------------------------------+
      // | 6( h16 ":" ) ls32                                                   |
      // +---------------------------------------------------------------------+
      int offset = skip_n_hextets(sv, 6);
      if (offset == -1) return false;
      sv = sv.substr(offset);
      offset = skip_n_hextets(sv, 2);
      if (offset == -1) return check_ip_v4_address(sv);
      return true;
    } else {
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
      if (sv.find("::", sep + 2) != std::string_view::npos) return false;
    }
    return false;
  };
  // +----------------+--------------------------------------------------------+
  // | IP-literal     | "[" ( IPv6address / IPvFuture ) "]"                    |
  // +----------------+--------------------------------------------------------+
  static auto check_ip_literal = [](std::string_view sv) -> bool {
    if (sv.size() < 2) return false;  // at least two characters..
    if (sv.back() != constants::character::kCloseBracket) return false;
    sv.remove_suffix(1);
    if (sv[0] == constants::character::kVLowerCase) {
      sv.remove_prefix(1);
      return check_ip_v_future(sv);
    }
    return check_ip_v6_address(sv);
  };
  // +----------------+--------------------------------------------------------+
  // | uri-host       | IP-literal / IPv4address / reg-name                    |
  // +----------------+--------------------------------------------------------+
  static auto check_uri_host = [](std::string_view sv) -> type {
    // if the provided field-value is empty then we assume it to be regname..
    if (sv.empty()) return type::kRegName;
    // let's do a first round to set the potential type based on initial char..
    if (sv[0] == constants::character::kOpenBracket) {
      // fully correct to assume this as IPLiteral..
      return check_ip_literal(sv.substr(1)) ? type::kIpLiteral : type::kUnknown;
    } else if (helpers::is_digit(sv[0])) {
      // not fully correct to assume this as IPv4address but enough for now..
      return check_ip_v4_address(sv) ? type::kIpV4Address : type::kUnknown;
    } else {
      // fully correct to assume this as reg-name..
      return type::kUnknown;
    }
  };
  return check_uri_host(sv) != type::kUnknown;
};
}  // namespace martianlabs::doba::protocol::http11::checkers

#endif
