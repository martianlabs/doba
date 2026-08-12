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
#include <type_traits>
#include <utility>

#include "protocol/http/common/query_parameter.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::query_parameter;
using martianlabs::doba::protocol::http::query_parameter_view;
}  // namespace

// +===========================================================================+
// | [>] aliases preserve owning and view semantics              ( test-case ) |
// +===========================================================================+
DOBA_TEST("aliases preserve owning and view semantics") {
  static_assert(
      std::same_as<query_parameter, std::pair<std::string, std::string>>);
  static_assert(std::same_as<query_parameter_view,
                             std::pair<std::string_view, std::string_view>>);
  query_parameter value{"name", "value"};
  query_parameter_view view{value.first, value.second};
  DOBA_EXPECT_EQUAL(view.first, "name");
  DOBA_EXPECT_EQUAL(view.second, "value");
  query_parameter empty;
  DOBA_EXPECT(empty.first.empty());
  DOBA_EXPECT(empty.second.empty());
}
