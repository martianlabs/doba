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

#ifndef martianlabs_doba_protocol_http11_reader_error_h
#define martianlabs_doba_protocol_http11_reader_error_h

#include <cstddef>
#include <cstdint>

namespace martianlabs::doba::protocol::http11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] reader_error                                            ( enum-class ) |
// +---------------------------------------------------------------------------+
// | Error codes shared by reader_raw and reader_chunked.                      |
// |                                                                           |
// | Actively produced by these classes:                                       |
// |   io_error, invalid_chunk_size, chunk_size_overflow, invalid_chunk_crlf,   |
// |   chunked_incomplete, raw_incomplete, chunk_extension_size_limit_exceeded, |
// |   trailer_size_limit_exceeded.                                            |
// |                                                                           |
// | Reserved (declared for API parity / future use, never produced here):     |
// |   invalid_trailer     - full trailer field-line syntax validation is not  |
// |                         implemented; trailers are only framed, not        |
// |                         validated as header fields (same scope as         |
// |                         writer_chunked).                                  |
// |   raw_size_limit_exceeded - Content-Length vs. policy limits are enforced |
// |                         (or should be) at the header/policy layer before  |
// |                         a body reader is ever constructed, since these    |
// |                         classes never see more bytes than the already     |
// |                         agreed-upon Content-Length.                       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class reader_error : std::uint8_t {
  none,
  io_error,
  invalid_chunk_size,
  chunk_size_overflow,
  invalid_chunk_crlf,
  invalid_trailer,
  chunked_incomplete,
  raw_incomplete,
  raw_size_limit_exceeded,
  chunk_extension_size_limit_exceeded,
  trailer_size_limit_exceeded
};
}  // namespace martianlabs::doba::protocol::http11::body

#endif
