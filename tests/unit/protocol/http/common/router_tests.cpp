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

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "protocol/http/common/header_names.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/common/router.h"
#include "test_helper.h"

namespace {
struct request {};
struct response {
  void set_header(std::string_view name, std::string_view value) {
    headers.emplace_back(name, value);
  }
  std::vector<std::pair<std::string, std::string>> headers;
  std::string value;
};
using martianlabs::doba::protocol::http::router;
using martianlabs::doba::protocol::http::router_match_result;

auto make_request() { return std::make_shared<const request>(); }
auto make_response() { return std::make_shared<response>(); }
}  // namespace

// +===========================================================================+
// | [>] router type is neither copyable nor movable             ( test-case ) |
// +===========================================================================+
DOBA_TEST("router type is neither copyable nor movable") {
  static_assert(!std::is_copy_constructible_v<router<request, response>>);
  static_assert(!std::is_copy_assignable_v<router<request, response>>);
  static_assert(!std::is_move_constructible_v<router<request, response>>);
  static_assert(!std::is_move_assignable_v<router<request, response>>);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] empty router reports not found                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("empty router reports not found") {
  router<request, response> value;
  auto res = make_response();
  bool sent = false;
  DOBA_EXPECT_EQUAL(value.match("GET", "/", make_request(), res,
                                [&sent](const auto&) { sent = true; }),
                    router_match_result::kNotFound);
  DOBA_EXPECT(!sent);
  DOBA_EXPECT(res->headers.empty());
}
// +===========================================================================+
// | [>] static routes match exact method and path               ( test-case ) |
// +===========================================================================+
DOBA_TEST("static routes match exact method and path") {
  router<request, response> value;
  value.add("GET", "/items",
            [](std::shared_ptr<const request>, std::shared_ptr<response> res) {
              res->value = "matched";
            });
  auto res = make_response();
  std::size_t sends = 0;
  DOBA_EXPECT_EQUAL(value.match("GET", "/items", make_request(), res,
                                [&sends](const auto&) { sends++; }),
                    router_match_result::kMatched);
  DOBA_EXPECT_EQUAL(res->value, "matched");
  DOBA_EXPECT_EQUAL(sends, 1);
  DOBA_EXPECT_EQUAL(value.match("GET", "/Items", make_request(), res,
                                [&sends](const auto&) { sends++; }),
                    router_match_result::kNotFound);
  DOBA_EXPECT_EQUAL(sends, 1);
}
// +===========================================================================+
// | [>] parametrized routes parse typed segments                ( test-case ) |
// +===========================================================================+
DOBA_TEST("parametrized routes parse typed segments") {
  router<request, response> value;
  value.add("GET", "/items/:id/:enabled",
            [](std::shared_ptr<const request>, std::shared_ptr<response> res,
               int id, bool enabled) {
              res->value = std::to_string(id) + (enabled ? ":true" : ":false");
            });
  auto res = make_response();
  bool sent = false;
  DOBA_EXPECT_EQUAL(value.match("GET", "/items/42/TRUE", make_request(), res,
                                [&sent](const auto&) { sent = true; }),
                    router_match_result::kMatched);
  DOBA_EXPECT(sent);
  DOBA_EXPECT_EQUAL(res->value, "42:true");
  res = make_response();
  sent = false;
  DOBA_EXPECT_EQUAL(value.match("GET", "/items/x/true", make_request(), res,
                                [&sent](const auto&) { sent = true; }),
                    router_match_result::kNotFound);
  DOBA_EXPECT(!sent);
}
// +===========================================================================+
// | [>] wildcard routes match their prefix                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("wildcard routes match their prefix") {
  router<request, response> value;
  value.add("GET", "/assets/*",
            [](std::shared_ptr<const request>, std::shared_ptr<response> res) {
              res->value = "asset";
            });
  constexpr std::string_view matching[] = {
      "/assets/",
      "/assets/a",
      "/assets/a/b",
  };
  for (const auto path : matching) {
    auto res = make_response();
    bool sent = false;
    DOBA_EXPECT_EQUAL(value.match("GET", path, make_request(), res,
                                  [&sent](const auto&) { sent = true; }),
                      router_match_result::kMatched);
    DOBA_EXPECT(sent);
    DOBA_EXPECT_EQUAL(res->value, "asset");
  }
  auto res = make_response();
  DOBA_EXPECT_EQUAL(
      value.match("GET", "/assets", make_request(), res, [](const auto&) {}),
      router_match_result::kNotFound);
}
// +===========================================================================+
// | [>] method mismatch sets deduplicated allow header          ( test-case ) |
// +===========================================================================+
DOBA_TEST("method mismatch sets deduplicated allow header") {
  router<request, response> value;
  auto handler = [](std::shared_ptr<const request>, std::shared_ptr<response>) {
  };
  value.add("GET", "/resource", handler);
  value.add("GET", "/resource", handler);
  value.add("POST", "/resource", handler);
  auto res = make_response();
  DOBA_EXPECT_EQUAL(
      value.match("PUT", "/resource", make_request(), res, [](const auto&) {}),
      router_match_result::kMethodNotAllowed);
  DOBA_EXPECT_EQUAL(res->headers.size(), 1);
  DOBA_EXPECT_EQUAL(res->headers[0].first, "Allow");
  DOBA_EXPECT_EQUAL(res->headers[0].second, "GET, POST");
}
// +===========================================================================+
// | [>] invalid wildcard forms throw                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("invalid wildcard forms throw") {
  constexpr std::string_view invalid[] = {
      "*", "/a*", "/a/*/b", "/a/**", "/a/*x",
  };
  for (const auto route : invalid) {
    router<request, response> value;
    bool threw = false;
    try {
      value.add(
          "GET", route,
          [](std::shared_ptr<const request>, std::shared_ptr<response>) {});
    } catch (const std::invalid_argument&) {
      threw = true;
    }
    DOBA_EXPECT(threw);
  }
}
// +===========================================================================+
// | [>] wildcard handlers reject typed parameters               ( test-case ) |
// +===========================================================================+
DOBA_TEST("wildcard handlers reject typed parameters") {
  router<request, response> value;
  bool threw = false;
  try {
    value.add(
        "GET", "/items/*",
        [](std::shared_ptr<const request>, std::shared_ptr<response>, int) {});
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
}
