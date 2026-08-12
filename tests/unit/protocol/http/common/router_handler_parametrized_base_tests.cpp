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

#include <memory>

#include "protocol/http/common/router_handler_parametrized_base.h"
#include "test_helper.h"

namespace {
struct request {};
struct response {};
using martianlabs::doba::protocol::http::route_parameters;
using martianlabs::doba::protocol::http::router_handler_parametrized_base;

class handler final
    : public router_handler_parametrized_base<request, response> {
 public:
  bool matches(const route_parameters& parameters) const override {
    return parameters.size() == 1 && parameters[0].second == "value";
  }
  bool invoke(std::shared_ptr<const request> req, std::shared_ptr<response> res,
              const route_parameters& parameters) const override {
    return req != nullptr && res != nullptr && matches(parameters);
  }
};
}  // namespace

// +===========================================================================+
// | [>] interface dispatches matches and invoke                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("interface dispatches matches and invoke") {
  std::unique_ptr<router_handler_parametrized_base<request, response>> value =
      std::make_unique<handler>();
  const route_parameters empty;
  const route_parameters valid{{"name", "value"}};
  DOBA_EXPECT(!value->matches(empty));
  DOBA_EXPECT(value->matches(valid));
  DOBA_EXPECT(!value->invoke(nullptr, std::make_shared<response>(), valid));
  DOBA_EXPECT(value->invoke(std::make_shared<const request>(),
                            std::make_shared<response>(), valid));
}
