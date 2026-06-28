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

#ifndef martianlabs_doba_common_hash_base_h
#define martianlabs_doba_common_hash_base_h

#include <unordered_set>
#include <string_view>

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] ascii_to_lower                                           ( function ) |
// +---------------------------------------------------------------------------+
// | This function converts an ASCII uppercase character to lowercase.         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
static constexpr unsigned char ascii_to_lower(unsigned char value) noexcept {
  if (value >= 'A' && value <= 'Z') {
    return static_cast<unsigned char>(value + ('a' - 'A'));
  }
  return value;
}
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] base_hash                                                  ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct is needed for the hash_set/hash_map helpers.                  |
// | It performs an ASCII case-insensitive hash.                               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct base_hash {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +=========================================================================+
  using is_transparent = void;
  // +=========================================================================+
  // | [>] operator()                                               ( public ) |
  // +=========================================================================+
  std::size_t operator()(std::string_view value) const noexcept {
    std::size_t hash;
    if constexpr (sizeof(std::size_t) == 8) {
      hash = static_cast<std::size_t>(14695981039346656037ull);
    } else {
      hash = static_cast<std::size_t>(2166136261u);
    }
    for (const char character : value) {
      hash ^= static_cast<std::size_t>(
          ascii_to_lower(static_cast<unsigned char>(character)));
      if constexpr (sizeof(std::size_t) == 8) {
        hash *= static_cast<std::size_t>(1099511628211ull);
      } else {
        hash *= static_cast<std::size_t>(16777619u);
      }
    }
    return hash;
  }
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] base_equal                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct is needed for the hash_set/hash_map helpers.                  |
// | It performs an ASCII case-insensitive comparison.                         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct base_equal {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +=========================================================================+
  using is_transparent = void;
  // +=========================================================================+
  // | [>] operator()                                               ( public ) |
  // +=========================================================================+
  bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
    if (lhs.size() != rhs.size()) return false;
    for (std::size_t index = 0; index < lhs.size(); ++index) {
      const auto left = ascii_to_lower(static_cast<unsigned char>(lhs[index]));
      const auto right = ascii_to_lower(static_cast<unsigned char>(rhs[index]));
      if (left != right) return false;
    }
    return true;
  }
};
}  // namespace martianlabs::doba::common

#endif
