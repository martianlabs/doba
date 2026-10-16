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

#ifndef martianlabs_doba_protocol_http_v11_buffer_pool_h
#define martianlabs_doba_protocol_http_v11_buffer_pool_h

#include <cstddef>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include "protocol/http/v11/policies.h"

namespace martianlabs::doba::protocol::http::v11 {
namespace detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] buffer_pool                                                 ( class ) |
// +---------------------------------------------------------------------------+
// | Each thread owns its free list; loans retain exclusive buffer ownership.  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class buffer_pool {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  struct buffer_pair {
    // +=======================================================================+
    // | [>] ATTRIBUTEs                                             ( public ) |
    // +-----------------------------------------------------------------------+
    std::unique_ptr<char[]> request;
    std::unique_ptr<char[]> response;
    std::size_t request_capacity{0};
    std::size_t response_capacity{0};

   private:
    friend class buffer_pool;
    // +=======================================================================+
    // | [>] ATTRIBUTEs                                            ( private ) |
    // +-----------------------------------------------------------------------+
    std::unique_ptr<buffer_pair> next_;
  };
  // +=========================================================================+
  // | [>] acquire                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  static std::unique_ptr<buffer_pair> acquire(const policies& configuration) {
    if (!configuration.request_head_buffer_capacity ||
        !configuration.response_head_buffer_capacity ||
        configuration.request_body_inline_capacity >
            std::numeric_limits<std::size_t>::max() -
                configuration.request_head_buffer_capacity ||
        configuration.response_body_inline_capacity >
            std::numeric_limits<std::size_t>::max() -
                configuration.response_head_buffer_capacity) {
      throw std::invalid_argument("Invalid HTTP buffer capacities");
    }
    const auto request_capacity = configuration.request_head_buffer_capacity +
                                  configuration.request_body_inline_capacity;
    const auto response_capacity = configuration.response_head_buffer_capacity +
                                   configuration.response_body_inline_capacity;
    auto* next = &available().head_;
    while (*next && ((*next)->request_capacity != request_capacity ||
                     (*next)->response_capacity != response_capacity)) {
      next = &(*next)->next_;
    }
    if (*next) {
      auto value = std::move(*next);
      *next = std::move(value->next_);
      return value;
    }
    auto value = std::make_unique<buffer_pair>();
    value->request_capacity = request_capacity;
    value->response_capacity = response_capacity;
    value->request = std::make_unique_for_overwrite<char[]>(request_capacity);
    value->response = std::make_unique_for_overwrite<char[]>(response_capacity);
    return value;
  }
  // +=========================================================================+
  // | [>] release                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Requires a nonempty pair; only the calling worker accesses its list.    |
  // +-------------------------------------------------------------------------+
  static void release(std::unique_ptr<buffer_pair> value) noexcept {
    auto& head = available().head_;
    value->next_ = std::move(head);
    head = std::move(value);
  }

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                ( private ) |
  // +-------------------------------------------------------------------------+
  buffer_pool() = default;
  ~buffer_pool() {
    while (head_) {
      auto value = std::move(head_);
      head_ = std::move(value->next_);
    }
  }
  // +=========================================================================+
  // | [>] available                                               ( private ) |
  // +-------------------------------------------------------------------------+
  static buffer_pool& available() noexcept {
    thread_local buffer_pool value;
    return value;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::unique_ptr<buffer_pair> head_;
};
}  // namespace detail
}  // namespace martianlabs::doba::protocol::http::v11

#endif
