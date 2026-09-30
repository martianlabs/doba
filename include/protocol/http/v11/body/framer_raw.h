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

#ifndef martianlabs_doba_protocol_http_v11_body_framer_raw_h
#define martianlabs_doba_protocol_http_v11_body_framer_raw_h

#include <algorithm>
#include <cstddef>
#include <span>

#include "protocol/http/v11/body/framer_state.h"

namespace martianlabs::doba::protocol::http::v11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] framer_raw                                                  ( class ) |
// +---------------------------------------------------------------------------+
// | Detects the Content-Length-framed body boundary within an already-wire-   |
// | encoded input buffer.                                                     |
// |                                                                           |
// | The caller pushes incoming transport spans via consume(); each call       |
// | returns a framer_state indicating how many bytes were consumed from the   |
// | span and whether the body is complete. Bytes beyond the declared          |
// | Content-Length are never touched - they belong to the next request.       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class framer_raw {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +=========================================================================+
  explicit framer_raw(std::size_t content_length) : expected_(content_length) {}
  // +=========================================================================+
  // | [>] consume                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Consumes up to (expected_ - accumulated_) bytes from input.             |
  // | Returns immediately with complete=true when Content-Length is reached.  |
  // | A zero Content-Length body completes on the first call with consumed=0. |
  // +=========================================================================+
  framer_state consume(std::span<const std::byte> input) {
    framer_state result;
    std::size_t remaining = expected_ - accumulated_;
    std::size_t to_consume = std::min(remaining, input.size());
    accumulated_ += to_consume;
    result.consumed = to_consume;
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
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
