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

#ifndef martianlabs_doba_transport_tcp_connection_h
#define martianlabs_doba_transport_tcp_connection_h

#include <cstddef>
#include <memory>
#include <span>
#include <stdexcept>

#include "protocol/contracts.h"

namespace martianlabs::doba::transport::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcp_connection                                             ( struct ) |
// +---------------------------------------------------------------------------+
// | Owns a connection's engine and fixed receive buffer.                      |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   ENty - engine type                                                      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct tcp_connection {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // | Template parameters:                                                    |
  // |   FAty - engine factory type                                            |
  // +-------------------------------------------------------------------------+
  template <protocol::contracts::engine_factory<ENty> FAty>
  tcp_connection(std::size_t buffer_size, const FAty& create_engine)
      : engine{create_engine()},
        buffer{std::make_unique<char[]>(buffer_size)},
        capacity{buffer_size} {}
  // +=========================================================================+
  // | [>] receive_buffer                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Writable space for the next socket receive.                             |
  // +-------------------------------------------------------------------------+
  std::span<char> receive_buffer() noexcept { return {buffer.get(), capacity}; }
  // +=========================================================================+
  // | [>] process                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Passes received buffer bytes to the engine; rejects oversized counts.   |
  // | The engine must copy any input it needs after this call.                |
  // +-------------------------------------------------------------------------+
  protocol::processing_result process(std::size_t received) {
    if (received > capacity) {
      throw std::out_of_range("TCP receive size exceeds buffer");
    }
    return engine.on_bytes_received({buffer.get(), received});
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Receive and processing calls must run one at a time.
  ENty engine;
  // Reused after each engine call returns.
  std::unique_ptr<char[]> buffer;
  // Receive capacity, separate from protocol message limits.
  const std::size_t capacity;
};
}  // namespace martianlabs::doba::transport::detail

#endif
