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
