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

#ifndef martianlabs_doba_protocol_http11_body_writer_raw_h
#define martianlabs_doba_protocol_http11_body_writer_raw_h

#include <algorithm>
#include <cstddef>
#include <span>

#include "common/writer.h"
#include "protocol/http11/body/writer_state.h"

namespace martianlabs::doba::protocol::http11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] writer_raw                                                  ( class ) |
// +---------------------------------------------------------------------------+
// | Accumulates a Content-Length-framed body into a common::writer.           |
// |                                                                           |
// | The caller pushes incoming transport spans via write(); each call         |
// | returns a feed_result indicating how many bytes were consumed from the    |
// | span and whether the body is complete. Bytes beyond the declared          |
// | Content-Length are never touched — they belong to the next request.       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class writer_raw {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +=========================================================================+
  explicit writer_raw(std::size_t content_length) : expected_(content_length) {}
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Writes up to (expected_ - accumulated_) bytes from input into dst.      |
  // | Returns immediately with complete=true when Content-Length is reached.  |
  // | A zero Content-Length body completes on the first call with consumed=0. |
  // +=========================================================================+
  writer_state write(std::span<const std::byte> input, common::writer& dst) {
    writer_state result;
    std::size_t remaining = expected_ - accumulated_;
    std::size_t to_consume = std::min(remaining, input.size());
    if (to_consume > 0) {
      if (!dst.write(input.subspan(0, to_consume))) {
        result.has_error = true;
        result.error = writer_error::io_error;
        return result;
      }
      accumulated_ += to_consume;
      result.consumed = to_consume;
    }
    result.complete = (accumulated_ == expected_);
    return result;
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  std::size_t expected_;
  std::size_t accumulated_{0};
};
}  // namespace martianlabs::doba::protocol::http11::body

#endif
