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

#ifndef martianlabs_doba_transport_server_tls_connection_h
#define martianlabs_doba_transport_server_tls_connection_h

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstring>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>

#include "protocol/contracts.h"
#include "transport/server/tcp_connection.h"
#include "transport/server/tls_session.h"

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls_connection                                             ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct wraps an engine and a TLS session to handle an encrypted      |
// | connection. It manages the encrypted and plaintext buffers, handles       |
// | the TLS handshake, and processes incoming and outgoing data. It also      |
// | lets you process received data, prepare output, and check the             |
// | connection state.                                                         |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   ENty - engine type being used.                                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct tls_connection {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using shared_state = tls_context;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  template <protocol::contracts::engine_factory<ENty> FAty>
  tls_connection(std::size_t receive_buffer_capacity,
                 std::size_t send_buffer_size,
                 const FAty& create_engine, shared_state context)
      : tls_connection(receive_buffer_capacity, send_buffer_size, create_engine,
                       std::move(context),
                       tls_policies::kDefaultEncryptedReceiveBufferCapacity,
                       tls_policies::kDefaultNetworkBioBufferCapacity) {}
  template <protocol::contracts::engine_factory<ENty> FAty>
  tls_connection(std::size_t receive_buffer_capacity,
                 std::size_t send_buffer_size,
                 const FAty& create_engine, shared_state context,
                 std::size_t encrypted_receive_buffer_capacity,
                 std::size_t network_bio_buffer_capacity)
      : engine{create_engine()},
        buffer{std::make_unique<char[]>(encrypted_receive_buffer_capacity)},
        capacity{encrypted_receive_buffer_capacity},
        session_{std::move(context), send_buffer_size,
                 network_bio_buffer_capacity},
        plaintext_{std::make_unique<char[]>(receive_buffer_capacity)},
        plaintext_capacity_{receive_buffer_capacity} {}
  // +=========================================================================+
  // | [>] process                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Reads data from the encrypted buffer and passes it to the engine.       |
  // | Only TLS retains encrypted leftovers. If the TLS handshake fails or    |
  // | the connection closes, it throws an exception.                          |
  // +-------------------------------------------------------------------------+
  std::span<char> receive_buffer() noexcept {
    return {buffer.get() + size, capacity - size};
  }
  void process(std::size_t count) {
    if (count > capacity - size) {
      throw std::out_of_range("TLS receive size exceeds buffer");
    }
    size += count;
    std::size_t received = 0;
    while (received < size) {
      const std::size_t next =
          session_.receive(std::span(buffer.get() + received, size - received));
      received += next;
      if (!process_plaintext_()) {
        throw std::runtime_error("TLS receive failed!");
      }
      if (input_paused()) break;
      if (!next) break;
    }
    size -= received;
    if (received && size) {
      std::memmove(buffer.get(), buffer.get() + received, size);
    }
    if (size == capacity && !input_paused()) throw std::runtime_error("TLS receive stalled");
  }
  // +=========================================================================+
  // | [>] prepare_output                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Gets the output buffer ready to send. It processes any pending TLS      |
  // | data, handles a requested TLS shutdown, and fills the buffer with       |
  // | encrypted data. It returns false if anything goes wrong; otherwise,     |
  // | it returns true.                                                        |
  // +-------------------------------------------------------------------------+
  bool prepare_output(send_state& output) {
    for (;;) {
      if (session_.pending()) return true;
      if (!session_.established()) {
        const auto state = session_.handshake();
        if (state == tls_session::status::failed || state == tls_session::status::closed)
          return false;
        if (session_.pending()) continue;
        if (!session_.established()) return true;
      }
      if (close_requested_ && output.offset == output.buffer.size() &&
          !output.queued()) {
        const auto state = session_.shutdown();
        if (state == tls_session::status::failed) return false;
        if (state == tls_session::status::need_output) continue;
        shutdown_complete_ = true;
        return true;
      }
      if (output.offset == output.buffer.size()) {
        if (!output.fill()) return false;
        if (output.buffer.empty()) {
          if (!close_requested_) return true;
          continue;
        }
      }
      const auto result =
          session_.write(std::span(output.buffer.data() + output.offset,
                                   output.buffer.size() - output.offset));
      if (result.state == tls_session::status::failed) return false;
      if (!output.consume(result.size)) return false;
      if (result.state == tls_session::status::need_input) return true;
      if (result.state == tls_session::status::need_output &&
          !session_.pending())
        return false;
    }
  }
  // +=========================================================================+
  // | [>] output_bytes                                             ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns a span of encrypted data that’s ready to send.                  |
  // | It uses `send_state` to find the current output buffer and offset.      |
  // +-------------------------------------------------------------------------+
  std::span<char> output_bytes(send_state&) { return session_.output_bytes(); }
  // +=========================================================================+
  // | [>] output_buffers                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Fills the given buffers with encrypted data that’s ready to send.       |
  // | It returns the number of buffers filled, or 0 if there’s nothing        |
  // | to send. It uses `send_state` to find the current output                |
  // | buffer and offset.                                                      |
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
  // | Tells the connection how many bytes were sent. It updates the TLS       |
  // | session and returns true.                                               |
  // +-------------------------------------------------------------------------+
  bool output_sent(send_state&, std::size_t sent) {
    session_.output_sent(sent);
    return true;
  }
  // +=========================================================================+
  // | [>] output_pending                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns true if there’s still data to send in either the TLS session    |
  // | or the output buffer. It uses `send_state` to find the current          |
  // | output buffer and offset.                                               |
  // +-------------------------------------------------------------------------+
  bool output_pending(const send_state& output) const {
    return session_.output_pending() || output.queued() ||
           output.offset != output.buffer.size() ||
           (close_requested_ && !shutdown_complete_);
  }
  // +=========================================================================+
  // | [>] established                                              ( public ) |
  // +-------------------------------------------------------------------------+
  bool established() const { return session_.established(); }
  // +=========================================================================+
  // | [>] peer_closed                                              ( public ) |
  // +-------------------------------------------------------------------------+
  bool peer_closed() const { return peer_closed_; }
  // +=========================================================================+
  // | [>] eof                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  bool eof() const { return peer_closed_; }
  // +=========================================================================+
  // | [>] close_output                                             ( public ) |
  // +-------------------------------------------------------------------------+
  bool close_output() {
    if (!session_.established()) return false;
    close_requested_ = true;
    return true;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  bool input_paused() const {
    if (plaintext_resume_.load(std::memory_order_relaxed)) return true;
    if constexpr (requires { engine.input_paused(); }) {
      return engine.input_paused();
    } else {
      return false;
    }
  }
  void resume_input() {
    plaintext_resume_.store(false, std::memory_order_relaxed);
    if constexpr (requires { engine.resume_input(); }) engine.resume_input();
    if (!input_paused()) {
      if (!process_plaintext_()) throw std::runtime_error("TLS receive failed!");
      if (!input_paused()) process(0);
    }
  }
  ENty engine;
  std::unique_ptr<char[]> buffer;
  const std::size_t capacity;
  std::size_t size{0};

 private:
  // +=========================================================================+
  // | [>] process_plaintext_                                      ( private ) |
  // +-------------------------------------------------------------------------+
  bool process_plaintext_() {
    if (!session_.established()) {
      const auto state = session_.handshake();
      if (state == tls_session::status::need_output) {
        plaintext_resume_.store(true, std::memory_order_relaxed);
        return true;
      }
      if (state == tls_session::status::failed ||
          state == tls_session::status::closed) {
        return false;
      }
    }
    if (!session_.established()) return true;
    for (;;) {
      if (input_paused()) return true;
      const auto result =
          session_.read(std::span(plaintext_.get(), plaintext_capacity_));
      if (result.state == tls_session::status::failed) return false;
      if (result.state == tls_session::status::closed) {
        peer_closed_ = true;
        return true;
      }
      if (result.state == tls_session::status::need_output)
        plaintext_resume_.store(true, std::memory_order_relaxed);
      if (result.state != tls_session::status::ready) return true;
      if (!result.size) return true;
      engine.on_bytes_received({plaintext_.get(), result.size});
      if (close_requested_) return true;
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // A TLS read needs another turn after its required output has drained.
  std::atomic<bool> plaintext_resume_{false};
  tls_session session_;
  std::unique_ptr<char[]> plaintext_;
  const std::size_t plaintext_capacity_;
  std::atomic<bool> close_requested_{false};
  bool shutdown_complete_{false};
  bool peer_closed_{false};
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
