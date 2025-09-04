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

#ifndef martianlabs_doba_protocol_http11_helpers_h
#define martianlabs_doba_protocol_http11_helpers_h

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
  static auto get_current_date() {
    static std::shared_ptr<common::date_server> date_server_ptr;
    if (!date_server_ptr) {
      date_server_ptr = std::make_shared<common::date_server>();
      date_server_ptr->start();
    }
    return date_server_ptr->get();
  }
  static auto is_digit(std::string_view val) {
    if (!val.size()) return false;
    for (auto const& c : val) {
      if (c < constants::character::k0 || c > constants::character::k9) {
        return false;
      }
    }
    return true;
  }
  static auto is_digit(uint8_t val) {
    return val >= constants::character::k0 && val <= constants::character::k9;
  }
  static auto is_hex_digit(uint8_t val) {
    return is_digit(val) ||
           (val >= constants::character::kAUpperCase &&
            val <= constants::character::kFUpperCase) ||
           (val >= constants::character::kALowerCase &&
            val <= constants::character::kFLowerCase);
  }
  static auto is_alpha(uint8_t val) {
    return (val >= constants::character::kAUpperCase &&
            val <= constants::character::kZUpperCase) ||
           (val >= constants::character::kALowerCase &&
            val <= constants::character::kZLowerCase);
  }
  static auto is_token(uint8_t val) {
    return is_digit(val) || is_alpha(val) ||
           val == constants::character::kExclamation ||
           val == constants::character::kHash ||
           val == constants::character::kDollar ||
           val == constants::character::kPercent ||
           val == constants::character::kAmpersand ||
           val == constants::character::kApostrophe ||
           val == constants::character::kAsterisk ||
           val == constants::character::kPlus ||
           val == constants::character::kHyphen ||
           val == constants::character::kDot ||
           val == constants::character::kCircumflex ||
           val == constants::character::kUnderscore ||
           val == constants::character::kBackTick ||
           val == constants::character::kVerticalBar ||
           val == constants::character::kTilde;
  }
  static auto is_token(std::string_view s) {
    for (auto c : s) {
      if (!is_token(c)) return false;
    }
    return true;
  }
  static auto is_unreserved(uint8_t val) {
    return is_digit(val) || is_alpha(val) ||
           val == constants::character::kHyphen ||
           val == constants::character::kDot ||
           val == constants::character::kUnderscore ||
           val == constants::character::kTilde;
  }
  static auto is_sub_delim(uint8_t val) {
    return val == constants::character::kExclamation ||
           val == constants::character::kDollar ||
           val == constants::character::kAmpersand ||
           val == constants::character::kApostrophe ||
           val == constants::character::kLParenthesis ||
           val == constants::character::kRParenthesis ||
           val == constants::character::kAsterisk ||
           val == constants::character::kPlus ||
           val == constants::character::kComma ||
           val == constants::character::kSemiColon ||
           val == constants::character::kEquals;
  }
  static auto is_pchar(uint8_t val) {
    return is_unreserved(val) || is_sub_delim(val) ||
           val == constants::character::kColon ||
           val == constants::character::kAt;
  }
  static auto is_vchar(uint8_t val) {
    return val >= constants::character::kExclamation &&
           val <= constants::character::kTilde;
  }
  static auto is_obs_text(uint8_t val) {
    return val >= constants::character::kObsTextStart &&
           val <= constants::character::kObsTextEnd;
  }
  static auto is_ows(uint8_t val) {
    return val == constants::character::kSpace ||
           val == constants::character::kHTab;
  }
  static auto ows_ltrim(std::string_view s) {
    while (!s.empty() && helpers::is_ows(static_cast<unsigned char>(s.front())))
      s.remove_prefix(1);
    return s;
  }
  static auto ows_rtrim(std::string_view s) {
    while (!s.empty() && helpers::is_ows(static_cast<unsigned char>(s.back())))
      s.remove_suffix(1);
    return s;
  }
  static auto are_equal(std::string_view a, std::string_view b) {
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), [](char ac, char bc) {
             return std::tolower(static_cast<unsigned char>(ac)) ==
                    std::tolower(static_cast<unsigned char>(bc));
           });
  }
  static auto tolower_ascii(char c) {
    unsigned char u = static_cast<unsigned char>(c);
    return (u >= constants::character::kAUpperCase &&
            u <= constants::character::kZUpperCase)
               ? static_cast<unsigned char>(u + 32)
               : u;
  };
};
}  // namespace martianlabs::doba::protocol::http11

#endif
