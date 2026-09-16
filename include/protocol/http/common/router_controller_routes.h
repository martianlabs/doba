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

#ifndef martianlabs_doba_protocol_http_router_controller_routes_h
#define martianlabs_doba_protocol_http_router_controller_routes_h

#include <cstddef>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

#include "protocol/http/common/router_handler_signature.h"

namespace martianlabs::doba::protocol::http {
template <typename RQty, typename RSty>
class router;

namespace detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_controller_routes                                    ( class ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used                                               |
// |   RSty - response being used                                              |
// |   Cty - controller being registered                                       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, typename Cty>
class router_controller_routes {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  router_controller_routes(router<RQty, RSty>& owner,
                           std::shared_ptr<Cty> instance)
      : owner_(owner), instance_(std::move(instance)) {}
  router_controller_routes(const router_controller_routes&) = delete;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  router_controller_routes& operator=(const router_controller_routes&) = delete;
  // +=========================================================================+
  // | [>] add                                                      ( public ) |
  // +=========================================================================+
  template <typename Mty>
    requires std::is_member_function_pointer_v<Mty>
  void add(std::string_view method, std::string_view route, Mty member) {
    owner_.add(method, route,
               router_handler_signature<Mty>::bind(instance_, member));
    count_++;
  }

 private:
  friend class router<RQty, RSty>;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  router<RQty, RSty>& owner_;
  std::shared_ptr<Cty> instance_;
  std::size_t count_{0};
};
}  // namespace detail
}  // namespace martianlabs::doba::protocol::http

#endif
