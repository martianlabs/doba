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

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

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
  // +-------------------------------------------------------------------------+
  explicit router_controller_routes(std::shared_ptr<Cty> instance)
      : instance_(std::move(instance)) {}
  router_controller_routes(const router_controller_routes&) = delete;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  router_controller_routes& operator=(const router_controller_routes&) = delete;
  // +=========================================================================+
  // | [>] add                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  template <typename Mty>
    requires std::is_member_function_pointer_v<Mty>
  void add(std::string_view method, std::string_view route, Mty member) {
    registrations_.emplace_back(
        [method = std::string(method), route = std::string(route),
         handler = router_handler_signature<Mty>::bind(instance_, member)](
            router<RQty, RSty>& target) mutable {
          target.add(method, route, std::move(handler));
        });
  }
  // +=========================================================================+
  // | [>] empty                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  bool empty() const { return registrations_.empty(); }
  // +=========================================================================+
  // | [>] apply                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void apply(router<RQty, RSty>& target) {
    for (auto& register_route : registrations_) register_route(target);
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::shared_ptr<Cty> instance_;
  std::vector<std::function<void(router<RQty, RSty>&)>> registrations_;
};
}  // namespace detail
}  // namespace martianlabs::doba::protocol::http

#endif
