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

#include "protocol/http/v11/headers/rules/framing.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::context;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::rules::framing;
}  // namespace

// +===========================================================================+
// | [>] accepts unambiguous framing combinations                ( test-case ) |
// +===========================================================================+
DOBA_TEST("accepts unambiguous framing combinations") {
  context ctx;
  DOBA_EXPECT_EQUAL(framing::apply(ctx), verdict::kAccept);
  ctx.has_content_length = true;
  DOBA_EXPECT_EQUAL(framing::apply(ctx), verdict::kAccept);
  ctx = context{};
  ctx.has_transfer_encoding = true;
  ctx.connection.transfer_codings = {"gzip", "CHUNKED"};
  DOBA_EXPECT_EQUAL(framing::apply(ctx), verdict::kAccept);
}
// +===========================================================================+
// | [>] rejects multiple content length fields                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects multiple content length fields") {
  context ctx;
  ctx.multiple_content_length = true;
  DOBA_EXPECT_EQUAL(framing::apply(ctx), verdict::kReject);
  ctx.has_content_length = true;
  DOBA_EXPECT_EQUAL(framing::apply(ctx), verdict::kReject);
}
// +===========================================================================+
// | [>] rejects transfer encoding with content length           ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects simultaneous transfer encoding and content length") {
  context ctx;
  ctx.has_content_length = true;
  ctx.has_transfer_encoding = true;
  ctx.connection.transfer_codings = {"chunked"};
  DOBA_EXPECT_EQUAL(framing::apply(ctx), verdict::kReject);
}
// +===========================================================================+
// | [>] rejects chunked unless it is the final coding           ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects chunked unless it is the final coding") {
  constexpr std::string_view spellings[] = {
      "chunked",
      "Chunked",
      "CHUNKED",
  };
  for (const auto spelling : spellings) {
    context ctx;
    ctx.has_transfer_encoding = true;
    ctx.connection.transfer_codings = {spelling, "gzip"};
    DOBA_EXPECT_EQUAL(framing::apply(ctx), verdict::kReject);
    ctx.connection.transfer_codings = {"gzip", spelling};
    DOBA_EXPECT_EQUAL(framing::apply(ctx), verdict::kAccept);
  }
  context ctx;
  ctx.has_transfer_encoding = true;
  ctx.connection.transfer_codings = {"gzip"};
  DOBA_EXPECT_EQUAL(framing::apply(ctx), verdict::kReject);
  ctx.connection.transfer_codings.clear();
  DOBA_EXPECT_EQUAL(framing::apply(ctx), verdict::kReject);
}
