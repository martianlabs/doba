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
#include <exception>
#include <functional>

#include "protocol/http/common/router_handler_static.h"
#include "router_empty_request.h"
#include "router_empty_response.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::task;
using martianlabs::doba::protocol::http::router_handler_static;
using martianlabs::doba::tests::unit::router_empty_request;
using martianlabs::doba::tests::unit::router_empty_response;
}  // namespace

// +===========================================================================+
// | [>] alias accepts and invokes the documented callback       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("alias accepts and invokes the documented callback") {
  static_assert(
      std::same_as<
          router_handler_static<router_empty_request, router_empty_response>,
          std::function<task<void>(const router_empty_request&,
                             router_empty_response&)>>);
  bool invoked = false;
  router_handler_static<router_empty_request, router_empty_response> handler =
      [&invoked](const router_empty_request&, router_empty_response&)
          -> task<void> {
        invoked = true;
        co_return;
      };
  router_empty_request req;
  router_empty_response res;
  bool completed = false;
  auto work = handler(req, res);
  auto completion = [&](std::exception_ptr error) {
    DOBA_EXPECT(!error);
    completed = true;
  };
  work.start(&completion, [](void* context, std::exception_ptr error) {
    (*static_cast<decltype(completion)*>(context))(error);
  });
  DOBA_EXPECT(completed);
  DOBA_EXPECT(invoked);
}
