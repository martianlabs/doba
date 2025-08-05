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

#ifndef martianlabs_doba_hash_base_h
#define martianlabs_doba_hash_base_h

#include <unordered_set>
#include <string_view>

namespace martianlabs::doba {
// =============================================================================
// base_hash                                                          ( struct )
// -----------------------------------------------------------------------------
// This struct is needed for the hash_set/hash_map helpers.
// -----------------------------------------------------------------------------
// =============================================================================
struct base_hash {
  using is_transparent = void;
  size_t operator()(std::string_view s) const noexcept {
    return std::hash<std::string_view>{}(s);
  }
};
// =============================================================================
// base_equal                                                         ( struct )
// -----------------------------------------------------------------------------
// This struct is needed for the hash_set/hash_map jelpers.
// -----------------------------------------------------------------------------
// =============================================================================
struct base_equal {
  using is_transparent = void;
  bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
    return lhs == rhs;
  }
};
}  // namespace martianlabs::doba

#endif
