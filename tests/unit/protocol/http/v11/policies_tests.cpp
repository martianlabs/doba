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

#include "protocol/http/v11/policies.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::policies;
}  // namespace

// +===========================================================================+
// | [>] defaults are unlimited and allow supported features     ( test-case ) |
// +===========================================================================+
DOBA_TEST("defaults are unlimited and allow supported features") {
  const policies value;
  DOBA_EXPECT_EQUAL(value.max_content_length, 0);
  DOBA_EXPECT_EQUAL(value.max_forwarding_hops, 0);
  DOBA_EXPECT_EQUAL(value.max_transfer_codings, 0);
  DOBA_EXPECT_EQUAL(value.max_uri_length, 0);
  DOBA_EXPECT_EQUAL(value.max_header_section_size, 0);
  DOBA_EXPECT(value.allow_chunked);
  DOBA_EXPECT(value.allow_upgrade);
}
// +===========================================================================+
// | [>] fields retain configured boundaries and switches        ( test-case ) |
// +===========================================================================+
DOBA_TEST("fields retain configured boundaries and switches") {
  const policies value{.max_content_length = 1,
                       .max_forwarding_hops = 2,
                       .max_transfer_codings = 3,
                       .max_uri_length = 4,
                       .max_header_section_size = 5,
                       .allow_chunked = false,
                       .allow_upgrade = false};
  DOBA_EXPECT_EQUAL(value.max_content_length, 1);
  DOBA_EXPECT_EQUAL(value.max_forwarding_hops, 2);
  DOBA_EXPECT_EQUAL(value.max_transfer_codings, 3);
  DOBA_EXPECT_EQUAL(value.max_uri_length, 4);
  DOBA_EXPECT_EQUAL(value.max_header_section_size, 5);
  DOBA_EXPECT(!value.allow_chunked);
  DOBA_EXPECT(!value.allow_upgrade);
}
