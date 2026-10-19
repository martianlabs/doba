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
#include <string>
#include <string_view>
#include <utility>

#include "protocol/http/common/router_handler_parametrized.h"
#include "router_empty_request.h"
#include "router_input_case.h"
#include "router_value_response.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::detail::make_router_handler_parametrized;
using martianlabs::doba::tests::unit::router_empty_request;
using martianlabs::doba::tests::unit::router_input_case;
using martianlabs::doba::tests::unit::router_value_response;
}  // namespace

// +===========================================================================+
// | [>] matches routes with typed parameters                    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("matches routes with typed parameters") {
  auto handler =
      make_router_handler_parametrized<router_empty_request,
                                       router_value_response, std::uint64_t,
                                       bool, double, std::string_view>(
          "/items/:id/:enabled/:score/:name",
          [](const router_empty_request&, router_value_response&, std::uint64_t,
             bool, double, std::string_view) {});
  DOBA_EXPECT(handler.matches("/items/42/TRUE/1.5/doba"));
  DOBA_EXPECT(!handler.matches("/items/x/true/1.5/doba"));
  DOBA_EXPECT(!handler.matches("/items/42/yes/1.5/doba"));
  DOBA_EXPECT(!handler.matches("/items/42/true/score/doba"));
}

// +===========================================================================+
// | [>] matching requires the complete route shape              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("matching requires the complete route shape") {
  auto handler = make_router_handler_parametrized<router_empty_request,
                                                  router_value_response, int>(
      "/items/:id",
      [](const router_empty_request&, router_value_response&, int) {});
  DOBA_EXPECT(handler.matches("/items/42"));
  DOBA_EXPECT(!handler.matches("/items/"));
  DOBA_EXPECT(!handler.matches("/items/42/"));
  DOBA_EXPECT(!handler.matches("/items/42/details"));
}

// +===========================================================================+
// | [>] invoke passes parsed values to the callback             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("invoke passes parsed values to the callback") {
  bool invoked = false;
  auto handler =
      make_router_handler_parametrized<router_empty_request,
                                       router_value_response, std::uint64_t,
                                       bool, double, std::string>(
          "/items/:id/:enabled/:score/:name",
          [&invoked](const router_empty_request&, router_value_response& res,
                     std::uint64_t id, bool enabled, double score,
                     const std::string& name) {
            invoked = id == 42 && enabled && score == 1.5 && name == "doba";
            res.value = name;
          });
  router_empty_request req;
  router_value_response res;
  handler.invoke(req, res, "/items/42/true/1.5/doba");
  DOBA_EXPECT(invoked);
  DOBA_EXPECT_EQUAL(res.value, "doba");
}

// +===========================================================================+
// | [>] invoke rejects paths with invalid parameters            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("invoke rejects paths with invalid parameters") {
  bool invoked = false;
  auto handler = make_router_handler_parametrized<router_empty_request,
                                                  router_value_response, int>(
      "/items/:id",
      [&invoked](const router_empty_request&, router_value_response&, int) {
        invoked = true;
      });
  router_empty_request req;
  router_value_response res;
  bool threw = false;
  try {
    handler.invoke(req, res, "/items/value");
  } catch (const std::runtime_error& error) {
    threw = std::string_view(error.what()) ==
            "The route parameters could not be parsed";
  }
  DOBA_EXPECT(threw);
  DOBA_EXPECT(!invoked);
  threw = false;
  try {
    handler.invoke(req, res, "/items");
  } catch (const std::runtime_error& error) {
    threw = std::string_view(error.what()) ==
            "The route parameters could not be extracted";
  }
  DOBA_EXPECT(threw);
  DOBA_EXPECT(!invoked);
}

// +===========================================================================+
// | [>] accepts every boolean spelling                          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("route conversion accepts every boolean spelling") {
  struct test_case {
    std::string_view input;
    bool expected;
  };
  const test_case cases[] = {
      {"false", false}, {"FALSE", false}, {"FaLsE", false},
      {"0", false},     {"1", true},
  };
  for (const auto& test : cases) {
    const std::string path = "/value/" + std::string(test.input);
    martianlabs::doba::tests::unit::test_helper::set_context(test.input);
    std::size_t sync_calls = 0;
    bool sync_value{};
    auto handler =
        make_router_handler_parametrized<router_empty_request,
                                         router_value_response, bool>(
            "/value/:value", [&](const router_empty_request&,
                                 router_value_response&, bool value) {
              sync_calls++;
              sync_value = value;
            });
    DOBA_EXPECT_EQUAL(handler.matches(path), true);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
    router_empty_request req;
    router_value_response res;
    handler.invoke(req, res, path);
    DOBA_EXPECT_EQUAL(sync_calls, 1);
    DOBA_EXPECT_EQUAL(sync_value, test.expected);
  }
}

