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

#ifndef martianlabs_doba_protocol_http11_body_reader_h
#define martianlabs_doba_protocol_http11_body_reader_h

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>

#include "protocol/http11/body_common.h"
#include "protocol/http11/body_storage.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] body_reader                                                 ( class ) |
// +---------------------------------------------------------------------------+
// | Opaque byte source over body_storage. No codec knowledge: reads raw bytes |
// | from the storage backend sequentially.                                    |
// | body_deserializer<CDty> owns the codec and drives body_reader as its src. |
// | response::serialize() also uses body_reader directly to drain a           |
// | body_serializer's released storage to the wire sink.                      |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
class body_reader {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  body_reader() = default;
  explicit body_reader(body_storage storage) : storage_(std::move(storage)) {}
  body_reader(const body_reader&) = delete;
  body_reader(body_reader&&) noexcept = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  body_reader& operator=(const body_reader&) = delete;
  body_reader& operator=(body_reader&&) noexcept = default;
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Reads up to output.size() raw bytes. Returns 0 at end-of-storage or on  |
  // | I/O error (check failed() to distinguish).                              |
  // +=========================================================================+
  std::size_t read(std::span<std::byte> output) {
    if (output.empty()) return 0;
    std::size_t n =
        storage_.read(reinterpret_cast<char*>(output.data()), output.size());
    if (n == 0 && !storage_.ok()) failed_ = true;
    return n;
  }
  // +=========================================================================+
  // | [>] fetch                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Reads one byte via the decode cursor.                                   |
  // +=========================================================================+
  bool fetch(std::byte& b) {
    if (!storage_.fetch(b)) {
      if (!storage_.ok()) failed_ = true;
      return false;
    }
    return true;
  }
  // +=========================================================================+
  // | [>] eof                                                      ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool eof() const noexcept { return storage_.exhausted(); }
  // +=========================================================================+
  // | [>] failed                                                   ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool failed() const noexcept { return failed_; }
  // +=========================================================================+
  // | [>] ok                                                       ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool ok() const noexcept { return storage_.ok(); }
  // +=========================================================================+
  // | [>] exhausted                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | exhausted() is an alias for eof(), used by codec internals.             |
  // +=========================================================================+
  [[nodiscard]] bool exhausted() const noexcept { return storage_.exhausted(); }
  // +=========================================================================+
  // | [>] size                                                     ( public ) |
  // +=========================================================================+
  [[nodiscard]] std::optional<std::uint64_t> size() const noexcept {
    return storage_.total_size();
  }
  // +=========================================================================+
  // | [>] read_all                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // | Reads all remaining bytes into out. Returns total bytes appended.       |
  // +=========================================================================+
  std::size_t read_all(std::string& out) {
    std::byte buf[kDefaultBufferSize];
    std::size_t total = 0;
    while (!eof() && !failed()) {
      std::size_t n = read(std::span<std::byte>(buf, sizeof(buf)));
      if (n == 0) break;
      out.append(reinterpret_cast<const char*>(buf), n);
      total += n;
    }
    return total;
  }

 private:
  // +=========================================================================+
  // | [>] FRIENDs                                                 ( private ) |
  // +=========================================================================+
  template <typename CDty>
  friend class body_serializer;
  template <typename CDty>
  friend class body_deserializer;
  // +=========================================================================+
  // | [>] release                                                 ( private ) |
  // +=========================================================================+
  [[nodiscard]] body_storage release() { return std::move(storage_); }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  body_storage storage_;
  bool failed_{false};
};
}  // namespace martianlabs::doba::protocol::http11

#endif
