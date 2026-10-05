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

#include "common/reader.h"
#include "protocol/contracts.h"

namespace martianlabs::doba::transport::server {
struct send_state {
  struct source_entry {
    std::string prefix;
    std::unique_ptr<common::reader> source;
  };

  explicit send_state(std::size_t capacity) : capacity_{capacity} {}

  bool push(std::string_view head, std::string_view body,
            std::unique_ptr<common::reader> source) {
    const std::size_t size = head.size() + body.size();
    if (size > capacity_ - inline_bytes_ - source_reservation_) {
      return false;
    }
    if (source) {
      if (sources_.empty() && !active_from_source_) {
        source_reservation_ = std::min<std::size_t>(
            8192, capacity_ - inline_bytes_ - size);
      }
      pending_.append(head);
      pending_.append(body);
      inline_bytes_ += size;
      sources_.push_back({std::move(pending_), std::move(source)});
      pending_.clear();
    } else {
      pending_.append(head);
      pending_.append(body);
      inline_bytes_ += size;
    }
    return true;
  }

  bool fill() {
    buffer.clear();
    offset = 0;
    active_from_source_ = false;
    for (;;) {
      if (sources_.empty()) {
        buffer.swap(pending_);
        return true;
      }
      auto& entry = sources_.front();
      if (!entry.prefix.empty()) {
        buffer.swap(entry.prefix);
        return true;
      }
      if (entry.source->failed()) return false;
      if (entry.source->eof()) {
        sources_.pop_front();
        if (sources_.empty()) source_reservation_ = 0;
        continue;
      }
      const std::size_t available = std::min<std::size_t>(8192, capacity_);
      buffer.resize(available);
      const std::size_t count = entry.source->read(
          std::span<std::byte>(reinterpret_cast<std::byte*>(buffer.data()),
                               available));
      buffer.resize(count);
      if (entry.source->failed() || (!count && !entry.source->eof())) {
        return false;
      }
      if (count) {
        active_from_source_ = true;
        return true;
      }
    }
  }

  bool consume(std::size_t sent) {
    if (sent > buffer.size() - offset) return false;
    offset += sent;
    if (!active_from_source_) inline_bytes_ -= sent;
    return true;
  }

  bool queued() const { return !pending_.empty() || !sources_.empty(); }

  void clear() {
    buffer.clear();
    pending_.clear();
    sources_.clear();
    offset = 0;
    inline_bytes_ = 0;
    source_reservation_ = 0;
    active_from_source_ = false;
  }

  std::string buffer;
  std::size_t offset{0};

 private:
  std::string pending_;
  std::deque<source_entry> sources_;
  std::size_t capacity_;
  std::size_t inline_bytes_{0};
  std::size_t source_reservation_{0};
  bool active_from_source_{false};
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcp_connection                                             ( struct ) |
// +---------------------------------------------------------------------------+
// | Adapts a TCP byte stream to an HTTP engine.                               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
struct tcp_connection {
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
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
  std::size_t process() {
    return engine.on_bytes_received(buffer.get(), size, capacity);
  }
  // +=========================================================================+
  // | [>] consume                                                  ( public ) |
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
  bool prepare_output(send_state& output) {
    return output.offset != output.buffer.size() || output.fill();
  }
  std::span<char> output_bytes(send_state& output) {
    return std::span(output.buffer).subspan(output.offset);
  }
  // +=========================================================================+
  // | [>] output_buffers                                           ( public ) |
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
  bool output_sent(send_state& output, std::size_t sent) {
    return output.consume(sent);
  }
  // +=========================================================================+
  // | [>] output_pending                                           ( public ) |
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
