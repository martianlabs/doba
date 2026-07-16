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

#ifndef martianlabs_doba_common_reader_h
#define martianlabs_doba_common_reader_h

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <string>

#include "common/byte_storage.h"

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] reader                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | This class provides a reader interface for reading bytes from a           |
// | byte_storage backend. It supports reading into a span of bytes, fetching  |
// | individual bytes, and checking the state of the reader (e.g., whether it  |
// | has reached the end of the storage or encountered an error). It can also  |
// | read all remaining bytes into a string.                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class reader {
 public:
  // Creates a non-owning reader over stable caller storage. The referenced
  // bytes must outlive this reader and every object to which it is moved.
  static reader borrowed(std::span<const std::byte> source) noexcept {
    return reader(source, borrowed_tag{});
  }

  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  reader() = default;
  explicit reader(byte_storage storage) : storage_(std::move(storage)) {}
  reader(const reader&) = delete;
  reader(reader&& in) noexcept
      : storage_(std::move(in.storage_)),
        borrowed_(in.borrowed_),
        borrowed_pos_(in.borrowed_pos_),
        failed_(in.failed_),
        is_borrowed_(in.is_borrowed_) {
    in.borrowed_ = {};
    in.borrowed_pos_ = 0;
    in.failed_ = false;
    in.is_borrowed_ = false;
  }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  reader& operator=(const reader&) = delete;
  reader& operator=(reader&& in) noexcept {
    if (this == &in) return *this;
    storage_ = std::move(in.storage_);
    borrowed_ = in.borrowed_;
    borrowed_pos_ = in.borrowed_pos_;
    failed_ = in.failed_;
    is_borrowed_ = in.is_borrowed_;
    in.borrowed_ = {};
    in.borrowed_pos_ = 0;
    in.failed_ = false;
    in.is_borrowed_ = false;
    return *this;
  }
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +=========================================================================+
  std::size_t read(std::span<std::byte> output) {
    if (output.empty()) return 0;
    if (is_borrowed_) {
      if (borrowed_pos_ == borrowed_.size()) return 0;
      std::size_t bytes =
          std::min(output.size(), borrowed_.size() - borrowed_pos_);
      std::memcpy(output.data(), borrowed_.data() + borrowed_pos_, bytes);
      borrowed_pos_ += bytes;
      return bytes;
    }
    std::size_t bytes =
        storage_.read(reinterpret_cast<char*>(output.data()), output.size());
    if (bytes == 0 && !storage_.ok()) failed_ = true;
    return bytes;
  }
  // +=========================================================================+
  // | [>] fetch                                                    ( public ) |
  // +=========================================================================+
  bool fetch(std::byte& byte) {
    if (is_borrowed_) {
      if (borrowed_pos_ == borrowed_.size()) return false;
      byte = borrowed_[borrowed_pos_++];
      return true;
    }
    if (storage_.fetch(byte)) return true;
    if (!storage_.ok()) failed_ = true;
    return false;
  }
  // +=========================================================================+
  // | [>] eof                                                      ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool eof() const noexcept {
    return is_borrowed_ ? borrowed_pos_ == borrowed_.size()
                        : storage_.exhausted();
  }
  // +=========================================================================+
  // | [>] exhausted                                                ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool exhausted() const noexcept { return eof(); }
  // +=========================================================================+
  // | [>] failed                                                   ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool failed() const noexcept { return failed_; }
  // +=========================================================================+
  // | [>] ok                                                       ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool ok() const noexcept {
    return is_borrowed_ || storage_.ok();
  }
  // +=========================================================================+
  // | [>] size                                                     ( public ) |
  // +=========================================================================+
  [[nodiscard]] std::optional<std::size_t> size() const noexcept {
    return is_borrowed_ ? std::optional<std::size_t>(borrowed_.size())
                        : storage_.total_size();
  }
  // +=========================================================================+
  // | [>] read_all                                                 ( public ) |
  // +=========================================================================+
  std::size_t read_all(std::string& out) {
    std::byte buffer[8192];
    std::size_t total = 0;
    while (!eof() && !failed()) {
      std::size_t bytes = read(std::span<std::byte>(buffer, sizeof(buffer)));
      if (bytes == 0) break;
      out.append(reinterpret_cast<const char*>(buffer), bytes);
      total += bytes;
    }
    return total;
  }

 private:
  struct borrowed_tag {};

  reader(std::span<const std::byte> source, borrowed_tag) noexcept
      : borrowed_(source), is_borrowed_(true) {}

  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  byte_storage storage_;
  std::span<const std::byte> borrowed_;
  std::size_t borrowed_pos_{0};
  bool failed_{false};
  bool is_borrowed_{false};
};
}  // namespace martianlabs::doba::common

#endif
