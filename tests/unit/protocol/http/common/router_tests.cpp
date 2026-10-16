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

#include <concepts>
#include <memory>
#include <exception>
#include <coroutine>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "protocol/http/common/router_match_result.h"
#include "protocol/http/common/router.h"
#include "protocol/http/v11/contracts.h"
#include "common/task.h"
#include "router_value_response.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::tests::unit::router_value_response;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] request                                                    ( struct ) |
// +---------------------------------------------------------------------------+
// | Test message representation.                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct request {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  int event{0};
};

using martianlabs::doba::protocol::http::router;
using martianlabs::doba::protocol::http::router_match_result;
using martianlabs::doba::common::task;

static_assert(
    std::same_as<decltype(router<request, router_value_response>{}.match("GET",
                                                                         "/")),
                 router_match_result<request, router_value_response>>);
static_assert(martianlabs::doba::protocol::http::v11::contracts::router<
              router<request, router_value_response>, request,
              router_value_response>);
static_assert(!martianlabs::doba::protocol::http::v11::contracts::router<
              int, request, router_value_response>);

struct route_only_router {
  template <typename Hty>
  void add(std::string_view method, std::string_view path, Hty handler) {
    routes_.add(method, path, std::move(handler));
  }
  template <typename CTty, typename... Args>
  void add_controller(Args&&... args) {
    routes_.template add_controller<CTty>(std::forward<Args>(args)...);
  }
  router_match_result<request, router_value_response> match(
      std::string_view method, std::string_view path) const {
    return routes_.match(method, path);
  }
  std::string allowed_methods(std::string_view path) const {
    return routes_.allowed_methods(path);
  }

 private:
  router<request, router_value_response> routes_;
};
static_assert(martianlabs::doba::protocol::http::v11::contracts::router<
              route_only_router, request, router_value_response>);

void invoke(const router_match_result<request, router_value_response>& match,
            const request& req, router_value_response& res,
            std::string_view path = {}) {
  martianlabs::doba::protocol::http::detail::
      router_parameter_storage parameters;
  auto work = match.handler ? (*match.handler)(req, res)
                            : match.parametrized_handler->invoke(
                                  req, res, path, parameters);
  bool completed = false;
  std::exception_ptr error;
  auto completion = [&](std::exception_ptr result) {
    error = result;
    completed = true;
  };
  work.start(&completion, [](void* context, std::exception_ptr error) {
    (*static_cast<decltype(completion)*>(context))(error);
  });
  if (error) std::rethrow_exception(error);
  if (!completed) throw std::runtime_error("Route handler did not complete");
}

struct suspend_once {
  std::coroutine_handle<>& next;
  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> handle) const noexcept {
    next = handle;
  }
  void await_resume() const noexcept {}
};
}  // namespace

DOBA_TEST("router selects asynchronous route forms") {
  router<request, router_value_response> value;
  value.add("GET", "/items/*",
            [](const request&, router_value_response& res) -> task<void> {
              res.value = "wildcard";
              co_return;
            });
  value.add(
      "GET", "/items/:id",
      [](const request&, router_value_response& res, int id) -> task<void> {
        res.value = std::to_string(id);
        co_return;
      });
  value.add("GET", "/items/42",
            [](const request&, router_value_response& res) -> task<void> {
              res.value = "static";
              co_return;
            });
  request req;
  router_value_response res;
  auto match = value.match("GET", "/items/42");
  DOBA_EXPECT(match.handler != nullptr);
  auto selected = (*match.handler)(req, res);
  selected.start(nullptr, [](void*, std::exception_ptr error) {
    DOBA_EXPECT(!error);
  });
  DOBA_EXPECT_EQUAL(res.value, "static");
  match = value.match("GET", "/items/7");
  DOBA_EXPECT(match.parametrized_handler != nullptr);
  martianlabs::doba::protocol::http::detail::
      router_parameter_storage parameters;
  auto parametrized = match.parametrized_handler->invoke(
      req, res, "/items/7", parameters);
  parametrized.start(nullptr, [](void*, std::exception_ptr error) {
    DOBA_EXPECT(!error);
  });
  DOBA_EXPECT_EQUAL(res.value, "7");
  match = value.match("GET", "/items/name/extra");
  DOBA_EXPECT(match.handler != nullptr);
  auto wildcard = (*match.handler)(req, res);
  wildcard.start(nullptr, [](void*, std::exception_ptr error) {
    DOBA_EXPECT(!error);
  });
  DOBA_EXPECT_EQUAL(res.value, "wildcard");
}

