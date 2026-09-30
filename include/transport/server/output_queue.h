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

#ifndef martianlabs_doba_transport_server_output_queue_h
#define martianlabs_doba_transport_server_output_queue_h

#include <algorithm>
#include <cstddef>
#include <list>
#include <memory>
#include <span>
#include <string>
#include <tuple>
#include <utility>

#include "common/reader.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] output_queue                                               ( struct ) |
// +---------------------------------------------------------------------------+
// | Queues pending transport output segments.                                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct output_queue {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  explicit output_queue(std::size_t capacity) : capacity_{capacity} {}
  // +=========================================================================+
  // | [>] push                                                     ( public ) |
  // +=========================================================================+
  bool push(std::string&& head, std::string&& body,
            std::unique_ptr<common::reader>&& source) {
    const std::size_t available = capacity_ - queued_bytes_ - buffer.size();
    if (head.size() > available || body.size() > available - head.size()) {
      return false;
    }
    const std::size_t size = head.size() + body.size();
    const std::size_t reserved = source
        ? std::max(size, std::min<std::size_t>(8192, capacity_)) : size;
    if (reserved > available) return false;
    if (!size && !source) return true;
    queue_.emplace_back(std::move(head), std::move(body), 0, 0,
                        std::move(source), reserved);
    queued_bytes_ += reserved;
    return true;
  }
  // +=========================================================================+
  // | [>] prefix_bytes                                            ( public )  |
  // +=========================================================================+
  std::span<char> prefix_bytes() {
    if (queue_.empty()) return {};
    auto& entry = queue_.front();
    auto& [head, body, head_offset, body_offset, source, reserved] = entry;
    if (head_offset < head.size()) {
      return std::span(head).subspan(head_offset);
    }
    if (body_offset < body.size()) {
      return std::span(body).subspan(body_offset);
    }
    return {};
  }
  // +=========================================================================+
  // | [>] prefix_buffers                                          ( public ) |
  // +=========================================================================+
  std::size_t prefix_buffers(std::span<std::span<char>> buffers) {
    std::size_t count = 0;
    for (auto& entry : queue_) {
      auto& [head, body, head_offset, body_offset, source, reserved] = entry;
      if (head_offset < head.size()) {
        if (count == buffers.size()) break;
        buffers[count++] = std::span(head).subspan(head_offset);
      }
      if (body_offset < body.size()) {
        if (count == buffers.size()) break;
        buffers[count++] = std::span(body).subspan(body_offset);
      }
      if (source) break;
    }
    return count;
  }
  // +=========================================================================+
  // | [>] consume_prefixes                                        ( public )  |
  // +=========================================================================+
  bool consume_prefixes(std::size_t sent) {
    while (sent) {
      if (queue_.empty()) return false;
      auto& entry = queue_.front();
      auto& [head, body, head_offset, body_offset, source, reserved] = entry;
      const std::size_t head_left = head.size() - head_offset;
      const std::size_t body_left = body.size() - body_offset;
      if (!head_left && !body_left) return false;
      const std::size_t taken = std::min(sent,
                                         head_left ? head_left : body_left);
      if (head_left) head_offset += taken;
      else body_offset += taken;
      reserved -= taken;
      queued_bytes_ -= taken;
      sent -= taken;
      if (head_offset == head.size() && body_offset == body.size() &&
          !source) queue_.pop_front();
    }
    return true;
  }
  // +=========================================================================+
  // | [>] prefix_pending                                          ( public )  |
  // +=========================================================================+
  bool prefix_pending() const {
    return !queue_.empty() &&
           (std::get<2>(queue_.front()) < std::get<0>(queue_.front()).size() ||
            std::get<3>(queue_.front()) < std::get<1>(queue_.front()).size());
  }
  // +=========================================================================+
  // | [>] fill                                                     ( public ) |
  // +=========================================================================+
  bool fill() {
    buffer.clear();
    offset = 0;
    while (!queue_.empty()) {
      auto& [head, body, head_offset, body_offset, source, reserved] =
          queue_.front();
      queued_bytes_ -= reserved;
      reserved = 0;
      if (head_offset != head.size()) {
        buffer.append(head.data() + head_offset, head.size() - head_offset);
        head.clear();
        head_offset = 0;
      }
      if (body_offset != body.size()) {
        buffer.append(body.data() + body_offset, body.size() - body_offset);
        body.clear();
        body_offset = 0;
      }
      if (source) {
        if (source->failed()) return !buffer.empty();
        while (!source->eof()) {
          const std::size_t available = std::min<std::size_t>(
              8192, capacity_ - queued_bytes_ - buffer.size());
          if (!available) return true;
          const std::size_t next = buffer.size();
          buffer.resize(next + available);
          const std::size_t read = source->read(std::span<std::byte>(
              reinterpret_cast<std::byte*>(buffer.data() + next),
              available));
          buffer.resize(next + read);
          if (source->failed() || (!read && !source->eof())) {
            return !buffer.empty();
          }
        }
      }
      queue_.pop_front();
    }
    return true;
  }
  // +=========================================================================+
  // | [>] queued                                                   ( public ) |
  // +=========================================================================+
  bool queued() const { return !queue_.empty(); }
  // +=========================================================================+
  // | [>] clear                                                    ( public ) |
  // +=========================================================================+
  void clear() {
    queue_.clear();
    buffer.clear();
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  std::string buffer;
  std::size_t offset{0};

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::list<std::tuple<std::string, std::string, std::size_t, std::size_t,
                       std::unique_ptr<common::reader>, std::size_t>> queue_;
  const std::size_t capacity_;
  std::size_t queued_bytes_{0};
};
}  // namespace martianlabs::doba::transport::server

#endif
