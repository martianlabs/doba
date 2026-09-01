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

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include "protocol/http/common/router.h"
#include "test_helper.h"

namespace {
struct request {};
struct response {
  std::string value;
};
using martianlabs::doba::protocol::http::router;
using martianlabs::doba::common::task;
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
// | [>] static routes use exact path matching                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("static routes use exact path matching") {
  router<request, response> value;
  auto handler = [](const request&, response&) {};
  value.add("GET", "/assets", handler);
  DOBA_EXPECT(static_cast<bool>(value.match("GET", "/assets")));
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/assets/a")));
}
// +===========================================================================+
// | [>] wildcard routes match their prefix                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("wildcard routes match their prefix") {
  router<request, response> value;
  bool invoked = false;
  value.add("GET", "/assets/*",
            [&invoked](const request&, response& res) {
              invoked = true;
              res.value = "wildcard";
            });
  constexpr std::string_view matching[] = {
      "/assets/",
      "/assets/a",
      "/assets/a/b",
  };
  for (const auto path : matching) {
    auto match = value.match("GET", path);
    DOBA_EXPECT(match.handler != nullptr);
    DOBA_EXPECT(match.parametrized_handler == nullptr);
  }
  DOBA_EXPECT(!invoked);
  request req;
  response res;
  value.match("GET", "/assets/a").handler->callback(req, res);
  DOBA_EXPECT(invoked);
  DOBA_EXPECT_EQUAL(res.value, "wildcard");
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/assets")));
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/Assets/a")));
}
// +===========================================================================+
// | [>] static routes take precedence over parametrized routes  ( test-case ) |
// +===========================================================================+
DOBA_TEST("static routes take precedence over parametrized routes") {
  router<request, response> value;
  value.add(
      "GET", "/items/:id",
      [](const request&, response& res, int) {
        res.value = "parametrized";
      });
  value.add(
      "GET", "/items/42",
      [](const request&, response& res) {
        res.value = "static";
      });
  auto match = value.match("GET", "/items/42");
  DOBA_EXPECT(match.handler != nullptr);
  DOBA_EXPECT(match.parametrized_handler == nullptr);
  request req;
  response res;
  match.handler->callback(req, res);
  DOBA_EXPECT_EQUAL(res.value, "static");
}
// +===========================================================================+
// | [>] route precedence ends with wildcard routes              ( test-case ) |
// +===========================================================================+
DOBA_TEST("route precedence ends with wildcard routes") {
  router<request, response> value;
  value.add(
      "GET", "/items/*",
      [](const request&, response& res) {
        res.value = "wildcard";
      });
  value.add(
      "GET", "/items/:id",
      [](const request&, response& res, int) {
        res.value = "parametrized";
      });
  value.add(
      "GET", "/items/42",
      [](const request&, response& res) {
        res.value = "static";
      });
  request req;
  response res;
  auto match = value.match("GET", "/items/42");
  match.handler->callback(req, res);
  DOBA_EXPECT_EQUAL(res.value, "static");
  match = value.match("GET", "/items/7");
  match.parametrized_handler->invoke(req, res, "/items/7");
  DOBA_EXPECT_EQUAL(res.value, "parametrized");
  match = value.match("GET", "/items/name");
  match.handler->callback(req, res);
  DOBA_EXPECT_EQUAL(res.value, "wildcard");
}
// +===========================================================================+
// | [>] parametrized routes preserve typed registration order   ( test-case ) |
// +===========================================================================+
DOBA_TEST("parametrized routes preserve typed registration order") {
  router<request, response> value;
  value.add(
      "GET", "/items/:value",
      [](const request&, response& res, int) {
        res.value = "integer";
      });
  value.add(
      "GET", "/items/:value",
      [](const request&, response& res, std::string_view) {
        res.value = "text";
      });
  request req;
  response res;
  auto integer = value.match("GET", "/items/42");
  integer.parametrized_handler->invoke(req, res, "/items/42");
  DOBA_EXPECT_EQUAL(res.value, "integer");
  auto text = value.match("GET", "/items/value");
  text.parametrized_handler->invoke(req, res, "/items/value");
  DOBA_EXPECT_EQUAL(res.value, "text");
}
// +===========================================================================+
// | [>] parametrized routes validate pattern and handler shape  ( test-case ) |
// +===========================================================================+
DOBA_TEST("parametrized routes validate pattern and handler shape") {
  router<request, response> value;
  bool threw = false;
  try {
    value.add("GET", "/items/:id", [](const request&, response&) {});
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  threw = false;
  try {
    value.add("GET", "/items",
              [](const request&, response&, int) {});
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  threw = false;
  try {
    value.add("GET", "/items/:",
              [](const request&, response&, int) {});
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
}
// +===========================================================================+
// | [>] wildcard routes validate pattern and handler shape      ( test-case ) |
// +===========================================================================+
DOBA_TEST("wildcard routes validate pattern and handler shape") {
  constexpr std::string_view invalid[] = {
      "*", "/a*", "/a/*/b", "/a/**", "/a/*x",
  };
  for (const auto route : invalid) {
    router<request, response> value;
    bool threw = false;
    try {
      value.add("GET", route, [](const request&, response&) {});
    } catch (const std::invalid_argument&) {
      threw = true;
    }
    DOBA_EXPECT(threw);
  }
  router<request, response> value;
  bool threw = false;
  try {
    value.add("GET", "/items/:id/*", [](const request&, response&) {});
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  threw = false;
  try {
    value.add("GET", "/items/*",
              [](const request&, response&, int) {});
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
}
// +===========================================================================+
// | [>] match preserves first handler                           ( test-case ) |
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
// +===========================================================================+
// | [>] allowed methods include matching parametrized routes    ( test-case ) |
// +===========================================================================+
DOBA_TEST("allowed methods include matching parametrized routes") {
  router<request, response> value;
  value.add("GET", "/items/:id", [](const request&, response&, int) {});
  value.add("POST", "/items/:name",
            [](const request&, response&, std::string_view) {});
  DOBA_EXPECT_EQUAL(value.allowed_methods("/items/42"), "GET, POST");
  DOBA_EXPECT_EQUAL(value.allowed_methods("/items/name"), "POST");
}
// +===========================================================================+
// | [>] allowed methods include matching wildcard routes        ( test-case ) |
// +===========================================================================+
DOBA_TEST("allowed methods include matching wildcard routes") {
  router<request, response> value;
  value.add("GET", "/assets/logo", [](const request&, response&) {});
  value.add("GET", "/assets/*", [](const request&, response&) {});
  value.add("POST", "/assets/:id",
            [](const request&, response&, int) {});
  value.add("DELETE", "/assets/*", [](const request&, response&) {});
  DOBA_EXPECT_EQUAL(value.allowed_methods("/assets/logo"), "GET, DELETE");
  DOBA_EXPECT_EQUAL(value.allowed_methods("/assets/42"),
                    "POST, GET, DELETE");
  DOBA_EXPECT(value.allowed_methods("/assets").empty());
}
// +===========================================================================+
// | [>] sync and async routes share one router                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("sync and async routes share one router") {
  router<request, response> value;
  value.add("GET", "/sync", [](const request&, response&) {});
  value.add("GET", "/async",
            [](std::shared_ptr<const request>) -> task<response> {
              co_return response{"async"};
            });
  value.add("GET", "/items/:id",
            [](std::shared_ptr<const request>, int) -> task<response> {
              co_return response{"parametrized"};
            });
  value.add("GET", "/assets/*",
            [](std::shared_ptr<const request>) -> task<response> {
              co_return response{"wildcard"};
            });
  auto sync = value.match("GET", "/sync");
  auto async = value.match("GET", "/async");
  auto parametrized = value.match("GET", "/items/42");
  auto wildcard = value.match("GET", "/assets/logo");
  DOBA_EXPECT(!sync.handler->is_async());
  DOBA_EXPECT(async.handler->is_async());
  DOBA_EXPECT(parametrized.parametrized_handler->is_async());
  DOBA_EXPECT(wildcard.handler->is_async());
}
