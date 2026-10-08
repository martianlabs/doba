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

#ifndef martianlabs_doba_transport_server_send_state_h
#define martianlabs_doba_transport_server_send_state_h

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
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] send_state                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct manages the state of outgoing data for a TCP connection.      |
// | It maintains a buffer for inline data and a queue of sources for          |
// | additional data to be sent. It provides methods to push new data, fill    |
// | the buffer, consume sent data, and check if there is pending data         |
// | to be sent.                                                               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct send_state {
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  struct source_entry {
    // Prefix data to be sent before reading from the source (memory based)
    std::string prefix;
    // Source reader for additional data (stream based)
    std::unique_ptr<common::reader> source;
  };
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit send_state(std::size_t capacity) : capacity_{capacity} {}
  // +=========================================================================+
  // | [>] push                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Tries to add a new data entry to the send state.                        |
  // | It checks whether there is enough room for the head, body, and any      |
  // | space reserved for sources. If everything fits, it adds the head and    |
  // | body to the pending buffer and puts the source in the sources queue,    |
  // | if one was provided. Returns true if the push succeeds,                 |
  // | or false if it doesn’t.                                                 |
  // +-------------------------------------------------------------------------+
  bool push(std::string_view head, std::string_view body,
            std::unique_ptr<common::reader> source) {
    const std::size_t size = head.size() + body.size();
    if (size > capacity_ - inline_bytes_ - source_reservation_) return false;
    if (source) {
      if (sources_.empty() && !active_from_source_) {
        source_reservation_ =
            std::min<std::size_t>(8192, capacity_ - inline_bytes_ - size);
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
  // +=========================================================================+
  // | [>] fill                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Fills the buffer with data from the pending buffer or the               |
  // | sources queue. First, it clears the current buffer and resets the       |
  // | offset. If there are no sources, it swaps the pending buffer into the   |
  // | main buffer and returns true. Otherwise, it checks the first source     |
  // | for a prefix to send. If it finds one, it swaps it into the main        |
  // | buffer and returns true. If the source has failed or reached EOF,       |
  // | it removes it from the queue and moves on. When source data is          |
  // | available, it reads up to 8192 bytes—or whatever space is left—into     |
  // | the main buffer and returns true if the read succeeds.If the read       |
  // | fails or there’s no data available, it returns false.                   |
  // +-------------------------------------------------------------------------+
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
      const std::size_t count = entry.source->read(std::span<std::byte>(
          reinterpret_cast<std::byte*>(buffer.data()), available));
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
  // +=========================================================================+
  // | [>] consume                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Removes the requested number of bytes from the buffer. If that’s more   |
  // | than the number of bytes left, it returns false. Otherwise, it moves    |
  // | the offset forward. If the data came from the pending buffer rather     |
  // | than an external source, it also reduces the inline byte count.         |
  // | Returns true when the bytes have been consumed successfully.            |
  // +-------------------------------------------------------------------------+
  bool consume(std::size_t sent) {
    if (sent > buffer.size() - offset) return false;
    offset += sent;
    if (!active_from_source_) inline_bytes_ -= sent;
    return true;
  }
  // +=========================================================================+
  // | [>] queued                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  // | Checks whether there’s any data waiting to be sent. Returns true if the |
  // | pending buffer or the sources queue contains data.                      |
  // +-------------------------------------------------------------------------+
  bool queued() const { return !pending_.empty() || !sources_.empty(); }
  // +=========================================================================+
  // | [>] clear                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Clears the send state by removing all pending data and resetting the    |
  // | buffer, offset, inline byte count, source reservation, and active       |
  // | source flag.                                                            |
  // +-------------------------------------------------------------------------+
  void clear() {
    buffer.clear();
    pending_.clear();
    sources_.clear();
    offset = 0;
    inline_bytes_ = 0;
    source_reservation_ = 0;
    active_from_source_ = false;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  std::string buffer;
  std::size_t offset{0};

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::string pending_;
  std::deque<source_entry> sources_;
  std::size_t capacity_;
  std::size_t inline_bytes_{0};
  std::size_t source_reservation_{0};
  bool active_from_source_{false};
};
}  // namespace martianlabs::doba::transport::server

#endif
