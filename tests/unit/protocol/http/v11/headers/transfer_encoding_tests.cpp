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

#include "protocol/http/v11/headers/transfer_encoding.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::connection;
using martianlabs::doba::protocol::http::v11::parsed_parameter_list;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::transfer_encoding;
}  // namespace

// +===========================================================================+
// | [>] check accepts transfer codings                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts transfer codings") {
  constexpr std::string_view cases[] = {
      "",
      "chunked",
      "gzip",
      "compress, chunked",
      "x-coding;level=1",
      "x-coding; level = \"high\"",
  };
  for (const auto source : cases) {
    parsed_parameter_list parsed;
    DOBA_EXPECT(transfer_encoding::check(source, parsed));
    const std::string padded = "x" + std::string(source) + "y";
    parsed_parameter_list bounded;
    DOBA_EXPECT(transfer_encoding::check(
        std::string_view(padded).substr(1, source.size()), bounded));
  }
}
// +===========================================================================+
// | [>] check rejects invalid transfer codings                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid transfer codings") {
  constexpr std::string_view cases[] = {
      " ",           "gzip;",          "gzip;=1",    "gzip;level",
      "gzip;level=", "gzip; level = ", "gzip/\"x\"",
  };
  for (const auto source : cases) {
    parsed_parameter_list parsed;
    DOBA_EXPECT(!transfer_encoding::check(source, parsed));
  }
  parsed_parameter_list parsed;
  DOBA_EXPECT(!transfer_encoding::check(std::string_view{"gzip\0", 5}, parsed));
}
// +===========================================================================+
// | [>] interpret applies transfer policies                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("interpret applies transfer policies") {
  parsed_parameter_list parsed;
  DOBA_EXPECT(transfer_encoding::check("gzip, CHUNKED", parsed));
  martianlabs::doba::protocol::http::v11::connection state;
  policies policy;
  DOBA_EXPECT_EQUAL(transfer_encoding::interpret(parsed, state, policy),
                    verdict::kAccept);
  DOBA_EXPECT(state.chunked);
  DOBA_EXPECT_EQUAL(state.transfer_codings.size(), 2u);
  policy.allow_chunked = false;
  martianlabs::doba::protocol::http::v11::connection denied;
  DOBA_EXPECT_EQUAL(transfer_encoding::interpret(parsed, denied, policy),
                    verdict::kReject);
  policy.allow_chunked = true;
  policy.max_transfer_codings = 1;
  martianlabs::doba::protocol::http::v11::connection limited;
  DOBA_EXPECT_EQUAL(transfer_encoding::interpret(parsed, limited, policy),
                    verdict::kReject);
  DOBA_EXPECT(limited.transfer_codings.empty());
}
