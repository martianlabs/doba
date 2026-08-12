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
#include <type_traits>

#include "protocol/http/common/router_handler_static.h"
#include "test_helper.h"

namespace {
struct request {};
struct response {};
using martianlabs::doba::protocol::http::router_handler_static;
}  // namespace

// +===========================================================================+
// | [>] alias accepts and invokes the documented callback       ( test-case ) |
// +===========================================================================+
DOBA_TEST("alias accepts and invokes the documented callback") {
  using expected = std::function<void(std::shared_ptr<const request>,
                                      std::shared_ptr<response>)>;
  static_assert(
      std::same_as<router_handler_static<request, response>, expected>);
  bool invoked = false;
  router_handler_static<request, response> handler =
      [&invoked](std::shared_ptr<const request> req,
                 std::shared_ptr<response> res) {
        invoked = req != nullptr && res != nullptr;
      };
  handler(std::make_shared<const request>(), std::make_shared<response>());
  DOBA_EXPECT(invoked);
}
