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

#ifndef martianlabs_doba_protocol_http_v11_body_reader_raw_h
#define martianlabs_doba_protocol_http_v11_body_reader_raw_h

#include <algorithm>
#include <cstddef>
#include <span>

#include "common/reader.h"
#include "protocol/http/v11/body/reader_state.h"

namespace martianlabs::doba::protocol::http::v11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] reader_raw                                                   ( class ) |
// +---------------------------------------------------------------------------+
// | Decodes a Content-Length-framed body by pulling wire bytes from a         |
// | common::reader source and filling the caller-supplied output span.        |
// |                                                                           |
// | Since a raw (Content-Length) body carries no wire framing of its own, the |
// | decoded payload is identical to the source bytes: this reader simply     |
// | copies bytes from src into output, up to the declared Content-Length. It  |
// | exists to keep the reader API symmetrical with reader_chunked.           |
// |                                                                           |
// | If src reaches definitive eof() before Content-Length is satisfied, this  |
// | is a protocol error reported as reader_error::raw_incomplete. Once an     |
// | error is reported, it is latched: every subsequent call returns the same  |
// | has_error/error without touching src again.                               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class reader_raw {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +=========================================================================+
  explicit reader_raw(std::size_t content_length) : expected_(content_length) {}
  // +=========================================================================+
  // | [>] read                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Pulls up to (expected_ - accumulated_) bytes from src into output.      |
  // | Returns immediately with complete=true when Content-Length is reached.  |
  // | A zero Content-Length body completes on the first call with produced=0. |
  // +=========================================================================+
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

 private:
  // +=========================================================================+
  // | [>] fail                                                    ( private ) |
  // +=========================================================================+
  reader_state fail(reader_state& result, reader_error err) {
    has_error_ = true;
    error_ = err;
    result.has_error = true;
    result.error = err;
    return result;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::size_t expected_;
  std::size_t accumulated_{0};
  bool has_error_{false};
  reader_error error_{reader_error::none};
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
