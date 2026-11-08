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

#ifndef martianlabs_doba_common_hash_base_h
#define martianlabs_doba_common_hash_base_h

#include <cstddef>
#include <string_view>

namespace martianlabs::doba::common::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] ascii_to_lower                                           ( function ) |
// +---------------------------------------------------------------------------+
// | Lowercase ASCII letters; leave other bytes alone.                         |
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
// | Hash keys without caring about ASCII case.                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct base_hash {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using is_transparent = void;
  // +=========================================================================+
  // | [>] operator()                                               ( public ) |
  // +-------------------------------------------------------------------------+
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
// | Compare keys without caring about ASCII case.                             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct base_equal {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using is_transparent = void;
  // +=========================================================================+
  // | [>] operator()                                               ( public ) |
  // +-------------------------------------------------------------------------+
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
}  // namespace martianlabs::doba::common::detail

#endif
