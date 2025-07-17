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

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// helpers                                                            ( struct )
// -----------------------------------------------------------------------------
// This struct holds for the http 1.1 helper functions.
// -----------------------------------------------------------------------------
// =============================================================================
struct helpers {
  static inline bool is_digit(uint8_t val) {
    return val >= constants::characters::k0 && val <= constants::characters::k9;
  }
  static inline bool is_hex_digit(uint8_t val) {
    return is_digit(val) ||
           (val >= constants::characters::kAUpperCase &&
            val <= constants::characters::kFUpperCase) ||
           (val >= constants::characters::kALowerCase &&
            val <= constants::characters::kFLowerCase);
  }
  static inline bool is_alpha(uint8_t val) {
    return (val >= constants::characters::kAUpperCase &&
            val <= constants::characters::kZUpperCase) ||
           (val >= constants::characters::kALowerCase &&
            val <= constants::characters::kZLowerCase);
  }
  static inline bool is_token(uint8_t val) {
    return is_digit(val) || is_alpha(val) ||
           val == constants::characters::kExclamation ||
           val == constants::characters::kHash ||
           val == constants::characters::kDollar ||
           val == constants::characters::kPercent ||
           val == constants::characters::kAmpersand ||
           val == constants::characters::kApostrophe ||
           val == constants::characters::kAsterisk ||
           val == constants::characters::kPlus ||
           val == constants::characters::kHyphen ||
           val == constants::characters::kDot ||
           val == constants::characters::kCircumflex ||
           val == constants::characters::kUnderscore ||
           val == constants::characters::kBackTick ||
           val == constants::characters::kVerticalBar ||
           val == constants::characters::kTilde;
  }
  static inline bool is_unreserved(uint8_t val) {
    return is_digit(val) || is_alpha(val) ||
           val == constants::characters::kHyphen ||
           val == constants::characters::kDot ||
           val == constants::characters::kUnderscore ||
           val == constants::characters::kTilde;
  }
  static inline bool is_sub_delim(uint8_t val) {
    return val == constants::characters::kExclamation ||
           val == constants::characters::kDollar ||
           val == constants::characters::kAmpersand ||
           val == constants::characters::kApostrophe ||
           val == constants::characters::kLParenthesis ||
           val == constants::characters::kRParenthesis ||
           val == constants::characters::kAsterisk ||
           val == constants::characters::kPlus ||
           val == constants::characters::kComma ||
           val == constants::characters::kSemiColon ||
           val == constants::characters::kEquals;
  }
  static inline bool is_pchar(uint8_t val) {
    return is_unreserved(val) || is_sub_delim(val) ||
           val == constants::characters::kColon ||
           val == constants::characters::kAt;
  }
  static inline bool is_vchar(uint8_t val) {
    return (val >= constants::characters::kExclamation &&
            val <= constants::characters::kTilde);
  }
  static inline bool is_obs_text(uint8_t val) {
    return (val >= constants::characters::kObsTextStart &&
            val <= constants::characters::kObsTextEnd);
  }
};
}  // namespace martianlabs::doba::protocol::http11

#endif
