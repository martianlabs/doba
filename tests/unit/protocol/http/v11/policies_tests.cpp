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
// | [>] defaults are bounded and allow supported features       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("defaults are bounded and allow supported features") {
  const policies value;
  DOBA_EXPECT_EQUAL(value.max_content_length, 16 * 1024 * 1024);
  DOBA_EXPECT_EQUAL(value.max_forwarding_hops, 32);
  DOBA_EXPECT_EQUAL(value.max_transfer_codings, 4);
  DOBA_EXPECT_EQUAL(policies::kMaxRequestHeadSizeInMemory, 4 * 1024);
  DOBA_EXPECT_EQUAL(policies::kMaxRequestBodySizeInMemory, 16 * 1024);
  DOBA_EXPECT_EQUAL(policies::kMaxResponseHeadSizeInMemory, 4 * 1024);
  DOBA_EXPECT_EQUAL(policies::kMaxResponseBodySizeInMemory, 16 * 1024);
  DOBA_EXPECT_EQUAL(policies::kMaxQueryParameters, 128);
  DOBA_EXPECT_EQUAL(policies::kMaxChunkedExtensionSize, 1024);
  DOBA_EXPECT_EQUAL(policies::kMaxChunkedTrailerSize, 4096);
  DOBA_EXPECT(value.allow_chunked);
  DOBA_EXPECT(value.allow_upgrade);
  DOBA_EXPECT_EQUAL(value.max_pending_requests, 32);
  DOBA_EXPECT_EQUAL(value.max_query_parameters, 128);
  DOBA_EXPECT_EQUAL(value.max_chunk_extension_size, 1024);
  DOBA_EXPECT_EQUAL(value.max_trailer_section_size, 4096);
  DOBA_EXPECT_EQUAL(value.max_request_head_size, 4 * 1024);
  DOBA_EXPECT_EQUAL(value.request_body_memory_threshold, 16 * 1024);
  DOBA_EXPECT_EQUAL(value.max_response_head_size, 4 * 1024);
  DOBA_EXPECT_EQUAL(value.response_body_inline_capacity, 16 * 1024);
}

// +===========================================================================+
// | [>] fields retain configured boundaries and switches        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("fields retain configured boundaries and switches") {
  const policies value{.max_content_length = 1,
                       .max_forwarding_hops = 2,
                       .max_transfer_codings = 3,
                       .allow_chunked = false,
                       .allow_upgrade = false,
                       .max_pending_requests = 2,
                       .max_query_parameters = 1,
                       .max_chunk_extension_size = 5,
                       .max_trailer_section_size = 6,
                       .max_request_head_size = 512,
                       .request_body_memory_threshold = 7,
                       .max_response_head_size = 512,
                       .response_body_inline_capacity = 8};
  DOBA_EXPECT_EQUAL(value.max_content_length, 1);
  DOBA_EXPECT_EQUAL(value.max_forwarding_hops, 2);
  DOBA_EXPECT_EQUAL(value.max_transfer_codings, 3);
  DOBA_EXPECT(!value.allow_chunked);
  DOBA_EXPECT(!value.allow_upgrade);
  DOBA_EXPECT_EQUAL(value.max_pending_requests, 2);
  DOBA_EXPECT_EQUAL(value.max_query_parameters, 1);
  DOBA_EXPECT_EQUAL(value.max_chunk_extension_size, 5);
  DOBA_EXPECT_EQUAL(value.max_trailer_section_size, 6);
  DOBA_EXPECT_EQUAL(value.max_request_head_size, 512);
  DOBA_EXPECT_EQUAL(value.request_body_memory_threshold, 7);
  DOBA_EXPECT_EQUAL(value.max_response_head_size, 512);
  DOBA_EXPECT_EQUAL(value.response_body_inline_capacity, 8);
}
