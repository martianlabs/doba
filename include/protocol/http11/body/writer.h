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

#ifndef martianlabs_doba_protocol_http11_body_writer_h
#define martianlabs_doba_protocol_http11_body_writer_h

#include <cstddef>
#include <span>
#include <string_view>
#include <utility>
#include <variant>

#include "common/byte_storage.h"
#include "common/writer.h"
#include "protocol/http11/body/writer_chunked.h"
#include "protocol/http11/body/writer_raw.h"

namespace martianlabs::doba::protocol::http11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] body_writer                                                 ( class ) |
// +---------------------------------------------------------------------------+
// | Owns the wire-level common::writer sink together with the body encoder    |
// | (writer_chunked or writer_raw) matching the framing chosen for the        |
// | outgoing response (Transfer-Encoding: chunked vs Content-Length). This    |
// | lets callers push raw payload bytes via write() without ever having to    |
// | know which framing will encode them, mirroring body::reader on the        |
// | request side.                                                             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class body_writer {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +=========================================================================+
  static body_writer chunked(common::byte_storage_options opts = {}) {
    return body_writer(common::writer(std::move(opts)), writer_chunked());
  }
  static body_writer raw(common::byte_storage_options opts = {}) {
    return body_writer(common::writer(std::move(opts)), writer_raw());
  }
  body_writer(const body_writer&) = delete;
  body_writer(body_writer&&) noexcept = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  body_writer& operator=(const body_writer&) = delete;
  body_writer& operator=(body_writer&&) noexcept = default;
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Encodes the caller-supplied raw payload using the framing selected at   |
  // | construction time and appends it into the owned sink.                   |
  // +=========================================================================+
  bool write(std::span<const std::byte> payload) {
    if (!std::visit(
            [this, payload](auto& encoder) {
              return encoder.write(payload, sink_);
            },
            encoder_)) {
      return false;
    }
    bytes_written_ += payload.size();
    return true;
  }
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +=========================================================================+
  bool write(std::string_view payload) {
    if (!std::visit(
            [this, payload](auto& encoder) {
              return encoder.write(payload, sink_);
            },
            encoder_)) {
      return false;
    }
    bytes_written_ += payload.size();
    return true;
  }
  // +=========================================================================+
  // | [>] end                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  // | Finalizes the body framing (emits the terminating chunk for chunked     |
  // | encoding; a no-op for raw). Idempotent.                                 |
  // +=========================================================================+
  bool end() {
    return std::visit([this](auto& encoder) { return encoder.end(sink_); },
                      encoder_);
  }
  // +=========================================================================+
  // | [>] release                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Finalizes the framing and the underlying storage, then hands over the   |
  // | accumulated (possibly spilled to disk) body bytes to the caller.        |
  // +=========================================================================+
  [[nodiscard]] common::byte_storage release() {
    end();
    return sink_.release();
  }
  // +=========================================================================+
  // | [>] is_chunked                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // | Reports whether this instance was constructed via chunked() (as opposed |
  // | to raw()). Lets callers that receive an already-built/used body_writer  |
  // | (e.g. response::set_body) pick the matching wire framing headers.       |
  // +=========================================================================+
  [[nodiscard]] bool is_chunked() const noexcept {
    return std::holds_alternative<writer_chunked>(encoder_);
  }
  // +=========================================================================+
  // | [>] bytes_written                                            ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns the total number of raw payload bytes passed to write() so far  |
  // | (i.e. before framing overhead). Lets callers that receive an already-   |
  // | used raw body_writer (e.g. response::set_body) derive Content-Length    |
  // | without tracking it themselves.                                         |
  // +=========================================================================+
  [[nodiscard]] std::size_t bytes_written() const noexcept {
    return bytes_written_;
  }

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                            ( private ) |
  // +=========================================================================+
  body_writer(common::writer sink,
              std::variant<writer_chunked, writer_raw> encoder)
      : sink_(std::move(sink)), encoder_(std::move(encoder)) {}
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  common::writer sink_;
  std::variant<writer_chunked, writer_raw> encoder_;
  std::size_t bytes_written_{0};
};
}  // namespace martianlabs::doba::protocol::http11::body

#endif
