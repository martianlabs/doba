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

#ifndef martianlabs_doba_protocol_http_v11_context_h
#define martianlabs_doba_protocol_http_v11_context_h

#include <cstddef>

#include "protocol/http/v11/connection.h"
#include "protocol/http/v11/parsed_types.h"
#include "protocol/http/v11/policies.h"
#include "protocol/http/v11/rejection_reason.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] context                                                    ( struct ) |
// +---------------------------------------------------------------------------+
// | Give cross-header rules the whole picture: presence flags, parsed values, |
// | connection state and policy. deserialize() fills this in its one header   |
// | pass, then applies the rules.                                             |
// |                                                                           |
// | Keep mutable hop-by-hop state in connection and read-only settings in     |
// | policies. Other flags and values are only for comparisons across headers. |
// |                                                                           |
// | Views borrow the request's field-value buffer; keep it alive through this |
// | context.                                                                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct context {
  // Inbound policy settings, not values from the request.
  struct policies policies;
  // Hop-by-hop state derived from the request.
  v11::connection connection;
  // Track Content-Length and Transfer-Encoding presence, duplicate lengths and
  // the parsed length for cross-header checks.
  bool has_content_length = false;
  bool multiple_content_length = false;
  std::size_t content_length = 0;
  bool has_transfer_encoding = false;
  // Track Host and Target-Authority presence, Host count and parsed values for
  // cross-header checks.
  bool has_host = false;
  bool multiple_host = false;
  parsed_host_port host;
  bool has_target_authority = false;
  parsed_host_port target_authority;
  // Forwarding hop counts from Via, Forwarded and X-Forwarded-For.
  std::size_t forwarding_hops = 0;
  // Why a semantic check rejected the request, so HTTP can pick a status. kNone
  // leaves syntax errors at 400 Bad Request.
  martianlabs::doba::protocol::http::v11::rejection_reason rejection_reason =
      martianlabs::doba::protocol::http::v11::rejection_reason::kNone;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
