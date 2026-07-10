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
// | After finish(), call release_reader() to obtain a body_reader over the   |
// | populated storage, ready to be handed to response::set_body().           |
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
  // | [>] static factories                                          ( public ) |
  // +=========================================================================+
  static body_serializer<body_codec_raw> raw(
      body_storage_options opts = {}) {
    return body_serializer<body_codec_raw>(body_codec_raw{},
                                          body_writer(std::move(opts)));
  }
  static body_serializer<body_codec_chunked> chunked(
      body_storage_options opts = {},
      std::uint64_t max_chunk_extension_size = 0,
      std::uint64_t max_trailer_size = 0) {
    return body_serializer<body_codec_chunked>(
        body_codec_chunked(max_chunk_extension_size, max_trailer_size),
        body_writer(std::move(opts)));
  }

  body_serializer(const body_serializer&) = delete;
  body_serializer& operator=(const body_serializer&) = delete;
  body_serializer(body_serializer&&) noexcept = default;
  body_serializer& operator=(body_serializer&&) noexcept = default;

  // +=========================================================================+
  // | [>] write                                                     ( public ) |
  // +---------------------------------------------------------------------------
  // | Encodes data through CDty and appends to storage. Returns *this for     |
  // | chaining. A write after an error or after finish() is a no-op.         |
  // +=========================================================================+
  body_serializer& write(std::string_view data) {
    return write(std::span<const std::byte>(
        reinterpret_cast<const std::byte*>(data.data()), data.size()));
  }
  body_serializer& write(std::span<const std::byte> data) {
    if (status_ != body_status::open) return *this;
    if (data.empty()) return *this;
    encode_result er;
    if (!codec_.encode(writer_, reinterpret_cast<const char*>(data.data()),
                       data.size(), er)) {
      fail(er.has_error ? er.error : body_error::io_error);
      return *this;
    }
    if (!writer_.ok()) { fail(body_error::io_error); return *this; }
    stored_size_ += er.stored;
    return *this;
  }
  // +=========================================================================+
  // | [>] finish                                                    ( public ) |
  // +---------------------------------------------------------------------------
  // | Emits any trailing framing (chunked: "0\r\n\r\n"; raw: none). Idempotent.|
  // +=========================================================================+
  void finish() {
    if (status_ != body_status::open) return;
    encode_result er;
    if (!codec_.finish(writer_, er)) {
      fail(er.has_error ? er.error : body_error::io_error);
      return;
    }
    if (!writer_.ok()) { fail(body_error::io_error); return; }
    stored_size_ += er.stored;
    writer_.finish(stored_size_);
    status_ = body_status::complete;
  }
  // +=========================================================================+
  // | [>] release_reader                                            ( public ) |
  // +---------------------------------------------------------------------------
  // | Finalises (if not already done) and returns a body_reader over the      |
  // | populated storage. Call this once and pass the result to               |
  // | response::set_body().                                                   |
  // +=========================================================================+
  body_reader release_reader() {
    finish();
    if (status_ == body_status::error) return body_reader{};
    return body_reader(writer_.release());
  }
  // +=========================================================================+
  // | [>] stored_size                                               ( public ) |
  // +---------------------------------------------------------------------------+
  // | Known stored size after finish(), nullopt before finish() for chunked.  |
  // +=========================================================================+
  [[nodiscard]] std::optional<std::uint64_t> stored_size() const noexcept {
    if (status_ == body_status::complete) return stored_size_;
    return std::nullopt;
  }
  [[nodiscard]] bool failed() const noexcept {
    return status_ == body_status::error;
  }
  [[nodiscard]] body_status status() const noexcept { return status_; }
  [[nodiscard]] body_error error() const noexcept { return error_; }

 private:
  body_serializer(CDty codec, body_writer writer)
      : codec_(std::move(codec)), writer_(std::move(writer)) {}

  void fail(body_error e) { status_ = body_status::error; error_ = e; }

  CDty codec_;
  body_writer writer_;
  body_status status_{body_status::open};
  body_error error_{body_error::none};
  std::uint64_t stored_size_{0};
};
}  // namespace martianlabs::doba::protocol::http11

#endif
