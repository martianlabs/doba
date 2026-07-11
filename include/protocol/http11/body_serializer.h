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

#ifndef martianlabs_doba_protocol_http11_body_serializer_h
#define martianlabs_doba_protocol_http11_body_serializer_h

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "protocol/http11/body_codec_chunked.h"
#include "protocol/http11/body_codec_raw.h"
#include "protocol/http11/body_common.h"
#include "protocol/http11/body_writer.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] body_serializer                                             ( class ) |
// +---------------------------------------------------------------------------+
// | Encodes payload bytes into body_storage using codec CDty. Owns the codec  |
// | state and drives body_writer as its opaque sink.                          |
// |                                                                           |
// | After finish(), call release_reader() to obtain a body_reader over the    |
// | populated storage, ready to be handed to response::set_body().            |
// |                                                                           |
// | Static factories:                                                         |
// |   body_serializer<body_codec_raw>::raw()     - identity framing           |
// |   body_serializer<body_codec_chunked>::chunked() - chunked framing        |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
template <typename CDty = body_codec_raw>
class body_serializer {
 public:
  // +=========================================================================+
  // | [>] raw                                                      ( public ) |
  // +=========================================================================+
  static body_serializer<body_codec_raw> raw(body_storage_options opts = {}) {
    return body_serializer<body_codec_raw>(body_codec_raw{},
                                           body_writer(std::move(opts)));
  }
  // +=========================================================================+
  // | [>] chunked                                                  ( public ) |
  // +=========================================================================+
  static body_serializer<body_codec_chunked> chunked(
      body_storage_options opts = {},
      std::uint64_t max_chunk_extension_size = 0,
      std::uint64_t max_trailer_size = 0) {
    return body_serializer<body_codec_chunked>(
        body_codec_chunked(max_chunk_extension_size, max_trailer_size),
        body_writer(std::move(opts)));
  }
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  body_serializer(const body_serializer&) = delete;
  body_serializer(body_serializer&&) noexcept = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  body_serializer& operator=(const body_serializer&) = delete;
  body_serializer& operator=(body_serializer&&) noexcept = default;
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Encodes data through CDty and appends to storage. Returns *this for     |
  // | chaining. A write after an error or after finish() is a no-op.          |
  // +=========================================================================+
  body_serializer& write(std::string_view data) {
    return write(std::span<const std::byte>(
        reinterpret_cast<const std::byte*>(data.data()), data.size()));
  }
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Encodes data through CDty and appends to storage. Returns *this for     |
  // | chaining. A write after an error or after finish() is a no-op.          |
  // +=========================================================================+
  body_serializer& write(std::span<const std::byte> data) {
    if (status_ != body_status::open) return *this;
    if (data.empty()) return *this;
    encode_result er;
    if (!codec_.encode(writer_, reinterpret_cast<const char*>(data.data()),
                       data.size(), er)) {
      fail(er.has_error ? er.error : body_error::io_error);
      return *this;
    }
    if (!writer_.ok()) {
      fail(body_error::io_error);
      return *this;
    }
    stored_size_ += er.stored;
    return *this;
  }
  // +=========================================================================+
  // | [>] finish                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  // | Emits any trailing framing (chunked: "0\r\n\r\n"; raw: none).           |
  // +=========================================================================+
  void finish() {
    if (status_ != body_status::open) return;
    encode_result er;
    if (!codec_.finish(writer_, er)) {
      fail(er.has_error ? er.error : body_error::io_error);
      return;
    }
    if (!writer_.ok()) {
      fail(body_error::io_error);
      return;
    }
    stored_size_ += er.stored;
    writer_.finish(stored_size_);
    status_ = body_status::complete;
  }
  // +=========================================================================+
  // | [>] release_reader                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Finalises (if not already done) and returns a body_reader over the      |
  // | populated storage. Call this once and pass the result to                |
  // | response::set_body().                                                   |
  // +=========================================================================+
  body_reader release_reader() {
    finish();
    if (status_ == body_status::error) return body_reader{};
    return body_reader(writer_.release());
  }
  // +=========================================================================+
  // | [>] stored_size                                              ( public ) |
  // +-------------------------------------------------------------------------+
  // | Known stored size after finish(), nullopt before finish() for chunked.  |
  // +=========================================================================+
  [[nodiscard]] std::optional<std::uint64_t> stored_size() const noexcept {
    if (status_ == body_status::complete) return stored_size_;
    return std::nullopt;
  }
  // +=========================================================================+
  // | [>] failed                                                   ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool failed() const noexcept {
    return status_ == body_status::error;
  }
  // +=========================================================================+
  // | [>] status                                                   ( public ) |
  // +=========================================================================+
  [[nodiscard]] body_status status() const noexcept { return status_; }
  // +=========================================================================+
  // | [>] error                                                    ( public ) |
  // +=========================================================================+
  [[nodiscard]] body_error error() const noexcept { return error_; }

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  body_serializer(CDty codec, body_writer writer)
      : codec_(std::move(codec)), writer_(std::move(writer)) {}
  // +=========================================================================+
  // | [>] fail                                                     ( public ) |
  // +=========================================================================+
  void fail(body_error e) {
    status_ = body_status::error;
    error_ = e;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  CDty codec_;
  body_writer writer_;
  body_status status_{body_status::open};
  body_error error_{body_error::none};
  std::uint64_t stored_size_{0};
};
}  // namespace martianlabs::doba::protocol::http11

#endif
