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

#ifndef martianlabs_doba_protocol_http_router_handler_parametrized_async_h
#define martianlabs_doba_protocol_http_router_handler_parametrized_async_h

#include <array>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "common/task.h"
#include "protocol/http/common/router_handler_parametrized.h"

namespace martianlabs::doba::protocol::http {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_parametrized_async                           ( class ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used                                               |
// |   RSty - response being used                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty>
class router_handler_parametrized_async {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  using matcher_type = bool (*)(std::string_view, std::string_view);
  using callback_type = std::function<common::task<void>(
      const RQty&, RSty&, std::string_view, std::string_view,
      std::shared_ptr<void>&)>;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  router_handler_parametrized_async(std::string pattern, matcher_type matcher,
                                    callback_type callback)
      : pattern_{std::move(pattern)},
        matcher_{matcher},
        callback_{std::move(callback)} {}
  // +=========================================================================+
  // | [>] matches                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  [[nodiscard]] bool matches(std::string_view path) const {
    return matcher_(pattern_, path);
  }
  // +=========================================================================+
  // | [>] invoke                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  common::task<void> invoke(const RQty& req, RSty& res, std::string_view path,
                            std::shared_ptr<void>& parameters) const {
    return callback_(req, res, pattern_, path, parameters);
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::string pattern_;     // The route pattern to match against.
  matcher_type matcher_;    // The function to match the route pattern against.
  callback_type callback_;  // The function to invoke when there's a match.
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] make_router_handler_parametrized_async                     (function) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used                                               |
// |   RSty - response being used                                              |
// |   Args - route parameters being used                                      |
// |   Hty - handler being used                                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, typename... Args, typename Hty>
auto make_router_handler_parametrized_async(std::string_view pattern,
                                            Hty&& handler) {
  using handler_type = std::decay_t<Hty>;
  return router_handler_parametrized_async<RQty, RSty>(
      std::string(pattern), &detail::match_route_parameters<Args...>,
      [handler = handler_type(std::forward<Hty>(handler))](
          const RQty& req, RSty& res, std::string_view route_pattern,
          std::string_view path,
          std::shared_ptr<void>& storage) mutable -> common::task<void> {
        std::array<std::string_view, sizeof...(Args)> parameters;
        if (!detail::extract_route_parameters(route_pattern, path,
                                              parameters)) {
          throw std::runtime_error(
              "The route parameters could not be extracted");
        }
        auto values = std::make_shared<std::tuple<std::decay_t<Args>...>>();
        if (!detail::parse_route_parameters_<Args...>(
                parameters, *values, std::index_sequence_for<Args...>{})) {
          throw std::runtime_error("The route parameters could not be parsed");
        }
        storage = values;
        return std::apply(
            [&handler, &req, &res](auto&... value) {
              return std::invoke(handler, req, res, value...);
            },
            *values);
      });
}
}  // namespace martianlabs::doba::protocol::http

#endif