// +===========================================================================+
// | [>] accepts signed integer boundaries                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("route conversion accepts signed integer boundaries") {
  struct test_case {
    std::string_view input;
    std::int64_t expected;
  };
  const test_case cases[] = {
      {"-9223372036854775808", std::numeric_limits<std::int64_t>::min()},
      {"9223372036854775807", std::numeric_limits<std::int64_t>::max()},
      {"-1", -1},
      {"0", 0},
  };
  for (const auto& test : cases) {
    const std::string path = "/value/" + std::string(test.input);
    martianlabs::doba::tests::unit::test_helper::set_context(test.input);
    std::size_t sync_calls = 0;
    std::int64_t sync_value{};
    auto handler =
        make_router_handler_parametrized<router_empty_request,
                                         router_value_response, std::int64_t>(
            "/value/:value", [&](const router_empty_request&,
                                 router_value_response&, std::int64_t value) {
              sync_calls++;
              sync_value = value;
            });
    DOBA_EXPECT_EQUAL(handler.matches(path), true);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
    router_empty_request req;
    router_value_response res;
    handler.invoke(req, res, path);
    DOBA_EXPECT_EQUAL(sync_calls, 1);
    DOBA_EXPECT_EQUAL(sync_value, test.expected);
  }
}

// +===========================================================================+
// | [>] accepts unsigned integer boundaries                     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("route conversion accepts unsigned integer boundaries") {
  struct test_case {
    std::string_view input;
    std::uint64_t expected;
  };
  const test_case cases[] = {
      {"0", 0},
      {"18446744073709551615", std::numeric_limits<std::uint64_t>::max()},
  };
  for (const auto& test : cases) {
    const std::string path = "/value/" + std::string(test.input);
    martianlabs::doba::tests::unit::test_helper::set_context(test.input);
    std::size_t sync_calls = 0;
    std::uint64_t sync_value{};
    auto handler =
        make_router_handler_parametrized<router_empty_request,
                                         router_value_response, std::uint64_t>(
            "/value/:value", [&](const router_empty_request&,
                                 router_value_response&, std::uint64_t value) {
              sync_calls++;
              sync_value = value;
            });
    DOBA_EXPECT_EQUAL(handler.matches(path), true);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
    router_empty_request req;
    router_value_response res;
    handler.invoke(req, res, path);
    DOBA_EXPECT_EQUAL(sync_calls, 1);
    DOBA_EXPECT_EQUAL(sync_value, test.expected);
  }
}

// +===========================================================================+
// | [>] rejects integer overflow                                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("route conversion rejects integer overflow") {
  {
    const router_input_case cases[] = {
        {"-9223372036854775809"},
        {"9223372036854775808"},
    };
    for (const auto& test : cases) {
      const std::string path = "/value/" + std::string(test.input);
      martianlabs::doba::tests::unit::test_helper::set_context(test.input);
      std::size_t sync_calls = 0;
      auto handler =
          make_router_handler_parametrized<router_empty_request,
                                           router_value_response, std::int64_t>(
              "/value/:value",
              [&](const router_empty_request&, router_value_response&,
                  std::int64_t) { sync_calls++; });
      DOBA_EXPECT_EQUAL(handler.matches(path), false);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
      router_empty_request req;
      router_value_response res;
      bool sync_threw = false;
      try {
        handler.invoke(req, res, path);
      } catch (const std::runtime_error& error) {
        sync_threw = std::string_view(error.what()) ==
                     "The route parameters could not be parsed";
      }
      DOBA_EXPECT(sync_threw);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
    }
  }
  {
    const router_input_case cases[] = {
        {"18446744073709551616"},
    };
    for (const auto& test : cases) {
      const std::string path = "/value/" + std::string(test.input);
      martianlabs::doba::tests::unit::test_helper::set_context(test.input);
      std::size_t sync_calls = 0;
      auto handler = make_router_handler_parametrized<
          router_empty_request, router_value_response, std::uint64_t>(
          "/value/:value",
          [&](const router_empty_request&, router_value_response&,
              std::uint64_t) { sync_calls++; });
      DOBA_EXPECT_EQUAL(handler.matches(path), false);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
      router_empty_request req;
      router_value_response res;
      bool sync_threw = false;
      try {
        handler.invoke(req, res, path);
      } catch (const std::runtime_error& error) {
        sync_threw = std::string_view(error.what()) ==
                     "The route parameters could not be parsed";
      }
      DOBA_EXPECT(sync_threw);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
    }
  }
}

