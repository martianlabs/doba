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

#include "protocol/http/v11/headers/x_forwarded_for.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::connection;
using martianlabs::doba::protocol::http::v11::parsed_token_list;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::x_forwarded_for;
}  // namespace

// +===========================================================================+
// | [>] check parses forwarding nodes                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("check parses forwarding nodes") {
  parsed_token_list parsed;
  DOBA_EXPECT(x_forwarded_for::check(
      "192.0.2.1, 2001:db8::1, [2001:db8::2], unknown", parsed));
  DOBA_EXPECT_EQUAL(parsed.elements.size(), 4u);
  DOBA_EXPECT_EQUAL(parsed.elements[0], "192.0.2.1");
  DOBA_EXPECT_EQUAL(parsed.elements[3], "unknown");
  const std::string padded = "x192.0.2.1, unknowny";
  parsed_token_list bounded;
  DOBA_EXPECT(
      x_forwarded_for::check(std::string_view(padded).substr(1, 18), bounded));
  DOBA_EXPECT_EQUAL(bounded.elements.size(), 2u);
  parsed_token_list empty;
  DOBA_EXPECT(x_forwarded_for::check("", empty));
}
// +===========================================================================+
// | [>] check rejects invalid forwarding nodes                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid forwarding nodes") {
  constexpr std::string_view cases[] = {
      " ",       "UNKNOWN",      "example.com", "999.0.0.1",
      "192.0.2", "[2001:db8::1", "2001:db8::g",
  };
  for (const auto source : cases) {
    parsed_token_list parsed;
    DOBA_EXPECT(!x_forwarded_for::check(source, parsed));
  }
  parsed_token_list parsed;
  DOBA_EXPECT(
      !x_forwarded_for::check(std::string_view{"unknown\0", 8}, parsed));
}
// +===========================================================================+
// | [>] interpret limits forwarding nodes                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("interpret limits forwarding nodes") {
  parsed_token_list parsed;
  DOBA_EXPECT(x_forwarded_for::check("192.0.2.1, unknown", parsed));
  martianlabs::doba::protocol::http::v11::connection state;
  policies policy;
  DOBA_EXPECT_EQUAL(x_forwarded_for::interpret(parsed, state, policy),
                    verdict::kAccept);
  policy.max_forwarding_hops = 2;
  DOBA_EXPECT_EQUAL(x_forwarded_for::interpret(parsed, state, policy),
                    verdict::kAccept);
  policy.max_forwarding_hops = 1;
  DOBA_EXPECT_EQUAL(x_forwarded_for::interpret(parsed, state, policy),
                    verdict::kReject);
}
