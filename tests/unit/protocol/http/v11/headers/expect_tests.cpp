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

#include "protocol/http/v11/headers/expect.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::connection;
using martianlabs::doba::protocol::http::v11::parsed_parameter_list;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::expect;
}  // namespace

// +===========================================================================+
// | [>] check accepts expectations                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("check accepts expectations") {
  constexpr std::string_view cases[] = {
      "",
      "100-continue",
      "foo=bar",
      "foo=\"bar baz\"",
      "foo=bar; p=v",
      "foo=bar; p=\"a,b\"",
      "foo, bar=baz",
  };
  for (const auto source : cases) {
    parsed_parameter_list parsed;
    DOBA_EXPECT(expect::check(source, parsed));
    const std::string padded = "x" + std::string(source) + "y";
    parsed_parameter_list bounded;
    DOBA_EXPECT(expect::check(std::string_view(padded).substr(1, source.size()),
                              bounded));
  }
  parsed_parameter_list parsed;
  DOBA_EXPECT(expect::check("100-continue, foo=bar", parsed));
  DOBA_EXPECT_EQUAL(parsed.elements.size(), 2u);
  DOBA_EXPECT_EQUAL(parsed.elements[0], "100-continue");
}
// +===========================================================================+
// | [>] check rejects invalid expectations                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("check rejects invalid expectations") {
  constexpr std::string_view cases[] = {
      " ",        "=bar", "foo=",        "foo =bar",
      "foo= bar", "foo;", "foo=bar; p=", "foo=\"unterminated",
  };
  for (const auto source : cases) {
    parsed_parameter_list parsed;
    DOBA_EXPECT(!expect::check(source, parsed));
  }
  parsed_parameter_list parsed;
  DOBA_EXPECT(!expect::check(std::string_view{"foo\0", 4}, parsed));
}
// +===========================================================================+
// | [>] interpret recognizes only 100 continue                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("interpret recognizes only 100 continue") {
  policies policy;
  martianlabs::doba::protocol::http::v11::connection state;
  parsed_parameter_list parsed{{"100-CONTINUE"}};
  DOBA_EXPECT_EQUAL(expect::interpret(parsed, state, policy), verdict::kAccept);
  DOBA_EXPECT(state.expects_continue);
  parsed.elements.push_back("extension");
  DOBA_EXPECT_EQUAL(expect::interpret(parsed, state, policy), verdict::kReject);
}
