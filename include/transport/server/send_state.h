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

#ifndef martianlabs_doba_transport_server_send_state_h
#define martianlabs_doba_transport_server_send_state_h

#include <algorithm>
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "common/reader.h"

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] send_state                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// | Own the active socket batch, buffered output and one terminal reader.     |
// | While I/O is pending, only consume may change that active batch.          |
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
  // Copy inline bytes before returning. A reader ends this input call's output:
  // reject later nonempty sends until its final batch drains.
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
  // WSASend still borrows these bytes, so don't move or resize them. Send the
  // prefix before reading the source. Read source blocks only after inline
  // output drains; no speculative credit needed.
  bool fill() {
    if (offset != buffer.size()) return true;
    buffer.clear();
    offset = 0;
    // Reuse idle TX storage for the next RX batch's output.
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
        (!count && !source_->eof())) return false;
    buffer.resize(count);
    if (!count) source_.reset();
    return true;
  }
  // +=========================================================================+
  // | [>] consume                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // Charge the whole active batch even after partial sends. Keep the reader
  // until its last socket block completes.
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
  bool queued() const noexcept { return !pending_.empty() || source_; }
  // +=========================================================================+
  // | [>] has_source                                               ( public ) |
  // +-------------------------------------------------------------------------+
  bool has_source() const noexcept { return source_ != nullptr; }
  // +=========================================================================+
  // | [>] clear                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // Call only when no pending socket I/O borrows buffer.
  void clear() {
    buffer.clear();
    pending_.clear();
    source_.reset();
    offset = 0;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Active socket bytes; keep their address stable until completion.
  std::string buffer;
  // Bytes already sent from the active batch.
  std::size_t offset{0};

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Inline bytes added while the kernel owns the active batch.
  std::string pending_;
  // One terminal reader, held through its last socket completion.
  std::unique_ptr<common::reader> source_;
  // Cap active and buffered bytes together; don't preallocate the budget.
  const std::size_t capacity_;
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
