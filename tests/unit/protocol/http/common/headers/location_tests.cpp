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

#include "protocol/http/common/headers/location.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::headers::location;
}  // namespace

// +===========================================================================+
// | [>] check accepts valid values                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts valid values") {
  constexpr std::string_view cases[] = {
      "",
      "/",
      "/index.html",
      "../resource",
      "#fragment",
      "/path#fragment",
      "https://example.com/a#x",
      "?query=value",
      "urn:isbn:0451450523",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(location::check(source));
    const std::string padded = "x" + std::string(source) + "y";
    DOBA_EXPECT(
        location::check(std::string_view(padded).substr(1, source.size())));
  }
}
// +===========================================================================+
// | [>] check rejects invalid values                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid values") {
  constexpr std::string_view cases[] = {
      "a b", "%", "%2", "%GG", "http://[invalid", "a\r", "a\n",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(!location::check(source));
  }
}
// +===========================================================================+
// | [>] check handles string view boundaries                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("check handles string view boundaries") {
  DOBA_EXPECT(!location::check(std::string_view{"\0", 1}));
  DOBA_EXPECT(!location::check(std::string_view{"a\0", 2}));
  DOBA_EXPECT(!location::check(std::string_view{"a\r", 2}));
  DOBA_EXPECT(!location::check(std::string_view{"a\n", 2}));
  DOBA_EXPECT(!location::check(std::string_view{"\x80", 1}));
}
