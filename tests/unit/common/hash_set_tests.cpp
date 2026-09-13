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

#include <string>
#include <string_view>

#include "common/hash_set.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::hash_set;
}  // namespace

// +===========================================================================+
// | [>] owned keys retain case insensitive identity             ( test-case ) |
// +===========================================================================+
DOBA_TEST("owned keys retain case insensitive identity") {
  hash_set<std::string> values;
  std::string key = "Content-Type";
  DOBA_EXPECT(values.emplace(key).second);
  key.assign("changed");
  DOBA_EXPECT(!values.emplace("CONTENT-TYPE").second);
  DOBA_EXPECT(values.contains(std::string_view("content-type")));
  DOBA_EXPECT_EQUAL(values.size(), 1);
  DOBA_EXPECT(!values.contains(key));
  values.emplace(std::string("x\0a", 3));
  values.emplace(std::string("x\0b", 3));
  DOBA_EXPECT(values.contains(std::string_view("X\0A", 3)));
  DOBA_EXPECT_EQUAL(values.size(), 3);
  DOBA_EXPECT(!values.contains("x"));
}
