//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
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
#include <exception>
#include <memory>
#include <string>

#include "protocol/http/common/router_handler_parametrized_async.h"
#include "router_empty_request.h"
#include "router_value_response.h"
#include "test_helper.h"

namespace {
using request = martianlabs::doba::tests::unit::router_empty_request;
using response = martianlabs::doba::tests::unit::router_value_response;
using martianlabs::doba::common::task;
using martianlabs::doba::protocol::http::make_router_handler_parametrized_async;
using martianlabs::doba::protocol::http::router_handler_parametrized_async;
}  // namespace

DOBA_TEST("asynchronous parametrized handler matches and invokes") {
  auto handler = make_router_handler_parametrized_async<request, response, int>(
      "/items/:id", [](const request&, response& res, int id) -> task<void> {
        res.value = std::to_string(id);
        co_return;
      });
  static_assert(std::same_as<decltype(handler),
                             router_handler_parametrized_async<request,
                                                               response>>);
  DOBA_EXPECT(handler.matches("/items/42"));
  request req;
  response res;
  std::shared_ptr<void> parameters;
  auto work = handler.invoke(req, res, "/items/42", parameters);
  work.start([](std::exception_ptr error) { DOBA_EXPECT(!error); });
  DOBA_EXPECT_EQUAL(res.value, "42");
}
