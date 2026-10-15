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

#ifndef martianlabs_doba_protocol_http_v11_policies_h
#define martianlabs_doba_protocol_http_v11_policies_h

#include <cstddef>

#include "common/byte_storage.h"
#include "transport/server/tcp_policies.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] policies                                                   ( struct ) |
// +---------------------------------------------------------------------------+
// | HTTP/1.1 resource limits, storage capacities and feature switches.        |
// | The defaults below preserve the standard server behavior.                 |
// | A limit of 0 means unlimited unless noted.                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct policies {
  // +=========================================================================+
  // | [>] CONSTANTs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // Default maximum query parameter pairs per request.
  static constexpr std::size_t kDefaultMaxQueryParameters = 128;
  // Default maximum chunk extension bytes per chunk.
  static constexpr std::size_t kDefaultMaxChunkExtensionSize = 1024;
  // Default maximum trailer section bytes per chunked body.
  static constexpr std::size_t kDefaultMaxTrailerSectionSize = 4096;
  // Request head bytes retained by the decoder.
  static constexpr std::size_t kDefaultRequestHeadBufferCapacity = 4 * 1024;
  // Request body wire bytes retained before spilling to disk.
  static constexpr std::size_t kDefaultRequestBodyInlineCapacity = 16 * 1024;
  // Response head bytes in the decoder's reusable storage.
  static constexpr std::size_t kDefaultResponseHeadBufferCapacity = 4 * 1024;
  // Response body bytes kept inline before using a writer.
  static constexpr std::size_t kDefaultResponseBodyInlineCapacity = 16 * 1024;
  // Maximum accepted body size, in octets (0 means unlimited).
  std::size_t max_request_body_size = 16 * 1024 * 1024;
  // Maximum number of forwarding hops accepted across Via / Forwarded /
  // X-Forwarded-For (0 means unlimited).
  std::size_t max_forwarding_hops = 32;
  // Maximum number of transfer-codings accepted in Transfer-Encoding
  // (0 means unlimited).
  std::size_t max_transfer_codings = 4;
  // Whether the server allows requests carrying a chunked Transfer-Encoding.
  bool allow_chunked = true;
  // Whether the server allows protocol upgrades offered via Upgrade.
  //
  // NOTE: no protocol upgrade is ever completed. This flag only decides
  // whether an offer is rejected or ignored: true accepts the request and
  // serves it over HTTP/1.1, discarding the offer (RFC 9110 S7.8 allows a
  // server to ignore Upgrade); false rejects the request outright. Honouring
  // an upgrade is deferred to a future release.
  bool allow_upgrade = true;
  // Maximum requests awaiting ordered response delivery (0 means unlimited).
  std::size_t max_pending_requests = 32;
  // Maximum query parameter pairs (0 means unlimited).
  std::size_t max_query_parameters = kDefaultMaxQueryParameters;
  // Maximum chunk extension bytes (0 means unlimited).
  std::size_t max_chunk_extension_size = kDefaultMaxChunkExtensionSize;
  // Maximum trailer section bytes (0 means unlimited).
  std::size_t max_trailer_section_size = kDefaultMaxTrailerSectionSize;
  // Request head buffer capacity and hard limit (must be positive).
  std::size_t request_head_buffer_capacity = kDefaultRequestHeadBufferCapacity;
  // Request body buffer capacity (0 spills immediately).
  std::size_t request_body_inline_capacity = kDefaultRequestBodyInlineCapacity;
  // Response head buffer capacity and hard limit (must be positive).
  std::size_t response_head_buffer_capacity =
      kDefaultResponseHeadBufferCapacity;
  // Response body buffer capacity (0 disables inline storage).
  std::size_t response_body_inline_capacity =
      kDefaultResponseBodyInlineCapacity;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