DOBA_TEST("asynchronous route parameters survive suspension") {
  router<request, router_value_response> value;
  std::coroutine_handle<> next;
  value.add("GET", "/items/:name",
            [&next](const request&, router_value_response& res,
                    const std::string& name) -> task<void> {
              co_await suspend_once{next};
              res.value = name;
            });
  request req;
  router_value_response res;
  martianlabs::doba::protocol::http::detail::
      router_parameter_storage parameters;
  auto match = value.match("GET", "/items/example");
  DOBA_EXPECT(match.parametrized_handler != nullptr);
  auto work = match.parametrized_handler->invoke(
      req, res, "/items/example", parameters);
  work.start(nullptr, [](void*, std::exception_ptr error) {
    DOBA_EXPECT(!error);
  });
  DOBA_EXPECT(res.value.empty());
  next.resume();
  DOBA_EXPECT_EQUAL(res.value, "example");
}

DOBA_TEST("router preserves precedence across execution models") {
  router<request, router_value_response> value;
  value.add("GET", "/items/*", [](const request&, router_value_response& res)
      -> task<void> {
    res.value = "wildcard";
    co_return;
  });
  value.add(
      "GET", "/items/:id",
      [](const request&, router_value_response& res, int id) -> task<void> {
        res.value = std::to_string(id);
        co_return;
      });
  value.add("GET", "/items/42", [](const request&, router_value_response& res)
      -> task<void> {
    res.value = "static";
    co_return;
  });
  value.add("POST", "/items/:id",
            [](const request&, router_value_response&, int) -> task<void> {
                            co_return;
            });
  DOBA_EXPECT(value.match("GET", "/items/42").handler != nullptr);
  DOBA_EXPECT(value.match("GET", "/items/7").parametrized_handler !=
              nullptr);
  DOBA_EXPECT(value.match("GET", "/items/name").handler != nullptr);
  DOBA_EXPECT_EQUAL(value.allowed_methods("/items/7"), "GET, POST");
}

// +===========================================================================+
// | [>] router type is neither copyable nor movable             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("router type is neither copyable nor movable") {
  static_assert(
      !std::is_copy_constructible_v<router<request, router_value_response>>);
  static_assert(
      !std::is_copy_assignable_v<router<request, router_value_response>>);
  static_assert(
      !std::is_move_constructible_v<router<request, router_value_response>>);
  static_assert(
      !std::is_move_assignable_v<router<request, router_value_response>>);
  DOBA_EXPECT(true);
}

// +===========================================================================+
// | [>] match returns handler without executing it              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("match returns handler without executing it") {
  router<request, router_value_response> value;
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/items")));
  bool invoked = false;
  value.add("GET", "/items",
            [&invoked](const request&, router_value_response& res)
                -> task<void> {
              invoked = true;
              res.value = "matched";
              co_return;
            });
  auto match = value.match("GET", "/items");
  DOBA_EXPECT(static_cast<bool>(match));
  DOBA_EXPECT(!invoked);
  request req;
  router_value_response res;
  invoke(match, req, res);
  DOBA_EXPECT(invoked);
  DOBA_EXPECT_EQUAL(res.value, "matched");
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/Items")));
}

// +===========================================================================+
// | [>] static routes use exact path matching                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("static routes use exact path matching") {
  router<request, router_value_response> value;
  auto handler = [](const request&, router_value_response&) -> task<void> {
    co_return;
  };
  value.add("GET", "/assets", handler);
  DOBA_EXPECT(static_cast<bool>(value.match("GET", "/assets")));
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/assets/a")));
}

