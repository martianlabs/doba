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

#include "protocol/http/common/auth/basic.h"
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
static_assert(auth::contracts::provider<auth::basic, request>);
}  // namespace

// +---------------------------------------------------------------------------+
// | [>] Basic decodes credentials and splits the first colon    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Basic decodes credentials and splits the first colon") {
  int calls = 0;
  auth::basic provider("demo", [&](std::string_view user,
                                   std::string_view password) {
    calls++;
    return user == "alice" && password == "secret:tail";
  });
  auto req = credentials({{"Authorization",
                           "bAsIc  YWxpY2U6c2VjcmV0OnRhaWw="}});
  const auto result = provider.authenticate(req);
  DOBA_EXPECT(result.status == auth::authentication_status::kAccepted);
  DOBA_EXPECT(result.challenge.empty());
  DOBA_EXPECT_EQUAL(calls, 1);
}

// +---------------------------------------------------------------------------+
// | [>] Basic challenges missing wrong and rejected credentials ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Basic challenges missing wrong and rejected credentials") {
  auth::basic provider("demo", [](auto, auto) { return false; });
  const std::string_view inputs[] = {"", "Bearer token",
                                    "Basic YWxpY2U6c2VjcmV0"};
  for (const auto input : inputs) {
    auto req = credentials(input.empty()
        ? std::vector<std::pair<std::string_view, std::string_view>>{}
        : std::vector<std::pair<std::string_view, std::string_view>>{
              {"Authorization", input}});
    const auto result = provider.authenticate(req);
    DOBA_EXPECT(result.status == auth::authentication_status::kUnauthenticated);
    DOBA_EXPECT_EQUAL(result.challenge, "Basic realm=\"demo\"");
  }
}

// +---------------------------------------------------------------------------+
// | [>] Basic rejects malformed credentials                     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Basic rejects malformed credentials before calling validator") {
  int calls = 0;
  auth::basic provider("demo", [&](auto, auto) { calls++; return true; });
  const std::string_view inputs[] = {
      "Basic", "Basic ", "Basic !!!=", "Basic YQ==", "Basic YTo=",
      "Basic YTo", "Basic YTp=", "Basic YToK", "Basic YTp/",
      "Basic YTo===", "Basic Y=To", "Basic Og== trailing"};
  for (const auto input : inputs) {
    auto req = credentials({{"Authorization", input}});
    const auto result = provider.authenticate(req);
    if (input == "Basic YTo=") {
      DOBA_EXPECT(result.status == auth::authentication_status::kAccepted);
    } else {
      DOBA_EXPECT(result.status ==
                  auth::authentication_status::kUnauthenticated);
    }
  }
  DOBA_EXPECT_EQUAL(calls, 1);
}

// +---------------------------------------------------------------------------+
// | [>] Basic allows empty user and password                    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Basic allows empty user and password") {
  auth::basic provider("demo", [](auto user, auto password) {
    return user.empty() && password.empty();
  });
  auto req = credentials({{"authorization", "Basic Og=="}});
  DOBA_EXPECT(provider.authenticate(req).status ==
              auth::authentication_status::kAccepted);
}

// +---------------------------------------------------------------------------+
// | [>] Basic rejects duplicate Authorization fields            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Basic rejects duplicate Authorization fields") {
  auth::basic provider("demo", [](auto, auto) { return true; });
  auto req = credentials({{"Authorization", "Basic Og=="},
                           {"authorization", "Basic Og=="}});
  DOBA_EXPECT(provider.authenticate(req).status ==
              auth::authentication_status::kInvalidRequest);
}

// +---------------------------------------------------------------------------+
// | [>] Basic escapes realm and rejects control characters      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Basic escapes realm and rejects control characters") {
  auth::basic provider("a\"b\\c", [](auto, auto) { return false; });
  auto req = credentials({});
  DOBA_EXPECT_EQUAL(provider.authenticate(req).challenge,
                    "Basic realm=\"a\\\"b\\\\c\"");
  bool rejected = false;
  try {
    auth::basic invalid("demo\r\nInjected: yes", [](auto, auto) {
      return false;
    });
  } catch (const std::invalid_argument&) {
    rejected = true;
  }
  DOBA_EXPECT(rejected);
  rejected = false;
  try {
    auth::basic invalid("demo", {});
  } catch (const std::invalid_argument&) {
    rejected = true;
  }
  DOBA_EXPECT(rejected);
}

// +---------------------------------------------------------------------------+
// | [>] Basic preserves credential octets                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Basic preserves credential octets without conversion") {
  auth::basic provider("demo", [](auto user, auto password) {
    return user == "a" && password == std::string_view("\xff", 1);
  });
  auto req = credentials({{"Authorization", "Basic YTr/"}});
  DOBA_EXPECT(provider.authenticate(req).status ==
              auth::authentication_status::kAccepted);
}
