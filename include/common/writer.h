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

#ifndef martianlabs_doba_common_writer_h
#define martianlabs_doba_common_writer_h

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include "common/byte_storage.h"

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] writer ( class )                                                      |
// +---------------------------------------------------------------------------+
// | This class provides a writer interface for writing bytes to               |
// | a byte_storage backend. It supports writing from a pointer, a string_view,|
// | or a span of bytes. It also provides methods to finish the writing process|
// | and check the state of the writer.                                        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class writer {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  writer() = default;
  explicit writer(byte_storage_options opts) : storage_(std::move(opts)) {}
  writer(const writer&) = delete;
  writer(writer&&) noexcept = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  writer& operator=(const writer&) = delete;
  writer& operator=(writer&&) noexcept = default;
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +=========================================================================+
  bool write(const char* ptr, std::size_t len) {
    if (!storage_.write(ptr, len)) return false;
    bytes_ += len;
    return true;
  }
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +=========================================================================+
  bool write(std::string_view bytes) {
    return write(bytes.data(), bytes.size());
  }
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +=========================================================================+
  bool write(std::span<const std::byte> bytes) {
    return write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
  }
  // +=========================================================================+
  // | [>] finish                                                   ( public ) |
  // +=========================================================================+
  void finish(std::size_t bytes) {
    storage_.finish(bytes);
    finished_ = true;
  }
  // +=========================================================================+
  // | [>] ok                                                       ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool ok() const noexcept { return storage_.ok(); }
  // +=========================================================================+
  // | [>] release                                                  ( public ) |
  // +=========================================================================+
  [[nodiscard]] byte_storage release() {
    if (!finished_) finish(bytes_);
    return std::move(storage_);
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  byte_storage storage_;
  std::size_t bytes_{0};
  bool finished_{false};
};
}  // namespace martianlabs::doba::common

#endif
