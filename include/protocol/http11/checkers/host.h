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
  auto check_ip_v4_address = [](std::string_view sv, std::size_t& i) -> type {
    auto check = [](std::string_view sv) -> bool {
      if (sv.size() == 0x1) {
        return helpers::is_digit(sv[0]);
      } else if (sv.size() == 0x2) {
        return helpers::is_digit(sv[0]) && helpers::is_digit(sv[1]);
      } else if (sv.size() == 0x3) {
        if (!helpers::is_digit(sv[0]) || !helpers::is_digit(sv[1]) ||
            !helpers::is_digit(sv[2])) {
          return false;
        }
        if (sv[0] == constants::character::k1) {
          return true;
        } else if (sv[0] == constants::character::k2) {
          if (sv[1] >= constants::character::k0 &&
              sv[1] <= constants::character::k4) {
            return true;
          } else if ((sv[1] == constants::character::k5)) {
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
    if (!sz || !check(std::string_view(ptr, dots[0]))) {
      return type::kUnknown;
    }
    sz = dots[1] - dots[0];
    ptr = &sv.data()[dots[0] + 1];
    if (!sz || !check(std::string_view(ptr, sz - 1))) {
      return type::kUnknown;
    }
    sz = dots[2] - dots[1];
    ptr = &sv.data()[dots[1] + 1];
    if (!sz || !check(std::string_view(ptr, sz - 1))) {
      return type::kUnknown;
    }
    sz = i - dots[2];
    ptr = &sv.data()[dots[2] + 1];
    if (!sz || !check(std::string_view(ptr, sz - 1))) {
      return type::kUnknown;
    }
    return type::kIpV4Address;
  };
  auto check_ip_literal = [check_ip_v4_address](std::string_view sv,
                                                std::size_t& i) -> type {
    std::size_t pos = sv.find(constants::character::kCloseBracket);
    if (pos == std::string_view::npos) return type::kUnknown;
    std::string_view fixed(sv.begin() + 1, sv.begin() + pos);
    if (i >= fixed.size()) return type::kUnknown;
    if (fixed[i] == 'v' && ++i) {
      // [IPvFuture]..
      std::size_t j = 0;
      while (i < fixed.size()) {
        if (!helpers::is_hex_digit(fixed[i])) break;
        i++;
        j++;
      }
      if (j < 1 || j > 4 || i == fixed.size()) return type::kUnknown;
      if (fixed[i++] != constants::character::kDot) return type::kUnknown;
      while (i < fixed.size()) {
        if (!helpers::is_unreserved(fixed[i]) &&
            !helpers::is_sub_delim(fixed[i]) &&
            fixed[i] != constants::character::kColon) {
          return type::kUnknown;
        }
        i++;
      }
      i++;
      return type::kIpLiteral;
    } else {
      // [IPv6address]..
      std::vector<std::size_t> colons;
      while (i < fixed.size()) {
        if (fixed[i] == constants::character::kColon) colons.emplace_back(i);
        i++;
      }
      if (colons.size() < 2 || colons.size() > 8) return type::kUnknown;
      bool double_colon = false;
      std::size_t prev_start = colons[0], j = 1;
      while (j < colons.size()) {
        if ((colons[j] - prev_start) == 1) {
          if (double_colon) {
            return type::kUnknown;
          }
          double_colon = true;
        } else if ((colons[j] - prev_start) <= 5) {
          for (auto z = prev_start + 1; z < colons[j]; z++) {
            if (!helpers::is_hex_digit(fixed[z])) {
              return type::kUnknown;
            }
          }
        } else {
          return type::kUnknown;
        }
        prev_start = colons[j++];
      }
      std::size_t start = colons[j - 1] + 1;
      std::string_view sub(fixed.begin() + start, fixed.end());
      if (check_ip_v4_address(sub, j = 0) != type::kIpV4Address) {
        for (j = start; j < fixed.length(); j++) {
          if (!helpers::is_hex_digit(fixed[j])) {
            return type::kUnknown;
          }
        }
      }
      i = fixed.size() + 2;
      return type::kIpLiteral;
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
    probable_type = check_ip_literal(sv, ++i);
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
};
}  // namespace martianlabs::doba::protocol::http11::checkers

#endif
