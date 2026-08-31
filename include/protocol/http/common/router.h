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
#include <stdexcept>
#include <utility>
#include <vector>

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
  };
  struct route_match {
    const handler_data* handler{nullptr};
    const router_handler_parametrized<RQty, RSty>* parametrized_handler{
        nullptr};
    [[nodiscard]] explicit operator bool() const {
      return handler != nullptr || parametrized_handler != nullptr;
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
  void add(std::string_view method, std::string_view route, Hty handler) {
    perform_checks<Hty>();
    using signature =
        router_handler_signature<decltype(&std::decay_t<Hty>::operator())>;
    const std::size_t parameter_count = count_parameters(route);
    if constexpr (signature::parameter_count == 0) {
      if (parameter_count != 0) {
        throw std::invalid_argument(
            "The route parameters and handler arguments do not match");
      }
      route_data data{
          std::string(route),
          {router_handler_static<RQty, RSty>(std::move(handler))}};
      for (auto& [static_method, handlers] : handlers_) {
        if (static_method == method) {
          handlers.push_back(std::move(data));
          return;
        }
      }
      handlers_.push_back({std::string(method), {std::move(data)}});
    } else {
      if (parameter_count != signature::parameter_count) {
        throw std::invalid_argument(
            "The route parameters and handler arguments do not match");
      }
      auto data = signature::template make_parametrized<RQty, RSty>(
          route, std::move(handler));
      for (auto& [parametrized_method, handlers] : parametrized_handlers_) {
        if (parametrized_method == method) {
          handlers.push_back(std::move(data));
          return;
        }
      }
      parametrized_handlers_.push_back(
          {std::string(method), {std::move(data)}});
    }
  }
  // +=========================================================================+
  // | [>] match                                                    ( public ) |
  // +=========================================================================+
  [[nodiscard]]
  route_match match(std::string_view method, std::string_view path) const {
    for (const auto& [static_method, handlers] : handlers_) {
      if (static_method != method) continue;
      for (const auto& route : handlers) {
        if (route.path == path) return {&route.handler, nullptr};
      }
    }
    for (const auto& [parametrized_method, handlers] :
         parametrized_handlers_) {
      if (parametrized_method != method) continue;
      for (const auto& handler : handlers) {
        if (handler.matches(path)) return {nullptr, &handler};
      }
    }
    return {};
  }
  // +=========================================================================+
  // | [>] allowed_methods                                          ( public ) |
  // +=========================================================================+
  [[nodiscard]]
  std::string allowed_methods(std::string_view path) const {
    std::vector<std::string_view> allowed;
    for (const auto& [method, handlers] : handlers_) {
      for (const auto& route : handlers) {
        if (route.path != path) continue;
        allowed.push_back(method);
        break;
      }
    }
    for (const auto& [method, handlers] : parametrized_handlers_) {
      bool found = false;
      for (const auto allowed_method : allowed) {
        if (allowed_method == method) {
          found = true;
          break;
        }
      }
      if (found) continue;
      for (const auto& handler : handlers) {
        if (!handler.matches(path)) continue;
        allowed.push_back(method);
        break;
      }
    }
    std::string methods;
    for (const auto method : allowed) {
      if (!methods.empty()) methods += ", ";
      methods += method;
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
  using parametrized_handler_pair =
      std::pair<std::string,
                std::vector<router_handler_parametrized<RQty, RSty>>>;
  // +=========================================================================+
  // | [>] count_parameters                                      ( private ) |
  // +=========================================================================+
  static std::size_t count_parameters(std::string_view route) {
    std::size_t count = 0;
    std::size_t pos = 0;
    for (;;) {
      const std::size_t end = route.find('/', pos);
      const std::string_view segment = route.substr(
          pos, end == std::string_view::npos ? route.size() - pos : end - pos);
      if (!segment.empty() && segment.front() == ':') {
        if (segment.size() == 1) {
          throw std::invalid_argument("A route parameter must have a name");
        }
        count++;
      }
      if (end == std::string_view::npos) return count;
      pos = end + 1;
    }
  }
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
                     RQty>,
        "The first route handler argument must be const RQty&");
    static_assert(
        std::is_lvalue_reference_v<typename signature::request_type> &&
            std::is_const_v<std::remove_reference_t<
                typename signature::request_type>>,
        "The first route handler argument must be const RQty&");
    static_assert(
        std::same_as<std::decay_t<typename signature::response_type>,
                     RSty>,
        "The second route handler argument must be RSty&");
    static_assert(
        std::is_lvalue_reference_v<typename signature::response_type> &&
            !std::is_const_v<std::remove_reference_t<
                typename signature::response_type>>,
        "The second route handler argument must be RSty&");
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::vector<handler_pair> handlers_;
  std::vector<parametrized_handler_pair> parametrized_handlers_;
};
}  // namespace martianlabs::doba::protocol::http

#endif
