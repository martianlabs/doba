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

#ifndef martianlabs_doba_common_cache_statistics_h
#define martianlabs_doba_common_cache_statistics_h

#include <cstddef>

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] cache_statistics                                           ( struct ) |
// +---------------------------------------------------------------------------+
// | Resource counters captured by the generic cache statistics operation.     |
// | Costs use the same caller-defined units as the admission policies.        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct cache_statistics {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Resident entries, including expired entries not yet removed.
  std::size_t entries = 0;
  // Declared cost currently owned by resident entries.
  std::size_t resident_cost = 0;
  // Declared cost of load reservations and all retained values, including
  // evicted values still pinned by consumers.
  std::size_t live_cost = 0;
  // Loaders still running, including invalidated loads awaiting completion.
  std::size_t loads = 0;
  // Callers currently waiting for an existing load of the same key.
  std::size_t waiters = 0;
};
}  // namespace martianlabs::doba::common

#endif
