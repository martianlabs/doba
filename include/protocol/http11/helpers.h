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
  static constexpr bool is_digit(std::string_view val) noexcept {
    if (val.empty()) return false;
    for (auto const& c : val) {
      if (!is_digit(static_cast<uint8_t>(c))) return false;
    }
    return true;
  }
  static constexpr bool is_digit(uint8_t val) noexcept {
    return val >= constants::character::k0 && val <= constants::character::k9;
  }
  static constexpr bool is_hex_digit(uint8_t val) noexcept {
    return is_digit(val) ||
           (val >= constants::character::kAUpperCase &&
            val <= constants::character::kFUpperCase) ||
           (val >= constants::character::kALowerCase &&
            val <= constants::character::kFLowerCase);
  }
  static constexpr bool is_alpha(uint8_t val) noexcept {
    return (val >= constants::character::kAUpperCase &&
            val <= constants::character::kZUpperCase) ||
           (val >= constants::character::kALowerCase &&
            val <= constants::character::kZLowerCase);
  }
  static constexpr bool is_token(uint8_t val) noexcept {
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
  static constexpr bool is_token(std::string_view s) noexcept {
    for (auto c : s) {
      if (!is_token(static_cast<uint8_t>(c))) return false;
    }
    return true;
  }
  static constexpr bool is_unreserved(uint8_t val) noexcept {
    return is_digit(val) || is_alpha(val) ||
           val == constants::character::kHyphen ||
           val == constants::character::kDot ||
           val == constants::character::kUnderscore ||
           val == constants::character::kTilde;
  }
  static constexpr bool is_sub_delim(uint8_t val) noexcept {
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
  static constexpr bool is_pchar(uint8_t val) noexcept {
    return is_unreserved(val) || is_sub_delim(val) ||
           val == constants::character::kColon ||
           val == constants::character::kAt;
  }
  static constexpr bool is_vchar(uint8_t val) noexcept {
    return val >= constants::character::kExclamation &&
           val <= constants::character::kTilde;
  }
  static constexpr bool is_obs_text(uint8_t val) noexcept {
    return val >= constants::character::kObsTextStart &&
           val <= constants::character::kObsTextEnd;
  }
  static constexpr bool is_ows(uint8_t val) noexcept {
    return val == constants::character::kSpace ||
           val == constants::character::kHTab;
  }
  static constexpr std::string_view ows_ltrim(std::string_view s) noexcept {
    while (!s.empty() && helpers::is_ows(static_cast<uint8_t>(s.front())))
      s.remove_prefix(1);
    return s;
  }
  static constexpr std::string_view ows_rtrim(std::string_view s) noexcept {
    while (!s.empty() && helpers::is_ows(static_cast<uint8_t>(s.back())))
      s.remove_suffix(1);
    return s;
  }
  static constexpr uint8_t tolower_ascii(uint8_t c) noexcept {
    return (c >= constants::character::kAUpperCase &&
            c <= constants::character::kZUpperCase)
               ? c + 32
               : c;
  };
  static constexpr bool are_equal(std::string_view a,
                                  std::string_view b) noexcept {
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), [](char ac, char bc) {
             return tolower_ascii(static_cast<uint8_t>(ac)) ==
                    tolower_ascii(static_cast<uint8_t>(bc));
           });
  }
};
}  // namespace martianlabs::doba::protocol::http11

#endif