// +===========================================================================+
// | [>] rejects partial numbers spaces and plus signs           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("route conversion rejects partial numbers spaces and plus signs") {
  {
    const router_input_case cases[] = {
        {"42x"},
        {" 42"},
        {"42 "},
        {"+42"},
    };
    for (const auto& test : cases) {
      const std::string path = "/value/" + std::string(test.input);
      martianlabs::doba::tests::unit::test_helper::set_context(test.input);
      std::size_t sync_calls = 0;
      auto handler =
          make_router_handler_parametrized<router_empty_request,
                                           router_value_response, int>(
              "/value/:value",
              [&](const router_empty_request&, router_value_response&, int) {
                sync_calls++;
              });
      DOBA_EXPECT_EQUAL(handler.matches(path), false);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
      router_empty_request req;
      router_value_response res;
      bool sync_threw = false;
      try {
        handler.invoke(req, res, path);
      } catch (const std::runtime_error& error) {
        sync_threw = std::string_view(error.what()) ==
                     "The route parameters could not be parsed";
      }
      DOBA_EXPECT(sync_threw);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
    }
  }
  {
    const router_input_case cases[] = {
        {"1.5x"},
        {"+1.5"},
    };
    for (const auto& test : cases) {
      const std::string path = "/value/" + std::string(test.input);
      martianlabs::doba::tests::unit::test_helper::set_context(test.input);
      std::size_t sync_calls = 0;
      auto handler =
          make_router_handler_parametrized<router_empty_request,
                                           router_value_response, double>(
              "/value/:value",
              [&](const router_empty_request&, router_value_response&, double) {
                sync_calls++;
              });
      DOBA_EXPECT_EQUAL(handler.matches(path), false);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
      router_empty_request req;
      router_value_response res;
      bool sync_threw = false;
      try {
        handler.invoke(req, res, path);
      } catch (const std::runtime_error& error) {
        sync_threw = std::string_view(error.what()) ==
                     "The route parameters could not be parsed";
      }
      DOBA_EXPECT(sync_threw);
      DOBA_EXPECT_EQUAL(sync_calls, 0);
    }
  }
}

// +===========================================================================+
// | [>] rejects negative unsigned values                        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("route conversion rejects negative unsigned values") {
  const router_input_case cases[] = {
      {"-1"},
      {"-0"},
  };
  for (const auto& test : cases) {
    const std::string path = "/value/" + std::string(test.input);
    martianlabs::doba::tests::unit::test_helper::set_context(test.input);
    std::size_t sync_calls = 0;
    auto handler =
        make_router_handler_parametrized<router_empty_request,
                                         router_value_response, std::uint64_t>(
            "/value/:value",
            [&](const router_empty_request&, router_value_response&,
                std::uint64_t) { sync_calls++; });
    DOBA_EXPECT_EQUAL(handler.matches(path), false);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
    router_empty_request req;
    router_value_response res;
    bool sync_threw = false;
    try {
      handler.invoke(req, res, path);
    } catch (const std::runtime_error& error) {
      sync_threw = std::string_view(error.what()) ==
                   "The route parameters could not be parsed";
    }
    DOBA_EXPECT(sync_threw);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
  }
}

