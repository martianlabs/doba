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

#ifndef martianlabs_doba_protocol_http11_body_decoder_h
#define martianlabs_doba_protocol_http11_body_decoder_h

#include <cstddef>
#include <cstdint>

namespace martianlabs::doba::protocol::http11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] decoder_status                                         ( enum-class ) |
// +---------------------------------------------------------------------------+
// | This enum class represents the status of the HTTP/1.1 body decoder.       |
// | It can be in one of three states: 'open' (the decoder is ready to process |
// | data), 'complete' (the decoding process has finished successfully),       |
// | or 'error' (an error occurred during decoding).                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class decoder_status : std::uint8_t { open, complete, error };
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] decoder_error                                          ( enum-class ) |
// +---------------------------------------------------------------------------+
// | This enum class represents the error status of the HTTP/1.1 body decoder. |
// | It can be in one of several states, indicating the type of error that     |
// | occurred during decoding.                                                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class decoder_error : std::uint8_t {
  none,
  io_error,
  invalid_chunk_size,
  chunk_size_overflow,
  invalid_chunk_crlf,
  invalid_trailer,
  chunked_incomplete,
  raw_size_limit_exceeded,
  source_size_limit_exceeded,
  chunk_extension_size_limit_exceeded,
  trailer_size_limit_exceeded
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] CONSTANTs                                                  ( public ) |
// +---------------------------------------------------------------------------+
// | This constant represents the default buffer size used in the              |
// | HTTP/1.1 body decoder. It is set to 8192 bytes, which is a common size    |
// | for buffer operations in network programming. This value can be used as a |
// | guideline for allocating buffers when decoding HTTP bodies, ensuring      |
// | efficient memory usage and performance.                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
inline constexpr std::size_t kDefaultBufferSize = 8192;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] decode_result                                              ( struct ) |
// +---------------------------------------------------------------------------+
// | This struct represents the result of a decode operation in the HTTP/1.1   |
// | body decoder. It contains information about the number of bytes produced, |
// | whether the decoding is complete, and any errors that occurred.           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct decode_result {
  std::size_t produced = 0;
  bool complete = false;
  bool has_error = false;
  decoder_error error = decoder_error::none;
};
}  // namespace martianlabs::doba::protocol::http11::body

#endif
