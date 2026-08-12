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
#include <limits>
#include <memory>
#include <string>
#include <type_traits>

#include "protocol/http/common/helpers.h"
#include "protocol/http/common/router_handler_parametrized.h"
#include "test_helper.h"

namespace {
struct request {};
struct response {};
using martianlabs::doba::protocol::http::route_parameters;
using martianlabs::doba::protocol::http::router_handler_lambda;
using martianlabs::doba::protocol::http::router_handler_parametrized;
using martianlabs::doba::protocol::http::router_handler_signature;
}  // namespace

// +===========================================================================+
// | [>] signature exposes handler contract                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("signature exposes handler contract") {
  auto callback = [](std::shared_ptr<const request>, std::shared_ptr<response>,
                     int, std::string) {};
  using signature =
      router_handler_signature<decltype(&decltype(callback)::operator())>;
  static_assert(router_handler_lambda<decltype(callback)>);
  static_assert(std::same_as<typename signature::return_type, void>);
  static_assert(std::same_as<typename signature::request_type,
                             std::shared_ptr<const request>>);
  static_assert(std::same_as<typename signature::response_type,
                             std::shared_ptr<response>>);
  static_assert(signature::parameter_count == 2);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] handler parses supported parameter types                ( test-case ) |
// +===========================================================================+
DOBA_TEST("handler parses supported parameter types") {
  bool invoked = false;
  router_handler_parametrized<request, response, std::string, bool, int,
                              unsigned int, double>
      handler([&invoked](std::shared_ptr<const request> req,
                         std::shared_ptr<response> res, const std::string& text,
                         const bool& boolean, const int& integer,
                         const unsigned int& unsigned_value,
                         const double& decimal) {
        invoked = req != nullptr && res != nullptr && text == "doba" &&
                  boolean && integer == -42 && unsigned_value == 17 &&
                  decimal == 1.5;
      });
  const route_parameters parameters{{"text", "doba"},
                                    {"boolean", "TrUe"},
                                    {"integer", "-42"},
                                    {"unsigned", "17"},
                                    {"decimal", "1.5"}};
  DOBA_EXPECT(handler.matches(parameters));
  DOBA_EXPECT(handler.invoke(std::make_shared<const request>(),
                             std::make_shared<response>(), parameters));
  DOBA_EXPECT(invoked);
}
// +===========================================================================+
// | [>] bool parser accepts documented spellings                ( test-case ) |
// +===========================================================================+
DOBA_TEST("bool parser accepts documented spellings") {
  constexpr std::string_view valid[] = {
      "true", "TRUE", "TrUe", "1", "false", "FALSE", "FaLsE", "0",
  };
  router_handler_parametrized<request, response, bool> handler(
      [](std::shared_ptr<const request>, std::shared_ptr<response>,
         const bool&) {});
  for (const auto value : valid) {
    DOBA_EXPECT(handler.matches({{"value", value}}));
  }
  constexpr std::string_view invalid[] = {
      "", "yes", "no", "2", " true", "true ", "00", "\x80",
  };
  for (const auto value : invalid) {
    DOBA_EXPECT(!handler.matches({{"value", value}}));
  }
}
// +===========================================================================+
// | [>] numeric parsers reject partial and out of range values  ( test-case ) |
// +===========================================================================+
DOBA_TEST("numeric parsers reject partial and out of range values") {
  router_handler_parametrized<request, response, std::int8_t, unsigned int,
                              double>
      handler([](std::shared_ptr<const request>, std::shared_ptr<response>,
                 const std::int8_t&, const unsigned int&, const double&) {});
  DOBA_EXPECT(handler.matches({{"a", "-128"}, {"b", "0"}, {"c", "0"}}));
  DOBA_EXPECT(
      handler.matches({{"a", "127"}, {"b", "4294967295"}, {"c", "-1.25"}}));
  DOBA_EXPECT(!handler.matches({{"a", "128"}, {"b", "0"}, {"c", "0"}}));
  DOBA_EXPECT(!handler.matches({{"a", "1x"}, {"b", "0"}, {"c", "0"}}));
  DOBA_EXPECT(!handler.matches({{"a", "1"}, {"b", "-1"}, {"c", "0"}}));
  DOBA_EXPECT(!handler.matches({{"a", "1"}, {"b", "1x"}, {"c", "0"}}));
  DOBA_EXPECT(!handler.matches({{"a", "1"}, {"b", "1"}, {"c", "1x"}}));
  DOBA_EXPECT(!handler.matches({{"a", ""}, {"b", "1"}, {"c", "1"}}));
}
// +===========================================================================+
// | [>] handler rejects wrong parameter counts                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("handler rejects wrong parameter counts") {
  bool invoked = false;
  router_handler_parametrized<request, response, int> handler(
      [&invoked](std::shared_ptr<const request>, std::shared_ptr<response>,
                 const int&) { invoked = true; });
  DOBA_EXPECT(!handler.matches({}));
  DOBA_EXPECT(!handler.matches({{"a", "1"}, {"b", "2"}}));
  DOBA_EXPECT(!handler.invoke(std::make_shared<const request>(),
                              std::make_shared<response>(), {}));
  DOBA_EXPECT(!invoked);
}
