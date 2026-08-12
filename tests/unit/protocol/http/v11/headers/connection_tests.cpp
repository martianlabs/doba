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

#include "protocol/http/v11/headers/connection.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::parsed_token_list;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::verdict;
using header = martianlabs::doba::protocol::http::v11::headers::connection;
}  // namespace

// +===========================================================================+
// | [>] check parses connection options                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("check parses connection options") {
  parsed_token_list parsed;
  DOBA_EXPECT(header::check("keep-alive, Upgrade, close", parsed));
  DOBA_EXPECT_EQUAL(parsed.elements.size(), 3u);
  DOBA_EXPECT_EQUAL(parsed.elements[0], "keep-alive");
  DOBA_EXPECT_EQUAL(parsed.elements[1], "Upgrade");
  DOBA_EXPECT_EQUAL(parsed.elements[2], "close");
  const std::string padded = "xkeep-alive, closey";
  parsed_token_list bounded;
  DOBA_EXPECT(header::check(std::string_view(padded).substr(1, 17), bounded));
  DOBA_EXPECT_EQUAL(bounded.elements.size(), 2u);
  parsed_token_list empty;
  DOBA_EXPECT(header::check("", empty));
  DOBA_EXPECT(empty.elements.empty());
}
// +===========================================================================+
// | [>] check rejects invalid connection options                ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid connection options") {
  constexpr std::string_view cases[] = {
      " ", "keep alive", "\"close\"", "close/upgrade", "close;upgrade",
  };
  for (const auto source : cases) {
    parsed_token_list parsed;
    DOBA_EXPECT(!header::check(source, parsed));
  }
  parsed_token_list parsed;
  DOBA_EXPECT(!header::check(std::string_view{"close\0", 6}, parsed));
}
// +===========================================================================+
// | [>] interpret applies all connection options                ( test-case ) |
// +===========================================================================+
DOBA_TEST("interpret applies all connection options") {
  parsed_token_list parsed;
  DOBA_EXPECT(header::check("keep-alive, CLOSE, Upgrade", parsed));
  martianlabs::doba::protocol::http::v11::connection state;
  policies policy;
  DOBA_EXPECT_EQUAL(header::interpret(parsed, state, policy), verdict::kAccept);
  DOBA_EXPECT_EQUAL(state.options.size(), 3u);
  DOBA_EXPECT(state.close_requested);
  DOBA_EXPECT(!state.persistent);
}
