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

#ifndef martianlabs_doba_protocol_http11_router_handler_parametrized_base_h
#define martianlabs_doba_protocol_http11_router_handler_parametrized_base_h

#include <memory>
#include <string_view>
#include <utility>
#include <vector>

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router_handler_parametrized_base                            ( class ) |
// +---------------------------------------------------------------------------+
// | This class is an abstract base class for parameterized router handlers.   |
// | It defines the interface for invoking a handler with route parameters.    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty>
class router_handler_parametrized_base {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  virtual ~router_handler_parametrized_base() = default;
  // +=========================================================================+
  // | [>] CONTRACT                                                 ( public ) |
  // +=========================================================================+
  [[nodiscard]]
  virtual bool invoke(
      std::shared_ptr<const RQty> req, std::shared_ptr<RSty> res,
      const std::vector<std::pair<std::string_view, std::string_view>>&
          parameters) const = 0;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
