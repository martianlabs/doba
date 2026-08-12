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

#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "protocol/http/common/router_types.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::route_parameters;
using martianlabs::doba::protocol::http::router_match_result;
}  // namespace

// +===========================================================================+
// | [>] route parameter alias stores non-owning pairs           ( test-case ) |
// +===========================================================================+
DOBA_TEST("route parameter alias stores non-owning name value pairs") {
  static_assert(
      std::same_as<route_parameters,
                   std::vector<std::pair<std::string_view, std::string_view>>>);
  route_parameters parameters{{"id", "42"}, {"name", "doba"}};
  DOBA_EXPECT_EQUAL(parameters.size(), 2);
  DOBA_EXPECT_EQUAL(parameters[0].first, "id");
  DOBA_EXPECT_EQUAL(parameters[0].second, "42");
  DOBA_EXPECT_EQUAL(parameters[1].first, "name");
  DOBA_EXPECT_EQUAL(parameters[1].second, "doba");
}
// +===========================================================================+
// | [>] match results are distinct                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("match results are distinct") {
  DOBA_EXPECT(router_match_result::kMatched != router_match_result::kNotFound);
  DOBA_EXPECT(router_match_result::kNotFound !=
              router_match_result::kMethodNotAllowed);
  DOBA_EXPECT(router_match_result::kMatched !=
              router_match_result::kMethodNotAllowed);
}
