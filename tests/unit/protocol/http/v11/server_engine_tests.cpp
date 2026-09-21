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
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "protocol/http/v11/server.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::reader;
namespace protocol = martianlabs::doba::protocol;
namespace http = martianlabs::doba::protocol::http::v11;
namespace tr = martianlabs::doba::transport::server;
using routes_type =
    martianlabs::doba::protocol::http::router<http::request, http::response>;
using engine_type = http::engine<http::request, http::response>;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] observation                                                ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct observation {
  int starts{0};
  int stops{0};
  bool fail_start{false};
  std::string bytes;
  int configuration{0};
} observed;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] fake_transport                                             ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ENty, typename FNty>
struct fake_transport {
  using policies_type = std::unique_ptr<int>;
  fake_transport(policies_type configuration, FNty create_engine)
      : factory(std::move(create_engine)) {
    observed.configuration = configuration ? *configuration : 0;
  }
  void set_on_connection(std::function<void()>) {}
  void set_on_disconnection(std::function<void()>) {}
  void start() {
    if (observed.fail_start) throw std::runtime_error("start failed");
    observed.starts++;
    const std::string request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    for (int i = 0; i < 2; i++) {
      auto value = factory();
      value.set_on_send([](std::unique_ptr<char[]> bytes, std::size_t size,
                           std::optional<reader> source) {
        observed.bytes.append(bytes.get(), size);
        if (source) source->read_all(observed.bytes);
      });
      value.set_on_close([]() {});
      value.on_bytes_received(request.data(), request.size(), 4096);
    }
  }
  void stop() { observed.stops++; }
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
  using policies_type = tr::policies;
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
struct bad_engine : engine_type {
  explicit bad_engine(http::policies);
};

template <typename ENty, template <typename, typename> class TRty>
concept accepts_server = requires {
  typename http::server<http::request, http::response, routes_type, ENty, TRty>;
};
static_assert(protocol::contracts::engine<engine_type>);
static_assert(!std::constructible_from<engine_type, http::policies>);
static_assert(accepts_server<engine_type, fake_transport>);
static_assert(accepts_server<engine_type, tr::tcpip>);
static_assert(!accepts_server<engine_type, bad_transport>);
static_assert(!accepts_server<engine_type, bad_start_transport>);
static_assert(!accepts_server<engine_type, bad_policies_transport>);
static_assert(!accepts_server<engine_type, private_policies_transport>);
static_assert(protocol::contracts::engine<bad_engine>);
static_assert(!accepts_server<bad_engine, fake_transport>);
static_assert(!accepts_server<int, fake_transport>);

using server_type = http::server<http::request, http::response, routes_type,
                                 engine_type, fake_transport>;
using factory_type = http::engine_factory<engine_type, routes_type>;
static_assert(std::same_as<
              typename tr::tcpip<engine_type, factory_type>::policies_type,
              tr::policies>);
static_assert(std::constructible_from<server_type, std::unique_ptr<int>>);
static_assert(std::constructible_from<server_type, std::unique_ptr<int>,
                                      http::policies>);
static_assert(!std::constructible_from<server_type, http::policies,
                                       std::unique_ptr<int>>);
static_assert(!std::constructible_from<server_type, tr::policies,
                                       http::policies>);
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] controller                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct controller {
  explicit controller(int& calls) : calls_(calls) {}
  template <typename ROty>
  void register_routes(ROty& routes) {
    routes.add("GET", "/", &controller::handle);
  }
  http::response handle(const http::request&) {
    auto response = http::response::ok_200();
    response.set_body(std::to_string(++calls_));
    return response;
  }
  int& calls_;
};
}  // namespace

// +===========================================================================+
// | [>] server shares its router between engines                ( test-case ) |
// +===========================================================================+
DOBA_TEST("server shares its router between engines") {
  observed = {};
  int calls = 0;
  auto transport_configuration = std::make_unique<int>(3);
  {
    server_type value(std::move(transport_configuration));
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
      value.add_route("GET", "/late", [](const http::request&) {
        return http::response::ok_200();
      });
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
    value.add_route("GET", "/new", [](const http::request&) {
      return http::response::ok_200();
    });
    value.start();
    DOBA_EXPECT_EQUAL(calls, 4);
  }
  DOBA_EXPECT_EQUAL(observed.stops, 2);
}
// +===========================================================================+
// | [>] server recovers from startup failure                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("server recovers from startup failure") {
  observed = {};
  observed.fail_start = true;
  server_type value({}, {.max_uri_length = 1});
  bool failed = false;
  try {
    value.start();
  } catch (const std::runtime_error&) {
    failed = true;
  }
  DOBA_EXPECT(failed);
  value.add_route("GET", "/", [](const http::request&) {
    auto response = http::response::ok_200();
    response.set_body("ready");
    return response;
  });
  observed.fail_start = false;
  value.start();
  DOBA_EXPECT(observed.bytes.ends_with("\r\n\r\nready"));
  value.stop();
  DOBA_EXPECT_EQUAL(observed.stops, 1);
}
