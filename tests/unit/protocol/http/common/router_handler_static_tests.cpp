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
#include <stop_token>
#include <type_traits>

#include "protocol/http/common/router_handler_static.h"
#include "test_helper.h"

namespace {
struct request {};
struct response {};
using martianlabs::doba::common::task;
using martianlabs::doba::protocol::http::router_async_handler_lambda;
using martianlabs::doba::protocol::http::router_handler_lambda;
using martianlabs::doba::protocol::http::router_handler_static;
}  // namespace

// +===========================================================================+
// | [>] alias accepts and invokes the documented callback       ( test-case ) |
// +===========================================================================+
DOBA_TEST("alias accepts and invokes the documented callback") {
  static_assert(
      std::same_as<router_handler_static<request, response>,
                   std::function<response(const request&)>>);
  bool invoked = false;
  router_handler_static<request, response> handler =
      [&invoked](const request&) {
        response res;
        invoked = true;
        return res;
      };
  request req;
  response res;
  res = handler(req);
  DOBA_EXPECT(invoked);
}
// +===========================================================================+
// | [>] handler concepts distinguish sync and async callbacks   ( test-case ) |
// +===========================================================================+
DOBA_TEST("handler concepts distinguish sync and async callbacks") {
  auto sync = [](const request&) {
    response res;
    return res;
  };
  auto async = [](std::shared_ptr<const request>,
                  std::stop_token) -> task<response> {
    co_return response{};
  };
  auto legacy = [](const request&, response&) {};
  static_assert(!router_handler_lambda<decltype(legacy)>);
  static_assert(router_handler_lambda<decltype(sync)>);
  static_assert(!router_async_handler_lambda<decltype(sync)>);
  static_assert(!router_handler_lambda<decltype(async)>);
  static_assert(router_async_handler_lambda<decltype(async)>);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] mutable handler signatures retain their argument count  ( test-case ) |
// +===========================================================================+
DOBA_TEST("mutable handler signatures retain their argument count") {
  auto sync = [](const request&, int) mutable {
    return response{};
  };
  auto async = [](std::shared_ptr<const request>,
                           std::stop_token, int) mutable -> task<response> {
    co_return response{};
  };
  using namespace martianlabs::doba::protocol::http;
  static_assert(router_handler_lambda<decltype(sync)>);
  static_assert(router_async_handler_lambda<decltype(async)>);
  DOBA_EXPECT_EQUAL(
      router_handler_signature<decltype(&decltype(sync)::operator())>::
          parameter_count, 1);
  DOBA_EXPECT_EQUAL(
      router_async_handler_signature<decltype(&decltype(async)::operator())>::
          parameter_count, 1);
}
// +===========================================================================+
// | [>] noexcept sync handlers retain signature compatibility   ( test-case ) |
// +===========================================================================+
DOBA_TEST("noexcept sync handlers retain signature compatibility") {
  auto handler = [](const request&) noexcept {
    return response{};
  };
  DOBA_EXPECT(router_handler_lambda<decltype(handler)>);
}
// +===========================================================================+
// | [>] noexcept async handlers retain signature compatibility  ( test-case ) |
// +===========================================================================+
DOBA_TEST("noexcept async handlers retain signature compatibility") {
  auto handler = [](std::shared_ptr<const request>,
                     std::stop_token) noexcept -> task<response> {
    co_return response{};
  };
  DOBA_EXPECT(router_async_handler_lambda<decltype(handler)>);
}
// +===========================================================================+
// | [>] sync signatures reject mutable and value requests       ( test-case ) |
// +===========================================================================+
DOBA_TEST("sync signatures reject mutable and value requests") {
  auto mutable_request = [](request&) { return response{}; };
  auto value_request = [](request) { return response{}; };
  auto rvalue_request = [](const request&&) { return response{}; };
  auto no_result = [](const request&) {};
  DOBA_EXPECT(!router_handler_lambda<decltype(mutable_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(value_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(rvalue_request)>);
  DOBA_EXPECT(!router_handler_lambda<decltype(no_result)>);
}
