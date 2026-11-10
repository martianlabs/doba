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

#include <iostream>
#include <memory>
#include <string>

#include "common/cache.h"
#include "common/cache_policies.h"

using namespace martianlabs::doba::common;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// | Load a value, drop it from the cache and keep reading it.                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main() {
  cache<std::string> names({.max_entries = 2, .max_cost = 64});
  unsigned loads = 0;
  auto load = [&] {
    ++loads;
    return std::make_shared<const std::string>("Mars");
  };
  auto first = names.try_get_or_load("planet", 4, load);
  auto second = names.try_get_or_load("planet", 4, load);
  if (!first || !second) return 1;
  std::cout << *second << ": " << loads << " load\n";
  names.erase("planet");
  if (names.find("planet")) return 1;
  std::cout << "After invalidation, consumer still reads: " << *first << '\n';
  // We can load a new value while readers keep the old one.
  auto replacement = names.try_get_or_load("planet", 4, load);
  if (!replacement) return 1;
  std::cout << "After reloading: " << loads << " loads, value " << *replacement
            << '\n';
  return 0;
}
