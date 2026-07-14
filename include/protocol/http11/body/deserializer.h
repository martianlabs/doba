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

#ifndef martianlabs_doba_protocol_http11_body_deserializer_h
#define martianlabs_doba_protocol_http11_body_deserializer_h

#include <cstddef>
#include <span>
#include <string>
#include <utility>

#include "common/reader.h"
#include "protocol/http11/body/decoder.h"
#include "protocol/http11/body/decoder_chunked.h"
#include "protocol/http11/body/decoder_raw.h"

namespace martianlabs::doba::protocol::http11::body {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] deserializer                                                ( class ) |
// +---------------------------------------------------------------------------+
// | Decodes payload bytes from common::byte_storage using decoder DEty. Owns  |
// | decoder state and drives common::reader as its opaque source.             |
// |                                                                           |
// | request::deserialize() constructs the correct specialisation based on     |
// | the incoming headers (Content-Length -> raw, Transfer-Encoding: chunked   |
// | -> chunked) and exposes it via get_body_deserializer().                   |
// |                                                                           |
// | Static factories:                                                         |
// |   deserializer<decoder_raw>::raw()     - identity framing                 |
// |   deserializer<decoder_chunked>::chunked() - chunked framing              |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
template <typename DEty>
class deserializer {
 public:
  // +=========================================================================+
  // | [>] raw                                                      ( public ) |
  // +=========================================================================+
  static deserializer<decoder_raw> raw(
      common::byte_storage source = common::byte_storage{},
      std::size_t max_raw_size = 0) {
    deserializer<decoder_raw> result(decoder_raw{},
                                     common::reader(std::move(source)));
    result.set_raw_size_limit(max_raw_size);
    return result;
  }
  // +=========================================================================+
  // | [>] chunked                                                  ( public ) |
  // +=========================================================================+
  static deserializer<decoder_chunked> chunked(
      common::byte_storage source = common::byte_storage{},
      std::size_t max_chunk_extension_size = kDefaultBufferSize,
      std::size_t max_trailer_size = kDefaultBufferSize) {
    return deserializer<decoder_chunked>(
        decoder_chunked(max_chunk_extension_size, max_trailer_size),
        common::reader(std::move(source)));
  }
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  deserializer(const deserializer&) = delete;
  deserializer(deserializer&&) noexcept = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  deserializer& operator=(const deserializer&) = delete;
  deserializer& operator=(deserializer&&) noexcept = default;
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Decodes up to output.size() logical bytes into output via DEty::decode. |
  // | Returns 0 on eof() or failed().                                         |
  // +=========================================================================+
  std::size_t read(std::span<std::byte> output) {
    if (output.empty()) return 0;
    if (status_ == decoder_status::error) return 0;
    if (status_ == decoder_status::complete) return 0;
    std::size_t consumed = 0;
    decode_result dr = decoder_.decode(reader_, output, consumed);
    if (dr.has_error) {
      fail(dr.error);
      return 0;
    }
    bytes_read_ += dr.produced;
    if (dr.produced > 0 && max_raw_size_ > 0 && bytes_read_ > max_raw_size_) {
      fail(decoder_error::raw_size_limit_exceeded);
      return 0;
    }
    if (dr.complete) status_ = decoder_status::complete;
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
    return status_ == decoder_status::complete;
  }
  // +=========================================================================+
  // | [>] failed                                                   ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool failed() const noexcept {
    return status_ == decoder_status::error;
  }
  // +=========================================================================+
  // | [>] status                                                   ( public ) |
  // +=========================================================================+
  [[nodiscard]] decoder_status status() const noexcept { return status_; }
  // +=========================================================================+
  // | [>] error                                                    ( public ) |
  // +=========================================================================+
  [[nodiscard]] decoder_error error() const noexcept { return error_; }
  // +=========================================================================+
  // | [>] bytes_read                                               ( public ) |
  // +=========================================================================+
  [[nodiscard]] std::size_t bytes_read() const noexcept { return bytes_read_; }
  // +=========================================================================+
  // | [>] release_reader                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Releases the underlying storage as a common::reader for zero-copy       |
  // | forwarding (e.g. proxy scenarios). Only valid after eof(); returns an   |
  // | empty reader if called while decoding is still in progress or           |
  // | after failed().                                                         |
  // +=========================================================================+
  common::reader release_reader() {
    if (status_ != decoder_status::complete) return common::reader{};
    return std::move(reader_);
  }

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( private) |
  // +=========================================================================+
  deserializer(DEty decoder, common::reader reader)
      : decoder_(std::move(decoder)), reader_(std::move(reader)) {}
  // +=========================================================================+
  // | [>] fail                                                     ( private) |
  // +=========================================================================+
  void fail(decoder_error e) {
    status_ = decoder_status::error;
    error_ = e;
  }
  // +=========================================================================+
  // | [>] set_raw_size_limit                                       ( private) |
  // +=========================================================================+
  void set_raw_size_limit(std::size_t max_raw_size) {
    max_raw_size_ = max_raw_size;
    if (max_raw_size == 0) return;
    if (auto size = reader_.size(); size && *size > max_raw_size) {
      fail(decoder_error::raw_size_limit_exceeded);
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( private) |
  // +=========================================================================+
  DEty decoder_;
  common::reader reader_;
  decoder_status status_{decoder_status::open};
  decoder_error error_{decoder_error::none};
  std::size_t bytes_read_{0};
  std::size_t max_raw_size_{0};
};
}  // namespace martianlabs::doba::protocol::http11::body

#endif
