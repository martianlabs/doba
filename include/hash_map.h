﻿//      _       _
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

#ifndef martianlabs_doba_hash_map_h
#define martianlabs_doba_hash_map_h

#include "hash_base.h"

namespace martianlabs::doba {
// =============================================================================
// hash_map                                                            ( class )
// -----------------------------------------------------------------------------
// This class holds for a generic hash based map.
// -----------------------------------------------------------------------------
// =============================================================================
template <typename KEty, typename VAty>
using hash_map = std::unordered_map<KEty, VAty, base_hash, base_equal>;
}  // namespace martianlabs::doba

#endif
