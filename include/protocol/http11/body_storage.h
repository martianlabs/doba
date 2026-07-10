// +---------------------------+
// |  d o b a                  |
// +---------------------------+
//
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
//        --- martianLabs Anti-AI Usage and Model-Training Addendum ---
//
// TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
//
// Copyright 2025 martianLabs
//
// Except as otherwise stated in this Addendum, this software is licensed
// under the Apache License, Version 2.0 (the "License"); you may not use
// this file except in compliance with the License.
//
// The following additional terms are hereby added to the Apache License for
// the purpose of restricting the use of this software by Artificial
// Intelligence systems, machine learning models, data-scraping bots, and
// automated systems.
//
// 1.  MACHINE LEARNING AND AI RESTRICTIONS
//     1.1. No entity, organization, or individual may use this software,
//          its source code, object code, or any derivative work for the
//          purpose of training, fine-tuning, evaluating, or improving any
//          machine learning model, artificial intelligence system, large
//          language model, or similar automated system.
//     1.2. No automated system may copy, parse, analyze, index, or
//          otherwise process this software for any AI-related purpose.
//     1.3. Use of this software as input, prompt material, reference
//          material, or evaluation data for AI systems is expressly
//          prohibited.
//
// 2.  SCRAPING AND AUTOMATED ACCESS RESTRICTIONS
//     2.1. No automated crawler, training pipeline, or data-extraction
//          system may collect, store, or incorporate any portion of this
//          software in any dataset used for machine learning or AI
//          training.
//     2.2. Any automated access must comply with this License and with
//          applicable copyright law.
//
// 3.  PROHIBITION ON DERIVATIVE DATASETS
//     3.1. You may not create datasets, corpora, embeddings, vector
//          stores, or similar derivative data intended for use by
//          automated systems, AI models, or machine learning algorithms.
//
// 4.  NO WAIVER OF RIGHTS
//     4.1. These restrictions apply in addition to, and do not limit,
//          the rights and protections provided to the copyright holder
//          under the Apache License Version 2.0 and applicable law.
//
// 5.  ACCEPTANCE
//     5.1. Any use of this software constitutes acceptance of both the
//          Apache License Version 2.0 and this Anti-AI Addendum.
//
// You may obtain a copy of the Apache License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.  See the License for the specific language governing
// permissions and limitations under the Apache License Version 2.0.

#ifndef martianlabs_doba_protocol_http11_body_storage_h
#define martianlabs_doba_protocol_http11_body_storage_h

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

#include "protocol/http11/body_common.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +==========================================================================+
// | [>] body_storage_options                                       ( struct ) |
// +==========================================================================+
// /////////////////////////////////////////////////////////////////////////////
struct body_storage_options {
  // Maximum bytes to keep in memory before spilling to a temporary file.
  // 0 means never spill (memory-only mode).
  std::uint64_t spill_threshold = 0;
  // Directory in which to create the temporary spill file. An empty string
  // means the platform's default temporary directory (std::filesystem::
  // temp_directory_path()). Ignored when spill_threshold == 0.
  std::string spill_dir;
};
// /////////////////////////////////////////////////////////////////////////////
// +==========================================================================+
// | [>] body_storage                                                ( class ) |
// +--------------------------------------------------------------------------+
// | Unified body storage backend for body_writer<CDty> and                   |
// | body_reader<CDty>. Starts in memory (std::string). If the total number   |
// | of wire bytes written exceeds spill_threshold (and spill_threshold > 0), |
// | the accumulated bytes are flushed to a uniquely-named temporary file and  |
// | all subsequent writes go there. The transition is transparent to the      |
// | codec and to the writer/reader.                                           |
// |                                                                           |
// | The decode cursor (fetch()/read()) and the wire cursor (wire_read()) are  |
// | independent: both can be driven on the same instance without interfering. |
// +==========================================================================+
// /////////////////////////////////////////////////////////////////////////////
class body_storage {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  body_storage() = default;
  explicit body_storage(body_storage_options opts)
      : spill_threshold_(opts.spill_threshold),
        spill_dir_(std::move(opts.spill_dir)) {}
  body_storage(const body_storage&) = delete;
  body_storage& operator=(const body_storage&) = delete;
  body_storage(body_storage&&) noexcept = default;
  body_storage& operator=(body_storage&&) noexcept = default;
  ~body_storage() {
    // Close the spill file before removing it so the OS can unlink the path.
    if (spilled_ && file_.is_open()) file_.close();
    if (!spill_path_.empty()) {
      std::error_code ec;
      std::filesystem::remove(spill_path_, ec);
    }
  }
  // +=========================================================================+
  // | [>] write                                                     ( sink )  |
  // +=========================================================================+
  bool write(const char* ptr, std::size_t len) {
    if (len == 0) return true;
    if (io_error_) return false;
    if (!spilled_ && spill_threshold_ > 0 &&
        mem_.size() + len > spill_threshold_) {
      if (!spill_to_file()) return false;
    }
    if (spilled_) {
      file_.write(ptr, static_cast<std::streamsize>(len));
      if (!file_) { io_error_ = true; return false; }
    } else {
      mem_.append(ptr, len);
    }
    return true;
  }
  // +=========================================================================+
  // | [>] on_finish                                                 ( sink )  |
  // +=========================================================================+
  void on_finish(std::uint64_t stored) noexcept {
    total_bytes_ = stored;
    if (spilled_) {
      file_.flush();
      if (!file_) { io_error_ = true; return; }
    }
    finished_ = true;
  }
  // +=========================================================================+
  // | [>] fetch                                                   ( source )  |
  // +=========================================================================+
  bool fetch(std::byte& b) {
    if (io_error_) return false;
    if (spilled_) return fetch_file(b);
    if (decode_pos_ >= mem_.size()) return false;
    b = static_cast<std::byte>(
        static_cast<unsigned char>(mem_[decode_pos_++]));
    return true;
  }
  // +=========================================================================+
  // | [>] read                                                    ( source )  |
  // +=========================================================================+
  std::size_t read(char* out, std::size_t want) {
    if (want == 0 || io_error_) return 0;
    if (spilled_) return read_file(out, want, decode_pos_);
    std::size_t remaining = mem_.size() - decode_pos_;
    std::size_t n = std::min(remaining, want);
    std::memcpy(out, mem_.data() + decode_pos_, n);
    decode_pos_ += n;
    return n;
  }
  // +=========================================================================+
  // | [>] wire_read                                               ( source )  |
  // +=========================================================================+
  std::size_t wire_read(char* out, std::size_t want) {
    if (want == 0 || io_error_) return 0;
    if (spilled_) return read_file(out, want, wire_pos_);
    std::size_t remaining = mem_.size() - wire_pos_;
    std::size_t n = std::min(remaining, want);
    std::memcpy(out, mem_.data() + wire_pos_, n);
    wire_pos_ += n;
    return n;
  }
  // +=========================================================================+
  // | [>] total_size / exhausted / ok                             ( source )  |
  // +=========================================================================+
  [[nodiscard]] std::optional<std::uint64_t> total_size() const noexcept {
    // total_bytes_ is set authoritatively by on_finish() in both modes.
    // Before on_finish() we cannot know the logical body size: a chunked codec
    // stores more wire bytes than logical bytes, so returning mem_.size() here
    // would expose the wire size and cause response to emit a wrong
    // Content-Length. Always return nullopt until the storage is finalised.
    if (finished_) return total_bytes_;
    return std::nullopt;
  }
  [[nodiscard]] bool exhausted() const noexcept {
    // Spill mode: must be finalised and cursor at end.
    if (spilled_) return finished_ && decode_pos_ >= total_bytes_;
    // Memory mode (write side still open, used by deserializer): cursor at end
    // of whatever has been written so far. This correctly signals EOF to
    // body_codec_raw::decode without requiring on_finish() to be called first.
    return decode_pos_ >= mem_.size();
  }
  [[nodiscard]] bool ok() const noexcept { return !io_error_; }

