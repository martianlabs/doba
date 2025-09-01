//      _       _           
//   __| | ___ | |__   __ _ 
//  / _` |/ _ \| '_ \ / _` |
// | (_| | (_) | |_) | (_| |
//  \__,_|\___/|_.__/ \__,_|
// 
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef martianlabs_doba_protocol_http11_checkers_host_h
#define martianlabs_doba_protocol_http11_checkers_host_h

#include <array>
#include <string_view>

namespace martianlabs::doba::protocol::http11::checkers {
// =============================================================================
// |                                                                  [ host ] |
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
// +----------------+----------------------------------------------------------+
// =============================================================================
static auto host_check_fn = [](std::string_view sv) -> bool {
  enum class type { kUnknown, kIpLiteral, kIpV4Address, kRegName };
  auto check_ip_literal = [](std::string_view sv, std::size_t& i) -> type {
    return type::kUnknown;
  };
  auto check_ip_v4_address = [](std::string_view sv, std::size_t& i) -> type {
    auto check = [](std::string_view sv) -> bool {
      if (sv.size() == 0x1) {
        if (!helpers::is_digit(sv[0])) {
          return false;
        }
      } else if (sv.size() == 0x2) {
        if (!helpers::is_digit(sv[0]) || !helpers::is_digit(sv[1])) {
          return false;
        }
      } else if (sv.size() == 0x3) {
        if (!helpers::is_digit(sv[0]) || !helpers::is_digit(sv[1]) ||
            !helpers::is_digit(sv[2])) {
          return false;
        }
        if (sv[0] == '1') {
          return true;
        } else if (sv[0] == '2') {
          if (sv[1] >= constants::character::k0 &&
              sv[1] <= constants::character::k4) {
            return true;
          } else if ((sv[1] == '5')) {
            if (sv[2] >= constants::character::k0 &&
                sv[2] <= constants::character::k5) {
              return true;
            }
          }
        }
      }
      return false;
    };
    const std::size_t kMaxDots = 3;
    std::size_t dots_found = 0;
    std::array<std::size_t, kMaxDots> dots;
    while (i < sv.length()) {
      if (!helpers::is_digit(sv[i])) {
        if (sv[i] == constants::character::kDot) {
          if (dots_found > (kMaxDots - 1)) {
            return type::kUnknown;
          }
          dots[dots_found++] = i;
        } else if (sv[i] == constants::character::kColon) {
          break;
        } else {
          return type::kUnknown;
        }
      }
      i++;
    }
    if (dots_found != kMaxDots) {
      return type::kUnknown;
    }
    std::size_t sz = dots[0];
    const char* ptr = &sv.data()[0];
    if (sz && check(std::string_view(ptr, dots[0]))) {
      sz = dots[1] - dots[0];
      ptr = &sv.data()[dots[0] + 1];
      if (sz && check(std::string_view(ptr, sz - 1))) {
        sz = dots[2] - dots[1];
        ptr = &sv.data()[dots[1] + 1];
        if (sz && check(std::string_view(ptr, sz - 1))) {
          sz = i - dots[2];
          ptr = &sv.data()[dots[2] + 1];
          if (sz && check(std::string_view(ptr, sz - 1))) {
            return type::kIpV4Address;
          }
        }
      }
    }
    return type::kUnknown;
  };
  auto check_reg_name = [](std::string_view sv, std::size_t& i) -> type {
    while (i < sv.size()) {
      if (helpers::is_unreserved(sv[i]) || helpers::is_sub_delim(sv[i])) {
        i++;
      } else if (sv[i] == constants::character::kPercent) {
        if (i + 2 < sv.size() && helpers::is_hex_digit(sv[i + 1]) &&
            helpers::is_hex_digit(sv[i + 2])) {
          i += 3;
        } else {
          return type::kUnknown;
        }
      } else if (sv[i] == constants::character::kColon) {
        break;
      } else {
        return type::kUnknown;
      }
    }
    return type::kRegName;
  };
  if (!sv.size()) return false;
  std::size_t i = 0;
  type probable_type = type::kUnknown;
  // By following the ABNF standard is hard to differentiate between an
  // ipv4address and a reg-name. Because of this, we'll try first the
  // ipv4address way and, if not going well, then trying the reg-name one..
  if (sv[0] == constants::character::kOpenBracket) {
    probable_type = check_ip_literal(sv, i);
  } else if (helpers::is_digit(sv[0])) {
    probable_type = check_ip_v4_address(sv, i);
  }
  // if anything else didn't work let's try to decode it as reg-name..
  if (probable_type == type::kUnknown) {
    probable_type = check_reg_name(sv, i = 0);
  }
  // if everything went well then we need to check the 'port' part..
  if (probable_type != type::kUnknown) {
    if (i < sv.size()) {
      if (sv[i++] == constants::character::kColon) {
        while (i < sv.size()) {
          if (!helpers::is_digit(sv[i++])) {
            probable_type = type::kUnknown;
            break;
          }
        }
      } else {
        probable_type = type::kUnknown;
      }
    }
  }
  return probable_type != type::kUnknown;

  /*
  auto prechech_ipv4 = [](std::string_view sv, std::size_t& cur) -> auto {
    std::vector<std::size_t> out;
    std::size_t i = 0;
    while (i < sv.length()) {
      if (!helpers::is_digit(sv[i])) {
        if (sv[i] == constants::character::kDot) {
          out.push_back(i);
        } else {
          break;
        }
      }
      i++;
    }
    if (out.size() == kMaxDots) {
      bool seems_ok = true;
      std::size_t next_to_third_dot = out[kMaxDots - 1] + 1;
      while (next_to_third_dot < sv.length()) {
        if (!helpers::is_digit(sv[next_to_third_dot])) {
          break;
        }
        next_to_third_dot++;
      }
      if (seems_ok) cur = next_to_third_dot;
    }
    return out;
  };
  auto check_ipv4 = [](std::string_view sv, std::size_t cursor,
                       const std::vector<std::size_t>& dots) -> bool {
    auto chk = [](std::string_view segment) -> bool {
      int segment_value = 0;
      auto [ptr, error_code] = std::from_chars(
          segment.data(), segment.data() + segment.size(), segment_value);
      return (error_code == std::errc() &&
              (segment_value >= 0 && segment_value <= 255));
    };
    std::size_t sz = dots[0];
    if (chk(std::string_view(sv.data(), sz))) {
      if ((sz = dots[1] - dots[0]) > 0) {
        if (chk(std::string_view(&sv.data()[dots[0] + 1], --sz))) {
          if ((sz = dots[2] - dots[1]) > 0) {
            if (chk(std::string_view(&sv.data()[dots[1] + 1], --sz))) {
              if ((sz = cursor - dots[2]) > 0) {
                if (chk(std::string_view(&sv.data()[dots[2] + 1], --sz))) {
                  return true;
                }
              }
            }
          }
        }
      }
    }
    return false;
  };
  enum class type { kUnknown, kIpLiteral, kIpV4Address, kRegName };
  if (!v.length()) return true;
  type potential_host_type = type::kUnknown, host_type = type::kUnknown;
  std::size_t cursor = v.length();
  std::vector<std::size_t> dots;
  if (v[0] == constants::character::kOpenBracket) {
    // we found a potential [IP-literal]..
    potential_host_type = type::kIpLiteral;
    std::size_t i = 1, cbrackets = 0;
    while (i < v.size()) {
      if (!helpers::is_digit(v[i]) && !helpers::is_hex_digit(v[i]) &&
          !helpers::is_unreserved(v[i]) && !helpers::is_sub_delim(v[i]) &&
          v[i] != constants::character::kColon &&
          v[i] != constants::character::kDot && v[i] != 'v' &&
          (v[i] == constants::character::kCloseBracket && cbrackets++)) {
        potential_host_type = type::kUnknown;
        break;
      }
      i++;
    }
  } else if ((dots = prechech_ipv4(v, cursor)).size() == kMaxDots) {
    // we found a potential [IPv4address]..
    potential_host_type = type::kIpV4Address;
  }
  if (potential_host_type == type::kIpLiteral) {
    // check potential [IP-literal]!
    host_type = potential_host_type;
  } else if (potential_host_type == type::kIpV4Address) {
    // check potential [IPv4address]!
    if (check_ipv4(v, cursor, dots)) {
      host_type = potential_host_type;
    }
  }
  if (host_type == type::kUnknown) {
    // we may found a potential [reg-name]..
    std::size_t i = 0;
    host_type = type::kRegName;
    while (i < v.size()) {
      if (helpers::is_unreserved(v[i]) || helpers::is_sub_delim(v[i])) {
        i++;
      } else if (v[i] == constants::character::kPercent) {
        if (i + 2 < v.size() && helpers::is_hex_digit(v[i + 1]) &&
            helpers::is_hex_digit(v[i + 2])) {
          i += 3;
        } else {
          host_type = type::kUnknown;
          break;
        }
      } else if (v[i] == constants::character::kColon) {
        break;
      } else {
        host_type = type::kUnknown;
        break;
      }
    }
    cursor = i;
  }
  // check for [ ":" port ] part..
  if (host_type != type::kUnknown) {
    if (cursor < v.size()) {
      if (v[cursor++] == constants::character::kColon) {
        while (cursor < v.size()) {
          if (!helpers::is_digit(v[cursor++])) {
            host_type = type::kUnknown;
            break;
          }
        }
      } else {
        host_type = type::kUnknown;
      }
    }
  }
  return host_type != type::kUnknown;
  */

  /*
  pepe fin
  */
};
}  // namespace martianlabs::doba::protocol::http11::checkers

#endif
