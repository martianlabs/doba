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
// | Report wire bytes consumed, completion and any error. Advance by          |
// | consumed; the remaining bytes belong to the next request.                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct framer_state {
  // Wire bytes consumed, framing included.
  std::size_t consumed = 0;
  // Body parsed; no more consume() calls needed.
  bool complete = false;
  // A protocol or size-limit check failed.
  bool has_error = false;
  // Error code when has_error is set.
  framer_error error = framer_error::none;
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
