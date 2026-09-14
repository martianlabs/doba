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

#include "common/hash_map.h"

#include <string>
#include <string_view>

#include "test_helper.h"

namespace {
using martianlabs::doba::common::hash_map;
}

// +===========================================================================+
// | [>] hash map header is self contained                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("hash map header is self contained") {
  hash_map<std::string_view, int> values;
  values.emplace("key", 1);
  DOBA_EXPECT_EQUAL(values.at("KEY"), 1);
}
// +===========================================================================+
// | [>] owned keys use complete case insensitive lookup         ( test-case ) |
// +===========================================================================+
DOBA_TEST("owned keys use complete case insensitive lookup") {
  hash_map<std::string, int> values;
  std::string key = "Content-Length";
  values.emplace(key, 7);
  key.assign("changed");
  DOBA_EXPECT(!values.emplace("CONTENT-LENGTH", 9).second);
  const auto found = values.find(std::string_view("content-length"));
  DOBA_EXPECT(found != values.end());
  DOBA_EXPECT_EQUAL(found->second, 7);
  values.emplace(std::string("x\0a", 3), 1);
  values.emplace(std::string("x\0b", 3), 2);
  DOBA_EXPECT_EQUAL(values.at(std::string("X\0A", 3)), 1);
  DOBA_EXPECT_EQUAL(values.at(std::string("X\0B", 3)), 2);
  DOBA_EXPECT(values.find("x") == values.end());
  DOBA_EXPECT_EQUAL(values.size(), 3);
}
