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

#include <stdexcept>

#include "common/execution_policy.h"
#include "common/thread_pool.h"
#include "protocol/http/common/router_handler_parametrized.h"
#include "protocol/http/common/router_handler_static.h"
#include "protocol/http/common/router_types.h"

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
  // | [>] start                                                    ( public ) |
  // +=========================================================================+
  void start() { thread_pool_.start(); }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +=========================================================================+
  void stop() { thread_pool_.stop(); }
  // +=========================================================================+
  // | [>] add                                                      ( public ) |
  // +=========================================================================+
  template <router_handler_lambda Hty>
  void add(std::string_view method, std::string_view route, Hty handler,
           common::execution_policy policy =
               common::execution_policy::kSynchronous) {
    perform_checks<Hty>();
    const auto wildcard_position = route.find('*');
    const bool is_wildcard =
        wildcard_position != std::string_view::npos &&
        wildcard_position == route.size() - 1 && wildcard_position > 0 &&
        route[wildcard_position - 1] == '/' &&
        route.find('*', wildcard_position + 1) == std::string_view::npos;
    if (wildcard_position != std::string_view::npos && !is_wildcard) {
      throw std::invalid_argument(
          "The wildcard must be the final route segment");
    }
    // Let's check (at compile time) if the handler has parameters or not, and
    // add it to the appropriate list of handlers
    using signature =
        router_handler_signature<decltype(&std::decay_t<Hty>::operator())>;
    if constexpr (signature::parameter_count == 0) {
      if (is_wildcard) {
        add_wildcard_route(method, route, std::move(handler), policy);
      } else {
        add_static_route(method, route, std::move(handler), policy);
      }
      return;
    }
    if (is_wildcard) {
      throw std::invalid_argument(
          "The wildcard route handler cannot have typed parameters");
    }
    add_parametrized_route(method, route, std::move(handler), policy);
  }
  // +=========================================================================+
  // | [>] match                                                    ( public ) |
  // +=========================================================================+
  template <typename FNty>
  [[nodiscard]]
  router_match_result match(std::string_view method, std::string_view path,
                            std::shared_ptr<const RQty> req,
                            std::shared_ptr<RSty> res, const FNty& on_send) {
    // Let's check if we have a [static] route match for this method
    for (const auto& [static_method, handlers] : h_static_) {
      if (static_method != method) continue;
      for (const auto& handler_data : handlers) {
        if (handler_data.path == path) {
          switch (handler_data.policy) {
            case common::execution_policy::kSynchronous:
              handler_data.handler(req, res);
              on_send(res);
              break;
            case common::execution_policy::kAsynchronous:
              thread_pool_.enqueue([handler = handler_data.handler,
                                    req = std::move(req), res = std::move(res),
                                    on_send] {
                handler(req, res);
                on_send(res);
              });
              break;
          }
          return router_match_result::kMatched;
        }
      }
    }
    // Let's check if we have a [parametrized] route match for this method
    for (const auto& [param_method, handlers] : h_parametrized_) {
      if (param_method != method) continue;
      for (const auto& handler_data : handlers) {
        route_parameters parameters;
        if (helpers::get_parameters(handler_data.path, path, parameters)) {
          switch (handler_data.policy) {
            case common::execution_policy::kSynchronous:
              if (!handler_data.handler->invoke(req, res, parameters)) {
                // The handler did not match the parameters, let's continue
                // searching..
                continue;
              }
              on_send(res);
              break;
            case common::execution_policy::kAsynchronous:
              thread_pool_.enqueue([handler = handler_data.handler,
                                    parameters = std::move(parameters),
                                    req = std::move(req), res = std::move(res),
                                    on_send] {
                if (handler->invoke(req, res, parameters)) {
                  // The handler matched the parameters, let's send the
                  // response!
                  on_send(res);
                }
              });
              break;
          }
          return router_match_result::kMatched;
        }
      }
    }
    // Let's check if we have a [wildcard] route match for this method
    for (const auto& [wildcard_method, handlers] : h_wildcard_) {
      if (wildcard_method != method) continue;
      for (const auto& handler_data : handlers) {
        if (matches_wildcard_route(handler_data.path, path)) {
          switch (handler_data.policy) {
            case common::execution_policy::kSynchronous:
              handler_data.handler(req, res);
              on_send(res);
              break;
            case common::execution_policy::kAsynchronous:
              thread_pool_.enqueue([handler = handler_data.handler,
                                    req = std::move(req), res = std::move(res),
                                    on_send] {
                handler(req, res);
                on_send(res);
              });
              break;
          }
          return router_match_result::kMatched;
        }
      }
    }
    // Let's build [allow] only after a route match for this method has failed
    std::vector<std::string_view> allowed_methods;
    auto add_allowed_method =
        [&allowed_methods](std::string_view allowed_method) {
          for (const auto registered_method : allowed_methods) {
            if (registered_method == allowed_method) return;
          }
          allowed_methods.push_back(allowed_method);
        };
    for (const auto& [static_method, handlers] : h_static_) {
      if (static_method == method) continue;
      for (const auto& handler_data : handlers) {
        if (handler_data.path == path) add_allowed_method(static_method);
      }
    }
    for (const auto& [param_method, handlers] : h_parametrized_) {
      if (param_method == method) continue;
      for (const auto& handler_data : handlers) {
        route_parameters parameters;
        if (helpers::get_parameters(handler_data.path, path, parameters) &&
            handler_data.handler->matches(parameters)) {
          add_allowed_method(param_method);
        }
      }
    }
    for (const auto& [wildcard_method, handlers] : h_wildcard_) {
      if (wildcard_method == method) continue;
      for (const auto& handler_data : handlers) {
        if (matches_wildcard_route(handler_data.path, path)) {
          add_allowed_method(wildcard_method);
        }
      }
    }
    if (allowed_methods.empty()) return router_match_result::kNotFound;
    std::string allow;
    for (const auto allowed_method : allowed_methods) {
      if (!allow.empty()) allow += ", ";
      allow += allowed_method;
    }
    res->set_header(header_names::kAllow, allow);
    return router_match_result::kMethodNotAllowed;
  }

 private:
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  struct static_handler_data {
    std::string path;
    router_handler_static<RQty, RSty> handler;
    common::execution_policy policy;
  };
  struct parametrized_handler_data {
    std::string path;
    std::shared_ptr<router_handler_parametrized_base<RQty, RSty>> handler;
    common::execution_policy policy;
  };
  using static_handler_pair =
      std::pair<std::string, std::vector<static_handler_data>>;
  using wildcard_handler_pair =
      std::pair<std::string, std::vector<static_handler_data>>;
  using parametrized_handler_pair =
      std::pair<std::string, std::vector<parametrized_handler_data>>;
  // +=========================================================================+
  // | [>] add_static_route                                        ( private ) |
  // +=========================================================================+
  template <router_handler_lambda Hty>
  void add_static_route(std::string_view method, std::string_view route,
                        Hty handler, common::execution_policy policy) {
    router_handler_static<RQty, RSty> static_handler(std::move(handler));
    static_handler_data data{std::string(route), std::move(static_handler),
                             policy};
    for (auto& [static_method, handlers] : h_static_) {
      if (static_method == method) {
        handlers.push_back(std::move(data));
        return;
      }
    }
    h_static_.push_back({std::string(method), {std::move(data)}});
  }
  // +=========================================================================+
  // | [>] add_parametrized_route                                  ( private ) |
  // +=========================================================================+
  template <router_handler_lambda Hty>
  void add_parametrized_route(std::string_view method, std::string_view route,
                              Hty handler, common::execution_policy policy) {
    using signature =
        router_handler_signature<decltype(&std::decay_t<Hty>::operator())>;
    auto handler_parametrized =
        signature::template make_parametrized<RQty, RSty>(std::move(handler));
    parametrized_handler_data data{
        std::string(route),
        std::make_shared<decltype(handler_parametrized)>(
            std::move(handler_parametrized)),
        policy};
    for (auto& [param_method, handlers] : h_parametrized_) {
      if (param_method == method) {
        handlers.push_back(std::move(data));
        return;
      }
    }
    h_parametrized_.push_back({std::string(method), {std::move(data)}});
  }
  // +=========================================================================+
  // | [>] add_wildcard_route                                      ( private ) |
  // +=========================================================================+
  template <router_handler_lambda Hty>
  void add_wildcard_route(std::string_view method, std::string_view route,
                          Hty handler, common::execution_policy policy) {
    router_handler_static<RQty, RSty> static_handler(std::move(handler));
    static_handler_data data{std::string(route), std::move(static_handler),
                             policy};
    for (auto& [wildcard_method, handlers] : h_wildcard_) {
      if (wildcard_method == method) {
        handlers.push_back(std::move(data));
        return;
      }
    }
    h_wildcard_.push_back({std::string(method), {std::move(data)}});
  }
  // +=========================================================================+
  // | [>] perform_checks                                          ( private ) |
  // +=========================================================================+
  template <router_handler_lambda Hty>
  void perform_checks() {
    using signature =
        router_handler_signature<decltype(&std::decay_t<Hty>::operator())>;
    // Let's ensure that the handler has the correct signature!
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
  }
  // +=========================================================================+
  // | [>] match [wildcard-routes]                                 ( private ) |
  // +=========================================================================+
  static bool matches_wildcard_route(std::string_view ro, std::string_view pa) {
    return pa.starts_with(ro.substr(0, ro.size() - 1));
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::vector<static_handler_pair> h_static_;
  std::vector<parametrized_handler_pair> h_parametrized_;
  std::vector<wildcard_handler_pair> h_wildcard_;
  common::thread_pool thread_pool_;
};
}  // namespace martianlabs::doba::protocol::http

#endif
