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

#ifndef martianlabs_doba_protocol_http_router_match_result_h
#define martianlabs_doba_protocol_http_router_match_result_h

#include <functional>

#include "protocol/http/common/auth/authentication_result.h"
#include "protocol/http/common/router_handler_parametrized.h"
#include "protocol/http/common/router_handler_parametrized_async.h"
#include "protocol/http/common/router_handler_static.h"
#include "protocol/http/common/router_handler_static_async.h"

namespace martianlabs::doba::protocol::http {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_match_result                                        ( struct ) |
// +---------------------------------------------------------------------------+
// | Holds the matched handler and its optional authentication callback.       |
// | The pointers borrow the router's stored routes; they do not own them.     |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request type                                                     |
// |   RSty - response type                                                    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty>
struct router_match_result {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // | A router match sets one handler pointer; a miss leaves all four null.   |
  // +-------------------------------------------------------------------------+
  const detail::router_handler_static<RQty, RSty>* handler{nullptr};
  const detail::router_handler_parametrized<RQty, RSty>* parametrized_handler{
      nullptr};
  const detail::router_handler_static_async<RQty, RSty>* async_handler{nullptr};
  const detail::router_handler_parametrized_async<RQty, RSty>*
      async_parametrized_handler{nullptr};
  // Check both the pointer and the callback before calling it.
  const std::function<auth::authentication_result(const RQty&)>* authenticate{
      nullptr};
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | True when a handler was found, regardless of authentication.            |
  // +-------------------------------------------------------------------------+
  [[nodiscard]] explicit operator bool() const {
    return handler != nullptr || parametrized_handler != nullptr ||
           async_handler != nullptr || async_parametrized_handler != nullptr;
  }
};
}  // namespace martianlabs::doba::protocol::http

#endif
