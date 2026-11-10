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

#include <cstddef>

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] cache_policies                                             ( struct ) |
// +---------------------------------------------------------------------------+
// | How much we keep and how many loads we run at once.                       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct cache_policies {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // How many entries we keep. Set to 0 to turn caching off.
  std::size_t max_entries = 1024;
  // Cost limit for stored entries. Set to 0 to turn caching off.
  // Loads still running and values kept by responses don't count.
  // Keys and allocation overhead aren't included either.
  std::size_t max_cost = 512 * 1024 * 1024;
  // How many loads can run at once. Set to 0 to turn caching off.
  // Even discarded loads keep their slot until they finish.
  std::size_t max_loads = 16;
};
}  // namespace martianlabs::doba::common

#endif