// +===========================================================================+
// | [>] wildcard routes match their prefix                      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("wildcard routes match their prefix") {
  router<request, router_value_response> value;
  bool invoked = false;
  value.add("GET", "/assets/*",
            [&invoked](const request&, router_value_response& res)
                -> task<void> {
              invoked = true;
              res.value = "wildcard";
              co_return;
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
  router_value_response res;
  invoke(value.match("GET", "/assets/a"), req, res);
  DOBA_EXPECT(invoked);
  DOBA_EXPECT_EQUAL(res.value, "wildcard");
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/assets")));
  DOBA_EXPECT(!static_cast<bool>(value.match("GET", "/Assets/a")));
}

// +===========================================================================+
// | [>] protected routes resist lookalike paths and methods     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("protected routes resist lookalike paths and methods") {
  router<request, router_value_response> value;
  auto handler = [](const request&, router_value_response&) -> task<void> {
    co_return;
  };
  value.add("GET", "/admin", handler);
  value.add("GET", "/private/*", handler);
  value.add("GET", "/user/:id",
            [](const request&, router_value_response&, int) -> task<void> {
                            co_return;
            });
  const std::string nul_admin("/admin\0x", 7);
  const std::string_view paths[] = {
      "/admin/",   "/admin/x",  "/adminx",  "/Admin",     "/ADMIN",
      "admin",     "//admin",   "/admin ",  " /admin",    "/admin%00",
      nul_admin,   "/privatex", "/private", "/Private/a", "/privat/a",
      "private/a", "/user/1/x", "/user/",   "/user"};
  for (const auto path : paths) {
    martianlabs::doba::tests::unit::test_helper::set_context(path);
    DOBA_EXPECT(!static_cast<bool>(value.match("GET", path)));
  }
  for (const std::string_view method : {"get", "Get", "GET ", "", "HEAD"}) {
    martianlabs::doba::tests::unit::test_helper::set_context(method);
    DOBA_EXPECT(!static_cast<bool>(value.match(method, "/admin")));
  }
  DOBA_EXPECT(static_cast<bool>(value.match("GET", "/admin")));
  DOBA_EXPECT(static_cast<bool>(value.match("GET", "/private/a")));
}

// +===========================================================================+
// | [>] static routes take precedence over parametrized routes  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("static routes take precedence over parametrized routes") {
  router<request, router_value_response> value;
  value.add("GET", "/items/:id",
            [](const request&, router_value_response& res, int) -> task<void> {
              res.value = "parametrized";
              co_return;
            });
  value.add("GET", "/items/42", [](const request&, router_value_response& res)
      -> task<void> {
    res.value = "static";
    co_return;
  });
  auto match = value.match("GET", "/items/42");
  DOBA_EXPECT(match.handler != nullptr);
  DOBA_EXPECT(match.parametrized_handler == nullptr);
  request req;
  router_value_response res;
  invoke(match, req, res);
  DOBA_EXPECT_EQUAL(res.value, "static");
}

// +===========================================================================+
// | [>] route precedence ends with wildcard routes              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("route precedence ends with wildcard routes") {
  router<request, router_value_response> value;
  value.add("GET", "/items/*", [](const request&, router_value_response& res)
      -> task<void> {
    res.value = "wildcard";
    co_return;
  });
  value.add("GET", "/items/:id",
            [](const request&, router_value_response& res, int) -> task<void> {
              res.value = "parametrized";
              co_return;
            });
  value.add("GET", "/items/42", [](const request&, router_value_response& res)
      -> task<void> {
    res.value = "static";
    co_return;
  });
  request req;
  router_value_response res;
  auto match = value.match("GET", "/items/42");
  invoke(match, req, res);
  DOBA_EXPECT_EQUAL(res.value, "static");
  match = value.match("GET", "/items/7");
  invoke(match, req, res, "/items/7");
  DOBA_EXPECT_EQUAL(res.value, "parametrized");
  match = value.match("GET", "/items/name");
  invoke(match, req, res);
  DOBA_EXPECT_EQUAL(res.value, "wildcard");
}

