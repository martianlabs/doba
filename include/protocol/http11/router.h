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

#ifndef martianlabs_doba_protocol_http11_router_h
#define martianlabs_doba_protocol_http11_router_h

#include "protocol/http11/router_handler_parametrized.h"
#include "protocol/http11/router_handler_static.h"
#include "protocol/http11/router_types.h"

namespace martianlabs::doba::protocol::http11 {
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
  // | [>] add [lambda-handlers]                                     ( public ) |
  // +=========================================================================+
  template <detail::router_handler_lambda Hty>
  void add_route(std::string_view method, std::string_view route, Hty handler) {
    using signature = detail::router_handler_signature<
        decltype(&std::decay_t<Hty>::operator())>;
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
    if constexpr (signature::parameter_count == 0) {
      add_route(method, route,
                router_handler_static<RQty, RSty>(std::move(handler)));
    } else {
      auto handler_parametrized =
          signature::template make_parametrized<RQty, RSty>(
              std::move(handler));
      parametrized_handler_data data{
          std::string(route),
          std::make_shared<decltype(handler_parametrized)>(
              std::move(handler_parametrized))};
      for (auto& [param_method, handlers] : parametrized_handlers_) {
        if (param_method == method) {
          handlers.push_back(std::move(data));
          return;
        }
      }
      parametrized_handlers_.push_back(
          {std::string(method), {std::move(data)}});
    }
  }
  // +=========================================================================+
  // | [>] add [static-handlers]                                    ( public ) |
  // +=========================================================================+
  void add_route(std::string_view method, std::string_view route,
                 router_handler_static<RQty, RSty> handler) {
    static_handler_data data{std::string(route), std::move(handler)};
    for (auto& [static_method, handlers] : static_handlers_) {
      if (static_method == method) {
        handlers.push_back(std::move(data));
        return;
      }
    }
    static_handlers_.push_back({std::string(method), {std::move(data)}});
  }
  // +=========================================================================+
  // | [>] match_route                                              ( public ) |
  // +=========================================================================+
  [[nodiscard]]
  router_match_result match_route(std::string_view method,
                                  std::string_view path,
                                  std::shared_ptr<const RQty> req,
                                  std::shared_ptr<RSty> res) {
    bool path_exists = false;  // Flag to track if path exists for any method
    // -------------------------------------------------------------------------
    // Let's first check if we have a [static] route match
    // -------------------------------------------------------------------------
    for (const auto& [static_method, handlers] : static_handlers_) {
      for (const auto& handler_data : handlers) {
        // case insensitive comparison for path
        if (helpers::iequals(handler_data.path, path)) {
          path_exists = true;
          // case sensitive comparison for method
          if (static_method == method) {
            handler_data.handler(req, res);
            return router_match_result::kMatched;
          }
        }
      }
    }
    // -------------------------------------------------------------------------
    // Let's second check if we have a [parametrized] route match
    // -------------------------------------------------------------------------
    for (const auto& [param_method, handlers] : parametrized_handlers_) {
      for (const auto& handler_data : handlers) {
        // case sensitive comparison for method
        if (param_method == method) {
          std::vector<std::pair<std::string_view, std::string_view>> parameters;
          if (helpers::get_parameters(handler_data.path, path, parameters)) {
            if (handler_data.handler->invoke(req, res, parameters)) {
              return router_match_result::kMatched;
            }
          }
        }
      }
    }
    return path_exists ? router_match_result::kMethodNotAllowed
                       : router_match_result::kNotFound;
  }

 private:
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  struct static_handler_data {
    std::string path;
    router_handler_static<RQty, RSty> handler;
  };
  struct parametrized_handler_data {
    std::string path;
    std::shared_ptr<router_handler_parametrized_base<RQty, RSty>> handler;
  };
  using static_handler_pair =
      std::pair<std::string, std::vector<static_handler_data>>;
  using parametrized_handler_pair =
      std::pair<std::string, std::vector<parametrized_handler_data>>;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  std::vector<static_handler_pair> static_handlers_;
  std::vector<parametrized_handler_pair> parametrized_handlers_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
