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

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <deque>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "send_state.h"
#include "common/reader.h"
#include "protocol/contracts.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcp_connection                                             ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct keeps track of a TCP connection used by an engine.            |
// | It stores the engine and an input buffer, and handles incoming data,      |
// | consumed bytes, and outgoing data. It also lets you check the connection  |
// | status and manage the output buffers.                                     |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   ENty - engine type being used.                                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct tcp_connection {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using shared_state = std::nullptr_t;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
  // | Processes the received bytes in the input buffer using the engine.      |
  // +-------------------------------------------------------------------------+
  std::size_t process() {
    return engine.on_bytes_received(buffer.get(), size, capacity);
  }
  // +=========================================================================+
  // | [>] consume                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Removes the specified number of processed bytes from the input buffer.  |
  // +-------------------------------------------------------------------------+
  bool consume(std::size_t processed) {
    if (processed > size) return false;
    size -= processed;
    if (processed && size) {
      std::memmove(buffer.get(), buffer.get() + processed, size);
    }
    return true;
  }
  // +=========================================================================+
  // | [>] prepare_output                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Gets the output buffer ready to send data.                              |
  // | If it already contains unsent bytes, or the engine can add more data    |
  // | to it, the method returns true. Otherwise, it returns false.            |
  // +-------------------------------------------------------------------------+
  bool prepare_output(send_state& output) {
    return output.offset != output.buffer.size() || output.fill();
  }
  // +=========================================================================+
  // | [>] output_bytes                                             ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns a span of the unsent bytes in the output buffer.                |
  // | If there are no unsent bytes, it returns an empty span.                 |
  // +-------------------------------------------------------------------------+
  std::span<char> output_bytes(send_state& output) {
    return std::span(output.buffer).subspan(output.offset);
  }
  // +=========================================================================+
  // | [>] output_buffers                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Fills the provided array of buffers with the unsent bytes               |
  // | in the output buffer.                                                   |
  // +-------------------------------------------------------------------------+
  std::size_t output_buffers(send_state& output,
                             std::span<std::span<char>> buffers) {
    if (buffers.empty()) return 0;
    buffers[0] = output_bytes(output);
    return buffers[0].empty() ? 0 : 1;
  }
  // +=========================================================================+
  // | [>] output_sent                                              ( public ) |
  // +-------------------------------------------------------------------------+
  // | Updates the output buffer after sending the specified number of bytes.  |
  // +-------------------------------------------------------------------------+
  bool output_sent(send_state& output, std::size_t sent) {
    return output.consume(sent);
  }
  // +=========================================================================+
  // | [>] output_pending                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Checks if there are any unsent bytes in the output buffer.              |
  // +-------------------------------------------------------------------------+
  bool output_pending(const send_state& output) const {
    return output.queued() || output.offset != output.buffer.size();
  }
  // +=========================================================================+
  // | [>] established                                              ( public ) |
  // +-------------------------------------------------------------------------+
  bool established() const { return true; }
  // +=========================================================================+
  // | [>] peer_closed                                              ( public ) |
  // +-------------------------------------------------------------------------+
  bool peer_closed() const { return false; }
  // +=========================================================================+
  // | [>] eof                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  bool eof() const { return true; }
  // +=========================================================================+
  // | [>] close_output                                             ( public ) |
  // +-------------------------------------------------------------------------+
  bool close_output() { return true; }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  ENty engine;
  std::unique_ptr<char[]> buffer;
  const std::size_t capacity;
  std::size_t size{0};
};
}  // namespace martianlabs::doba::transport::server

#endif
