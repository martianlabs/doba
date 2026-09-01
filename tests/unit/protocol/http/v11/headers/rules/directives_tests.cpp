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

#include "protocol/http/v11/headers/rules/directives.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::context;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::rules::directives;
}  // namespace

// +===========================================================================+
// | [>] accepts empty and extension connection options          ( test-case ) |
// +===========================================================================+
DOBA_TEST("accepts empty and extension connection options") {
  context ctx;
  DOBA_EXPECT_EQUAL(directives::apply(ctx), verdict::kAccept);
  ctx.connection.options = {"close", "keep-alive", "X-Custom"};
  DOBA_EXPECT_EQUAL(directives::apply(ctx), verdict::kAccept);
}
// +===========================================================================+
// | [>] upgrade option requires an offered protocol             ( test-case ) |
// +===========================================================================+
DOBA_TEST("upgrade option requires an offered protocol") {
  constexpr std::string_view spellings[] = {
      "upgrade",
      "Upgrade",
      "UPGRADE",
  };
  for (const auto spelling : spellings) {
    context ctx;
    ctx.connection.options = {spelling};
    DOBA_EXPECT_EQUAL(directives::apply(ctx), verdict::kReject);
    ctx.connection.upgrade_offer = {"websocket"};
    DOBA_EXPECT_EQUAL(directives::apply(ctx), verdict::kAccept);
  }
}
// +===========================================================================+
// | [>] rejects nominated control fields case insensitively     ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects nominated control fields case insensitively") {
  constexpr std::string_view fields[] = {
      "connection",        "HOST", "Content-Length",
      "transfer-encoding", "Te",   "TRAILER",
  };
  for (const auto field : fields) {
    context ctx;
    ctx.connection.options = {field};
    DOBA_EXPECT_EQUAL(directives::apply(ctx), verdict::kReject);
  }
}
