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

#include "protocol/http/common/method.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::methods;
}  // namespace

// +===========================================================================+
// | [>] constants contain standard method names                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("constants contain standard method names") {
  constexpr std::string_view actual[] = {
      methods::kGet,    methods::kHead,    methods::kPost,    methods::kPut,
      methods::kDelete, methods::kConnect, methods::kOptions, methods::kTrace,
  };
  constexpr std::string_view expected[] = {
      "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE",
  };
  static_assert(std::size(actual) == std::size(expected));
  for (std::size_t i = 0; i < std::size(actual); i++) {
    DOBA_EXPECT_EQUAL(actual[i], expected[i]);
  }
}
