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

#ifndef martianlabs_doba_protocol_http_v11_body_reader_h
#define martianlabs_doba_protocol_http_v11_body_reader_h

#include <cstddef>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <variant>

#include "common/reader.h"
#include "protocol/http/v11/body/reader_chunked.h"
#include "protocol/http/v11/body/reader_raw.h"
#include "protocol/http/v11/body/reader_state.h"

namespace martianlabs::doba::protocol::http::v11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] reader                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | Reads decoded body bytes from borrowed memory or owned storage.           |
// | Raw bodies use Content-Length; chunked bodies strip framing.              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class reader {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +-------------------------------------------------------------------------+
  reader(const reader&) = delete;
  reader(reader&&) noexcept = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  reader& operator=(const reader&) = delete;
  reader& operator=(reader&&) noexcept = default;
  // +=========================================================================+
  // | [>] chunked                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Borrows chunked bytes; keep source alive while reading.                 |
  // | Limits count extension bytes per chunk and trailer bytes per body.      |
  // | A limit of 0 means unlimited.                                           |
  // +-------------------------------------------------------------------------+
  static reader chunked(std::span<const std::byte> source,
                        std::size_t extension_limit =
                            reader_chunked::kDefaultMaxChunkExtensionSize,
                        std::size_t trailer_limit =
                            reader_chunked::kDefaultMaxTrailerSectionSize) {
    if (source.empty()) throw std::invalid_argument("Empty body source");
    return reader(common::reader::borrowed(source),
                  reader_chunked(extension_limit, trailer_limit));
  }
  // +=========================================================================+
  // | [>] chunked                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Takes an open file containing chunked bytes.                            |
  // | Limits count extension bytes per chunk and trailer bytes per body.      |
  // | A limit of 0 means unlimited.                                           |
  // +-------------------------------------------------------------------------+
  static reader chunked(common::filesystem&& source,
                        std::size_t extension_limit =
                            reader_chunked::kDefaultMaxChunkExtensionSize,
                        std::size_t trailer_limit =
                            reader_chunked::kDefaultMaxTrailerSectionSize) {
    if (!source.is_open()) throw std::invalid_argument("Body file is not open");
    return reader(common::reader(std::move(source)),
                  reader_chunked(extension_limit, trailer_limit));
  }
  // +=========================================================================+
  // | [>] raw                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  // | Borrows a Content-Length body; keep source alive while reading.         |
  // | content_length is the expected byte count, not a size limit.            |
  // +-------------------------------------------------------------------------+
  static reader raw(std::span<const std::byte> source,
                    std::size_t content_length) {
    if (source.empty()) throw std::invalid_argument("Empty body source");
    return reader(common::reader::borrowed(source), reader_raw(content_length),
                  source);
  }
  // +=========================================================================+
  // | [>] raw                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  // | Takes an open file and reads exactly content_length body bytes.         |
  // +-------------------------------------------------------------------------+
  static reader raw(common::filesystem&& source, std::size_t content_length) {
    if (!source.is_open()) throw std::invalid_argument("Body file is not open");
    return reader(common::reader(std::move(source)),
                  reader_raw(content_length));
  }
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Fills output with payload bytes, removing any chunked framing.          |
  // | Reports bytes produced, completion and any read or framing error.       |
  // +-------------------------------------------------------------------------+
  reader_state read(std::span<std::byte> output) {
    return std::visit(
        [this, output](auto& decoder) { return decoder.read(source_, output); },
        decoder_);
  }
  // +=========================================================================+
  // | [>] take_borrowed_raw                                        ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns the remaining raw bytes without copying, if they fit maximum.   |
  // | Needs a complete borrowed buffer; marks the body as consumed.           |
  // +-------------------------------------------------------------------------+
  std::optional<std::span<const std::byte>> take_borrowed_raw(
      std::size_t maximum) {
    if (borrowed_raw_.empty()) return std::nullopt;
    auto* decoder = std::get_if<reader_raw>(&decoder_);
    return decoder ? decoder->take_view(borrowed_raw_, maximum) : std::nullopt;
  }
  // +=========================================================================+
  // | [>] raw                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  // | Takes stored body bytes, including any temporary file.                  |
  // | length is the expected body size.                                       |
  // +-------------------------------------------------------------------------+
  static reader raw(common::byte_storage source, std::size_t length) {
    return reader(common::reader(std::move(source)), reader_raw(length));
  }
  // +=========================================================================+
  // | [>] chunked                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Takes stored chunked bytes, including any temporary file.               |
  // | Limits count extensions per chunk and all trailers; 0 means unlimited.  |
  // +-------------------------------------------------------------------------+
  static reader chunked(common::byte_storage source,
                        std::size_t extension_limit,
                        std::size_t trailer_limit) {
    return reader(common::reader(std::move(source)),
                  reader_chunked(extension_limit, trailer_limit));
  }
  // +=========================================================================+
  // | [>] rebind_borrowed                                          ( public ) |
  // +-------------------------------------------------------------------------+
  // | Points borrowed views at relocated bytes, keeping the read position.    |
  // | source must contain the same bytes at their new location.               |
  // +-------------------------------------------------------------------------+
  void rebind_borrowed(std::span<const std::byte> source) noexcept {
    source_.rebind_borrowed(source);
    if (!borrowed_raw_.empty()) borrowed_raw_ = source;
  }

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                            ( private ) |
  // +-------------------------------------------------------------------------+
  reader(common::reader source,
         std::variant<reader_chunked, reader_raw> decoder,
         std::span<const std::byte> borrowed_raw = {})
      : source_(std::move(source)),
        decoder_(std::move(decoder)),
        borrowed_raw_(borrowed_raw) {}
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  common::reader source_;
  std::variant<reader_chunked, reader_raw> decoder_;
  std::span<const std::byte> borrowed_raw_;
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
