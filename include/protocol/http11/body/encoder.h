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

#ifndef martianlabs_doba_protocol_http11_body_encoder_h
#define martianlabs_doba_protocol_http11_body_encoder_h

#include <cstddef>
#include <cstdint>

namespace martianlabs::doba::protocol::http11::body {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] encoder_status                                         ( enum-class ) |
// +---------------------------------------------------------------------------+
// | Represents the current status of an encoder.                              |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
enum class encoder_status : std::uint8_t { open, complete, error };
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] encoder_error                                          ( enum-class ) |
// +---------------------------------------------------------------------------+
// | Represents the current error state of an encoder.                         |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
enum class encoder_error : std::uint8_t {
  none,
  io_error,
  chunk_size_overflow,
  stored_size_limit_exceeded
};
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] encode_result                                              ( struct ) |
// +---------------------------------------------------------------------------+
// | Represents the result of an encoding operation.                           |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
struct encode_result {
  std::size_t stored = 0;
  bool has_error = false;
  encoder_error error = encoder_error::none;
};
}  // namespace martianlabs::doba::protocol::http11::body

#endif
