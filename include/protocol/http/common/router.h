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

#ifndef martianlabs_doba_protocol_http_router_h
#define martianlabs_doba_protocol_http_router_h

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "common/execution_policy.h"
#include "protocol/http/common/router_handler_static.h"

namespace martianlabs::doba::protocol::http {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | This class holds for the http 1.1 router implementation.                  |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used                                               |
// |   RSty - response being used                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty>
class router {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  struct handler_data {
    router_handler_static<RQty, RSty> callback;
    common::execution_policy policy;
  };
  struct route_match {
    const handler_data* handler{nullptr};
    [[nodiscard]] explicit operator bool() const {
      return handler != nullptr;
    }
  };
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  router() = default;
  router(const router&) = delete;
  router(router&&) noexcept = delete;
  ~router() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  router& operator=(const router&) = delete;
  router& operator=(router&&) noexcept = delete;
  // +=========================================================================+
  // | [>] add                                                      ( public ) |
  // +=========================================================================+
  template <router_handler_lambda Hty>
  void add(std::string_view method, std::string_view route, Hty handler,
           common::execution_policy policy =
               common::execution_policy::kSynchronous) {
    perform_checks<Hty>();
    route_data data{
        std::string(route),
        {router_handler_static<RQty, RSty>(std::move(handler)), policy}};
    for (auto& [static_method, handlers] : handlers_) {
      if (static_method == method) {
        handlers.push_back(std::move(data));
        return;
      }
    }
    handlers_.push_back({std::string(method), {std::move(data)}});
  }
  // +=========================================================================+
  // | [>] match                                                    ( public ) |
  // +=========================================================================+
  [[nodiscard]]
  route_match match(std::string_view method, std::string_view path) const {
    for (const auto& [static_method, handlers] : handlers_) {
      if (static_method != method) continue;
      for (const auto& route : handlers) {
        if (route.path == path) return {&route.handler};
      }
    }
    return {};
  }
  // +=========================================================================+
  // | [>] allowed_methods                                          ( public ) |
  // +=========================================================================+
  [[nodiscard]]
  std::string allowed_methods(std::string_view path) const {
    std::string methods;
    for (const auto& [method, handlers] : handlers_) {
      for (const auto& route : handlers) {
        if (route.path != path) continue;
        if (!methods.empty()) methods += ", ";
        methods += method;
        break;
      }
    }
    return methods;
  }

 private:
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  struct route_data {
    std::string path;
    handler_data handler;
  };
  using handler_pair =
      std::pair<std::string, std::vector<route_data>>;
  // +=========================================================================+
  // | [>] perform_checks                                          ( private ) |
  // +=========================================================================+
  template <router_handler_lambda Hty>
  static void perform_checks() {
    using signature =
        router_handler_signature<decltype(&std::decay_t<Hty>::operator())>;
    static_assert(std::same_as<typename signature::return_type, void>,
                  "The route handler must return void");
    static_assert(
        std::same_as<std::decay_t<typename signature::request_type>,
                     std::shared_ptr<const RQty>>,
        "The first route handler argument must be std::shared_ptr<const RQty>");
    static_assert(
        std::same_as<std::decay_t<typename signature::response_type>,
                     std::shared_ptr<RSty>>,
        "The second route handler argument must be std::shared_ptr<RSty>");
    static_assert(signature::parameter_count == 0,
                  "Static route handlers accept exactly two arguments");
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::vector<handler_pair> handlers_;
};
}  // namespace martianlabs::doba::protocol::http

#endif
