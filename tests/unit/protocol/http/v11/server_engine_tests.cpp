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

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "http_server.h"
#include "protocol/http/common/auth/basic.h"
#include "protocol/http/common/auth/bearer.h"
#include "test_helper.h"

#ifdef DOBA_ENABLE_TLS
#include "https_server.h"
#endif

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] usings                                                     ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using martianlabs::doba::http_server;
#ifdef DOBA_ENABLE_TLS
using martianlabs::doba::https_server;
#endif
using martianlabs::doba::protocol::http::router;
using martianlabs::doba::common::reader;
namespace protocol = martianlabs::doba::protocol;
namespace http = martianlabs::doba::protocol::http::v11;
namespace tr = martianlabs::doba::transport::server;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] observation                                                ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct observation {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  int starts{0};
  int stops{0};
  bool fail_start{false};
  std::string bytes;
  std::string request;
  int configuration{0};
} observed;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] fake_transport                                             ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ENty, typename FNty>
struct fake_transport {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = std::unique_ptr<int>;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  fake_transport(policies_type configuration, FNty create_engine)
      : factory(std::move(create_engine)) {
    observed.configuration = configuration ? *configuration : 0;
  }
  // +=========================================================================+
  // | [>] set_on_connection                                        ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_connection(std::function<void()>) {}
  // +=========================================================================+
  // | [>] set_on_disconnection                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_disconnection(std::function<void()>) {}
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void start() {
    if (observed.fail_start) throw std::runtime_error("start failed");
    observed.starts++;
    const std::string request = observed.request.empty()
                                    ? "GET / HTTP/1.1\r\nHost: localhost\r\n"
                                      "\r\n"
                                    : observed.request;
    for (int i = 0; i < 2; i++) {
      auto value = factory();
      value.set_on_send([](std::string_view head, std::string_view body,
                           std::unique_ptr<reader> source) {
        observed.bytes.append(head);
        observed.bytes.append(body);
        if (source) source->read_all(observed.bytes);
      });
      value.set_on_close([]() {});
      value.on_bytes_received(request.data(), request.size());
    }
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void stop() { observed.stops++; }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  FNty factory;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] bad_transport                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ENty, typename FNty>
struct bad_transport : fake_transport<ENty, FNty> {
  using fake_transport<ENty, FNty>::fake_transport;
  int stop();
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] bad_start_transport                                        ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ENty, typename FNty>
struct bad_start_transport : fake_transport<ENty, FNty> {
  using fake_transport<ENty, FNty>::fake_transport;
  void start(const char*, const char*);
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] bad_policies_transport                                     ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ENty, typename FNty>
struct bad_policies_transport : fake_transport<ENty, FNty> {
  using fake_transport<ENty, FNty>::fake_transport;
  using policies_type = tr::tcp_policies;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] private_policies_transport                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ENty, typename FNty>
struct private_policies_transport : fake_transport<ENty, FNty> {
  using fake_transport<ENty, FNty>::fake_transport;

 private:
  using policies_type = typename fake_transport<ENty, FNty>::policies_type;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] bad_engine                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct bad_engine : http::engine<http::request, http::response> {
  explicit bad_engine(http::policies);
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] bad_receive_engine                                         ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct bad_receive_engine : http::engine<http::request, http::response> {
  void on_bytes_received(const char*, std::size_t, std::size_t);
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] bad_send_engine                                            ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct bad_send_engine : http::engine<http::request, http::response> {
  void set_on_send(std::function<void(const char*, std::size_t)>);
};

struct custom_request {};
struct custom_response {};
struct custom_routes final {};

struct custom_engine {
  using policies_type = http::policies;

