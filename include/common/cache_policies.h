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
// Copyright 2026 martianLabs
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

#ifndef martianlabs_doba_common_cache_policies_h
#define martianlabs_doba_common_cache_policies_h

#include <chrono>
#include <cstddef>

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] cache_policies                                             ( struct ) |
// +---------------------------------------------------------------------------+
// | Admission limits and expiration settings for the generic cache.           |
// | Costs use caller-defined units; they do not measure allocator overhead.   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct cache_policies {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Maximum resident entries (0 disables admission).
  std::size_t max_entries = 1024;
  // Maximum live declared cost, including reservations and retained values.
  // Values pinned by consumers remain charged after eviction; 0 disables
  // admission.
  std::size_t max_cost = 512 * 1024 * 1024;
  // Maximum concurrent loaders, including invalidated loads still running.
  // A value of 0 disables admission.
  std::size_t max_loads = 16;
  // Entry lifetime from successful loading, in milliseconds.
  // A value of 0 disables expiration; negative values are rejected by cache.
  std::chrono::milliseconds ttl{1000};
};
}  // namespace martianlabs::doba::common

#endif
