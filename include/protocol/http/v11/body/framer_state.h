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

#ifndef martianlabs_doba_protocol_http_v11_body_framer_state_h
#define martianlabs_doba_protocol_http_v11_body_framer_state_h

#include <cstddef>

#include "protocol/http/v11/body/framer_error.h"

namespace martianlabs::doba::protocol::http::v11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] framer_state                                               ( struct ) |
// +---------------------------------------------------------------------------+
// | Result of a body framer consume() call.                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct framer_state {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Bytes taken by this call, including framing. Advance input by this amount.
  std::size_t consumed = 0;
  // The body ended; any remaining input belongs to the next request.
  bool complete = false;
  // Framing was invalid or a size limit was exceeded.
  bool has_error = false;
  // Only meaningful when has_error is true.
  framer_error error = framer_error::none;
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
