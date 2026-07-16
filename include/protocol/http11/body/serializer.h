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

#ifndef martianlabs_doba_protocol_http11_body_serializer_h
#define martianlabs_doba_protocol_http11_body_serializer_h

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "common/reader.h"
#include "common/writer.h"
#include "protocol/http11/body/encoder.h"
#include "protocol/http11/body/encoder_chunked.h"
#include "protocol/http11/body/encoder_raw.h"

namespace martianlabs::doba::protocol::http11::body {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] serializer                                                  ( class ) |
// +---------------------------------------------------------------------------+
// | Encodes payload bytes into common::byte_storage using encoder ENty. Owns  |
// | encoder state and drives common::writer as its opaque sink.               |
// |                                                                           |
// | After finish(), call release_reader() to obtain a common::reader over the |
// | populated storage, ready to be moved into response::set_body().           |
// |                                                                           |
// | Static factories:                                                         |
// |   serializer<encoder_raw>::raw()       - identity framing                 |
// |   serializer<encoder_chunked>::chunked() - chunked framing                |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
template <typename ENty = encoder_raw>
class serializer {
 public:
  // +=========================================================================+
  // | [>] raw                                                      ( public ) |
  // +=========================================================================+
  static serializer<encoder_raw> raw(common::byte_storage_options opts = {}) {
    return serializer<encoder_raw>(encoder_raw{},
                                   common::writer(std::move(opts)));
  }
  // +=========================================================================+
  // | [>] chunked                                                  ( public ) |
  // +=========================================================================+
  static serializer<encoder_chunked> chunked(
      common::byte_storage_options opts = {}) {
    return serializer<encoder_chunked>(encoder_chunked{},
                                       common::writer(std::move(opts)));
  }
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  serializer(const serializer&) = delete;
  serializer(serializer&&) noexcept = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  serializer& operator=(const serializer&) = delete;
  serializer& operator=(serializer&&) noexcept = default;
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Encodes data through ENty and appends to storage. Returns *this for     |
  // | chaining. A write after an error or after finish() is a no-op.          |
  // +=========================================================================+
  serializer& write(std::string_view data) {
    return write(std::span<const std::byte>(
        reinterpret_cast<const std::byte*>(data.data()), data.size()));
  }
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Encodes data through ENty and appends to storage. Returns *this for     |
  // | chaining. A write after an error or after finish() is a no-op.          |
  // +=========================================================================+
  serializer& write(std::span<const std::byte> data) {
    if (status_ != encoder_status::open) return *this;
    if (data.empty()) return *this;
    encode_result er;
    if (!encoder_.encode(writer_, reinterpret_cast<const char*>(data.data()),
                         data.size(), er)) {
      fail(er.has_error ? er.error : encoder_error::io_error);
      return *this;
    }
    if (!writer_.ok()) {
      fail(encoder_error::io_error);
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
    if (status_ != encoder_status::open) return;
    encode_result er;
    if (!encoder_.finish(writer_, er)) {
      fail(er.has_error ? er.error : encoder_error::io_error);
      return;
    }
    if (!writer_.ok()) {
      fail(encoder_error::io_error);
      return;
    }
    stored_size_ += er.stored;
    writer_.finish(stored_size_);
    status_ = encoder_status::complete;
  }
  // +=========================================================================+
  // | [>] release_reader                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Finalises (if not already done) and returns a common::reader over the   |
  // | populated storage. This is intended for lower-level consumers; response |
  // | accepts the serializer directly so it can preserve its HTTP framing.    |
  // +=========================================================================+
  common::reader release_reader() {
    finish();
    if (status_ == encoder_status::error) return common::reader{};
    return common::reader(writer_.release());
  }
  // +=========================================================================+
  // | [>] stored_size                                              ( public ) |
  // +-------------------------------------------------------------------------+
  // | Known stored size after finish(), nullopt before finish() for chunked.  |
  // +=========================================================================+
  [[nodiscard]] std::optional<std::size_t> stored_size() const noexcept {
    if (status_ == encoder_status::complete) return stored_size_;
    return std::nullopt;
  }
  // +=========================================================================+
  // | [>] failed                                                   ( public ) |
  // +=========================================================================+
  [[nodiscard]] bool failed() const noexcept {
    return status_ == encoder_status::error;
  }
  // +=========================================================================+
  // | [>] status                                                   ( public ) |
  // +=========================================================================+
  [[nodiscard]] encoder_status status() const noexcept { return status_; }
  // +=========================================================================+
  // | [>] error                                                    ( public ) |
  // +=========================================================================+
  [[nodiscard]] encoder_error error() const noexcept { return error_; }

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  serializer(ENty encoder, common::writer writer)
      : encoder_(std::move(encoder)), writer_(std::move(writer)) {}
  // +=========================================================================+
  // | [>] fail                                                     ( public ) |
  // +=========================================================================+
  void fail(encoder_error e) {
    status_ = encoder_status::error;
    error_ = e;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  ENty encoder_;
  common::writer writer_;
  encoder_status status_{encoder_status::open};
  encoder_error error_{encoder_error::none};
  std::size_t stored_size_{0};
};
}  // namespace martianlabs::doba::protocol::http11::body

#endif
