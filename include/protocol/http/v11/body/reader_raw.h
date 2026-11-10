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

#ifndef martianlabs_doba_protocol_http_v11_body_reader_raw_h
#define martianlabs_doba_protocol_http_v11_body_reader_raw_h

#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>

#include "common/reader.h"
#include "protocol/http/v11/body/reader_state.h"

namespace martianlabs::doba::protocol::http::v11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] reader_raw                                                  ( class ) |
// +---------------------------------------------------------------------------+
// | Reads a Content-Length body from a common::reader.                        |
// | Stops at the declared length, leaving any later bytes untouched.          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class reader_raw {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +-------------------------------------------------------------------------+
  explicit reader_raw(std::size_t content_length) : expected_(content_length) {}
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Copies body bytes into output and reports progress or errors.           |
  // | Zero length completes without reading. Early EOF is raw_incomplete.     |
  // | Errors persist on later calls.                                          |
  // +-------------------------------------------------------------------------+
  reader_state read(common::reader& src, std::span<std::byte> output) {
    reader_state result;
    if (has_error_) {
      result.has_error = true;
      result.error = error_;
      return result;
    }
    std::size_t remaining = expected_ - accumulated_;
    std::size_t to_read = std::min(remaining, output.size());
    if (to_read > 0) {
      std::size_t got = src.read(output.subspan(0, to_read));
      if (got == 0) {
        if (src.failed()) {
          return fail(result, reader_error::io_error);
        }
        if (src.eof()) {
          return fail(result, reader_error::raw_incomplete);
        }
      }
      accumulated_ += got;
      result.produced = got;
    }
    result.complete = (accumulated_ == expected_);
    return result;
  }
  // +=========================================================================+
  // | [>] take_view                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns remaining bytes without copying; marks the body consumed.       |
  // | Needs the whole body in source and a remainder no larger than maximum.  |
  // | On failure, returns an empty optional without advancing.                |
  // +-------------------------------------------------------------------------+
  std::optional<std::span<const std::byte>> take_view(
      std::span<const std::byte> source, std::size_t maximum) {
    if (has_error_ || source.size() < expected_ ||
        expected_ - accumulated_ > maximum)
      return std::nullopt;
    const auto result = source.subspan(accumulated_, expected_ - accumulated_);
    accumulated_ = expected_;
    return result;
  }

 private:
  // +=========================================================================+
  // | [>] fail                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  reader_state fail(reader_state& result, reader_error err) {
    has_error_ = true;
    error_ = err;
    result.has_error = true;
    result.error = err;
    return result;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::size_t expected_;
  std::size_t accumulated_{0};
  bool has_error_{false};
  reader_error error_{reader_error::none};
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
