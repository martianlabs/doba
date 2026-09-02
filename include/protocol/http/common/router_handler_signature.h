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
#include <memory>
#include <stop_token>
#include <string_view>
#include <type_traits>
#include <utility>

#include "common/task.h"
#include "protocol/http/common/router_handler_parametrized.h"

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
// | [>] router_async_handler_signature                 ( forward-declaration) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename>
struct router_async_handler_signature;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_signature_base                              ( struct ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   LOty - return type of the handler                                       |
// |   LQty - type of the first argument of the handler (request)              |
// |   LSty - type of the second argument of the handler (response)            |
// |   Args - types of the remaining arguments of the handler (routing)        |
// +---------------------------------------------------------------------------+
// | This struct provides a base for the router_handler_signature and          |
// | router_async_handler_signature structs. It defines the return type,       |
// | request type, response type, and parameter count of the handler. It also  |
// | provides a static method to create a parametrized router handler and a    |
// | static method to check the types of the handler's arguments against the   |
// | expected types.                                                           |
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
    static_assert(std::is_lvalue_reference_v<LQty> &&
                      std::is_const_v<std::remove_reference_t<LQty>>,
                  "The first route handler argument must be const RQty&");
    static_assert(std::same_as<std::decay_t<LSty>, RSty>,
                  "The second route handler argument must be RSty&");
    static_assert(std::is_lvalue_reference_v<LSty> &&
                      !std::is_const_v<std::remove_reference_t<LSty>>,
                  "The second route handler argument must be RSty&");
  }
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_async_handler_signature_base                        ( struct ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   LOty - return type of the handler                                       |
// |   LQty - type of the first argument of the handler (request)              |
// |   LCty - type of the second argument of the handler (cancellation)        |
// |   Args - types of the remaining arguments of the handler (routing)        |
// +---------------------------------------------------------------------------+
// | This struct provides a base for the router_async_handler_signature        |
// | struct. It defines the return type, request type, cancellation type,      |
// | and parameter count of the handler. It also provides a static method to   |
// | create a parametrized router handler and a static method to check the     |
// | types of the handler's arguments against the expected types.              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename LOty, typename LQty, typename LCty, typename... Args>
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
    static_assert(std::same_as<LCty, std::stop_token>,
                  "The second asynchronous route handler argument must be "
                  "std::stop_token");
  }
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_signature [const]                           ( struct ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   Cty - class type of the handler                                         |
// |   Retty - return type of the handler                                      |
// |   Reqty - type of the first argument of the handler (request)             |
// |   Resty - type of the second argument of the handler (response)           |
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

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_signature                                   ( struct ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   Cty - class type of the handler                                         |
// |   Retty - return type of the handler                                      |
// |   Reqty - type of the first argument of the handler (request)             |
// |   Resty - type of the second argument of the handler (response)           |
// |   Args - types of the remaining arguments of the handler (routing)        |
// +---------------------------------------------------------------------------+
// | This specialization of the router_handler_signature struct handles the    |
// | case where the handler is a non-const member function. It inherits from   |
// | the router_handler_signature_base struct and provides the same type       |
// | aliases and static methods for creating parametrized router handlers and  |
// | checking argument types.                                                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Cty, typename Retty, typename Reqty, typename Resty,
          typename... Args>
struct router_handler_signature<Retty (Cty::*)(Reqty, Resty, Args...)>
    : router_handler_signature_base<Retty, Reqty, Resty, Args...> {};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_async_handler_signature                             ( struct ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   Cty - class type of the handler                                         |
// |   Retty - return type of the handler                                      |
// |   Reqty - type of the first argument of the handler (request)             |
// |   Cancelty - type of the second argument of the handler (cancellation)    |
// |   Args - types of the remaining arguments of the handler (routing)        |
// +---------------------------------------------------------------------------+
// | This specialization of the router_async_handler_signature struct handles  |
// | the case where the handler is a const member function. It inherits from   |
// | the router_async_handler_signature_base struct and provides the same      |
// | type aliases and static methods for creating parametrized router handlers |
// | and checking argument types.                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Cty, typename Retty, typename Reqty, typename Cancelty,
          typename... Args>
struct router_async_handler_signature<Retty (Cty::*)(Reqty, Cancelty, Args...)
                                          const>
    : router_async_handler_signature_base<Retty, Reqty, Cancelty, Args...> {};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_async_handler_signature                             ( struct ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   Cty - class type of the handler                                         |
// |   Retty - return type of the handler                                      |
// |   Reqty - type of the first argument of the handler (request)             |
// |   Cancelty - type of the second argument of the handler (cancellation)    |
// |   Args - types of the remaining arguments of the handler (routing)        |
// +---------------------------------------------------------------------------+
// | This specialization of the router_async_handler_signature struct handles  |
// | the case where the handler is a non-const member function. It inherits    |
// | from the router_async_handler_signature_base struct and provides the same |
// | type aliases and static methods for creating parametrized router handlers |
// | and checking argument types.                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Cty, typename Retty, typename Reqty, typename Cancelty,
          typename... Args>
struct router_async_handler_signature<Retty (Cty::*)(Reqty, Cancelty, Args...)>
    : router_async_handler_signature_base<Retty, Reqty, Cancelty, Args...> {};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_lambda                                      (concept ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   Hty - handler type being used                                           |
// +---------------------------------------------------------------------------+
// | This concept checks if a given handler type Hty is a valid router handler |
// | lambda. It requires that the handler has a request_type defined in its    |
// | signature and that its return type is void. If these conditions are met,  |
// | the concept evaluates to true; otherwise, it evaluates to false.          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Hty>
concept router_handler_lambda = requires {
  typename router_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::request_type;
  requires std::same_as<
      typename router_handler_signature<
          decltype(&std::decay_t<Hty>::operator())>::return_type,
      void>;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_returns_task                                  (struct) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   T - type being checked                                                  |
// +---------------------------------------------------------------------------+
// | This struct checks if a given type T is a specialization of common::task. |
// | It inherits from std::false_type by default, and from std::true_type if   |
// | T is a specialization of common::task. This can be used to determine if a |
// | handler's return type is an asynchronous task.                            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename>
struct router_handler_returns_task : std::false_type {};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_returns_task                                  (struct) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   T - type being checked                                                  |
// +---------------------------------------------------------------------------+
// | This struct checks if a given type T is a specialization of common::task. |
// | It inherits from std::false_type by default, and from std::true_type if   |
// | T is a specialization of common::task. This can be used to determine if a |
// | handler's return type is an asynchronous task.                            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename T>
struct router_handler_returns_task<common::task<T>> : std::true_type {};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_async_handler_lambda                                 (concept) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   Hty - type being checked                                                |
// +---------------------------------------------------------------------------+
// | This concept checks if a given type Hty is a lambda whose return type is  |
// | a specialization of common::task.                                         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Hty>
concept router_async_handler_lambda = requires {
  typename router_async_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::request_type;
  requires router_handler_returns_task<typename router_async_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::return_type>::value;
};
}  // namespace martianlabs::doba::protocol::http

#endif