// +===========================================================================+
// | [>] parametrized routes preserve typed registration order   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("parametrized routes preserve typed registration order") {
  router<request, router_value_response> value;
  value.add("GET", "/items/:value",
            [](const request&, router_value_response& res, int) -> task<void> {
              res.value = "integer";
              co_return;
            });
  value.add("GET", "/items/:value",
            [](const request&, router_value_response& res, std::string_view)
                -> task<void> {
              res.value = "text";
              co_return;
            });
  request req;
  router_value_response res;
  auto integer = value.match("GET", "/items/42");
  invoke(integer, req, res, "/items/42");
  DOBA_EXPECT_EQUAL(res.value, "integer");
  auto text = value.match("GET", "/items/value");
  invoke(text, req, res, "/items/value");
  DOBA_EXPECT_EQUAL(res.value, "text");
}

// +===========================================================================+
// | [>] parametrized routes validate pattern and handler shape  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("parametrized routes validate pattern and handler shape") {
  router<request, router_value_response> value;
  bool threw = false;
  try {
    value.add("GET", "/items/:id",
              [](const request&, router_value_response&) -> task<void> {
                                co_return;
              });
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  threw = false;
  try {
    value.add("GET", "/items",
              [](const request&, router_value_response&, int) -> task<void> {
                                co_return;
              });
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  threw = false;
  try {
    value.add("GET",
              "/items/:", [](const request&, router_value_response&, int)
                  -> task<void> {
                                co_return;
              });
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
}

// +===========================================================================+
// | [>] wildcard routes validate pattern and handler shape      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("wildcard routes validate pattern and handler shape") {
  constexpr std::string_view invalid[] = {
      "*", "/a*", "/a/*/b", "/a/**", "/a/*x",
  };
  for (const auto route : invalid) {
    router<request, router_value_response> value;
    bool threw = false;
    try {
      value.add("GET", route, [](const request&, router_value_response&)
          -> task<void> {
                co_return;
      });
    } catch (const std::invalid_argument&) {
      threw = true;
    }
    DOBA_EXPECT(threw);
  }
  router<request, router_value_response> value;
  bool threw = false;
  try {
    value.add("GET", "/items/:id/*",
              [](const request&, router_value_response&) -> task<void> {
                                co_return;
              });
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  threw = false;
  try {
    value.add("GET", "/items/*",
              [](const request&, router_value_response&, int) -> task<void> {
                                co_return;
              });
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
}

// +===========================================================================+
// | [>] match preserves first handler                           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("match preserves first handler") {
  router<request, router_value_response> value;
  value.add("GET", "/resource", [](const request&, router_value_response& res)
      -> task<void> {
    res.value = "first";
    co_return;
  });
  value.add("GET", "/resource", [](const request&, router_value_response& res)
      -> task<void> {
    res.value = "second";
    co_return;
  });
  value.add("POST", "/resource", [](const request&, router_value_response&)
      -> task<void> {
        co_return;
  });
  auto get = value.match("GET", "/resource");
  auto post = value.match("POST", "/resource");
  request req;
  router_value_response res;
  invoke(get, req, res);
  DOBA_EXPECT_EQUAL(res.value, "first");
  DOBA_EXPECT(static_cast<bool>(post));
  DOBA_EXPECT_EQUAL(value.allowed_methods("/resource"), "GET, POST");
  DOBA_EXPECT(value.allowed_methods("/missing").empty());
}

// +===========================================================================+
// | [>] allowed methods include matching parametrized routes    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("allowed methods include matching parametrized routes") {
  router<request, router_value_response> value;
  value.add("GET", "/items/:id",
            [](const request&, router_value_response&, int) -> task<void> {
                            co_return;
            });
  value.add("POST", "/items/:name",
            [](const request&, router_value_response&, std::string_view)
                -> task<void> {
                            co_return;
            });
  DOBA_EXPECT_EQUAL(value.allowed_methods("/items/42"), "GET, POST");
  DOBA_EXPECT_EQUAL(value.allowed_methods("/items/name"), "POST");
}

// +===========================================================================+
// | [>] allowed methods include matching wildcard routes        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("allowed methods include matching wildcard routes") {
  router<request, router_value_response> value;
  value.add("GET", "/assets/logo",
            [](const request&, router_value_response&) -> task<void> {
                            co_return;
            });
  value.add("GET", "/assets/*", [](const request&, router_value_response&)
      -> task<void> {
        co_return;
  });
  value.add("POST", "/assets/:id",
            [](const request&, router_value_response&, int) -> task<void> {
                            co_return;
            });
  value.add("DELETE", "/assets/*",
            [](const request&, router_value_response&) -> task<void> {
                            co_return;
            });
  DOBA_EXPECT_EQUAL(value.allowed_methods("/assets/logo"), "GET, DELETE");
  DOBA_EXPECT_EQUAL(value.allowed_methods("/assets/42"), "POST, GET, DELETE");
  DOBA_EXPECT(value.allowed_methods("/assets").empty());
}

