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

#ifndef martianlabs_doba_common_byte_storage_h
#define martianlabs_doba_common_byte_storage_h

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <utility>

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] byte_storage_options                                       ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct byte_storage_options {
  std::size_t spill_threshold = 0;
  std::string spill_dir;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] byte_storage                                                ( class ) |
// +---------------------------------------------------------------------------+
// | This class provides a storage backend for bytes, with optional spilling   |
// | to disk when a threshold is exceeded. It supports reading and             |
// | writing bytes, and can be used in scenarios where large amounts of        |
// | data need to be temporarily stored without consuming too much memory.     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class byte_storage {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  byte_storage() = default;
  explicit byte_storage(byte_storage_options opts)
      : spill_threshold_(opts.spill_threshold),
        spill_dir_(std::move(opts.spill_dir)) {}
  byte_storage(const byte_storage&) = delete;
  byte_storage(byte_storage&& in) noexcept { move_from(std::move(in)); }
  ~byte_storage() { reset(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  byte_storage& operator=(const byte_storage&) = delete;
  byte_storage& operator=(byte_storage&& in) noexcept {
    if (this == &in) return *this;
    reset();
    move_from(std::move(in));
    return *this;
  }
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +=========================================================================+
  std::size_t read(char* out, std::size_t want) {
    if (want == 0 || io_error_) return 0;
    if (spilled_) return read_file(out, want, read_pos_);
    std::size_t n = std::min(mem_.size() - read_pos_, want);
    std::memcpy(out, mem_.data() + read_pos_, n);
    read_pos_ += n;
    return n;
  }
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +=========================================================================+
  bool write(const char* ptr, std::size_t len) {
    if (len == 0) return true;
    if (io_error_) return false;
    if (!spilled_ && spill_threshold_ > 0 &&
        mem_.size() + len > spill_threshold_ && !spill_to_file()) {
      return false;
    }
    if (spilled_) {
      file_.write(ptr, static_cast<std::streamsize>(len));
      if (!file_) {
        io_error_ = true;
        return false;
      }
    } else {
      mem_.append(ptr, len);
    }
    return true;
  }
  // +=========================================================================+
  // | [>] finish                                                   ( public ) |
  // +=========================================================================+
  void finish(std::size_t stored) noexcept {
    total_bytes_ = stored;
    if (spilled_) {
      file_.flush();
      if (!file_) {
        io_error_ = true;
        return;
      }
    }
    finished_ = true;
  }
  // +=========================================================================+
  // | [>] fetch                                                    ( public ) |
  // +=========================================================================+
  bool fetch(std::byte& b) {
    if (io_error_) return false;
    if (spilled_) return fetch_file(b);
    if (read_pos_ >= mem_.size()) return false;
    b = static_cast<std::byte>(static_cast<unsigned char>(mem_[read_pos_++]));
    return true;
  }
  // +=========================================================================+
  // | [>] total_size                                               ( public ) |
  // +=========================================================================+
  [[nodiscard]] std::optional<std::size_t> total_size() const noexcept {
    return finished_ ? std::optional<std::size_t>(total_bytes_) : std::nullopt;
  }
  // +=========================================================================+
  // | [>] exhausted                                                ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool exhausted() const noexcept {
    return spilled_ ? finished_ && read_pos_ >= total_bytes_
                    : read_pos_ >= mem_.size();
  }
  // +=========================================================================+
  // | [>] ok                                                       ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool ok() const noexcept { return !io_error_; }

 private:
  // +=========================================================================+
  // | [>] reset                                                   ( private ) |
  // +=========================================================================+
  void reset() noexcept {
    if (file_.is_open()) file_.close();
    if (!spill_path_.empty()) {
      std::error_code ec;
      std::filesystem::remove(spill_path_, ec);
    }
  }
  // +=========================================================================+
  // | [>] move_from                                               ( private ) |
  // +=========================================================================+
  void move_from(byte_storage&& in) noexcept {
    mem_ = std::move(in.mem_);
    file_ = std::move(in.file_);
    spill_path_ = std::move(in.spill_path_);
    spill_threshold_ = in.spill_threshold_;
    spill_dir_ = std::move(in.spill_dir_);
    total_bytes_ = in.total_bytes_;
    read_pos_ = in.read_pos_;
    spilled_ = in.spilled_;
    finished_ = in.finished_;
    io_error_ = in.io_error_;
    in.spill_path_.clear();
    in.spill_threshold_ = 0;
    in.total_bytes_ = 0;
    in.read_pos_ = 0;
    in.spilled_ = false;
    in.finished_ = false;
    in.io_error_ = false;
  }
  // +=========================================================================+
  // | [>] spill_to_file                                           ( private ) |
  // +=========================================================================+
  bool spill_to_file() {
    spill_path_ = make_spill_path();
    file_.open(spill_path_, std::ios::in | std::ios::out | std::ios::trunc |
                                std::ios::binary);
    if (!file_.is_open()) {
      io_error_ = true;
      return false;
    }
    if (!mem_.empty()) {
      file_.write(mem_.data(), static_cast<std::streamsize>(mem_.size()));
      if (!file_) {
        io_error_ = true;
        return false;
      }
    }
    spilled_ = true;
    mem_.clear();
    mem_.shrink_to_fit();
    return true;
  }
  // +=========================================================================+
  // | [>] fetch_file                                              ( private ) |
  // +=========================================================================+
  bool fetch_file(std::byte& b) {
    if (!finished_ || read_pos_ >= total_bytes_) return false;
    char c;
    file_.seekg(static_cast<std::streamoff>(read_pos_), std::ios::beg);
    file_.read(&c, 1);
    if (file_.gcount() != 1) {
      io_error_ = true;
      return false;
    }
    b = static_cast<std::byte>(static_cast<unsigned char>(c));
    ++read_pos_;
    return true;
  }
  // +=========================================================================+
  // | [>] read_file                                               ( private ) |
  // +=========================================================================+
  std::size_t read_file(char* out, std::size_t want, std::size_t& pos) {
    if (!finished_ || pos >= total_bytes_) return 0;
    std::size_t n = std::min(want, total_bytes_ - pos);
    file_.seekg(static_cast<std::streamoff>(pos), std::ios::beg);
    file_.read(out, static_cast<std::streamsize>(n));
    std::size_t got = static_cast<std::size_t>(file_.gcount());
    if (got == 0 && !file_.eof()) io_error_ = true;
    pos += got;
    return got;
  }
  // +=========================================================================+
  // | [>] make_spill_path                                         ( private ) |
  // +=========================================================================+
  std::filesystem::path make_spill_path() const {
    namespace fs = std::filesystem;
    static std::atomic<std::uint64_t> counter{0};
    std::uint64_t ns = static_cast<std::uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    std::uint64_t seq = counter.fetch_add(1, std::memory_order_relaxed);
    std::string name =
        "doba_bytes_" + std::to_string(ns) + "_" + std::to_string(seq) + ".tmp";
    fs::path dir =
        spill_dir_.empty() ? fs::temp_directory_path() : fs::path(spill_dir_);
    return dir / name;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::string mem_;
  std::fstream file_;
  std::filesystem::path spill_path_;
  std::size_t spill_threshold_{0};
  std::string spill_dir_;
  std::size_t total_bytes_{0};
  std::size_t read_pos_{0};
  bool spilled_{false};
  bool finished_{false};
  bool io_error_{false};
};
}  // namespace martianlabs::doba::common

#endif
