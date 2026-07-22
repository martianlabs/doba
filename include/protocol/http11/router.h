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

#ifndef martianlabs_doba_protocol_http11_router_h
#define martianlabs_doba_protocol_http11_router_h

#include "common/hash_map.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] router                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | This class holds for the http 1.1 router implementation.                  |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   FNty - router handle function prototype.                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename FNty>
class router {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  router() = default;
  router(const router&) = delete;
  router(router&&) noexcept = delete;
  ~router() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  router& operator=(const router&) = delete;
  router& operator=(router&&) noexcept = delete;
  // +=========================================================================+
  // | [>] add                                                      ( public ) |
  // +=========================================================================+
  void add(const std::string& method, const std::string& route, FNty fn) {
    auto& map = routes_.try_emplace(method).first->second;
    map.emplace(route, fn);
  }
  // +=========================================================================+
  // | [>] match                                                    ( public ) |
  // +=========================================================================+
  std::optional<FNty> match(std::string_view method, std::string_view path) {
    std::optional<FNty> res = std::nullopt;
    if (auto it_m = routes_.find(method); it_m != routes_.end()) {
      if (auto it_h = it_m->second.find(path); it_h != it_m->second.end()) {
        res = it_h->second;
      }
    }
    return res;
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  common::hash_map<std::string, common::hash_map<std::string, FNty>> routes_;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