// +===========================================================================+
// | [>] noexcept handlers register and execute                  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("noexcept handlers register and execute") {
  router<request, router_value_response> value;
  value.add("GET", "/sync",
            [](const request&, router_value_response& res) noexcept
                -> task<void> {
              res.value = "sync";
              co_return;
            });
  value.add("GET", "/mutable/:id",
            [](const request&, router_value_response& res,
               int id) mutable noexcept -> task<void> {
              res.value = std::to_string(id);
              co_return;
            });
  request req;
  router_value_response res;
  const auto sync = value.match("GET", "/sync");
  invoke(sync, req, res);
  DOBA_EXPECT_EQUAL(res.value, "sync");
  const auto mutable_sync = value.match("GET", "/mutable/42");
  invoke(mutable_sync, req, res, "/mutable/42");
  DOBA_EXPECT_EQUAL(res.value, "42");
}

// +===========================================================================+
// | [>] registration owns method and all route pattern forms    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("registration owns method and all route pattern forms") {
  router<request, router_value_response> value;
  {
    std::string method = "GET";
    std::string fixed = "/fixed";
    std::string parameter = "/value/:id";
    std::string wildcard = "/assets/*";
    value.add(method, fixed, [](const request&, router_value_response& res)
        -> task<void> {
      res.value = "fixed";
      co_return;
    });
    value.add(method, parameter,
              [](const request&, router_value_response& res, int id)
                  -> task<void> {
                res.value = std::to_string(id);
                co_return;
              });
    value.add(method, wildcard, [](const request&, router_value_response& res)
        -> task<void> {
      res.value = "wildcard";
      co_return;
    });
    method.assign(method.size(), 'x');
    fixed.assign(fixed.size(), 'x');
    parameter.assign(parameter.size(), 'x');
    wildcard.assign(wildcard.size(), 'x');
  }
  request req;
  router_value_response res;
  const auto fixed = value.match("GET", "/fixed");
  DOBA_EXPECT(fixed.handler != nullptr);
  invoke(fixed, req, res);
  DOBA_EXPECT_EQUAL(res.value, "fixed");
  const auto parameter = value.match("GET", "/value/42");
  DOBA_EXPECT(parameter.parametrized_handler != nullptr);
  invoke(parameter, req, res, "/value/42");
  DOBA_EXPECT_EQUAL(res.value, "42");
  const auto wildcard = value.match("GET", "/assets/one");
  DOBA_EXPECT(wildcard.handler != nullptr);
  invoke(wildcard, req, res);
  DOBA_EXPECT_EQUAL(res.value, "wildcard");
}

