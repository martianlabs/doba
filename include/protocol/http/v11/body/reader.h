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
// | Holds a borrowed buffer or an owned file with the body decoder.           |
// | (reader_chunked or reader_raw) matching the encoding actually used by the |
// | request (Transfer-Encoding: chunked vs Content-Length). This lets callers |
// | pull already-decoded payload bytes via read() without ever having to know |
// | which framing produced them.                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class reader {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +-------------------------------------------------------------------------+
  static reader chunked(std::span<const std::byte> source) {
    if (source.empty()) throw std::invalid_argument("Empty body source");
    return reader(common::reader::borrowed(source), reader_chunked());
  }
  static reader chunked(common::filesystem_file&& source) {
    if (!source.is_open()) throw std::invalid_argument("Body file is not open");
    return reader(common::reader(std::move(source)), reader_chunked());
  }
  static reader raw(std::span<const std::byte> source,
                    std::size_t content_length) {
    if (source.empty()) throw std::invalid_argument("Empty body source");
    return reader(common::reader::borrowed(source), reader_raw(content_length),
                  source);
  }
  static reader raw(common::filesystem_file&& source,
                    std::size_t content_length) {
    if (!source.is_open()) throw std::invalid_argument("Body file is not open");
    return reader(common::reader(std::move(source)),
                  reader_raw(content_length));
  }
  reader(const reader&) = delete;
  reader(reader&&) noexcept = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  reader& operator=(const reader&) = delete;
  reader& operator=(reader&&) noexcept = default;
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Pulls wire bytes from the owned source, decodes them using the encoding |
  // | selected at construction time, and writes the decoded payload into      |
  // | output. See reader_chunked::read/reader_raw::read for semantics.        |
  // +-------------------------------------------------------------------------+
  reader_state read(std::span<std::byte> output) {
    return std::visit(
        [this, output](auto& decoder) { return decoder.read(source_, output); },
        decoder_);
  }
  // +=========================================================================+
  // | [>] take_borrowed_raw                                       ( public ) |
  // +-------------------------------------------------------------------------+
  std::optional<std::span<const std::byte>> take_borrowed_raw(
      std::size_t maximum) {
    if (borrowed_raw_.empty()) return std::nullopt;
    auto* decoder = std::get_if<reader_raw>(&decoder_);
    return decoder ? decoder->take_view(borrowed_raw_, maximum) : std::nullopt;
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
