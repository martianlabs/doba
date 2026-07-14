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


#ifndef martianlabs_doba_protocol_http11_policies_h
#define martianlabs_doba_protocol_http11_policies_h

#include <cstddef>
#include <cstdint>

namespace martianlabs::doba::protocol::http11 {
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
// | The defaults are permissive placeholders; a server tightens them at       |
// | configuration time. A limit of 0 means "unlimited" unless noted.          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct policies {
  // Maximum accepted Content-Length, in octets (0 means unlimited).
  std::size_t max_content_length = 0;
  // Maximum number of forwarding hops accepted across Via / Forwarded /
  // X-Forwarded-For (0 means unlimited).
  std::size_t max_forwarding_hops = 0;
  // Maximum number of transfer-codings accepted in Transfer-Encoding
  // (0 means unlimited).
  std::size_t max_transfer_codings = 0;
  // Whether the server allows requests carrying a chunked Transfer-Encoding.
  bool allow_chunked = true;
  // Whether the server allows protocol upgrades offered via Upgrade.
  bool allow_upgrade = true;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