// +===========================================================================+
// | [>] invalid registration preserves existing route selection ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("invalid registration preserves existing route selection") {
  router<request, router_value_response> value;
  value.add("GET", "/kept", [](const request&, router_value_response& res)
      -> task<void> {
    res.value = "kept";
    co_return;
  });
  for (std::string_view pattern : {"/:", "/x/*/tail", "/x/:id/*"}) {
    bool threw = false;
    try {
      value.add("GET", pattern,
                [](const request&, router_value_response& res, int)
                    -> task<void> {
                  res.value = "invalid";
                  co_return;
                });
    } catch (const std::invalid_argument&) {
      threw = true;
    }
    DOBA_EXPECT(threw);
    const auto match = value.match("GET", "/kept");
    DOBA_EXPECT(match.handler != nullptr);
    router_value_response res;
    invoke(match, request{}, res);
    DOBA_EXPECT_EQUAL(res.value, "kept");
    DOBA_EXPECT_EQUAL(value.allowed_methods("/kept"), "GET");
  }
}

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] controller_probe                                            ( class ) |
// +---------------------------------------------------------------------------+
// | Controller implementation.                                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class controller_probe {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  controller_probe(std::string prefix, int& alive)
      : prefix_(std::move(prefix)), alive_(alive) {
    alive_++;
  }
  controller_probe(const controller_probe&) = delete;
  controller_probe(controller_probe&&) = delete;
  ~controller_probe() { alive_--; }
  // +=========================================================================+
  // | [>] register_routes                                          ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename Rty>
  void register_routes(Rty& routes) {
    routes.add("GET", prefix_ + "/count", &controller_probe::count);
    routes.add("GET", prefix_ + "/value/:id", &controller_probe::value);
    routes.add("GET", prefix_ + "/wild/*", &controller_probe::value_zero);
  }
  // +=========================================================================+
  // | [>] count                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  task<void> count(const request&, router_value_response& res) {
    res.value = std::to_string(++count_);
    co_return;
  }
  // +=========================================================================+
  // | [>] value                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  task<void> value(const request&, router_value_response& res,
             int id) const noexcept {
    res.value = std::to_string(count_ + id);
    co_return;
  }
  // +=========================================================================+
  // | [>] value_zero                                               ( public ) |
  // +-------------------------------------------------------------------------+
  task<void> value_zero(const request&, router_value_response& res) const {
    res.value = prefix_;
    co_return;
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::string prefix_;
  int& alive_;
  int count_{0};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] failing_controller                                         ( struct ) |
// +---------------------------------------------------------------------------+
// | Controller implementation.                                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct failing_controller {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit failing_controller(int mode) : mode_(mode) {
    if (mode == 0) throw std::runtime_error("constructor");
  }
  // +=========================================================================+
  // | [>] register_routes                                          ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename Rty>
  void register_routes(Rty& routes) {
    if (mode_ == 1) return;
    routes.add("GET", "/new", &failing_controller::get);
    routes.add("POST", "/new", &failing_controller::get);
    routes.add("GET", "/new/:id", &failing_controller::typed);
    routes.add("GET", "/new/*", &failing_controller::get);
    routes.add("POST", "/new/:id", &failing_controller::typed);
    routes.add("POST", "/new/*", &failing_controller::get);
    routes.add("GET", "/invalid/:id", &failing_controller::get);
  }
  // +=========================================================================+
  // | [>] get                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  task<void> get(const request&, router_value_response&) {
    co_return;
  }
  // +=========================================================================+
  // | [>] typed                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  task<void> typed(const request&, router_value_response&, int) {
    co_return;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  int mode_;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] staged_controller                                         ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct staged_controller {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  router<request, router_value_response>& owner;
  bool& detached;
  // +=========================================================================+
  // | [>] register_routes                                          ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename Rty>
  void register_routes(Rty& routes) {
    routes.add("GET", "/staged", &staged_controller::get);
    detached = !owner.match("GET", "/staged");
  }
  // +=========================================================================+
  // | [>] get                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  task<void> get(const request&, router_value_response&) {
    co_return;
  }
};

struct async_controller {
  template <typename Rty>
  void register_routes(Rty& routes) {
    routes.add("GET", "/async/:id", &async_controller::get);
  }
  task<void> get(const request&, router_value_response& res, int id) {
    res.value = std::to_string(id);
    co_return;
  }
};
}  // namespace

// +===========================================================================+
// | [>] controllers share one instance per registration         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("controllers share one instance per registration") {
  int alive = 0;
  {
    router<request, router_value_response> value;
    value.add_controller<controller_probe>("/a", alive);
    value.add_controller<controller_probe>("/b", alive);
    DOBA_EXPECT_EQUAL(alive, 2);
    auto count = value.match("GET", "/a/count");
    router_value_response res;
    invoke(count, {}, res);
    DOBA_EXPECT_EQUAL(res.value, "1");
    invoke(count, {}, res);
    DOBA_EXPECT_EQUAL(res.value, "2");
    auto typed = value.match("GET", "/a/value/40");
    invoke(typed, {}, res, "/a/value/40");
    DOBA_EXPECT_EQUAL(res.value, "42");
    invoke(value.match("GET", "/b/count"), {}, res);
    DOBA_EXPECT_EQUAL(res.value, "1");
    invoke(value.match("GET", "/a/wild/x"), {}, res);
    DOBA_EXPECT_EQUAL(res.value, "/a");
  }
  DOBA_EXPECT_EQUAL(alive, 0);
}

