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
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License").
// You may obtain a copy of the License at:
//     http://www.apache.org/licenses/LICENSE-2.0

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
// | request::deserialize() constructs the correct specialisation based on    |
// | the incoming headers (Content-Length -> raw, Transfer-Encoding: chunked  |
// | -> chunked) and exposes it via get_body_deserializer().                  |
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
  // | [>] static factories                                          ( public ) |
  // +=========================================================================+
  static body_deserializer<body_codec_raw> raw(
      body_storage source = body_storage{},
      std::uint64_t max_raw_size = 0) {
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
  static body_deserializer<body_codec_chunked> chunked(
      body_storage source = body_storage{},
      std::uint64_t max_chunk_extension_size = kDefaultBufferSize,
      std::uint64_t max_trailer_size = kDefaultBufferSize) {
    return body_deserializer<body_codec_chunked>(
        body_codec_chunked(max_chunk_extension_size, max_trailer_size),
        body_reader(std::move(source)));
  }

  body_deserializer(const body_deserializer&) = delete;
  body_deserializer& operator=(const body_deserializer&) = delete;
  body_deserializer(body_deserializer&&) noexcept = default;
  body_deserializer& operator=(body_deserializer&&) noexcept = default;

  // +=========================================================================+
  // | [>] read                                                      ( public ) |
  // +---------------------------------------------------------------------------
  // | Decodes up to output.size() logical bytes into output via CDty::decode. |
  // | Returns 0 on eof() or failed().                                         |
  // +=========================================================================+
  std::size_t read(std::span<std::byte> output) {
    if (output.empty()) return 0;
    if (status_ == body_status::error) return 0;
    if (status_ == body_status::complete) return 0;
    std::uint64_t consumed = 0;
    decode_result dr = codec_.decode(reader_, output, consumed);
    if (dr.has_error) { fail(dr.error); return 0; }
    bytes_read_ += dr.produced;
    if (dr.produced > 0 && max_raw_size_ > 0 && bytes_read_ > max_raw_size_) {
      fail(body_error::raw_size_limit_exceeded);
      return 0;
    }
    if (dr.complete) status_ = body_status::complete;
    return dr.produced;
  }
  // +=========================================================================+
  // | [>] read_all                                                  ( public ) |
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

  [[nodiscard]] bool eof() const noexcept {
    return status_ == body_status::complete;
  }
  [[nodiscard]] bool failed() const noexcept {
    return status_ == body_status::error;
  }
  [[nodiscard]] body_status status() const noexcept { return status_; }
  [[nodiscard]] body_error error() const noexcept { return error_; }
  [[nodiscard]] std::uint64_t bytes_read() const noexcept {
    return bytes_read_;
  }
  // Releases the underlying storage as a body_reader for zero-copy forwarding
  // (e.g. proxy scenarios). Only valid after eof(); returns an empty reader if
  // called while decoding is still in progress or after failed().
  body_reader release_reader() {
    if (status_ != body_status::complete) return body_reader{};
    return std::move(reader_);
  }

 private:
  body_deserializer(CDty codec, body_reader reader)
      : codec_(std::move(codec)), reader_(std::move(reader)) {}

  void fail(body_error e) { status_ = body_status::error; error_ = e; }

  CDty codec_;
  body_reader reader_;
  body_status status_{body_status::open};
  body_error error_{body_error::none};
  std::uint64_t bytes_read_{0};
  std::uint64_t max_raw_size_{0};
};
}  // namespace martianlabs::doba::protocol::http11

#endif
