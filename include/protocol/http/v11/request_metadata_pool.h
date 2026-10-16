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

#ifndef martianlabs_doba_protocol_http_v11_request_metadata_pool_h
#define martianlabs_doba_protocol_http_v11_request_metadata_pool_h

#include <new>
#include <utility>
#include <vector>

#include "protocol/http/common/header.h"

namespace martianlabs::doba::protocol::http::v11 {
namespace detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] request_metadata_pool                                       ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class request_metadata_pool {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  using values = std::vector<header_view>;
  // +=========================================================================+
  // | [>] acquire                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  static values acquire() {
    auto& pool = available();
    if (pool.empty()) return {};
    auto value = std::move(pool.back());
    pool.pop_back();
    return value;
  }
  // +=========================================================================+
  // | [>] release                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  static void release(values value) noexcept {
    if (!value.capacity()) return;
    value.clear();
    try {
      available().push_back(std::move(value));
    } catch (const std::bad_alloc&) {
      // Cache growth must not make request destruction fail.
    }
  }

 private:
  // +=========================================================================+
  // | [>] available                                               ( private ) |
  // +-------------------------------------------------------------------------+
  static std::vector<values>& available() noexcept {
    thread_local std::vector<values> pool;
    return pool;
  }
};
}  // namespace detail
}  // namespace martianlabs::doba::protocol::http::v11

#endif
