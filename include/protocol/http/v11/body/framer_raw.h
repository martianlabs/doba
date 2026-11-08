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
// | Find a Content-Length body's end in the incoming wire bytes.              |
// |                                                                           |
// | Feed bytes through consume(); it reports how much belongs to this body.   |
// | Leave anything past Content-Length for the next request.                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class framer_raw {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs                                             ( public ) |
  // +-------------------------------------------------------------------------+
  explicit framer_raw(std::size_t content_length) : expected_(content_length) {}
  // +=========================================================================+
  // | [>] consume                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Consume up to the remaining Content-Length. At the limit, return        |
  // | complete immediately; a zero-length body consumes nothing.              |
  // +-------------------------------------------------------------------------+
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
  // +-------------------------------------------------------------------------+
  std::size_t expected_;
  std::size_t accumulated_{0};
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
