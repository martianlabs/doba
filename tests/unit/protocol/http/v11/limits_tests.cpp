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

#include "protocol/http/v11/limits.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::limits;
}  // namespace

// +===========================================================================+
// | [>] constants expose documented operational limits          ( test-case ) |
// +===========================================================================+
DOBA_TEST("constants expose documented operational limits") {
  DOBA_EXPECT_EQUAL(limits::kDefaultMaxUriLength, 1024);
  DOBA_EXPECT_EQUAL(limits::kDefaultMaxHeaderSectionSize, 4096);
  DOBA_EXPECT_EQUAL(limits::kDefaultMaxContentLength, 10 * 1024 * 1024);
  DOBA_EXPECT_EQUAL(limits::kDefaultMaxForwardingHops, 20);
  DOBA_EXPECT_EQUAL(limits::kDefaultMaxTransferCodings, 4);
  DOBA_EXPECT_EQUAL(limits::kMaxRequestHeadSize, 5120);
  DOBA_EXPECT_EQUAL(limits::kDecodingBufferSize, limits::kMaxRequestHeadSize);
  DOBA_EXPECT_EQUAL(limits::kMaxResponseSizeInMemory, 4096);
  DOBA_EXPECT_EQUAL(limits::kMaxResponseBodySizeInMemory, 2048);
  DOBA_EXPECT_EQUAL(limits::kMaxChunkedExtensionSize, 1024);
  DOBA_EXPECT_EQUAL(limits::kMaxChunkedTrailerSize,
                    limits::kDefaultMaxHeaderSectionSize);
  DOBA_EXPECT_EQUAL(limits::kMaxQueryParameters, 128);
}