// +===========================================================================+
// | [>] accepts finite floating point values                    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("route conversion accepts finite floating point values") {
  struct test_case {
    std::string_view input;
    double expected;
  };
  const test_case cases[] = {
      {"-1.5", -1.5},
      {"1e2", 100.0},
      {"0", 0.0},
      {"1.5e-2", 0.015},
  };
  for (const auto& test : cases) {
    const std::string path = "/value/" + std::string(test.input);
    martianlabs::doba::tests::unit::test_helper::set_context(test.input);
    std::size_t sync_calls = 0;
    double sync_value{};
    auto handler =
        make_router_handler_parametrized<router_empty_request,
                                         router_value_response, double>(
            "/value/:value", [&](const router_empty_request&,
                                 router_value_response&, double value) {
              sync_calls++;
              sync_value = value;
            });
    DOBA_EXPECT_EQUAL(handler.matches(path), true);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
    router_empty_request req;
    router_value_response res;
    handler.invoke(req, res, path);
    DOBA_EXPECT_EQUAL(sync_calls, 1);
    DOBA_EXPECT_EQUAL(sync_value, test.expected);
  }
}

// +===========================================================================+
// | [>] rejects floating point range errors                     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("route conversion rejects floating point range errors") {
  const router_input_case cases[] = {
      {"1e9999"},
      {"1e-9999"},
  };
  for (const auto& test : cases) {
    const std::string path = "/value/" + std::string(test.input);
    martianlabs::doba::tests::unit::test_helper::set_context(test.input);
    std::size_t sync_calls = 0;
    auto handler =
        make_router_handler_parametrized<router_empty_request,
                                         router_value_response, double>(
            "/value/:value",
            [&](const router_empty_request&, router_value_response&, double) {
              sync_calls++;
            });
    DOBA_EXPECT_EQUAL(handler.matches(path), false);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
    router_empty_request req;
    router_value_response res;
    bool sync_threw = false;
    try {
      handler.invoke(req, res, path);
    } catch (const std::runtime_error& error) {
      sync_threw = std::string_view(error.what()) ==
                   "The route parameters could not be parsed";
    }
    DOBA_EXPECT(sync_threw);
    DOBA_EXPECT_EQUAL(sync_calls, 0);
  }
}

// +===========================================================================+
// | [>] narrow integer conversions enforce each type boundary   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("narrow integer conversions enforce each type boundary") {
  const auto check = []<typename T>() {
    const long long minimum = std::numeric_limits<T>::min();
    const long long maximum = std::numeric_limits<T>::max();
    struct test_case {
      long long input;
      bool valid;
    };
    const test_case cases[] = {
        {minimum - 1, false}, {minimum, true},      {0, true},
        {maximum, true},      {maximum + 1, false},
    };
    for (const auto& test : cases) {
      const std::string path = "/value/" + std::to_string(test.input);
      martianlabs::doba::tests::unit::test_helper::set_context(path);
      std::size_t calls = 0;
      T converted{};
      auto handler = make_router_handler_parametrized<router_empty_request,
                                                      router_value_response, T>(
          "/value/:value",
          [&](const router_empty_request&, router_value_response&, T value) {
            calls++;
            converted = value;
          });
      DOBA_EXPECT_EQUAL(handler.matches(path), test.valid);
      bool threw = false;
      try {
        router_value_response res;
        handler.invoke(router_empty_request{}, res, path);
      } catch (const std::runtime_error& error) {
        threw = std::string_view(error.what()) ==
                "The route parameters could not be parsed";
      }
      DOBA_EXPECT_EQUAL(threw, !test.valid);
      DOBA_EXPECT_EQUAL(calls, test.valid ? 1 : 0);
      if (test.valid) DOBA_EXPECT_EQUAL(converted, test.input);
    }
  };
  check.operator()<std::int8_t>();
  check.operator()<std::uint8_t>();
  check.operator()<std::int16_t>();
  check.operator()<std::uint16_t>();
  check.operator()<std::int32_t>();
  check.operator()<std::uint32_t>();
}

// +===========================================================================+
// | [>] later parameter failures never invoke the callback      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("later parameter failures never invoke the callback") {
  std::size_t calls = 0;
  auto handler =
      make_router_handler_parametrized<router_empty_request,
                                       router_value_response, int, bool>(
          "/:id/:enabled", [&](const router_empty_request&,
                               router_value_response&, int, bool) { calls++; });
  for (std::string_view path : {"/7/yes", "/7/truex", "/7/2"}) {
    DOBA_EXPECT(!handler.matches(path));
    bool threw = false;
    try {
      router_value_response res;
      handler.invoke(router_empty_request{}, res, path);
    } catch (const std::runtime_error& error) {
      threw = std::string_view(error.what()) ==
              "The route parameters could not be parsed";
    }
    DOBA_EXPECT(threw);
    DOBA_EXPECT_EQUAL(calls, 0);
  }
}
