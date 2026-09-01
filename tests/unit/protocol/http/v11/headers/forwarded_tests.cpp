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

#include "protocol/http/v11/headers/forwarded.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::connection;
using martianlabs::doba::protocol::http::v11::parsed_forwarded_list;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::forwarded;
}  // namespace

// +===========================================================================+
// | [>] check parses forwarded elements                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("check parses forwarded elements") {
  parsed_forwarded_list parsed;
  DOBA_EXPECT(forwarded::check(
      "for=192.0.2.43;proto=http, for=\"[2001:db8::1]\";by=proxy", parsed));
  DOBA_EXPECT_EQUAL(parsed.elements.size(), 2u);
  DOBA_EXPECT_EQUAL(parsed.elements[0].pairs.size(), 2u);
  DOBA_EXPECT_EQUAL(parsed.elements[0].pairs[0].name, "for");
  DOBA_EXPECT_EQUAL(parsed.elements[0].pairs[0].value, "192.0.2.43");
  DOBA_EXPECT_EQUAL(parsed.elements[1].pairs[1].name, "by");
  const std::string padded = "xfor=host;proto=httpsy";
  parsed_forwarded_list bounded;
  DOBA_EXPECT(
      forwarded::check(std::string_view(padded).substr(1, 20), bounded));
  DOBA_EXPECT_EQUAL(bounded.elements.size(), 1u);
  parsed_forwarded_list trailing;
  DOBA_EXPECT(forwarded::check("for=host;", trailing));
}
// +===========================================================================+
// | [>] check rejects invalid forwarded values                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid forwarded values") {
  constexpr std::string_view cases[] = {
      "",           ",",          " ",
      "for",        "=value",     "for=",
      "for =value", "for= value", "for=\"unterminated",
  };
  for (const auto source : cases) {
    parsed_forwarded_list parsed;
    DOBA_EXPECT(!forwarded::check(source, parsed));
  }
  parsed_forwarded_list parsed;
  DOBA_EXPECT(!forwarded::check(std::string_view{"for=a\0", 6}, parsed));
}
// +===========================================================================+
// | [>] interpret limits forwarding hops                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("interpret limits forwarding hops") {
  parsed_forwarded_list parsed;
  DOBA_EXPECT(forwarded::check("for=a, for=b", parsed));
  martianlabs::doba::protocol::http::v11::connection state;
  policies policy;
  DOBA_EXPECT_EQUAL(forwarded::interpret(parsed, state, policy),
                    verdict::kAccept);
  policy.max_forwarding_hops = 2;
  DOBA_EXPECT_EQUAL(forwarded::interpret(parsed, state, policy),
                    verdict::kAccept);
  policy.max_forwarding_hops = 1;
  DOBA_EXPECT_EQUAL(forwarded::interpret(parsed, state, policy),
                    verdict::kReject);
}
