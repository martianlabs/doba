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
#include "transport/server/policies.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] policies                                                   ( struct ) |
// +---------------------------------------------------------------------------+
// | The inbound configuration the semantic layer consults when deciding       |
// | whether a syntactically valid message should be served. Where the         |
// | syntactic checkers only answer "is this well-formed?", policy rules turn  |
// | these limits and switches into an accept/reject verdict (for example, a   |
// | Content-Length above max_content_length, or more forwarding hops than     |
// | max_forwarding_hops).                                                     |
// |                                                                           |
// | A limit of 0 means "unlimited" unless noted.                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct policies {
  // +=========================================================================+
  // | [>] CONSTANTs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr std::size_t kMaxQueryParameters = 128;
  static constexpr std::size_t kMaxChunkedExtensionSize = 1024;
  static constexpr std::size_t kMaxChunkedTrailerSize = 4096;
  static constexpr std::size_t kMaxRequestHeadSizeInMemory =
      transport::server::policies::kDefaultRecvBufferSize;
  static constexpr std::size_t kMaxRequestBodySizeInMemory =
      common::byte_storage_options::kDefaultSpillThreshold;
  static constexpr std::size_t kMaxResponseHeadSizeInMemory = 4 * 1024;
  static constexpr std::size_t kMaxResponseBodySizeInMemory =
      common::byte_storage_options::kDefaultSpillThreshold;
  // Maximum accepted body size, in octets (0 means unlimited).
  std::size_t max_content_length = 16 * 1024 * 1024;
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
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
