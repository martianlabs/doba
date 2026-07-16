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

#ifndef martianlabs_doba_protocol_http11_body_decoder_raw_h
#define martianlabs_doba_protocol_http11_body_decoder_raw_h

#include <cstddef>
#include <span>

#include "protocol/http11/body/decoder.h"

namespace martianlabs::doba::protocol::http11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] decoder_raw                                                 ( class ) |
// +---------------------------------------------------------------------------+
// | This class represents the HTTP/1.1 raw body decoder.                      |
// | It handles the decoding of raw transfer encoding, managing the            |
// | state transitions and error handling.                                     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class decoder_raw {
 public:
  // +=========================================================================+
  // | [>] decode                                                   ( public ) |
  // +=========================================================================+
  template <typename RDty>
  decode_result decode(RDty& source, std::span<std::byte> output,
                       std::size_t& consumed) {
    decode_result result;
    std::size_t bytes = source.read(output);
    if (bytes == 0) {
      if (!source.ok()) {
        result.has_error = true;
        result.error = decoder_error::io_error;
      } else {
        result.complete = true;
      }
      return result;
    }
    result.produced = bytes;
    consumed += bytes;
    result.complete = source.exhausted();
    return result;
  }
};
}  // namespace martianlabs::doba::protocol::http11::body

#endif
