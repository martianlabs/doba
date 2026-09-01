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
#include <string>
#include <string_view>

#include "protocol/http/common/router_handler_parametrized.h"
#include "test_helper.h"

namespace {
struct request {};
struct response {
  std::string value;
};
using martianlabs::doba::protocol::http::make_router_handler_parametrized;
}  // namespace

// +===========================================================================+
// | [>] matches routes with typed parameters                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("matches routes with typed parameters") {
  auto handler = make_router_handler_parametrized<
      request, response, std::uint64_t, bool, double, std::string_view>(
      "/items/:id/:enabled/:score/:name",
      [](const request&, response&, std::uint64_t, bool, double,
         std::string_view) {});
  DOBA_EXPECT(handler.matches("/items/42/TRUE/1.5/doba"));
  DOBA_EXPECT(!handler.matches("/items/x/true/1.5/doba"));
  DOBA_EXPECT(!handler.matches("/items/42/yes/1.5/doba"));
  DOBA_EXPECT(!handler.matches("/items/42/true/score/doba"));
}
// +===========================================================================+
// | [>] matching requires the complete route shape              ( test-case ) |
// +===========================================================================+
DOBA_TEST("matching requires the complete route shape") {
  auto handler = make_router_handler_parametrized<request, response, int>(
      "/items/:id", [](const request&, response&, int) {});
  DOBA_EXPECT(handler.matches("/items/42"));
  DOBA_EXPECT(!handler.matches("/items/"));
  DOBA_EXPECT(!handler.matches("/items/42/"));
  DOBA_EXPECT(!handler.matches("/items/42/details"));
}
// +===========================================================================+
// | [>] handler records synchronous or asynchronous execution   ( test-case ) |
// +===========================================================================+
DOBA_TEST("handler records synchronous or asynchronous execution") {
  auto synchronous = make_router_handler_parametrized<request, response, int>(
      "/items/:id", [](const request&, response&, int) {});
  auto asynchronous =
      make_router_handler_parametrized<request, response, int>(
          "/items/:id", [](const request&, response&, int) {}, true);
  DOBA_EXPECT(!synchronous.asynchronous());
  DOBA_EXPECT(asynchronous.asynchronous());
}
// +===========================================================================+
// | [>] invoke passes parsed values to the callback             ( test-case ) |
// +===========================================================================+
DOBA_TEST("invoke passes parsed values to the callback") {
  bool invoked = false;
  auto handler = make_router_handler_parametrized<
      request, response, std::uint64_t, bool, double, std::string>(
      "/items/:id/:enabled/:score/:name",
      [&invoked](const request&, response& res, std::uint64_t id, bool enabled,
                 double score, const std::string& name) {
        invoked = id == 42 && enabled && score == 1.5 && name == "doba";
        res.value = name;
      });
  request req;
  response res;
  handler.invoke(req, res, "/items/42/true/1.5/doba");
  DOBA_EXPECT(invoked);
  DOBA_EXPECT_EQUAL(res.value, "doba");
}
// +===========================================================================+
// | [>] invoke ignores paths with invalid parameters            ( test-case ) |
// +===========================================================================+
DOBA_TEST("invoke ignores paths with invalid parameters") {
  bool invoked = false;
  auto handler = make_router_handler_parametrized<request, response, int>(
      "/items/:id",
      [&invoked](const request&, response&, int) { invoked = true; });
  request req;
  response res;
  handler.invoke(req, res, "/items/value");
  DOBA_EXPECT(!invoked);
}
