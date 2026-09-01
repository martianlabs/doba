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

#include "protocol/http/v11/headers/te.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::connection;
using martianlabs::doba::protocol::http::v11::parsed_parameter_list;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::te;
}  // namespace

// +===========================================================================+
// | [>] check accepts transfer codings                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts transfer codings") {
  constexpr std::string_view cases[] = {
      "",
      "trailers",
      "TRAILERS",
      "gzip",
      "deflate;q=0.5",
      "gzip;level=1",
      "gzip; level = 1; q=0.1",
      "trailers, gzip;q=0.5",
      "trailers;q=1",
  };
  for (const auto source : cases) {
    parsed_parameter_list parsed;
    DOBA_EXPECT(te::check(source, parsed));
    const std::string padded = "x" + std::string(source) + "y";
    parsed_parameter_list bounded;
    DOBA_EXPECT(
        te::check(std::string_view(padded).substr(1, source.size()), bounded));
  }
}
// +===========================================================================+
// | [>] check rejects invalid transfer codings                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid transfer codings") {
  constexpr std::string_view cases[] = {
      " ",
      "gzip;",
      "gzip;q=",
      "gzip;q=.5",
      "gzip;q=1.001",
      "gzip;q=0.5;level=1",
      "gzip;q=0.5;q=0.4",
  };
  for (const auto source : cases) {
    parsed_parameter_list parsed;
    DOBA_EXPECT(!te::check(source, parsed));
  }
  parsed_parameter_list parsed;
  DOBA_EXPECT(!te::check(std::string_view{"gzip\0", 5}, parsed));
}
// +===========================================================================+
// | [>] interpret records codings and trailers                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("interpret records codings and trailers") {
  parsed_parameter_list parsed;
  DOBA_EXPECT(te::check("gzip;q=0.5, TRAILERS, deflate", parsed));
  martianlabs::doba::protocol::http::v11::connection state;
  policies policy;
  DOBA_EXPECT_EQUAL(te::interpret(parsed, state, policy), verdict::kAccept);
  DOBA_EXPECT(state.accepts_trailers);
  DOBA_EXPECT_EQUAL(state.te_codings.size(), 2u);
  DOBA_EXPECT_EQUAL(state.te_codings[0], "gzip");
  DOBA_EXPECT_EQUAL(state.te_codings[1], "deflate");
}
