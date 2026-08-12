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

#include "protocol/http/v11/headers/via.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::connection;
using martianlabs::doba::protocol::http::v11::parsed_via_list;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::via;
}  // namespace

// +===========================================================================+
// | [>] check parses via members                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("check parses via members") {
  parsed_via_list parsed;
  DOBA_EXPECT(via::check("1.0 fred, HTTP/1.1 example.com:8080 (edge)", parsed));
  DOBA_EXPECT_EQUAL(parsed.elements.size(), 2u);
  DOBA_EXPECT_EQUAL(parsed.elements[0].received_protocol, "1.0");
  DOBA_EXPECT_EQUAL(parsed.elements[0].received_by, "fred");
  DOBA_EXPECT(parsed.elements[0].comment.empty());
  DOBA_EXPECT_EQUAL(parsed.elements[1].received_protocol, "HTTP/1.1");
  DOBA_EXPECT_EQUAL(parsed.elements[1].received_by, "example.com:8080");
  DOBA_EXPECT_EQUAL(parsed.elements[1].comment, "(edge)");
  const std::string padded = "x1.1 proxy (edge)y";
  parsed_via_list bounded;
  DOBA_EXPECT(via::check(std::string_view(padded).substr(1, 16), bounded));
  DOBA_EXPECT_EQUAL(bounded.elements.size(), 1u);
  parsed_via_list empty;
  DOBA_EXPECT(via::check("", empty));
  parsed_via_list empty_port;
  DOBA_EXPECT(via::check("1.1 proxy:", empty_port));
}
// +===========================================================================+
// | [>] check rejects invalid via values                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid via values") {
  constexpr std::string_view cases[] = {
      " ",
      "1.1",
      "1.1 ",
      "1.1/",
      "/1.1 proxy",
      "1.1proxy",
      "1.1 proxy (unterminated",
      "1.1 proxy trailing",
  };
  for (const auto source : cases) {
    parsed_via_list parsed;
    DOBA_EXPECT(!via::check(source, parsed));
  }
  parsed_via_list parsed;
  DOBA_EXPECT(!via::check(std::string_view{"1.1 a\0", 6}, parsed));
}
// +===========================================================================+
// | [>] interpret limits via hops                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("interpret limits via hops") {
  parsed_via_list parsed;
  DOBA_EXPECT(via::check("1.0 a, 1.1 b", parsed));
  martianlabs::doba::protocol::http::v11::connection state;
  policies policy;
  DOBA_EXPECT_EQUAL(via::interpret(parsed, state, policy), verdict::kAccept);
  policy.max_forwarding_hops = 2;
  DOBA_EXPECT_EQUAL(via::interpret(parsed, state, policy), verdict::kAccept);
  policy.max_forwarding_hops = 1;
  DOBA_EXPECT_EQUAL(via::interpret(parsed, state, policy), verdict::kReject);
}
