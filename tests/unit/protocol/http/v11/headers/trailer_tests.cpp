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

#include "protocol/http/v11/headers/trailer.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::connection;
using martianlabs::doba::protocol::http::v11::parsed_token_list;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::trailer;
}  // namespace

// +===========================================================================+
// | [>] check parses trailer names                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("check parses trailer names") {
  parsed_token_list parsed;
  DOBA_EXPECT(trailer::check("ETag, Digest, X-Checksum", parsed));
  DOBA_EXPECT_EQUAL(parsed.elements.size(), 3u);
  DOBA_EXPECT_EQUAL(parsed.elements[0], "ETag");
  const std::string padded = "xETag, Digesty";
  parsed_token_list bounded;
  DOBA_EXPECT(trailer::check(std::string_view(padded).substr(1, 12), bounded));
  DOBA_EXPECT_EQUAL(bounded.elements.size(), 2u);
  parsed_token_list empty;
  DOBA_EXPECT(trailer::check("", empty));
  DOBA_EXPECT(empty.elements.empty());
}
// +===========================================================================+
// | [>] check rejects invalid trailer names                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid trailer names") {
  constexpr std::string_view cases[] = {
      " ", "Content Length", "\"ETag\"", "ETag:", "ETag/Digest",
  };
  for (const auto source : cases) {
    parsed_token_list parsed;
    DOBA_EXPECT(!trailer::check(source, parsed));
  }
  parsed_token_list parsed;
  DOBA_EXPECT(!trailer::check(std::string_view{"ETag\0", 5}, parsed));
}
// +===========================================================================+
// | [>] interpret records trailer names                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("interpret records trailer names") {
  parsed_token_list parsed;
  DOBA_EXPECT(trailer::check("ETag, Digest", parsed));
  martianlabs::doba::protocol::http::v11::connection state;
  policies policy;
  DOBA_EXPECT_EQUAL(trailer::interpret(parsed, state, policy),
                    verdict::kAccept);
  DOBA_EXPECT_EQUAL(state.trailer_names.size(), 2u);
  DOBA_EXPECT_EQUAL(state.trailer_names[1], "Digest");
}
