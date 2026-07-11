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

#ifndef martianlabs_doba_protocol_http11_body_deserializer_h
#define martianlabs_doba_protocol_http11_body_deserializer_h

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <utility>

#include "protocol/http11/body_codec_chunked.h"
#include "protocol/http11/body_codec_raw.h"
#include "protocol/http11/body_common.h"
#include "protocol/http11/body_reader.h"

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] body_deserializer                                           ( class ) |
// +---------------------------------------------------------------------------+
// | Decodes payload bytes from body_storage using codec CDty. Owns the codec  |
// | state and drives body_reader as its opaque source.                        |
// |                                                                           |
// | request::deserialize() constructs the correct specialisation based on     |
// | the incoming headers (Content-Length -> raw, Transfer-Encoding: chunked   |
// | -> chunked) and exposes it via get_body_deserializer().                   |
// |                                                                           |
// | Static factories:                                                         |
// |   body_deserializer<body_codec_raw>::raw(storage, max_raw_size)           |
// |   body_deserializer<body_codec_chunked>::chunked(storage, ...)            |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
template <typename CDty = body_codec_raw>
class body_deserializer {
 public:
  // +=========================================================================+
  // | [>] raw                                                      ( public ) |
  // +=========================================================================+
  static body_deserializer<body_codec_raw> raw(
      body_storage source = body_storage{}, std::uint64_t max_raw_size = 0) {
    body_deserializer<body_codec_raw> d(body_codec_raw{},
                                        body_reader(std::move(source)));
    d.max_raw_size_ = max_raw_size;
    if (max_raw_size > 0) {
      if (auto sz = d.reader_.size()) {
        if (*sz > max_raw_size) d.fail(body_error::raw_size_limit_exceeded);
      }
    }
    return d;
  }
  // +=========================================================================+
  // | [>] chunked                                                  ( public ) |
  // +=========================================================================+
  static body_deserializer<body_codec_chunked> chunked(
      body_storage source = body_storage{},
      std::uint64_t max_chunk_extension_size = kDefaultBufferSize,
      std::uint64_t max_trailer_size = kDefaultBufferSize) {
    return body_deserializer<body_codec_chunked>(
        body_codec_chunked(max_chunk_extension_size, max_trailer_size),
        body_reader(std::move(source)));
  }
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  body_deserializer(const body_deserializer&) = delete;
  body_deserializer(body_deserializer&&) noexcept = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  body_deserializer& operator=(const body_deserializer&) = delete;
  body_deserializer& operator=(body_deserializer&&) noexcept = default;
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Decodes up to output.size() logical bytes into output via CDty::decode. |
  // | Returns 0 on eof() or failed().                                         |
  // +=========================================================================+
  std::size_t read(std::span<std::byte> output) {
    if (output.empty()) return 0;
    if (status_ == body_status::error) return 0;
    if (status_ == body_status::complete) return 0;
    std::size_t consumed = 0;
    decode_result dr = codec_.decode(reader_, output, consumed);
    if (dr.has_error) {
      fail(dr.error);
      return 0;
    }
    bytes_read_ += dr.produced;
    if (dr.produced > 0 && max_raw_size_ > 0 && bytes_read_ > max_raw_size_) {
      fail(body_error::raw_size_limit_exceeded);
      return 0;
    }
    if (dr.complete) status_ = body_status::complete;
    return dr.produced;
  }
  // +=========================================================================+
  // | [>] read_all                                                 ( public ) |
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
  // +=========================================================================+
  // | [>] eof                                                      ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool eof() const noexcept {
    return status_ == body_status::complete;
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
  // +=========================================================================+
  // | [>] bytes_read                                               ( public ) |
  // +=========================================================================+
  [[nodiscard]] std::uint64_t bytes_read() const noexcept {
    return bytes_read_;
  }
  // +=========================================================================+
  // | [>] bytes_read                                               ( public ) |
  // + ------------------------------------------------------------------------+
  // | Releases the underlying storage as a body_reader for zero-copy          |
  // | forwarding (e.g. proxy scenarios). Only valid after eof(); returns an   |
  // | empty reader if called while decoding is still in progress or           |
  // | after failed().                                                         |
  // +=========================================================================+
  body_reader release_reader() {
    if (status_ != body_status::complete) return body_reader{};
    return std::move(reader_);
  }

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( private) |
  // +=========================================================================+
  body_deserializer(CDty codec, body_reader reader)
      : codec_(std::move(codec)), reader_(std::move(reader)) {}

  // +=========================================================================+
  // | [>] fail                                                     ( private) |
  // +=========================================================================+
  void fail(body_error e) {
    status_ = body_status::error;
    error_ = e;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( private) |
  // +=========================================================================+
  CDty codec_;
  body_reader reader_;
  body_status status_{body_status::open};
  body_error error_{body_error::none};
  std::uint64_t bytes_read_{0};
  std::uint64_t max_raw_size_{0};
};
}  // namespace martianlabs::doba::protocol::http11

#endif
