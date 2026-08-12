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

#include "protocol/http/v11/headers/max_forwards.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::connection;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::max_forwards;
}  // namespace

// +===========================================================================+
// | [>] check parses decimal values                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("check parses decimal values") {
  constexpr std::string_view cases[] = {"0", "1", "10", "000"};
  for (const auto source : cases) {
    std::size_t parsed = 99;
    DOBA_EXPECT(max_forwards::check(source, parsed));
    const std::string padded = "x" + std::string(source) + "y";
    DOBA_EXPECT(max_forwards::check(
        std::string_view(padded).substr(1, source.size()), parsed));
  }
  const std::string maximum =
      std::to_string(std::numeric_limits<std::size_t>::max());
  std::size_t parsed = 0;
  DOBA_EXPECT(max_forwards::check(maximum, parsed));
  DOBA_EXPECT_EQUAL(parsed, std::numeric_limits<std::size_t>::max());
}
// +===========================================================================+
// | [>] check rejects invalid decimal values                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid decimal values") {
  constexpr std::string_view cases[] = {
      "", " ", "-1", "+1", "1.0", "1x", " 1", "1 ",
  };
  for (const auto source : cases) {
    std::size_t parsed = 0;
    DOBA_EXPECT(!max_forwards::check(source, parsed));
  }
  std::string overflow =
      std::to_string(std::numeric_limits<std::size_t>::max());
  overflow += '0';
  std::size_t parsed = 0;
  DOBA_EXPECT(!max_forwards::check(overflow, parsed));
  DOBA_EXPECT(!max_forwards::check(std::string_view{"1\0", 2}, parsed));
}
// +===========================================================================+
// | [>] interpret applies forwarding limit                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("interpret applies forwarding limit") {
  martianlabs::doba::protocol::http::v11::connection state;
  policies policy;
  DOBA_EXPECT_EQUAL(max_forwards::interpret(100, state, policy),
                    verdict::kAccept);
  policy.max_forwarding_hops = 10;
  DOBA_EXPECT_EQUAL(max_forwards::interpret(10, state, policy),
                    verdict::kAccept);
  DOBA_EXPECT_EQUAL(max_forwards::interpret(11, state, policy),
                    verdict::kReject);
}
