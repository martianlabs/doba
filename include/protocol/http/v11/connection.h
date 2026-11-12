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

#ifndef martianlabs_doba_protocol_http_v11_connection_h
#define martianlabs_doba_protocol_http_v11_connection_h

#include <string_view>
#include <vector>

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] connection                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// | Connection and body-framing choices read from the request headers.        |
// | Views borrow the request's header buffer; keep it alive while using them. |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct connection {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Keep the connection open after this message unless told otherwise.
  bool persistent = true;
  // The request included Connection: close.
  bool close_requested = false;
  // Transfer-Encoding names in order; the last coding wraps the others.
  std::vector<std::string_view> transfer_codings;
  // The final coding is chunked, so chunks mark the body boundaries.
  bool chunked = false;
  // Codings the client accepts through TE, in order.
  std::vector<std::string_view> te_codings;
  // TE includes trailers.
  bool accepts_trailers = false;
  // Header names announced by Trailer, in order.
  std::vector<std::string_view> trailer_names;
  // Upgrade offers in order. Checked, but no protocol switch is performed.
  std::vector<std::string_view> upgrade_offer;
  // Connection tokens in order, including close and nominated header names.
  std::vector<std::string_view> options;
  // The client sent Expect: 100-continue.
  bool expects_continue = false;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
