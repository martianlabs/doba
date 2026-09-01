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

#ifndef martianlabs_doba_protocol_http_router_handler_static_h
#define martianlabs_doba_protocol_http_router_handler_static_h

#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

#include "protocol/http/common/router_handler_parametrized.h"

namespace martianlabs::doba::protocol::http {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_signature                       ( forward-declaration) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename>
struct router_handler_signature;
template <typename>
struct router_async_handler_signature;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_signature_base                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename LOty, typename LQty, typename LSty, typename... Args>
struct router_handler_signature_base {
  using return_type = LOty;
  using request_type = LQty;
  using response_type = LSty;
  static constexpr std::size_t parameter_count = sizeof...(Args);
  template <typename RQty, typename RSty, typename Hty>
  static auto make_parametrized(std::string_view pattern, Hty&& handler) {
    return make_router_handler_parametrized<RQty, RSty, Args...>(
        pattern, std::forward<Hty>(handler));
  }
  template <typename RQty, typename RSty>
  static void check() {
    static_assert(std::same_as<LOty, void>,
                  "The route handler must return void");
    static_assert(std::same_as<std::decay_t<LQty>, RQty>,
                  "The first route handler argument must be const RQty&");
    static_assert(
        std::is_lvalue_reference_v<LQty> &&
            std::is_const_v<std::remove_reference_t<LQty>>,
        "The first route handler argument must be const RQty&");
    static_assert(std::same_as<std::decay_t<LSty>, RSty>,
                  "The second route handler argument must be RSty&");
    static_assert(
        std::is_lvalue_reference_v<LSty> &&
            !std::is_const_v<std::remove_reference_t<LSty>>,
        "The second route handler argument must be RSty&");
  }
};
template <typename LOty, typename LQty, typename... Args>
struct router_async_handler_signature_base {
  using return_type = LOty;
  using request_type = LQty;
  static constexpr std::size_t parameter_count = sizeof...(Args);
  template <typename RQty, typename RSty, typename Hty>
  static auto make_parametrized(std::string_view pattern, Hty&& handler) {
    return make_router_handler_parametrized_async<RQty, RSty, Args...>(
        pattern, std::forward<Hty>(handler));
  }
  template <typename RQty, typename RSty>
  static void check() {
    static_assert(std::same_as<LOty, common::task<RSty>>,
                  "The asynchronous route handler must return "
                  "common::task<RSty>");
    static_assert(std::same_as<LQty, std::shared_ptr<const RQty>>,
                  "The asynchronous route handler argument must be "
                  "std::shared_ptr<const RQty>");
  }
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_signature                                   ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Cty, typename Retty, typename Reqty, typename Resty,
          typename... Args>
struct router_handler_signature<Retty (Cty::*)(Reqty, Resty, Args...) const>
    : router_handler_signature_base<Retty, Reqty, Resty, Args...> {};
template <typename Cty, typename Retty, typename Reqty, typename Resty,
          typename... Args>
struct router_handler_signature<Retty (Cty::*)(Reqty, Resty, Args...)>
    : router_handler_signature_base<Retty, Reqty, Resty, Args...> {};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_async_handler_signature                             ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Cty, typename Retty, typename Reqty, typename... Args>
struct router_async_handler_signature<Retty (Cty::*)(Reqty, Args...) const>
    : router_async_handler_signature_base<Retty, Reqty, Args...> {};
template <typename Cty, typename Retty, typename Reqty, typename... Args>
struct router_async_handler_signature<Retty (Cty::*)(Reqty, Args...)>
    : router_async_handler_signature_base<Retty, Reqty, Args...> {};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_lambda                                      (concept ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Hty>
concept router_handler_lambda = requires {
  typename router_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::request_type;
  requires std::same_as<typename router_handler_signature<
                            decltype(&std::decay_t<Hty>::operator())>::
                            return_type,
                        void>;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_async_handler_lambda                                (concept ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename>
struct router_handler_returns_task : std::false_type {};
template <typename T>
struct router_handler_returns_task<common::task<T>> : std::true_type {};
template <typename Hty>
concept router_async_handler_lambda = requires {
  typename router_async_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::request_type;
  requires router_handler_returns_task<
      typename router_async_handler_signature<
          decltype(&std::decay_t<Hty>::operator())>::return_type>::value;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_static                                       ( using ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used                                               |
// |   RSty - response being used                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty>
using router_handler_static =
    std::function<void(const RQty&, RSty&)>;
}  // namespace martianlabs::doba::protocol::http

#endif
