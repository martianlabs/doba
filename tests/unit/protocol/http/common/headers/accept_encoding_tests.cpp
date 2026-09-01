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

#include "protocol/http/common/headers/accept_encoding.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::headers::accept_encoding;
}  // namespace

// +===========================================================================+
// | [>] check accepts valid values                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts valid values") {
  constexpr std::string_view cases[] = {
      "",          ",",
      "gzip",      "identity",
      "*",         "gzip, deflate, br",
      "gzip;q=0",  "br;q=0.875",
      "*;q=1.000", ", gzip, , identity;q=0,",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(accept_encoding::check(source));
    const std::string padded = "x" + std::string(source) + "y";
    DOBA_EXPECT(accept_encoding::check(
        std::string_view(padded).substr(1, source.size())));
  }
}
// +===========================================================================+
// | [>] check rejects invalid values                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid values") {
  constexpr std::string_view cases[] = {
      " ",
      "g zip",
      "gzip;",
      "gzip;q=",
      "gzip;q=.5",
      "gzip;q=1.1",
      "gzip;q=0.0000",
      "gzip;q=\"1\"",
      "gzip;level=1",
      "\"gzip\"",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(!accept_encoding::check(source));
  }
}
// +===========================================================================+
// | [>] check handles string view boundaries                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("check handles string view boundaries") {
  DOBA_EXPECT(!accept_encoding::check(std::string_view{"\\0", 1}));
  DOBA_EXPECT(!accept_encoding::check(std::string_view{"a\\0", 2}));
  DOBA_EXPECT(!accept_encoding::check(std::string_view{"a\\r", 2}));
  DOBA_EXPECT(!accept_encoding::check(std::string_view{"a\\n", 2}));
  DOBA_EXPECT(!accept_encoding::check(std::string_view{"\\x80", 1}));
}
