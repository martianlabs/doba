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

namespace martianlabs::doba::protocol::http {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_signature                       ( forward-declaration) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename>
struct router_handler_signature;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_signature_base                              ( struct ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   LOty - return type of the handler                                       |
// |   LQty - type of the first argument of the handler (request)              |
// |   Args - types of the remaining arguments of the handler (routing)        |
// +---------------------------------------------------------------------------+
// | This struct defines the return type, request type and parameter count.    |
// | It also binds controller methods and creates parametrized handlers.       |
// | provides a static method to create a parametrized router handler and a    |
// | static method to check the types of the handler's arguments against the   |
// | expected types.                                                           |
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
  // +=========================================================================+
  // | [>] bind                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | This static method binds a controller instance and a member function to |
  // | create a router handler. It returns a lambda that captures the instance |
  // | and method, and when invoked, it calls the member function on the       |
  // | instance with the provided request and routing arguments. The lambda is |
  // | noexcept if the member function is noexcept when invoked with the       |
  // | given arguments. The method uses std::invoke to call the member         |
  // | function, allowing for both regular and pointer-to-member function      |
  // | calls. The method also uses std::forward to perfectly forward the       |
  // | request and routing arguments to the member function.                   |
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
  // | This static method creates a parametrized router handler using the      |
  // | provided pattern and handler. It takes a string_view pattern and a      |
  // | handler of type Hty, and returns a router_handler_parametrized object   |
  // | that is constructed with the given pattern and handler. The method uses |
  // | std::forward to perfectly forward the handler argument to the           |
  // | constructor of router_handler_parametrized.                             |
  // +-------------------------------------------------------------------------+
  template <typename RQty, typename RSty, typename Hty>
  static auto make_parametrized(std::string_view pattern, Hty&& handler) {
    return make_router_handler_parametrized<RQty, RSty, Args...>(
        pattern, std::forward<Hty>(handler));
  }
  template <typename RQty, typename RSty, typename Hty>
  static auto make_parametrized_async(std::string_view pattern, Hty&& handler) {
    return make_router_handler_parametrized_async<RQty, RSty, Args...>(
        pattern, std::forward<Hty>(handler));
  }
  // +=========================================================================+
  // | [>] check                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | This static method checks if the types of the handler's arguments match |
  // | the expected types. It takes two template parameters, RQty and RSty,    |
  // | which represent the expected request and response types. It requires    |
  // | void(const RQty&, RSty&, ...).                                          |
  // +-------------------------------------------------------------------------+
  template <typename RQty, typename RSty>
  static void check() {
    static_assert(
        std::same_as<LOty, void> || std::same_as<LOty, common::task<void>>,
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
// |   Cty - class type of the handler                                         |
// |   Retty - return type of the handler                                      |
// |   Reqty - type of the first argument of the handler (request)             |
// |   Args - types of the remaining arguments of the handler (routing)        |
// +---------------------------------------------------------------------------+
// | This specialization of the router_handler_signature struct handles the    |
// | case where the handler is a const member function. It inherits from the   |
// | router_handler_signature_base struct and provides the same type aliases   |
// | and static methods for creating parametrized router handlers and checking |
// | argument types.                                                           |
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
// | [>] router_handler_lambda                                      (concept ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   Hty - handler type being used                                           |
// +---------------------------------------------------------------------------+
// | This concept checks if a given handler type Hty is a valid router handler |
// | lambda. It requires that the handler has a request_type defined in its    |
// | signature and returns void. If these conditions are met,                  |
// | the concept evaluates to true; otherwise, it evaluates to false.          |
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
  requires std::same_as<
               typename router_handler_signature<
                   decltype(&std::decay_t<Hty>::operator())>::return_type,
               void> ||
               std::same_as<
                   typename router_handler_signature<
                       decltype(&std::decay_t<Hty>::operator())>::return_type,
                   common::task<void>>;
  requires std::is_lvalue_reference_v<typename router_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::response_type>;
  requires !std::is_const_v<
      std::remove_reference_t<typename router_handler_signature<
          decltype(&std::decay_t<Hty>::operator())>::response_type>>;
};
}  // namespace martianlabs::doba::protocol::http

#endif