 private:
  // +=========================================================================+
  // | [>] spill_to_file                                           ( private ) |
  // +=========================================================================+
  // Generates a unique temporary path, opens the file for read+write,
  // writes whatever is currently in mem_ into it, and clears mem_.
  // Returns false and sets io_error_ on any failure.
  bool spill_to_file() {
    spill_path_ = make_spill_path();
    file_.open(spill_path_,
               std::ios::in | std::ios::out | std::ios::trunc |
                   std::ios::binary);
    if (!file_.is_open()) { io_error_ = true; return false; }
    if (!mem_.empty()) {
      file_.write(mem_.data(), static_cast<std::streamsize>(mem_.size()));
      if (!file_) { io_error_ = true; return false; }
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
    if (!finished_ || decode_pos_ >= total_bytes_) return false;
    char c;
    file_.seekg(static_cast<std::streamoff>(decode_pos_), std::ios::beg);
    file_.read(&c, 1);
    if (file_.gcount() != 1) { io_error_ = true; return false; }
    b = static_cast<std::byte>(static_cast<unsigned char>(c));
    ++decode_pos_;
    return true;
  }
  // +=========================================================================+
  // | [>] read_file                                               ( private ) |
  // +=========================================================================+
  std::size_t read_file(char* out, std::size_t want, std::uint64_t& pos) {
    if (!finished_ || pos >= total_bytes_) return 0;
    std::size_t remaining =
        static_cast<std::size_t>(total_bytes_ - pos);
    std::size_t n = std::min(want, remaining);
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
  // Produces a path that is unique across threads and processes by combining
  // a high-resolution timestamp with a process-lifetime counter.
  std::filesystem::path make_spill_path() const {
    namespace fs = std::filesystem;
    static std::atomic<std::uint64_t> counter{0};
    std::uint64_t ns =
        static_cast<std::uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count());
    std::uint64_t seq = counter.fetch_add(1, std::memory_order_relaxed);
    std::string name =
        "doba_body_" + std::to_string(ns) + "_" + std::to_string(seq) + ".tmp";
    fs::path dir = spill_dir_.empty()
                       ? fs::temp_directory_path()
                       : fs::path(spill_dir_);
    return dir / name;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::string             mem_;
  std::fstream            file_;
  std::filesystem::path   spill_path_;
  std::uint64_t           spill_threshold_{0};
  std::string             spill_dir_;
  std::uint64_t           total_bytes_{0};
  std::uint64_t           decode_pos_{0};
  std::uint64_t           wire_pos_{0};
  bool                    spilled_{false};
  bool                    finished_{false};
  bool                    io_error_{false};
};
}  // namespace martianlabs::doba::protocol::http11

#endif
