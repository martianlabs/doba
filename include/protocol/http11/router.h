//      _       _           
//   __| | ___ | |__   __ _ 
//  / _` |/ _ \| '_ \ / _` |
// | (_| | (_) | |_) | (_| |
//  \__,_|\___/|_.__/ \__,_|
// 
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef martianlabs_doba_protocol_http11_router_h
#define martianlabs_doba_protocol_http11_router_h

#include "method.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// router                                                              ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 router implementation.
// -----------------------------------------------------------------------------
// Template parameters:
//    RQty - request being used.
//    RSty - response being used.
// =============================================================================
template <typename RQty, typename RSty>
class router {
 public:
  // ___________________________________________________________________________
  // USINGs                                                           ( public )
  //
  using handler_fn = std::function<void(const RQty&, RSty&)>;

 private:
  // ___________________________________________________________________________
  // STRUCTs                                                         ( private )
  //
  struct hash {
    using is_transparent = void;
    size_t operator()(std::string_view s) const noexcept {
      return std::hash<std::string_view>{}(s);
    }
  };
  struct equal {
    using is_transparent = void;
    bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
      return lhs == rhs;
    }
  };
  // ___________________________________________________________________________
  // USINGs                                                          ( private )
  //
  using router_map = std::unordered_map<std::string, handler_fn, hash, equal>;

 public:
  // ___________________________________________________________________________
  // CONSTRUCTORs/DESTRUCTORs                                         ( public )
  //
  router() = default;
  router(const router&) = delete;
  router(router&&) noexcept = delete;
  ~router() = default;
  // ___________________________________________________________________________
  // OPERATORs                                                        ( public )
  //
  router& operator=(const router&) = delete;
  router& operator=(router&&) noexcept = delete;
  // ___________________________________________________________________________
  // METHODs                                                          ( public )
  //
  void add(method mtd, const std::string_view& route, handler_fn fn) {
    auto& map = rts_.try_emplace(mtd).first->second;
    map.emplace(route, std::move(fn));
  }
  std::optional<handler_fn> match(method method, const std::string_view& path) {
    if (auto it_m = rts_.find(method); it_m != rts_.end()) {
      if (auto it_h = it_m->second.find(path); it_h != it_m->second.end()) {
        return it_h->second;
      }
    }
    return std::nullopt;
  }

 private:
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  std::unordered_map<method, router_map> rts_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
