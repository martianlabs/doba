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

#include <limits>
#include <string>
#include <string_view>

#include "protocol/http/v11/headers/content_length.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::connection;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::content_length;
}  // namespace

// +===========================================================================+
// | [>] check accepts decimal values                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts decimal values") {
  struct test_case {
    std::string_view source;
    std::size_t expected;
  };
  constexpr test_case cases[] = {
      {"0", 0}, {"000", 0}, {"1", 1}, {"9", 9}, {"10", 10}, {"3495", 3495},
  };
  for (const auto& test : cases) {
    std::size_t parsed = 0;
    DOBA_EXPECT(content_length::check(test.source, parsed));
    DOBA_EXPECT_EQUAL(parsed, test.expected);
    const std::string padded = "x" + std::string(test.source) + "y";
    DOBA_EXPECT(content_length::check(
        std::string_view(padded).substr(1, test.source.size()), parsed));
    DOBA_EXPECT_EQUAL(parsed, test.expected);
  }
  constexpr std::size_t maximum = std::numeric_limits<std::size_t>::max();
  const std::string source = std::to_string(maximum);
  std::size_t parsed = 0;
  DOBA_EXPECT(content_length::check(source, parsed));
  DOBA_EXPECT_EQUAL(parsed, maximum);
}
// +===========================================================================+
// | [>] check rejects invalid values                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid values") {
  constexpr std::string_view cases[] = {
      "",    "+1",  "-1", " 1", "1 ", "\t1",  "1\t",
      "1.0", "1,1", "x",  "1x", "x1", "\x80",
  };
  for (const auto source : cases) {
    std::size_t parsed = 0;
    DOBA_EXPECT(!content_length::check(source, parsed));
  }
  std::size_t parsed = 0;
  DOBA_EXPECT(!content_length::check(std::string_view{}, parsed));
  DOBA_EXPECT(!content_length::check(std::string_view{"1\0", 2}, parsed));
  std::string ovf = std::to_string(std::numeric_limits<std::size_t>::max());
  ovf += '0';
  DOBA_EXPECT(!content_length::check(ovf, parsed));
}
// +===========================================================================+
// | [>] interpret applies the configured limit                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("interpret applies the configured limit") {
  connection connection;
  policies policies;
  DOBA_EXPECT_EQUAL(content_length::interpret(0, connection, policies),
                    verdict::kAccept);
  DOBA_EXPECT_EQUAL(
      content_length::interpret(std::numeric_limits<std::size_t>::max(),
                                connection, policies),
      verdict::kAccept);
  policies.max_content_length = 10;
  DOBA_EXPECT_EQUAL(content_length::interpret(0, connection, policies),
                    verdict::kAccept);
  DOBA_EXPECT_EQUAL(content_length::interpret(9, connection, policies),
                    verdict::kAccept);
  DOBA_EXPECT_EQUAL(content_length::interpret(10, connection, policies),
                    verdict::kAccept);
  DOBA_EXPECT_EQUAL(content_length::interpret(11, connection, policies),
                    verdict::kReject);
}
