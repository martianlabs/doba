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
// +===========================================================================+
// | [>] check accepts protocol and comment boundaries           ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts protocol and comment boundaries") {
  constexpr std::string_view cases[] = {
      "custom/2\tproxy:001\t(a(b)c)",
      "1.1 proxy (a\\)b\\(c)",
      "1.1 proxy (a\"b)",
      "1.1 proxy (a\\\\b)",
  };
  for (const auto source : cases) {
    martianlabs::doba::tests::unit::test_helper::set_context(source);
    parsed_via_list parsed;
    DOBA_EXPECT(via::check(source, parsed));
    DOBA_EXPECT_EQUAL(parsed.elements.size(), 1);
    DOBA_EXPECT_EQUAL(parsed.elements[0].comment,
                      source.substr(source.find('(')));
  }
}
// +===========================================================================+
// | [>] check rejects protocol and comment boundaries           ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects protocol and comment boundaries") {
  constexpr std::string_view cases[] = {
      "HTTP/ proxy",
      "HTTP/1/2 proxy",
      "1.1 proxy:abc",
      "1.1 proxy (x)junk",
      "1.1 proxy (x\\",
      "1.1 proxy (x) (y)",
      "1.1 proxy (a(b)",
      "1.1 proxy (a\\))junk",
      "1.1 proxy (a,b",
      "1.1 proxy (a)\t",
      " 1.1 proxy",
      "1.1 proxy\\x",
  };
  for (const auto source : cases) {
    martianlabs::doba::tests::unit::test_helper::set_context(source);
    parsed_via_list parsed;
    DOBA_EXPECT(!via::check(source, parsed));
  }
}
// +===========================================================================+
// | [>] check rejects an IPv6 received by host                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects an IPv6 received by host") {
  // RFC 9110 S7.6.3: received-by uses a token pseudonym, not uri-host.
  parsed_via_list parsed;
  DOBA_EXPECT(!via::check("HTTP/1.1 [::1]:8080", parsed));
  DOBA_EXPECT(parsed.elements.empty());
}
// +===========================================================================+
// | [>] check keeps commas inside comments in one member        ( test-case ) |
// +===========================================================================+
DOBA_TEST("check keeps commas inside comments in one member") {
  // RFC 9110 S5.6.5: a comma is valid ctext inside a comment.
  for (const std::string_view comment : {"(a,b)", "(a(b,c),d)", "(a\\),b)"}) {
    for (bool empty_elements : {false, true}) {
      const std::string source =
          std::string(empty_elements ? ",,1.1 proxy " : "1.1 proxy ") +
          std::string(comment) +
          (empty_elements ? " \t, ,\t1.0 other,," : ", 1.0 other");
      martianlabs::doba::tests::unit::test_helper::set_context(source);
      parsed_via_list parsed;
      DOBA_EXPECT(via::check(source, parsed));
      DOBA_EXPECT_EQUAL(parsed.elements.size(), 2);
      DOBA_EXPECT_EQUAL(parsed.elements[0].comment, comment);
      DOBA_EXPECT_EQUAL(parsed.elements[1].received_by, "other");
    }
  }
}
