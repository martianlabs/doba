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

#ifndef martianlabs_doba_transport_server_response_completion_h
#define martianlabs_doba_transport_server_response_completion_h

#include <cstdint>
#include <memory>
#include <utility>

#include "protocol/serialization.h"

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] response_completion                                        ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct response_completion {
  response_completion(
      uint64_t in_connection_key, uint64_t in_position,
      std::unique_ptr<protocol::serialization_result> in_response)
      : connection_key{in_connection_key},
        position{in_position},
        response{std::move(in_response)} {}

  const uint64_t connection_key;
  const uint64_t position;
  std::unique_ptr<protocol::serialization_result> response;
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
