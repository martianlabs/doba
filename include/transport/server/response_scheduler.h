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

#ifndef martianlabs_doba_transport_server_response_scheduler_h
#define martianlabs_doba_transport_server_response_scheduler_h

#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>

#include "protocol/serialization.h"

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] response_data                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct response_data {
  const uint64_t position;
  std::unique_ptr<protocol::serialization_result> response;
  bool prefix_written{false};
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] response_scheduler                                         ( class ) |
// +---------------------------------------------------------------------------+
// | This class holds ready responses in connection order.                    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class response_scheduler {
 public:
  // +=========================================================================+
  // | [>] push_ready                                               ( public ) |
  // +=========================================================================+
  uint64_t push_ready(
      std::unique_ptr<protocol::serialization_result> response) {
    uint64_t position = next_position_;
    responses_.push_back({position, std::move(response)});
    next_position_++;
    return position;
  }
  // +=========================================================================+
  // | [>] empty                                                    ( public ) |
  // +=========================================================================+
  bool empty() const { return responses_.empty(); }
  // +=========================================================================+
  // | [>] size                                                     ( public ) |
  // +=========================================================================+
  std::size_t size() const { return responses_.size(); }
  // +=========================================================================+
  // | [>] front                                                    ( public ) |
  // +=========================================================================+
  response_data& front() { return responses_.front(); }
  // +=========================================================================+
  // | [>] pop_front                                                ( public ) |
  // +=========================================================================+
  void pop_front() { responses_.pop_front(); }
  // +=========================================================================+
  // | [>] clear                                                    ( public ) |
  // +=========================================================================+
  void clear() { responses_.clear(); }

 private:
  // +=========================================================================+
  // | ATTRIBUTEs                                                  ( private ) |
  // +=========================================================================+
  std::deque<response_data> responses_;
  uint64_t next_position_{0};
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
