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
// | Collects parsed header values so rules can check them together.           |
// | Views borrow the request's header buffer; keep it alive while using them. |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct context {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Inbound configuration used to check the request.
  struct policies policies;
  // Connection choices read from the request headers.
  v11::connection connection;
  // Body length and header flags used to check message framing.
  bool has_content_length = false;
  bool multiple_content_length = false;
  std::size_t content_length = 0;
  bool has_transfer_encoding = false;
  // Host and request-target authority, kept for routing checks.
  bool has_host = false;
  bool multiple_host = false;
  parsed_host_port host;
  bool has_target_authority = false;
  parsed_host_port target_authority;
  // Forwarding hops counted across Via, Forwarded and X-Forwarded-For.
  std::size_t forwarding_hops = 0;
  // Rejection reason used to pick an HTTP status.
  // Syntax errors leave kNone and default to 400 Bad Request.
  martianlabs::doba::protocol::http::v11::rejection_reason rejection_reason =
      martianlabs::doba::protocol::http::v11::rejection_reason::kNone;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