  template <typename ROty>
  custom_engine(const policies_type&, const ROty&) {}
  void set_on_send(protocol::send_delegate) {}
  void set_on_close(std::function<void()>) {}
  void on_bytes_received(const char*, std::size_t) {
    return;
  }
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] asserts                                                    ( static ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
static_assert(!protocol::contracts::engine<bad_receive_engine>);
static_assert(!protocol::contracts::engine<bad_send_engine>);
static_assert(!protocol::contracts::engine_factory<
              decltype([]() { return 0; }),
              http::engine<http::request, http::response>>);
static_assert(!protocol::contracts::engine_factory<
              decltype([](int) -> http::engine<http::request, http::response> {
                throw 0;
              }),
              http::engine<http::request, http::response>>);
static_assert(!protocol::contracts::engine_factory<
              decltype([value = 0]() mutable
                           -> http::engine<http::request, http::response> {
                value++;
                throw value;
              }),
              http::engine<http::request, http::response>>);

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] accepts_server                                            ( concept ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ENty, template <typename, typename> class TRty>
concept accepts_server = requires {
  typename http_server<http::request, http::response,
                       router<http::request, http::response>, ENty, TRty>;
};

template <typename RQty, typename RSty, typename ROty>
concept accepts_custom_http_server = requires {
  typename http_server<RQty, RSty, ROty, custom_engine, fake_transport>;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] asserts                                                    ( static ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
static_assert(
    protocol::contracts::engine<http::engine<http::request, http::response>>);
static_assert(protocol::contracts::completion_engine<
              http::engine<http::request, http::response>>);
static_assert(!protocol::contracts::completion_engine<custom_engine>);
static_assert(!std::constructible_from<
              http::engine<http::request, http::response>, http::policies>);
static_assert(accepts_server<http::engine<http::request, http::response>,
                             fake_transport>);
static_assert(
    accepts_server<http::engine<http::request, http::response>, tr::tcp>);
static_assert(!accepts_server<http::engine<http::request, http::response>,
                              bad_transport>);
static_assert(!accepts_server<http::engine<http::request, http::response>,
                              bad_start_transport>);
static_assert(!accepts_server<http::engine<http::request, http::response>,
                              bad_policies_transport>);
static_assert(!accepts_server<http::engine<http::request, http::response>,
                              private_policies_transport>);
static_assert(protocol::contracts::engine<bad_engine>);
static_assert(!accepts_server<bad_engine, fake_transport>);
static_assert(!accepts_server<int, fake_transport>);
static_assert(
    accepts_custom_http_server<http::request, http::response,
                               router<http::request, http::response>>);
static_assert(
    !accepts_custom_http_server<custom_request, http::response,
                                router<http::request, http::response>>);
static_assert(
    !accepts_custom_http_server<http::request, custom_response,
                                router<http::request, http::response>>);
static_assert(!accepts_custom_http_server<http::request, http::response,
                                          custom_routes>);

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] usings                                                     ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
namespace auth = martianlabs::doba::protocol::http::auth;
struct custom_auth {
  int& checks;
  auth::authentication_result authenticate(const http::request&) const {
    checks++;
    return {auth::authentication_status::kAccepted, {}};
  }
};
struct invalid_auth {
  bool authenticate(const http::request&) const { return true; }
};
static_assert(!auth::contracts::provider<invalid_auth, http::request>);
template <typename Aty>
concept registered_auth = requires(
    http_server<http::request, http::response,
                router<http::request, http::response>,
                http::engine<http::request, http::response>, fake_transport,
                auth::basic, auth::bearer, custom_auth>& server) {
  server.template add_route<Aty>("GET", "/",
      [](const http::request&, http::response&) {});
};
static_assert(registered_auth<custom_auth>);
static_assert(!registered_auth<invalid_auth>);
#ifdef DOBA_ENABLE_TLS
static_assert(
    std::constructible_from<
        https_server<http::request, http::response,
                     router<http::request, http::response>,
                     http::engine<http::request, http::response>, auth::bearer>,
        tr::tls_policies, http::policies, std::tuple<auth::bearer>>);
#endif

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] asserts                                                    ( static ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
static_assert(
    std::same_as<typename tr::tcp<
                     http::engine<http::request, http::response>,
                     http::engine_factory<
                         http::engine<http::request, http::response>,
                         router<http::request, http::response>>>::policies_type,
                 tr::tcp_policies>);
static_assert(std::constructible_from<
              http_server<http::request, http::response,
                          router<http::request, http::response>,
                          http::engine<http::request, http::response>,
                          fake_transport>,
              std::unique_ptr<int>>);
static_assert(std::constructible_from<
              http_server<http::request, http::response,
                          router<http::request, http::response>,
                          http::engine<http::request, http::response>,
                          fake_transport>,
              std::unique_ptr<int>, http::policies>);
static_assert(!std::constructible_from<
              http_server<http::request, http::response,
                          router<http::request, http::response>,
                          http::engine<http::request, http::response>,
                          fake_transport>,
              http::policies, std::unique_ptr<int>>);
static_assert(!std::constructible_from<
              http_server<http::request, http::response,
                          router<http::request, http::response>,
                          http::engine<http::request, http::response>,
                          fake_transport>,
              tr::tcp_policies, http::policies>);

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] controller                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct controller {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit controller(int& calls) : calls_(calls) {}
  // +=========================================================================+
  // | [>] register_routes                                          ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename ROty>
  void register_routes(ROty& routes) {
    routes.add("GET", "/", &controller::handle);
  }
  // +=========================================================================+
  // | [>] handle                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  void handle(const http::request&, http::response& res) {
    res.ok_200();
    res.set_body(std::to_string(++calls_));
    return;
  }
  // +=========================================================================+
  // | [>] ATTRIUBUTEs                                              ( public ) |
  // +-------------------------------------------------------------------------+
  int& calls_;
};
}  // namespace

