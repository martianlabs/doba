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

#include "common/hash_base.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::ascii_to_lower;
using martianlabs::doba::common::base_equal;
using martianlabs::doba::common::base_hash;
}  // namespace

// +===========================================================================+
// | [>] ascii folding preserves every non-uppercase byte        ( test-case ) |
// +===========================================================================+
DOBA_TEST("ascii folding preserves every non-uppercase byte") {
  for (unsigned int byte = 0; byte <= 255; ++byte) {
    const auto expected = static_cast<unsigned char>(
        byte >= 'A' && byte <= 'Z' ? byte + 32 : byte);
    DOBA_EXPECT_EQUAL(ascii_to_lower(static_cast<unsigned char>(byte)),
                      expected);
  }
}
// +===========================================================================+
// | [>] hash equality includes embedded nulls and high bytes    ( test-case ) |
// +===========================================================================+
DOBA_TEST("hash equality includes embedded nulls and high bytes") {
  base_hash hash;
  base_equal equal;
  for (unsigned int byte = 0; byte <= 255; ++byte) {
    const std::string upper = std::string("A\0", 2) + static_cast<char>(byte);
    const std::string lower = std::string("a\0", 2) + static_cast<char>(byte);
    DOBA_EXPECT(equal(upper, lower));
    DOBA_EXPECT_EQUAL(hash(upper), hash(lower));
    DOBA_EXPECT(!equal(upper, std::string_view(upper).substr(0, 2)));
    DOBA_EXPECT(!equal(upper, std::string("b\0", 2) + static_cast<char>(byte)));
  }
  DOBA_EXPECT(equal({}, {}));
  DOBA_EXPECT_EQUAL(hash({}), hash(""));
}
