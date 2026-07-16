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

#ifndef martianlabs_doba_protocol_http11_body_encoder_chunked_h
#define martianlabs_doba_protocol_http11_body_encoder_chunked_h

#include <cstddef>
#include <cstdint>
#include <limits>

#include "protocol/http11/body/encoder.h"

namespace martianlabs::doba::protocol::http11::body {
// /////////////////////////////////////////////////////////////////////////////
// +===========================================================================+
// | [>] encoder_chunked                                             ( class ) |
// +---------------------------------------------------------------------------+
// | Represents a chunked encoder.                                             |
// +===========================================================================+
// /////////////////////////////////////////////////////////////////////////////
class encoder_chunked {
 public:
  // +=========================================================================+
  // | [>] encode                                                   ( public ) |
  // +=========================================================================+
  template <typename WRty>
  bool encode(WRty& sink, const char* data, std::size_t len,
              encode_result& out) {
    out.stored = 0;
    if (len == 0) return true;
    char size_line[24];
    int size_line_len = format_chunk_size(size_line, len);
    std::size_t frame_overhead = static_cast<std::size_t>(size_line_len) + 4;
    if (len > std::numeric_limits<std::size_t>::max() - frame_overhead) {
      out.has_error = true;
      out.error = encoder_error::chunk_size_overflow;
      return false;
    }
    if (!sink.write(size_line, static_cast<std::size_t>(size_line_len)) ||
        !sink.write("\r\n", 2) || !sink.write(data, len) ||
        !sink.write("\r\n", 2)) {
      return false;
    }
    out.stored = frame_overhead + len;
    return true;
  }
  // +=========================================================================+
  // | [>] finish                                                   ( public ) |
  // +=========================================================================+
  template <typename WRty>
  bool finish(WRty& sink, encode_result& out) {
    out.stored = 0;
    if (!sink.write("0\r\n\r\n", 5)) return false;
    out.stored = 5;
    return true;
  }

 private:
  // +=========================================================================+
  // | [>] format_chunk_size                                       ( private ) |
  // +=========================================================================+
  static int format_chunk_size(char* buffer, std::size_t size) {
    char reversed[16];
    int count = 0;
    if (size == 0) {
      buffer[0] = '0';
      return 1;
    }
    while (size > 0) {
      reversed[count++] = "0123456789abcdef"[size & 0xF];
      size >>= 4;
    }
    for (int index = 0; index < count; ++index) {
      buffer[index] = reversed[count - 1 - index];
    }
    return count;
  }
};
}  // namespace martianlabs::doba::protocol::http11::body

#endif