// +===========================================================================+
// | [>] HTTP server accepts a custom engine                     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HTTP server accepts a custom engine") {
  observed = {};
  http_server<http::request, http::response,
              router<http::request, http::response>, custom_engine,
              fake_transport>
      value(std::make_unique<int>(5));
  value.start();
  DOBA_EXPECT_EQUAL(observed.starts, 1);
  DOBA_EXPECT_EQUAL(observed.configuration, 5);
  value.stop();
  DOBA_EXPECT_EQUAL(observed.stops, 1);
}

// +===========================================================================+
// | [>] server shares its router between engines                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("server shares its router between engines") {
  observed = {};
  int calls = 0;
  auto transport_configuration = std::make_unique<int>(3);
  {
    http_server<http::request, http::response,
                router<http::request, http::response>,
                http::engine<http::request, http::response>, fake_transport>
        value(std::move(transport_configuration));
    value.add_controller<controller>(calls);
    value.start();
    value.start();
    DOBA_EXPECT_EQUAL(observed.starts, 1);
    DOBA_EXPECT_EQUAL(calls, 2);
    DOBA_EXPECT_EQUAL(observed.configuration, 3);
    DOBA_EXPECT(observed.bytes.find("\r\n\r\n1HTTP/1.1") != std::string::npos);
    DOBA_EXPECT(observed.bytes.ends_with("\r\n\r\n2"));
    bool rejected = false;
    try {
      value.add_route(
          "GET", "/late",
          [](const http::request&, http::response& res) { res.ok_200(); });
    } catch (const std::runtime_error&) {
      rejected = true;
    }
    DOBA_EXPECT(rejected);
    rejected = false;
    try {
      value.add_controller<controller>(calls);
    } catch (const std::runtime_error&) {
      rejected = true;
    }
    DOBA_EXPECT(rejected);
    value.stop();
    value.stop();
    DOBA_EXPECT_EQUAL(observed.stops, 1);
    value.add_route(
        "GET", "/new",
        [](const http::request&, http::response& res) { res.ok_200(); });
    value.start();
    DOBA_EXPECT_EQUAL(calls, 4);
  }
  DOBA_EXPECT_EQUAL(observed.stops, 2);
}

// +===========================================================================+
// | [>] server recovers from startup failure                    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("server recovers from startup failure") {
  observed = {};
  observed.fail_start = true;
  http_server<http::request, http::response,
              router<http::request, http::response>,
              http::engine<http::request, http::response>, fake_transport>
      value({}, {.max_request_body_size = 1});
  bool failed = false;
  try {
    value.start();
  } catch (const std::runtime_error&) {
    failed = true;
  }
  DOBA_EXPECT(failed);
  value.add_route("GET", "/", [](const http::request&, http::response& res) {
    res.ok_200();
    res.set_body("ready");
    return;
  });
  observed.fail_start = false;
  value.start();
  DOBA_EXPECT(observed.bytes.ends_with("\r\n\r\nready"));
  value.stop();
  DOBA_EXPECT_EQUAL(observed.stops, 1);
}

