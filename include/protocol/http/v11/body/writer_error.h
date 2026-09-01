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

#ifndef martianlabs_doba_protocol_http_v11_writer_error_h
#define martianlabs_doba_protocol_http_v11_writer_error_h

#include <cstddef>
#include <cstdint>

namespace martianlabs::doba::protocol::http::v11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] writer_error                                           ( enum-class ) |
// +---------------------------------------------------------------------------+
// | Error codes shared by writer_raw and writer_chunked.                      |
// |                                                                           |
// | Actively produced by these classes:                                       |
// |   io_error, invalid_chunk_size, chunk_size_overflow, invalid_chunk_crlf,  |
// |   chunk_extension_size_limit_exceeded, trailer_size_limit_exceeded.       |
// |                                                                           |
// | Reserved (declared for API parity / future use, never produced here):     |
// |   invalid_trailer     - full trailer field-line syntax validation is not  |
// |                         implemented; trailers are only framed, not        |
// |                         validated as header fields.                       |
// |   chunked_incomplete  - a body that never reaches its terminating CRLF    |
// |                         simply keeps requesting more bytes (kMoreBytes-   |
// |                         Needed at the decoder level); premature-EOF       |
// |                         detection belongs to connection/channel handling, |
// |                         not to this class.                                |
// |   raw_size_limit_exceeded - Content-Length vs. policy limits are enforced |
// |                         (or should be) at the header/policy layer before  |
// |                         a body writer is ever constructed.                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
enum class writer_error : std::uint8_t {
  none,
  io_error,
  invalid_chunk_size,
  chunk_size_overflow,
  invalid_chunk_crlf,
  invalid_trailer,
  chunked_incomplete,
  raw_size_limit_exceeded,
  chunk_extension_size_limit_exceeded,
  trailer_size_limit_exceeded
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
