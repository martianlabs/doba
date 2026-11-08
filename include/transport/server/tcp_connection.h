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

#ifndef martianlabs_doba_transport_server_tcp_connection_h
#define martianlabs_doba_transport_server_tcp_connection_h

#include <cstddef>
#include <memory>
#include <span>
#include <stdexcept>

#include "protocol/contracts.h"

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcp_connection                                             ( struct ) |
// +---------------------------------------------------------------------------+
// | Own the engine and its fixed RX buffer. Deliver fresh bytes               |
// | synchronously; the protocol keeps unfinished messages.                    |
// +---------------------------------------------------------------------------+
// | ENty is the protocol engine.                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct tcp_connection {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  template <protocol::contracts::engine_factory<ENty> FAty>
  tcp_connection(std::size_t buffer_size, const FAty& create_engine)
      : engine{create_engine()},
        buffer{std::make_unique<char[]>(buffer_size)},
        capacity{buffer_size} {}
  // +=========================================================================+
  // | [>] process                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Hand the received bytes to the engine.                                  |
  // +-------------------------------------------------------------------------+
  // +-------------------------------------------------------------------------+
  // | [>] receive_buffer                                           ( public ) |
  // +-------------------------------------------------------------------------+
  std::span<char> receive_buffer() noexcept {
    return {buffer.get(), capacity};
  }
  // +-------------------------------------------------------------------------+
  // | [>] process                                                  ( public ) |
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
  // Call this engine only from serialized receive processing.
  ENty engine;
  // Reuse this buffer once the engine call returns.
  std::unique_ptr<char[]> buffer;
  // Socket RX block size, separate from protocol message limits.
  const std::size_t capacity;
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
