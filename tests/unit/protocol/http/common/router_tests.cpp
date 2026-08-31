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
#include <type_traits>

#include "protocol/http/common/router.h"
#include "test_helper.h"

namespace {
struct request {};
struct response {
  std::string value;
};
using martianlabs::doba::protocol::http::router;
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
// | [>] match returns handler without executing it              ( test-case ) |
// +===========================================================================+
DOBA_TEST("match returns handler without executing it") {
  router<request, response> value;
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/items")));
  bool invoked = false;
  value.add("GET", "/items",
            [&invoked](const request&, response& res) {
              invoked = true;
              res.value = "matched";
            });
  auto match = value.match("GET", "/items");
  DOBA_EXPECT(static_cast<bool>(match));
  DOBA_EXPECT(!invoked);
  request req;
  response res;
  match.handler->callback(req, res);
  DOBA_EXPECT(invoked);
  DOBA_EXPECT_EQUAL(res.value, "matched");
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/Items")));
}
// +===========================================================================+
// | [>] routes use exact path matching                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("routes use exact path matching") {
  router<request, response> value;
  auto handler = [](const request&, response&) {};
  value.add("GET", "/assets/*", handler);
  value.add("GET", "/items/:id", handler);
  DOBA_EXPECT(static_cast<bool>(value.match("GET", "/assets/*")));
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/assets/a")));
  DOBA_EXPECT(static_cast<bool>(value.match("GET", "/items/:id")));
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/items/42")));
}
// +===========================================================================+
// | [>] match preserves first handler                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("match preserves first handler") {
  router<request, response> value;
  value.add(
      "GET", "/resource",
      [](const request&, response& res) {
        res.value = "first";
      });
  value.add(
      "GET", "/resource",
      [](const request&, response& res) {
        res.value = "second";
      });
  value.add(
      "POST", "/resource",
      [](const request&, response&) {});
  auto get = value.match("GET", "/resource");
  auto post = value.match("POST", "/resource");
  request req;
  response res;
  get.handler->callback(req, res);
  DOBA_EXPECT_EQUAL(res.value, "first");
  DOBA_EXPECT(static_cast<bool>(post));
  DOBA_EXPECT_EQUAL(value.allowed_methods("/resource"), "GET, POST");
  DOBA_EXPECT(value.allowed_methods("/missing").empty());
}
