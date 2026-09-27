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
#include <cstddef>
#include <cstring>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>

#include "protocol/contracts.h"
#include "transport/server/output_queue.h"
#include "transport/server/tls_session.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls_connection                                             ( struct ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct tls_connection {
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  using shared_state = tls_context;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  template <protocol::contracts::engine_factory<ENty> FAty>
  tls_connection(std::size_t recv_buffer_size,
                 std::size_t send_buffer_size, const FAty& create_engine,
                 shared_state context)
      : engine{create_engine()},
        buffer{std::make_unique<char[]>(17 * 1024)},
        capacity{17 * 1024},
        session_{std::move(context), send_buffer_size},
        plaintext_{std::make_unique<char[]>(recv_buffer_size)},
        plaintext_capacity_{recv_buffer_size} {}
  // +=========================================================================+
  // | [>] process                                                  ( public ) |
  // +=========================================================================+
  std::size_t process() {
    std::size_t received = 0;
    while (received < size) {
      const std::size_t next = session_.receive(
          std::span(buffer.get() + received, size - received));
      received += next;
      if (!process_plaintext_()) {
        throw std::runtime_error("TLS receive failed!");
      }
      if (!next) break;
    }
    return received;
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
  // +=========================================================================+
  // | [>] prepare_output                                           ( public ) |
  // +=========================================================================+
  bool prepare_output(output_queue& output) {
    for (;;) {
      if (session_.pending()) return true;
      if (!session_.established()) return !session_.failed();
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
      const auto result = session_.write(std::span(
          output.buffer.data() + output.offset,
          output.buffer.size() - output.offset));
      if (result.state == tls_session::status::failed) return false;
      output.offset += result.size;
      if (result.state == tls_session::status::need_input) return true;
      if (result.state == tls_session::status::need_output &&
          !session_.pending()) return false;
    }
  }
  // +=========================================================================+
  // | [>] output_bytes                                             ( public ) |
  // +=========================================================================+
  std::span<char> output_bytes(output_queue&) {
    return session_.output_bytes();
  }
  // +=========================================================================+
  // | [>] output_sent                                              ( public ) |
  // +=========================================================================+
  void output_sent(output_queue&, std::size_t size) {
    session_.output_sent(size);
  }
  // +=========================================================================+
  // | [>] output_pending                                           ( public ) |
  // +=========================================================================+
  bool output_pending(const output_queue& output) const {
    return session_.pending() ||
        output.queued() || output.offset != output.buffer.size() ||
        (close_requested_ && !shutdown_complete_);
  }
  // +=========================================================================+
  // | [>] established                                             ( public ) |
  // +=========================================================================+
  bool established() const { return session_.established(); }
  // +=========================================================================+
  // | [>] peer_closed                                             ( public ) |
  // +=========================================================================+
  bool peer_closed() const { return peer_closed_; }
  // +=========================================================================+
  // | [>] eof                                                     ( public ) |
  // +=========================================================================+
  bool eof() const { return peer_closed_; }
  // +=========================================================================+
  // | [>] close_output                                            ( public ) |
  // +=========================================================================+
  bool close_output() {
    if (!session_.established()) return false;
    close_requested_ = true;
    return true;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  ENty engine;
  std::unique_ptr<char[]> buffer;
  const std::size_t capacity;
  std::size_t size{0};

 private:
  // +=========================================================================+
  // | [>] process_plaintext_                                      ( private ) |
  // +=========================================================================+
  bool process_plaintext_() {
    if (!session_.established()) {
      const auto state = session_.handshake();
      if (state == tls_session::status::failed ||
          state == tls_session::status::closed) return false;
    }
    if (!session_.established()) return true;
    for (;;) {
      if (plaintext_size_ == plaintext_capacity_) return false;
      const auto result = session_.read(std::span(
          plaintext_.get() + plaintext_size_,
          plaintext_capacity_ - plaintext_size_));
      if (result.state == tls_session::status::failed) return false;
      if (result.state == tls_session::status::closed) {
        peer_closed_ = true;
        return true;
      }
      if (result.state != tls_session::status::ready) return true;
      if (!result.size) return true;
      plaintext_size_ += result.size;
      const std::size_t processed = engine.on_bytes_received(
          plaintext_.get(), plaintext_size_, plaintext_capacity_);
      if (processed > plaintext_size_) return false;
      plaintext_size_ -= processed;
      if (processed && plaintext_size_) {
        std::memmove(plaintext_.get(), plaintext_.get() + processed,
                     plaintext_size_);
      }
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  tls_session session_;
  std::unique_ptr<char[]> plaintext_;
  const std::size_t plaintext_capacity_;
  std::size_t plaintext_size_{0};
  bool close_requested_{false};
  bool shutdown_complete_{false};
  bool peer_closed_{false};
};
}  // namespace martianlabs::doba::transport::server

#endif
