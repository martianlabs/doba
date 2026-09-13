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

#include "protocol/http/v11/headers/x_forwarded_proto.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::connection;
using martianlabs::doba::protocol::http::v11::parsed_token_list;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::x_forwarded_proto;
}  // namespace

// +===========================================================================+
// | [>] check parses schemes                                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("check parses schemes") {
  parsed_token_list parsed;
  DOBA_EXPECT(
      x_forwarded_proto::check("http, HTTPS, custom+scheme, x.y-z", parsed));
  DOBA_EXPECT_EQUAL(parsed.elements.size(), 4u);
  DOBA_EXPECT_EQUAL(parsed.elements[0], "http");
  const std::string padded = "xhttp, httpsy";
  parsed_token_list bounded;
  DOBA_EXPECT(x_forwarded_proto::check(std::string_view(padded).substr(1, 11),
                                       bounded));
  DOBA_EXPECT_EQUAL(bounded.elements.size(), 2u);
  parsed_token_list empty;
  DOBA_EXPECT(x_forwarded_proto::check("", empty));
}
// +===========================================================================+
// | [>] check rejects invalid schemes                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid schemes") {
  constexpr std::string_view cases[] = {
      " ", "1http", "+http", "http:", "http/1", "http scheme", "\"http\"",
  };
  for (const auto source : cases) {
    parsed_token_list parsed;
    DOBA_EXPECT(!x_forwarded_proto::check(source, parsed));
  }
  parsed_token_list parsed;
  DOBA_EXPECT(!x_forwarded_proto::check(std::string_view{"http\0", 5}, parsed));
}
// +===========================================================================+
// | [>] interpret accepts parsed schemes                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("interpret accepts parsed schemes") {
  parsed_token_list parsed;
  DOBA_EXPECT(x_forwarded_proto::check("http, https", parsed));
  martianlabs::doba::protocol::http::v11::connection state;
  policies policy;
  DOBA_EXPECT_EQUAL(x_forwarded_proto::interpret(parsed, state, policy),
                    verdict::kAccept);
}
// +===========================================================================+
// | [>] check appends ordered views from repeated fields        ( test-case ) |
// +===========================================================================+
DOBA_TEST("check appends ordered views from repeated fields") {
  const std::string first = "https";
  const std::string second = "custom+secure";
  parsed_token_list parsed;
  DOBA_EXPECT(x_forwarded_proto::check(first, parsed));
  DOBA_EXPECT(x_forwarded_proto::check(second, parsed));
  DOBA_EXPECT_EQUAL(parsed.elements.size(), 2);
  DOBA_EXPECT_EQUAL(parsed.elements[0], first);
  DOBA_EXPECT_EQUAL(parsed.elements[1], second);
  DOBA_EXPECT_EQUAL(parsed.elements[0].data(), first.data());
  DOBA_EXPECT_EQUAL(parsed.elements[1].data(), second.data());
  DOBA_EXPECT(x_forwarded_proto::check(",,", parsed));
  DOBA_EXPECT_EQUAL(parsed.elements.size(), 2);
}
// +===========================================================================+
// | [>] check accepts scheme boundaries                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts scheme boundaries") {
  constexpr std::string_view cases[] = {
      "a",
      "A1+.-",
      "HTTP, HTTPS",
  };
  for (const auto source : cases) {
    martianlabs::doba::tests::unit::test_helper::set_context(source);
    parsed_token_list parsed;
    DOBA_EXPECT(x_forwarded_proto::check(source, parsed));
  }
}
// +===========================================================================+
// | [>] check rejects scheme boundaries                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects scheme boundaries") {
  constexpr std::string_view cases[] = {
      "https://",
      "http_https",
      "http;https",
  };
  for (const auto source : cases) {
    martianlabs::doba::tests::unit::test_helper::set_context(source);
    parsed_token_list parsed;
    DOBA_EXPECT(!x_forwarded_proto::check(source, parsed));
  }
}
