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

#ifndef martianlabs_doba_protocol_http_router_handler_signature_h
#define martianlabs_doba_protocol_http_router_handler_signature_h

#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

#include "protocol/http/common/router_handler_parametrized.h"
#include "protocol/http/common/router_handler_parametrized_async.h"

namespace martianlabs::doba::protocol::http::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_signature                      ( forward-declaration ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename>
struct router_handler_signature;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_signature_base                              ( struct ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   LOty - handler result                                                   |
// |   LQty - request argument                                                 |
// |   LSty - response argument                                                |
// |   Args - route parameter types                                            |
// +---------------------------------------------------------------------------+
// | Shares signature checks and handler creation across callable types.       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename LOty, typename LQty, typename LSty, typename... Args>
struct router_handler_signature_base {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using return_type = LOty;
  using request_type = LQty;
  using response_type = LSty;
  // +=========================================================================+
  // | [>] CONSTANTs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr std::size_t parameter_count = sizeof...(Args);
  static constexpr bool valid_return =
      std::same_as<LOty, void> || std::same_as<LOty, common::task<void>>;
  // +=========================================================================+
  // | [>] bind                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Wraps a controller method as a handler and keeps the instance alive.    |
  // +-------------------------------------------------------------------------+
  template <typename Cty, typename Mty>
  static auto bind(std::shared_ptr<Cty> instance, Mty method) {
    return
        [instance = std::move(instance), method](
            LQty req, LSty res,
            Args... args) noexcept(std::is_nothrow_invocable_v<Mty, Cty&, LQty,
                                                               LSty, Args...>)
            -> LOty {
          return std::invoke(method, *instance, std::forward<LQty>(req),
                             std::forward<LSty>(res),
                             std::forward<Args>(args)...);
        };
  }
  // +=========================================================================+
  // | [>] make_parametrized                                        ( public ) |
  // +-------------------------------------------------------------------------+
  // | Builds a synchronous route handler using Args as its parameter types.   |
  // +-------------------------------------------------------------------------+
  template <typename RQty, typename RSty, typename Hty>
  static auto make_parametrized(std::string_view pattern, Hty&& handler) {
    return make_router_handler_parametrized<RQty, RSty, Args...>(
        pattern, std::forward<Hty>(handler));
  }
  // +=========================================================================+
  // | [>] make_parametrized_async                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Builds an async route handler using the same route parameter types.     |
  // +-------------------------------------------------------------------------+
  template <typename RQty, typename RSty, typename Hty>
  static auto make_parametrized_async(std::string_view pattern, Hty&& handler) {
    return make_router_handler_parametrized_async<RQty, RSty, Args...>(
        pattern, std::forward<Hty>(handler));
  }
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Requires const RQty& and RSty&, returning void or task<void>.           |
  // | A mismatch fails at compile time.                                       |
  // +-------------------------------------------------------------------------+
  template <typename RQty, typename RSty>
  static void check() {
    static_assert(valid_return,
                  "The route handler must return void or task<void>");
    static_assert(std::same_as<std::decay_t<LQty>, RQty>,
                  "The first route handler argument must be const RQty&");
    static_assert(std::is_lvalue_reference_v<LQty> &&
                      std::is_const_v<std::remove_reference_t<LQty>>,
                  "The first route handler argument must be const RQty&");
    static_assert(std::same_as<std::remove_cvref_t<LSty>, RSty> &&
                      std::is_lvalue_reference_v<LSty> &&
                      !std::is_const_v<std::remove_reference_t<LSty>>,
                  "The second route handler argument must be RSty&");
  }
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_signature                                   ( struct ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   Cty - callable or controller class                                      |
// |   Retty - handler result                                                  |
// |   Reqty - request argument                                                |
// |   Resty - response argument                                               |
// |   Args - route parameter types                                            |
// +---------------------------------------------------------------------------+
// | Reads member function signatures, with or without const and noexcept.     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Cty, typename Retty, typename Reqty, typename Resty,
          typename... Args>
struct router_handler_signature<Retty (Cty::*)(Reqty, Resty, Args...) const>
    : router_handler_signature_base<Retty, Reqty, Resty, Args...> {};
template <typename Cty, typename Retty, typename Reqty, typename Resty,
          typename... Args>
struct router_handler_signature<Retty (Cty::*)(Reqty, Resty, Args...)
                                    const noexcept>
    : router_handler_signature_base<Retty, Reqty, Resty, Args...> {};
template <typename Cty, typename Retty, typename Reqty, typename Resty,
          typename... Args>
struct router_handler_signature<Retty (Cty::*)(Reqty, Resty, Args...)>
    : router_handler_signature_base<Retty, Reqty, Resty, Args...> {};
template <typename Cty, typename Retty, typename Reqty, typename Resty,
          typename... Args>
struct router_handler_signature<Retty (Cty::*)(Reqty, Resty, Args...) noexcept>
    : router_handler_signature_base<Retty, Reqty, Resty, Args...> {};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_lambda                                     ( concept ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   Hty - handler type being used                                           |
// +---------------------------------------------------------------------------+
// | Accepts callables with const request and mutable response references,     |
// | returning void or task<void>.                                             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Hty>
concept router_handler_lambda = requires {
  typename router_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::request_type;
  requires std::is_lvalue_reference_v<typename router_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::request_type>;
  requires std::is_const_v<
      std::remove_reference_t<typename router_handler_signature<
          decltype(&std::decay_t<Hty>::operator())>::request_type>>;
  requires router_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::valid_return;
  requires std::is_lvalue_reference_v<typename router_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::response_type>;
  requires !std::is_const_v<
      std::remove_reference_t<typename router_handler_signature<
          decltype(&std::decay_t<Hty>::operator())>::response_type>>;
};
}  // namespace martianlabs::doba::protocol::http::detail

#endif
