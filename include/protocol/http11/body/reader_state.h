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

#ifndef martianlabs_doba_protocol_http11_body_reader_state_h
#define martianlabs_doba_protocol_http11_body_reader_state_h

#include <cstddef>
#include <cstdint>

#include "protocol/http11/body/reader_error.h"

namespace martianlabs::doba::protocol::http11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] reader_state                                                ( struct ) |
// +---------------------------------------------------------------------------+
// | Result of a body reader read() call.                                      |
// |   produced  - bytes of decoded payload written into the caller-supplied   |
// |               output span during this call. Wire framing bytes consumed  |
// |               from the source reader are internal and not reported.      |
// |   complete  - body fully decoded; no further read() calls needed.        |
// |   has_error - a protocol or size-limit error was detected.                |
// |   error     - error code, if has_error is true.                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct reader_state {
  // Bytes of decoded payload written into the output span during this call.
  std::size_t produced = 0;
  // Body fully decoded; no further read() calls needed.
  bool complete = false;
  // A protocol or size-limit error was detected.
  bool has_error = false;
  // Error code, if has_error is true.
  reader_error error = reader_error::none;
};
}  // namespace martianlabs::doba::protocol::http11::body

#endif
