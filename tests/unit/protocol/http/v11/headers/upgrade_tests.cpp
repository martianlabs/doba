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

#include "protocol/http/v11/headers/upgrade.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::connection;
using martianlabs::doba::protocol::http::v11::parsed_token_list;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::upgrade;
}  // namespace

// +===========================================================================+
// | [>] check parses protocol offers                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("check parses protocol offers") {
  parsed_token_list parsed;
  DOBA_EXPECT(upgrade::check("HTTP/2.0, websocket, h2c/1", parsed));
  DOBA_EXPECT_EQUAL(parsed.elements.size(), 3u);
  DOBA_EXPECT_EQUAL(parsed.elements[0], "HTTP/2.0");
  DOBA_EXPECT_EQUAL(parsed.elements[1], "websocket");
  const std::string padded = "xwebsocket, h2cy";
  parsed_token_list bounded;
  DOBA_EXPECT(upgrade::check(std::string_view(padded).substr(1, 14), bounded));
  DOBA_EXPECT_EQUAL(bounded.elements.size(), 2u);
  parsed_token_list empty;
  DOBA_EXPECT(upgrade::check("", empty));
}
// +===========================================================================+
// | [>] check rejects invalid protocol offers                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid protocol offers") {
  constexpr std::string_view cases[] = {
      " ", "/1", "HTTP/", "HTTP//2", "HTTP 2", "\"websocket\"", "HTTP/2/extra",
  };
  for (const auto source : cases) {
    parsed_token_list parsed;
    DOBA_EXPECT(!upgrade::check(source, parsed));
  }
  parsed_token_list parsed;
  DOBA_EXPECT(!upgrade::check(std::string_view{"h2c\0", 4}, parsed));
}
// +===========================================================================+
// | [>] interpret applies upgrade policy                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("interpret applies upgrade policy") {
  parsed_token_list parsed;
  DOBA_EXPECT(upgrade::check("websocket, h2c", parsed));
  policies policy;
  martianlabs::doba::protocol::http::v11::connection state;
  DOBA_EXPECT_EQUAL(upgrade::interpret(parsed, state, policy),
                    verdict::kAccept);
  DOBA_EXPECT_EQUAL(state.upgrade_offer.size(), 2u);
  policy.allow_upgrade = false;
  martianlabs::doba::protocol::http::v11::connection denied;
  DOBA_EXPECT_EQUAL(upgrade::interpret(parsed, denied, policy),
                    verdict::kReject);
  DOBA_EXPECT_EQUAL(denied.upgrade_offer.size(), 2u);
}
// +===========================================================================+
// | [>] check appends ordered views from repeated fields        ( test-case ) |
// +===========================================================================+
DOBA_TEST("check appends ordered views from repeated fields") {
  const std::string first = "websocket";
  const std::string second = "custom/2";
  parsed_token_list parsed;
  DOBA_EXPECT(upgrade::check(first, parsed));
  DOBA_EXPECT(upgrade::check(second, parsed));
  DOBA_EXPECT_EQUAL(parsed.elements.size(), 2);
  DOBA_EXPECT_EQUAL(parsed.elements[0], first);
  DOBA_EXPECT_EQUAL(parsed.elements[1], second);
  DOBA_EXPECT_EQUAL(parsed.elements[0].data(), first.data());
  DOBA_EXPECT_EQUAL(parsed.elements[1].data(), second.data());
  DOBA_EXPECT(upgrade::check(",,", parsed));
  DOBA_EXPECT_EQUAL(parsed.elements.size(), 2);
}
// +===========================================================================+
// | [>] check accepts protocol version boundaries               ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts protocol version boundaries") {
  constexpr std::string_view cases[] = {
      "x/!#$%&'*+-.^_`|~",
      ",,x/1,,y,",
  };
  for (const auto source : cases) {
    martianlabs::doba::tests::unit::test_helper::set_context(source);
    parsed_token_list parsed;
    DOBA_EXPECT(upgrade::check(source, parsed));
  }
}
// +===========================================================================+
// | [>] check rejects protocol version boundaries               ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects protocol version boundaries") {
  constexpr std::string_view cases[] = {
      "x/",
      "x//1",
      "x/1/2",
      "x /1",
      "x/ 1",
      "x/1;v=2",
      "x/1,y/",
  };
  for (const auto source : cases) {
    martianlabs::doba::tests::unit::test_helper::set_context(source);
    parsed_token_list parsed;
    DOBA_EXPECT(!upgrade::check(source, parsed));
  }
}
// +===========================================================================+
// | [>] interpret applies policy to accumulated offers          ( test-case ) |
// +===========================================================================+
DOBA_TEST("interpret applies policy to accumulated offers") {
  connection state;
  policies policy;
  policy.allow_upgrade = true;
  parsed_token_list first{{"websocket"}};
  parsed_token_list second{{"custom/2"}};
  DOBA_EXPECT_EQUAL(upgrade::interpret(first, state, policy), verdict::kAccept);
  DOBA_EXPECT_EQUAL(upgrade::interpret(second, state, policy),
                    verdict::kAccept);
  DOBA_EXPECT_EQUAL(state.upgrade_offer.size(), 2);
  DOBA_EXPECT_EQUAL(state.upgrade_offer[0], "websocket");
  DOBA_EXPECT_EQUAL(state.upgrade_offer[1], "custom/2");
  policy.allow_upgrade = false;
  DOBA_EXPECT_EQUAL(upgrade::interpret({}, state, policy), verdict::kReject);
  connection empty;
  DOBA_EXPECT_EQUAL(upgrade::interpret({}, empty, policy), verdict::kAccept);
}
