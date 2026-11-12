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

#ifndef martianlabs_doba_protocol_http_v11_policies_h
#define martianlabs_doba_protocol_http_v11_policies_h

#include <cstddef>

#include "common/byte_storage.h"
#include "transport/tcp_policies.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] policies                                                   ( struct ) |
// +---------------------------------------------------------------------------+
// | HTTP/1.1 limits, memory thresholds and optional features.                 |
// | Limits use 0 for unlimited unless noted below.                            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct policies {
  // +=========================================================================+
  // | [>] CONSTANTs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr std::size_t kDefaultMaxQueryParameters = 128;
  static constexpr std::size_t kDefaultMaxChunkExtensionSize = 1024;
  static constexpr std::size_t kDefaultMaxTrailerSectionSize = 4096;
  static constexpr std::size_t kDefaultMaxRequestHeadSize = 4 * 1024;
  static constexpr std::size_t kDefaultRequestBodyMemoryThreshold = 16 * 1024;
  static constexpr std::size_t kDefaultMaxResponseHeadSize = 4 * 1024;
  static constexpr std::size_t kDefaultResponseBodyMemoryThreshold = 16 * 1024;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Request body limit, in bytes.
  std::size_t max_request_body_size = 16 * 1024 * 1024;
  // Total hops across Via, Forwarded and X-Forwarded-For.
  std::size_t max_forwarding_hops = 32;
  // Number of codings allowed in Transfer-Encoding.
  std::size_t max_transfer_codings = 4;
  // Accept chunked request bodies.
  bool allow_chunked = true;
  // Ignore valid Upgrade offers when true; reject them when false.
  // Requests still use HTTP/1.1.
  bool allow_upgrade = true;
  // Queued requests and responses; must be positive.
  std::size_t max_pending_requests = 128;
  // Queued response head and inline body bytes; must be positive.
  std::size_t max_pending_response_bytes = 1024 * 1024;
  // Work budget per engine turn; must be positive.
  std::size_t max_task_steps_per_turn = 64;
  // Input bytes waiting for processing capacity; must be positive.
  std::size_t max_deferred_input_size = 1024 * 1024;
  // Query parameter pairs per request.
  std::size_t max_query_parameters = kDefaultMaxQueryParameters;
  // Extension bytes per chunk.
  std::size_t max_chunk_extension_size = kDefaultMaxChunkExtensionSize;
  // Trailer section bytes per chunked body.
  std::size_t max_trailer_section_size = kDefaultMaxTrailerSectionSize;
  // Request head bytes; must be positive.
  std::size_t max_request_head_size = kDefaultMaxRequestHeadSize;
  // Body bytes kept as received before spilling to disk; 0 spills immediately.
  std::size_t request_body_memory_threshold =
      kDefaultRequestBodyMemoryThreshold;
  // Response head bytes; must be positive.
  std::size_t max_response_head_size = kDefaultMaxResponseHeadSize;
  // Response body bytes kept inline; 0 disables inline storage.
  std::size_t response_body_memory_threshold =
      kDefaultResponseBodyMemoryThreshold;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
