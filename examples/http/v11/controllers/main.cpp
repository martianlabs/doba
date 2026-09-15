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

#include <atomic>
#include <memory>
#include <stop_token>
#include <string>
#include <utility>

#include "common/console_logger.h"
#include "common/logo.h"
#include "common/signaler.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/v11/server.h"

using namespace martianlabs::doba::common;
using namespace martianlabs::doba::protocol::http;
using namespace martianlabs::doba::protocol::http::v11;

// +---------------------------------------------------------------------------+
// | [>] counter_controller                                          ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] counter_controller                                          ( class ) |
// +---------------------------------------------------------------------------+
// | Controller implementation.                                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class counter_controller {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+

  explicit counter_controller(std::string prefix)
      : prefix_(std::move(prefix)) {}
  // +=========================================================================+
  // | [>] register_routes                                          ( public ) |
  // +=========================================================================+

  template <typename Rty>
  void register_routes(Rty& routes) {
    routes.add(method_names::kGet, prefix_ + "/count",
               &counter_controller::count);
    routes.add(method_names::kGet, prefix_ + "/echo/:id",
               &counter_controller::echo);
    routes.add(method_names::kGet, prefix_ + "/async/:id",
               &counter_controller::async_echo);
  }

 private:
  // +=========================================================================+
  // | [>] count                                                   ( private ) |
  // +=========================================================================+

  response count(const request&) {
    auto result = response::ok_200();
    result.set_body(++count_);
    return result;
  }
  // +=========================================================================+
  // | [>] echo                                                    ( private ) |
  // +=========================================================================+

  response echo(const request&, int id) const {
    auto result = response::ok_200();
    result.set_body(id);
    return result;
  }
  // +=========================================================================+
  // | [>] async_echo                                              ( private ) |
  // +=========================================================================+

  task<response> async_echo(std::shared_ptr<const request>,
                            std::stop_token token, int id) const {
    auto result = response::ok_200();
    if (!token.stop_requested()) result.set_body(id);
    co_return result;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+

  std::string prefix_;
  std::atomic<unsigned int> count_{0};
};

int main() {
  server http_server;
  http_server.add_controller<counter_controller>("/first")
      .add_controller<counter_controller>("/second");
  http_server.start("8080");
  signaler::wait();
  http_server.stop();
  return 0;
}
