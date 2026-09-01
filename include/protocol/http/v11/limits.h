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

#ifndef martianlabs_doba_protocol_http_v11_limits_h
#define martianlabs_doba_protocol_http_v11_limits_h

#include <cstddef>

namespace martianlabs::doba::protocol::http::v11 {
// ///////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] limits                                                     ( struct ) |
// +---------------------------------------------------------------------------+
// | Centralized repository of http 1.1 operational limits. This file holds    |
// | exclusively constants: suggested default values that a server may adopt   |
// | (verbatim or overridden) when populating protocol::http::v11::policies.   |
// | It carries no logic and no dependency on decoder/request/response,        |
// | so any component may reference these numbers without pulling in           |
// | parsing code.                                                             |
// +---------------------------------------------------------------------------+
// | These defaults are deliberately conservative-but-practical values drawn   |
// | from common HTTP server practice; they are not wired in automatically.    |
// | Policies remain permissive (0 = unlimited) unless a server explicitly     |
// | assigns one of these constants (or its own value) to the matching         |
// | protocol::http::v11::policies field.                                      |
// +---------------------------------------------------------------------------+
// ///////////////////////////////////////////////////////////////////////////
struct limits {
  // Maximum accepted request-target length, in octets. A request-target
  // longer than this is rejected with 414 URI Too Long (RFC 9110 S15.5.15).
  static constexpr std::size_t kDefaultMaxUriLength = 1024;
  // Maximum accepted size of the whole header section (all header field
  // names and values combined, before CRLF/OWS overhead), in octets. A
  // header section larger than this is rejected with 431 Request Header
  // Fields Too Large (RFC 6585 S5).
  static constexpr std::size_t kDefaultMaxHeaderSectionSize = 4096;
  // Maximum accepted Content-Length, in octets. A body larger than this is
  // rejected with 413 Content Too Large (RFC 9110 S15.5.14).
  static constexpr std::size_t kDefaultMaxContentLength = 10 * 1024 * 1024;
  // Maximum number of forwarding hops accepted across Via / Forwarded /
  // X-Forwarded-For before the request is rejected.
  static constexpr std::size_t kDefaultMaxForwardingHops = 20;
  // Maximum number of transfer-codings accepted in a single Transfer-Encoding
  // header before the request is rejected.
  static constexpr std::size_t kDefaultMaxTransferCodings = 4;
  // Maximum size, in octets, of a request's head (request-line plus header
  // section) once mounted onto request::buffer_ (request.h). Derived from
  // the request-target and header-section limits above, since the head is
  // exactly their concatenation (plus the small, fixed-size method/version
  // tokens, negligible against these budgets).
  static constexpr std::size_t kMaxRequestHeadSize =
      kDefaultMaxUriLength + kDefaultMaxHeaderSectionSize;
  // Size, in octets, of the decoder's internal accumulation buffer. Bounds
  // how many bytes of a not-yet-fully-received request (request-line,
  // headers and, when applicable, body framing) the decoder holds in memory
  // at once (decoder.h). Must be able to hold a full request head (see
  // kMaxRequestHeadSize above), since the decoder buffers it whole before
  // mounting the request object.
  static constexpr std::size_t kDecodingBufferSize = kMaxRequestHeadSize;
  // Size, in octets, of response's internal in-memory buffer, holding the
  // status line, headers and (when small enough) the body (response.h).
  static constexpr std::size_t kMaxResponseSizeInMemory = 4096;
  // Maximum size, in octets, of a response body kept in the in-memory region
  // before it is spilled into a streaming body::body_writer (response.h).
  static constexpr std::size_t kMaxResponseBodySizeInMemory = 2048;
  // Maximum size, in octets, of a single chunk-extension (the optional
  // ";name=value" segment following a chunk-size) accepted while decoding a
  // chunked body, before the chunk is rejected as malformed (RFC 9112
  // S7.1.1). Shared by body::framer_chunked and body::reader_chunked.
  static constexpr std::size_t kMaxChunkedExtensionSize = 1024;
  // Maximum accepted size, in octets, of the trailer section following the
  // last chunk of a chunked body, before the body is rejected (RFC 9112
  // S7.1.2). Shared by body::framer_chunked and body::reader_chunked. A
  // chunked trailer is itself a header-field section (RFC 9112 S7.1.2), so
  // it reuses the same operational budget as the request header section.
  static constexpr std::size_t kMaxChunkedTrailerSize =
      kDefaultMaxHeaderSectionSize;
  // Maximum number of query parameters accepted in a request-target's query
  // component before the request is rejected (decoder.h).
  static constexpr std::size_t kMaxQueryParameters = 128;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
