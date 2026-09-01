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

#include <type_traits>

#include "protocol/http/v11/verdict.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::verdict;
}  // namespace

// +===========================================================================+
// | [>] accept and reject are distinct enum values              ( test-case ) |
// +===========================================================================+
DOBA_TEST("accept and reject are distinct enum values") {
  static_assert(std::is_enum_v<verdict>);
  DOBA_EXPECT(verdict::kAccept != verdict::kReject);
  DOBA_EXPECT_EQUAL(static_cast<int>(verdict::kAccept), 0);
  DOBA_EXPECT_EQUAL(static_cast<int>(verdict::kReject), 1);
}
