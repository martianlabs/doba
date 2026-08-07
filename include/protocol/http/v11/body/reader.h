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
#include <span>
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
// | Owns the wire-level common::reader source together with the body decoder  |
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
  // +=========================================================================+
  static reader chunked(common::reader source) {
    return reader(std::move(source), reader_chunked());
  }
  static reader raw(common::reader source, std::size_t content_length) {
    return reader(std::move(source), reader_raw(content_length));
  }
  reader(const reader&) = delete;
  reader(reader&&) noexcept = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +=========================================================================+
  reader& operator=(const reader&) = delete;
  reader& operator=(reader&&) noexcept = default;
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Pulls wire bytes from the owned source, decodes them using the encoding |
  // | selected at construction time, and writes the decoded payload into      |
  // | output. See reader_chunked::read/reader_raw::read for semantics.        |
  // +=========================================================================+
  reader_state read(std::span<std::byte> output) {
    return std::visit(
        [this, output](auto& decoder) { return decoder.read(source_, output); },
        decoder_);
  }

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                            ( private ) |
  // +=========================================================================+
  reader(common::reader source,
         std::variant<reader_chunked, reader_raw> decoder)
      : source_(std::move(source)), decoder_(std::move(decoder)) {}
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  common::reader source_;
  std::variant<reader_chunked, reader_raw> decoder_;
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
