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

#include "protocol/http/common/headers/retry_after.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::headers::retry_after;
}  // namespace

// +===========================================================================+
// | [>] check accepts valid values                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts valid values") {
  constexpr std::string_view cases[] = {
      "0",
      "1",
      "120",
      "000",
      "999999999999999999999",
      "Sun, 06 Nov 1994 08:49:37 GMT",
      "Monday, 06-Nov-94 08:49:37 GMT",
      "Mon Nov  6 08:49:37 1994",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(retry_after::check(source));
    const std::string padded = "x" + std::string(source) + "y";
    DOBA_EXPECT(
        retry_after::check(std::string_view(padded).substr(1, source.size())));
  }
}
// +===========================================================================+
// | [>] check rejects invalid values                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid values") {
  constexpr std::string_view cases[] = {
      "",
      " ",
      "-1",
      "+1",
      "1.0",
      "120s",
      "Sun, 6 Nov 1994 08:49:37 GMT",
      "Sun, 06 Nov 1994 08:49:37 UTC",
  };
  for (const auto source : cases) {
    DOBA_EXPECT(!retry_after::check(source));
  }
}
// +===========================================================================+
// | [>] check handles string view boundaries                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("check handles string view boundaries") {
  DOBA_EXPECT(!retry_after::check(std::string_view{"\0", 1}));
  DOBA_EXPECT(!retry_after::check(std::string_view{"a\0", 2}));
  DOBA_EXPECT(!retry_after::check(std::string_view{"a\r", 2}));
  DOBA_EXPECT(!retry_after::check(std::string_view{"a\n", 2}));
  DOBA_EXPECT(!retry_after::check(std::string_view{"\x80", 1}));
  constexpr std::string_view seed = "120";
  const std::size_t positions[] = {0, seed.size() / 2, seed.size() - 1};
  for (std::size_t position : positions) {
    for (char control : {'\0', '\r', '\n', '\x7f'}) {
      std::string source(seed);
      source[position] = control;
      martianlabs::doba::tests::unit::test_helper::set_context(
          std::string(seed) + ", position " + std::to_string(position) +
          ", byte " + std::to_string(static_cast<unsigned char>(control)));
      DOBA_EXPECT(!retry_after::check(source));
    }
  }
  std::string padded(seed);
  padded.push_back('\0');
  padded += "suffix";
  DOBA_EXPECT(retry_after::check(std::string_view(padded.data(), seed.size())));
}
// +===========================================================================+
// | [>] check scans the complete decimal value                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("check scans the complete decimal value") {
  for (unsigned int byte = 0; byte <= 255; ++byte) {
    std::string source(64, '9');
    source += static_cast<char>(byte);
    martianlabs::doba::tests::unit::test_helper::set_context(
        "byte " + std::to_string(byte));
    DOBA_EXPECT_EQUAL(retry_after::check(source), byte >= '0' && byte <= '9');
  }
}
// +===========================================================================+
// | [>] check accepts date alternative boundaries               ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts date alternative boundaries") {
  constexpr std::string_view cases[] = {
      "Wednesday, 01-Jan-00 00:00:00 GMT",
      "Mon Jan  1 00:00:00 2001",
  };
  for (const auto source : cases) {
    martianlabs::doba::tests::unit::test_helper::set_context(source);
    DOBA_EXPECT(retry_after::check(source));
  }
}
// +===========================================================================+
// | [>] check rejects date alternative boundaries               ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects date alternative boundaries") {
  constexpr std::string_view cases[] = {
      "Sun, 06 Nov 1994 08:49:37 GMT,1",
      "1,Sun, 06 Nov 1994 08:49:37 GMT",
      "Sun, 06 Nov 1994 08:49:37 GM",
      "Sun Nov  6 08:49:37 199",
  };
  for (const auto source : cases) {
    martianlabs::doba::tests::unit::test_helper::set_context(source);
    DOBA_EXPECT(!retry_after::check(source));
  }
}
