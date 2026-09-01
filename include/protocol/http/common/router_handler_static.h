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
  static auto make_parametrized(std::string_view pattern, Hty&& handler,
                                bool asynchronous = false) {
    return make_router_handler_parametrized<RQty, RSty, Args...>(
        pattern, std::forward<Hty>(handler), asynchronous);
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
// | [>] router_handler_lambda                                      (concept ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename Hty>
concept router_handler_lambda = requires {
  typename router_handler_signature<
      decltype(&std::decay_t<Hty>::operator())>::request_type;
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
