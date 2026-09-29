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
// | Internal implementation detail.                                           |
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
  bool push(std::unique_ptr<char[]>&& prefix, std::size_t size,
            std::unique_ptr<common::reader>&& source) {
    const std::size_t reserved = source
        ? std::max(size, std::min<std::size_t>(8192, capacity_)) : size;
    if ((size && !prefix) ||
        reserved > capacity_ - queued_bytes_ - buffer.size()) return false;
    if (!size && !source) return true;
    queue_.emplace_back(std::move(prefix), size, std::move(source), reserved);
    queued_bytes_ += reserved;
    return true;
  }
  // +=========================================================================+
  // | [>] fill                                                     ( public ) |
  // +=========================================================================+
  bool fill() {
    buffer.clear();
    offset = 0;
    while (!queue_.empty()) {
      auto& [prefix, size, source, reserved] = queue_.front();
      queued_bytes_ -= reserved;
      reserved = 0;
      if (size) {
        buffer.append(prefix.get(), size);
        prefix.reset();
        size = 0;
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
  std::list<std::tuple<std::unique_ptr<char[]>, std::size_t,
                       std::unique_ptr<common::reader>, std::size_t>> queue_;
  const std::size_t capacity_;
  std::size_t queued_bytes_{0};
};
}  // namespace martianlabs::doba::transport::server

#endif
