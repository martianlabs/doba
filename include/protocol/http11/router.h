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

#include "common/hash_map.h"
#include "common/execution_policy.h"
#include "method.h"
#include "request.h"
#include "response.h"

namespace martianlabs::doba::protocol::http11 {
// =============================================================================
// router                                                              ( class )
// -----------------------------------------------------------------------------
// This class holds for the http 1.1 router implementation.
// -----------------------------------------------------------------------------
// =============================================================================
class router {
 public:
  // ___________________________________________________________________________
  // USINGs                                                           ( public )
  //
  using handler = std::function<void(const request&, response&)>;
  using hpair = std::pair<handler, common::execution_policy>;
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
  void add(method method, std::string_view route, handler handler,
           common::execution_policy policy) {
    auto& map = routes_.try_emplace(method).first->second;
    map.emplace(route, std::make_pair(std::move(handler), policy));
  }
  auto match(method method, std::string_view path) {
    std::optional<hpair> res = std::nullopt;
    if (auto it_m = routes_.find(method); it_m != routes_.end()) {
      if (auto it_h = it_m->second.find(path); it_h != it_m->second.end()) {
        res = it_h->second;
      }
    }
    return res;
  }

 private:
  // ___________________________________________________________________________
  // ATTRIBUTEs                                                      ( private )
  //
  std::unordered_map<method, common::hash_map<std::string, hpair>> routes_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
