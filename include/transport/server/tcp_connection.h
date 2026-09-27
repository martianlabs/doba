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

#ifndef martianlabs_doba_transport_server_tcp_connection_h
#define martianlabs_doba_transport_server_tcp_connection_h

#include <cstddef>
#include <cstring>
#include <memory>
#include <span>

#include "protocol/contracts.h"
#include "transport/server/output_queue.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcp_connection                                             ( struct ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct tcp_connection {
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  using shared_state = std::nullptr_t;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  template <protocol::contracts::engine_factory<ENty> FAty>
  tcp_connection(std::size_t buffer_size, const FAty& create_engine)
      : engine{create_engine()},
        buffer{std::make_unique<char[]>(buffer_size)},
        capacity{buffer_size} {}
  template <protocol::contracts::engine_factory<ENty> FAty>
  tcp_connection(std::size_t buffer_size, std::size_t,
                 const FAty& create_engine, shared_state)
      : tcp_connection(buffer_size, create_engine) {}
  // +=========================================================================+
  // | [>] process                                                  ( public ) |
  // +=========================================================================+
  std::size_t process() {
    return engine.on_bytes_received(buffer.get(), size, capacity);
  }
  // +=========================================================================+
  // | [>] consume                                                  ( public ) |
  // +=========================================================================+
  bool consume(std::size_t processed) {
    if (processed > size) return false;
    size -= processed;
    if (processed && size) {
      std::memmove(buffer.get(), buffer.get() + processed, size);
    }
    return true;
  }
  bool prepare_output(output_queue& output) {
    return output.offset != output.buffer.size() || output.fill();
  }
  std::span<char> output_bytes(output_queue& output) {
    return {output.buffer.data() + output.offset,
            output.buffer.size() - output.offset};
  }
  void output_sent(output_queue& output, std::size_t sent) {
    output.offset += sent;
  }
  bool output_pending(const output_queue& output) const {
    return output.queued() || output.offset != output.buffer.size();
  }
  bool established() const { return true; }
  bool peer_closed() const { return false; }
  bool eof() const { return true; }
  bool close_output() { return true; }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  ENty engine;
  std::unique_ptr<char[]> buffer;
  const std::size_t capacity;
  std::size_t size{0};
};
}  // namespace martianlabs::doba::transport::server

#endif
