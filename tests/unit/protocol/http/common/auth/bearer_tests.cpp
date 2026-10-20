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

#include <stdexcept>
#include <string_view>
#include <vector>

#include "protocol/http/common/auth/bearer.h"
#include "protocol/http/common/auth/contracts.h"
#include "protocol/http/v11/request.h"
#include "test_helper.h"

namespace {
namespace auth = martianlabs::doba::protocol::http::auth;
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::target;

request credentials(std::vector<
                    std::pair<std::string_view, std::string_view>> headers) {
  return request::from("GET", "/", target::kOriginForm,
                       std::move(headers), {}, {}, {}, {}, {}, {}, {},
                       false, {});
}
static_assert(auth::contracts::provider<auth::bearer, request>);
}  // namespace

// +---------------------------------------------------------------------------+
// | [>] Bearer passes token unchanged to validator              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Bearer passes token unchanged to validator") {
  int calls = 0;
  auth::bearer provider("demo", [&](std::string_view token) {
    calls++;
    return token == "abc-._~+/123===";
  });
  auto req = credentials({{"authorization", "bEaReR  abc-._~+/123==="}});
  DOBA_EXPECT(provider.authenticate(req).status ==
              auth::authentication_status::kAccepted);
  DOBA_EXPECT_EQUAL(calls, 1);
}

// +---------------------------------------------------------------------------+
// | [>] Bearer omits error without credentials                  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Bearer omits error when no Bearer credentials were supplied") {
  auth::bearer provider("demo", [](auto) { return false; });
  for (const auto input : {"", "Basic Og=="}) {
    auto req = credentials({{"Authorization", input}});
    const auto result = provider.authenticate(req);
    DOBA_EXPECT(result.status == auth::authentication_status::kUnauthenticated);
    DOBA_EXPECT_EQUAL(result.challenge, "Bearer realm=\"demo\"");
  }
  auto req = credentials({});
  DOBA_EXPECT_EQUAL(provider.authenticate(req).challenge,
                    "Bearer realm=\"demo\"");
}

// +---------------------------------------------------------------------------+
// | [>] Bearer reports invalid_token on rejection               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Bearer reports invalid_token when validator rejects token") {
  auth::bearer provider("demo", [](auto) { return false; });
  auto req = credentials({{"Authorization", "Bearer token"}});
  const auto result = provider.authenticate(req);
  DOBA_EXPECT(result.status == auth::authentication_status::kUnauthenticated);
  DOBA_EXPECT_EQUAL(result.challenge,
                    "Bearer realm=\"demo\", error=\"invalid_token\"");
}

// +---------------------------------------------------------------------------+
// | [>] Bearer rejects malformed token                          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Bearer rejects malformed token without calling validator") {
  int calls = 0;
  auth::bearer provider("demo", [&](auto) { calls++; return true; });
  for (const auto input : {"Bearer", "Bearer ", "Bearer ==",
                           "Bearer a=b", "Bearer a b", "Bearer a!"}) {
    auto req = credentials({{"Authorization", input}});
    const auto result = provider.authenticate(req);
    DOBA_EXPECT(result.status == auth::authentication_status::kInvalidRequest);
    DOBA_EXPECT_EQUAL(result.challenge,
                      "Bearer realm=\"demo\", error=\"invalid_request\"");
  }
  DOBA_EXPECT_EQUAL(calls, 0);
}

// +---------------------------------------------------------------------------+
// | [>] Bearer rejects duplicate Authorization fields           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Bearer rejects duplicate Authorization fields") {
  auth::bearer provider("demo", [](auto) { return true; });
  auto req = credentials({{"Authorization", "Bearer abc"},
                           {"authorization", "Bearer def"}});
  DOBA_EXPECT(provider.authenticate(req).status ==
              auth::authentication_status::kInvalidRequest);
}

// +---------------------------------------------------------------------------+
// | [>] Bearer validates realm and callable                     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Bearer escapes realm and rejects invalid configuration") {
  auth::bearer provider("a\"b\\c", [](auto) { return false; });
  auto req = credentials({});
  DOBA_EXPECT_EQUAL(provider.authenticate(req).challenge,
                    "Bearer realm=\"a\\\"b\\\\c\"");
  bool rejected = false;
  try {
    auth::bearer invalid("demo\n", [](auto) { return false; });
  } catch (const std::invalid_argument&) {
    rejected = true;
  }
  DOBA_EXPECT(rejected);
  rejected = false;
  try {
    auth::bearer invalid("demo", {});
  } catch (const std::invalid_argument&) {
    rejected = true;
  }
  DOBA_EXPECT(rejected);
}
