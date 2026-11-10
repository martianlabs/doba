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

#ifndef martianlabs_doba_transport_send_state_h
#define martianlabs_doba_transport_send_state_h

#include <algorithm>
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "common/reader.h"

namespace martianlabs::doba::transport::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] send_state                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// | Keeps pending output, the active send buffer and one final reader.        |
// | Keep the active buffer unchanged until socket I/O completes.              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct send_state {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit send_state(std::size_t capacity) : capacity_{capacity} {}
  // +=========================================================================+
  // | [>] push                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Copies head/body and takes the optional reader.                         |
  // | Rejects nonempty output while a reader is held.                         |
  // | Inline bytes must fit the remaining capacity.                           |
  // +-------------------------------------------------------------------------+
  bool push(std::string_view head, std::string_view body,
            std::unique_ptr<common::reader> source) {
    if (head.empty() && body.empty() && !source) return true;
    if (source_) return false;
    const auto available = capacity_ - buffer.size() - pending_.size();
    if (head.size() > available || body.size() > available - head.size())
      return false;
    pending_.append(head);
    pending_.append(body);
    source_ = std::move(source);
    return true;
  }
  // +=========================================================================+
  // | [>] fill                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Leaves an unfinished active buffer untouched.                           |
  // | Pending bytes go first, then blocks from the reader.                    |
  // +-------------------------------------------------------------------------+
  bool fill() {
    if (offset != buffer.size()) return true;
    buffer.clear();
    offset = 0;
    // Reuse the drained buffer for later output.
    if (!pending_.empty() || !source_) {
      buffer.swap(pending_);
      return true;
    }
    if (source_->failed()) return false;
    if (source_->eof()) {
      source_.reset();
      return true;
    }
    buffer.resize(std::min<std::size_t>(8192, capacity_));
    const auto count = source_->read(std::as_writable_bytes(std::span(buffer)));
    if (count > buffer.size() || source_->failed() ||
        (!count && !source_->eof()))
      return false;
    buffer.resize(count);
    if (!count) source_.reset();
    return true;
  }
  // +=========================================================================+
  // | [>] consume                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Marks sent bytes as complete; clears the buffer when fully sent.        |
  // | Partial sends still count the whole buffer against capacity.            |
  // +-------------------------------------------------------------------------+
  bool consume(std::size_t sent) {
    if (sent > buffer.size() - offset) return false;
    offset += sent;
    if (offset == buffer.size()) {
      buffer.clear();
      offset = 0;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] queued                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  // | Checks waiting bytes or a reader; excludes the active buffer.           |
  // +-------------------------------------------------------------------------+
  bool queued() const noexcept { return !pending_.empty() || source_; }
  // +=========================================================================+
  // | [>] has_source                                               ( public ) |
  // +-------------------------------------------------------------------------+
  bool has_source() const noexcept { return source_ != nullptr; }
  // +=========================================================================+
  // | [>] clear                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Discard output only after all I/O using buffer has completed.           |
  // +-------------------------------------------------------------------------+
  void clear() {
    buffer.clear();
    pending_.clear();
    source_.reset();
    offset = 0;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Active socket batch; its address stays valid until I/O completes.
  std::string buffer;
  // Bytes already sent from buffer.
  std::size_t offset{0};

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Inline output waiting behind the active buffer.
  std::string pending_;
  // Owned until its last send has completed.
  std::unique_ptr<common::reader> source_;
  // Byte limit for active and pending buffers together.
  const std::size_t capacity_;
};
}  // namespace martianlabs::doba::transport::detail

#endif
