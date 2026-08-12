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

#include "protocol/http/common/headers/range.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::headers::range;
}  // namespace

// +===========================================================================+
// | [>] check accepts valid values                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts valid values") {
  constexpr std::string_view cases[] = {
      "bytes=0-499",    "bytes=500-999",  "bytes=9500-", "bytes=-500",
      "bytes=0-0,-1",   "bytes=0-0, 2-3", "bytes= 0-1",  "bytes=0-1,",
      "bytes=0-1,,2-3", "items=1-2",      "example=abc",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(range::check(source));
    const std::string padded = "x" + std::string(source) + "y";
    DOBA_EXPECT(
        range::check(std::string_view(padded).substr(1, source.size())));
  }
}
// +===========================================================================+
// | [>] check rejects invalid values                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid values") {
  constexpr std::string_view cases[] = {
      "",        " ",           "bytes",
      "bytes=",  "bytes =0-1",  "bytes=-",
      "bytes=0", "bytes=0--1",  "bytes=0-1/2",
      "bytes=,", "example=a b",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(!range::check(source));
  }
}
// +===========================================================================+
// | [>] check handles string view boundaries                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("check handles string view boundaries") {
  DOBA_EXPECT(!range::check(std::string_view{"\0", 1}));
  DOBA_EXPECT(!range::check(std::string_view{"a\0", 2}));
  DOBA_EXPECT(!range::check(std::string_view{"a\r", 2}));
  DOBA_EXPECT(!range::check(std::string_view{"a\n", 2}));
  DOBA_EXPECT(!range::check(std::string_view{"\x80", 1}));
}