DOBA_TEST("controller registers an asynchronous parametrized route") {
  router<request, router_value_response> value;
  value.add_controller<async_controller>();
  auto match = value.match("GET", "/async/7");
  DOBA_EXPECT(match.parametrized_handler != nullptr);
  request req;
  router_value_response res;
  martianlabs::doba::protocol::http::detail::
      router_parameter_storage parameters;
  auto work = match.parametrized_handler->invoke(
      req, res, "/async/7", parameters);
  work.start(nullptr, [](void*, std::exception_ptr error) {
    DOBA_EXPECT(!error);
  });
  DOBA_EXPECT_EQUAL(res.value, "7");
}

// +===========================================================================+
// | [>] controller routes are staged before registration        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("controller routes are staged before registration") {
  router<request, router_value_response> value;
  bool detached = false;
  value.add_controller<staged_controller>(value, detached);
  DOBA_EXPECT(detached);
  DOBA_EXPECT(value.match("GET", "/staged").handler);
}

// +===========================================================================+
// | [>] controller registration rolls back every route category ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("controller registration rolls back every route category") {
  router<request, router_value_response> value;
  value.add("GET", "/old", [](const request&, router_value_response& res)
      -> task<void> {
    res.value = "old";
    co_return;
  });
  value.add("GET", "/old/:id",
            [](const request&, router_value_response& res, int) -> task<void> {
              res.value = "typed";
              co_return;
            });
  value.add("GET", "/old/*", [](const request&, router_value_response& res)
      -> task<void> {
    res.value = "wild";
    co_return;
  });
  for (int mode : {0, 1, 2}) {
    bool threw = false;
    try {
      value.add_controller<failing_controller>(mode);
    } catch (const std::exception&) {
      threw = true;
    }
    DOBA_EXPECT(threw);
    DOBA_EXPECT(!value.match("GET", "/new"));
    DOBA_EXPECT(!value.match("POST", "/new"));
    DOBA_EXPECT(!value.match("GET", "/new/42"));
    DOBA_EXPECT(!value.match("POST", "/new/x"));
    DOBA_EXPECT_EQUAL(value.allowed_methods("/new/42"), "");
    router_value_response res;
    invoke(value.match("GET", "/old"), {}, res);
    DOBA_EXPECT_EQUAL(res.value, "old");
    DOBA_EXPECT(value.match("GET", "/old/42").parametrized_handler);
    DOBA_EXPECT(value.match("GET", "/old/x").handler);
  }
}


DOBA_TEST("router associates authentication with selected route only") {
  namespace auth = martianlabs::doba::protocol::http::auth;
  router<request, router_value_response> routes;
  int checks = 0;
  routes.add(
      "GET", "/items/*", [](const request&, router_value_response&)
          -> task<void> {
                co_return;
      },
      [&](const request&) -> auth::authentication_result {
        checks++;
        return {auth::authentication_status::kUnauthenticated, "Custom"};
      });
  routes.add("GET", "/items/public",
             [](const request&, router_value_response&) -> task<void> {
                              co_return;
             });
  const auto selected = routes.match("GET", "/items/public");
  DOBA_EXPECT(static_cast<bool>(selected));
  DOBA_EXPECT(selected.authenticate && !*selected.authenticate);
  const auto wildcard = routes.match("GET", "/items/private");
  DOBA_EXPECT(wildcard.authenticate && *wildcard.authenticate);
  DOBA_EXPECT_EQUAL(checks, 0);
  DOBA_EXPECT((*wildcard.authenticate)(request{}).status ==
              auth::authentication_status::kUnauthenticated);
  DOBA_EXPECT_EQUAL(checks, 1);
}
