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

#include "protocol/http/v11/headers/rules/routing.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::context;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::rules::routing;
}  // namespace

// +===========================================================================+
// | [>] requires exactly one host field                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("requires exactly one host field") {
  context ctx;
  DOBA_EXPECT_EQUAL(routing::apply(ctx), verdict::kReject);
  ctx.has_host = true;
  DOBA_EXPECT_EQUAL(routing::apply(ctx), verdict::kAccept);
  ctx.multiple_host = true;
  DOBA_EXPECT_EQUAL(routing::apply(ctx), verdict::kReject);
}
// +===========================================================================+
// | [>] target and host names compare case insensitively        ( test-case ) |
// +===========================================================================+
DOBA_TEST("target and host names compare case insensitively") {
  context ctx;
  ctx.has_host = true;
  ctx.host.host = "Example.COM";
  ctx.has_target_authority = true;
  ctx.target_authority.host = "example.com";
  DOBA_EXPECT_EQUAL(routing::apply(ctx), verdict::kAccept);
  ctx.target_authority.host = "other.example";
  DOBA_EXPECT_EQUAL(routing::apply(ctx), verdict::kReject);
}
// +===========================================================================+
// | [>] absolute form normalizes scheme default ports           ( test-case ) |
// +===========================================================================+
DOBA_TEST("absolute form normalizes scheme default ports") {
  struct test_case {
    std::string_view scheme;
    std::string_view target_port;
    std::string_view host_port;
    verdict expected;
  };
  constexpr test_case cases[] = {
      {"http", "", "80", verdict::kAccept},
      {"HTTP", "80", "", verdict::kAccept},
      {"https", "", "443", verdict::kAccept},
      {"HTTPS", "443", "", verdict::kAccept},
      {"http", "", "8080", verdict::kReject},
      {"https", "443", "80", verdict::kReject},
      {"ftp", "", "", verdict::kAccept},
      {"ftp", "", "21", verdict::kReject},
  };
  for (const auto& test : cases) {
    context ctx;
    ctx.has_host = true;
    ctx.host.host = "example.com";
    ctx.host.port = test.host_port;
    ctx.has_target_authority = true;
    ctx.target_authority.host = "example.com";
    ctx.target_authority.port = test.target_port;
    ctx.target_authority.scheme = test.scheme;
    DOBA_EXPECT_EQUAL(routing::apply(ctx), test.expected);
  }
}
// +===========================================================================+
// | [>] authority form requires exact port equality             ( test-case ) |
// +===========================================================================+
DOBA_TEST("authority form requires exact port equality") {
  context ctx;
  ctx.has_host = true;
  ctx.host = {.host = "example.com", .port = "443", .scheme = {}};
  ctx.has_target_authority = true;
  ctx.target_authority = {
      .host = "example.com", .port = "443", .scheme = {}};
  DOBA_EXPECT_EQUAL(routing::apply(ctx), verdict::kAccept);
  ctx.target_authority.port = "";
  DOBA_EXPECT_EQUAL(routing::apply(ctx), verdict::kReject);
}
