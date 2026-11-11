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
#include <functional>

#include "common/task_execution.h"
#include "protocol/http/common/router_handler_static_async.h"
#include "router_empty_request.h"
#include "router_empty_response.h"
#include "test_helper.h"

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_static_async_tests                      ( test-suite ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
namespace {
using martianlabs::doba::common::task;
using martianlabs::doba::common::task_execution;
using martianlabs::doba::protocol::http::detail::router_handler_static_async;
using martianlabs::doba::tests::unit::router_empty_request;
using martianlabs::doba::tests::unit::router_empty_response;
}  // namespace

DOBA_TEST("asynchronous static alias accepts a coroutine") {
  static_assert(
      std::same_as<router_handler_static_async<router_empty_request,
                                               router_empty_response>,
                   std::function<task<void>(const router_empty_request&,
                                            router_empty_response&)>>);
  bool invoked = false;
  router_handler_static_async<router_empty_request, router_empty_response>
      handler = [&invoked](const router_empty_request&,
                           router_empty_response&) -> task<void> {
    invoked = true;
    co_return;
  };
  router_empty_request req;
  router_empty_response res;
  task_execution work(handler(req, res));
  DOBA_EXPECT(work.resume());
  DOBA_EXPECT(work.done());
  DOBA_EXPECT(!work.error());
  DOBA_EXPECT(invoked);
}
