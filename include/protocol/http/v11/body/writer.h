//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef martianlabs_doba_protocol_http_v11_body_writer_h
#define martianlabs_doba_protocol_http_v11_body_writer_h

#include <cstddef>
#include <span>
#include <string_view>
#include <utility>
#include <variant>

#include "common/byte_storage.h"
#include "common/writer.h"
#include "protocol/http/v11/body/writer_chunked.h"
#include "protocol/http/v11/body/writer_raw.h"

namespace martianlabs::doba::protocol::http::v11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] body_writer                                                 ( class ) |
// +---------------------------------------------------------------------------+
// | Collects body payload as raw bytes or chunked HTTP output.                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class body_writer {
 public:
  // +=========================================================================+
  // | [>] chunked                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Stores payload with chunked framing. opts controls the backing storage. |
  // +-------------------------------------------------------------------------+
  static body_writer chunked(common::byte_storage_options opts = {}) {
    return body_writer(common::writer(std::move(opts)), writer_chunked());
  }
  // +=========================================================================+
  // | [>] raw                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  // | Stores payload as-is. opts controls the backing storage.                |
  // +-------------------------------------------------------------------------+
  static body_writer raw(common::byte_storage_options opts = {}) {
    return body_writer(common::writer(std::move(opts)), writer_raw());
  }
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +-------------------------------------------------------------------------+
  body_writer(const body_writer&) = delete;
  body_writer(body_writer&&) noexcept = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  body_writer& operator=(const body_writer&) = delete;
  body_writer& operator=(body_writer&&) noexcept = default;
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Appends payload using the selected framing; false means writing failed. |
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
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
  // | Adds the final chunk once; raw bodies need no terminator.               |
  // +-------------------------------------------------------------------------+
  bool end() {
    return std::visit([this](auto& encoder) { return encoder.end(sink_); },
                      encoder_);
  }
  // +=========================================================================+
  // | [>] release                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Finishes framing and transfers the stored bytes, including any file.    |
  // +-------------------------------------------------------------------------+
  [[nodiscard]] common::byte_storage release() {
    end();
    return sink_.release();
  }
  // +=========================================================================+
  // | [>] is_chunked                                               ( public ) |
  // +-------------------------------------------------------------------------+
  [[nodiscard]] bool is_chunked() const noexcept {
    return std::holds_alternative<writer_chunked>(encoder_);
  }
  // +=========================================================================+
  // | [>] bytes_written                                            ( public ) |
  // +-------------------------------------------------------------------------+
  // | Payload bytes from successful write() calls, without framing.           |
  // +-------------------------------------------------------------------------+
  [[nodiscard]] std::size_t bytes_written() const noexcept {
    return bytes_written_;
  }

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                            ( private ) |
  // +-------------------------------------------------------------------------+
  body_writer(common::writer sink,
              std::variant<writer_chunked, writer_raw> encoder)
      : sink_(std::move(sink)), encoder_(std::move(encoder)) {}
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  common::writer sink_;
  std::variant<writer_chunked, writer_raw> encoder_;
  std::size_t bytes_written_{0};
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
