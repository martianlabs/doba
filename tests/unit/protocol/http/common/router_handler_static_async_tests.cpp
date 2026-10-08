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
#include <functional>

#include "protocol/http/common/router_handler_static_async.h"
#include "router_empty_request.h"
#include "router_empty_response.h"
#include "test_helper.h"

namespace {
using request = martianlabs::doba::tests::unit::router_empty_request;
using response = martianlabs::doba::tests::unit::router_empty_response;
using martianlabs::doba::protocol::http::router_handler_static_async;
using martianlabs::doba::common::task;
}  // namespace

DOBA_TEST("asynchronous static alias accepts a coroutine") {
  static_assert(std::same_as<router_handler_static_async<request, response>,
                             std::function<task<void>(const request&,
                                                      response&)>>);
  bool invoked = false;
  router_handler_static_async<request, response> handler =
      [&invoked](const request&, response&) -> task<void> {
    invoked = true;
    co_return;
  };
  request req;
  response res;
  auto work = handler(req, res);
  work.start([](std::exception_ptr error) { DOBA_EXPECT(!error); });
  DOBA_EXPECT(invoked);
}