// +===========================================================================+
// | [>] engine factories preserve policies and independence     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("engine factories preserve policies and independence") {
  router<http::request, http::response> routes;
  int calls = 0;
  routes.add("GET", "/first", [&](const http::request&, http::response& res) {
    calls++;
    res.ok_200();
    res.set_body("first");
    return;
  });
  http::policies configuration;
  configuration.max_request_body_size = 1;
  auto factory =
      http::make_engine_factory<http::engine<http::request, http::response>>(
          configuration, routes);
  configuration.max_request_body_size = 100;
  auto first = factory();
  auto second = factory();
  http::engine<http::request, http::response>* engines[] = {&first, &second};
  std::string bytes[2];
  int closed[2] = {};
  for (int i = 0; i < 2; i++) {
    engines[i]->set_on_send([&, i](std::string_view head, std::string_view body,
                                   std::unique_ptr<reader> source) {
      bytes[i].append(head);
      bytes[i].append(body);
      if (source) source->read_all(bytes[i]);
    });
    engines[i]->set_on_close([&, i]() { closed[i]++; });
  }
  const std::string partial = "GET /first HTTP/1.1\r\nHost: local";
  first.on_bytes_received(partial.data(), partial.size());
  const std::string complete = partial + "host\r\n\r\n";
  second.on_bytes_received(complete.data(), complete.size());
  DOBA_EXPECT(bytes[0].empty());
  DOBA_EXPECT(bytes[1].ends_with("\r\n\r\nfirst"));
  DOBA_EXPECT_EQUAL(calls, 1);
  first.on_bytes_received(complete.data() + partial.size(),
                          complete.size() - partial.size());
  DOBA_EXPECT_EQUAL(calls, 2);
  const std::string rejected =
      "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 2\r\n\r\n";
  for (int i = 0; i < 2; i++) {
    bytes[i].clear();
    engines[i]->on_bytes_received(rejected.data(), rejected.size());
    DOBA_EXPECT(bytes[i].starts_with("HTTP/1.1 413 "));
    DOBA_EXPECT_EQUAL(closed[i], 1);
  }
  DOBA_EXPECT_EQUAL(calls, 2);
}

// +===========================================================================+
// | [>] server forwards engine policies to every connection     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("server forwards engine policies to every connection") {
  observed = {};
  observed.request =
      "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 2\r\n\r\n";
  int calls = 0;
  http_server<http::request, http::response,
              router<http::request, http::response>,
              http::engine<http::request, http::response>, fake_transport>
      value(std::make_unique<int>(7), {.max_request_body_size = 1});
  value.add_route("GET", "/", [&](const http::request&, http::response& res) {
    calls++;
    res.ok_200();
  });
  value.start();
  DOBA_EXPECT_EQUAL(observed.configuration, 7);
  DOBA_EXPECT_EQUAL(calls, 0);
  const auto first = observed.bytes.find("HTTP/1.1 413 ");
  DOBA_EXPECT_EQUAL(first, 0);
  const auto second = observed.bytes.find("HTTP/1.1 413 ", 1);
  DOBA_EXPECT(second != std::string::npos);
  value.stop();
  DOBA_EXPECT_EQUAL(observed.stops, 1);
}

DOBA_TEST("server owns and selects authentication providers across engines") {
  observed = {};
  int checks = 0;
  http_server<http::request, http::response,
              router<http::request, http::response>,
              http::engine<http::request, http::response>, fake_transport,
              auth::basic, auth::bearer, custom_auth>
      value({}, {},
            std::tuple{auth::basic{"demo", [](auto, auto) { return false; }},
                       auth::bearer{"demo", [](auto) { return false; }},
                       custom_auth{checks}});
  value.add_route<custom_auth>("GET", "/",
      [](const http::request&, http::response& res) { res.ok_200(); });
  value.start();
  DOBA_EXPECT_EQUAL(checks, 2);
  bool rejected = false;
  try {
    value.add_route<custom_auth>("GET", "/late",
        [](const http::request&, http::response&) {});
  } catch (const std::runtime_error&) {
    rejected = true;
  }
  DOBA_EXPECT(rejected);
  value.stop();
  value.start();
  DOBA_EXPECT_EQUAL(checks, 4);
  value.stop();
}
