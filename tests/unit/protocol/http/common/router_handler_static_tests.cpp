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
                   std::function<void(const request&, response&)>>);
  bool invoked = false;
  router_handler_static<request, response> handler =
      [&invoked](const request&, response&) { invoked = true; };
  request req;
  response res;
  handler(req, res);
  DOBA_EXPECT(invoked);
}
// +===========================================================================+
// | [>] handler concepts distinguish sync and async callbacks   ( test-case ) |
// +===========================================================================+
DOBA_TEST("handler concepts distinguish sync and async callbacks") {
  auto sync = [](const request&, response&) {};
  auto async = [](std::shared_ptr<const request>,
                  std::stop_token) -> task<response> {
    co_return response{};
  };
  static_assert(router_handler_lambda<decltype(sync)>);
  static_assert(!router_async_handler_lambda<decltype(sync)>);
  static_assert(!router_handler_lambda<decltype(async)>);
  static_assert(router_async_handler_lambda<decltype(async)>);
  DOBA_EXPECT(true);
}
