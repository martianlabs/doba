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
// | Keep HTTP/1.1 hop-by-hop state separate from the request and generic      |
// | decode result. Header interpreters and framing rules fill it from         |
// | Connection, Transfer-Encoding, TE, Trailer and Upgrade.                   |
// |                                                                           |
// | Views borrow the request's field-value buffer; keep that buffer alive.    |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct connection {
  // Keep the connection open after this message? HTTP/1.1 defaults to yes
  // unless Connection says close.
  bool persistent = true;
  // The peer asked for Connection: close.
  bool close_requested = false;
  // Transfer-Encoding names in order, outermost last.
  std::vector<std::string_view> transfer_codings;
  // The final coding is chunked, so chunks determine the body boundary.
  bool chunked = false;
  // Accepted TE codings, in order.
  std::vector<std::string_view> te_codings;
  // The peer accepts TE: trailers.
  bool accepts_trailers = false;
  // Trailer field names, in order.
  std::vector<std::string_view> trailer_names;
  // Offered Upgrade protocols, or none. Parsed but never upgraded; see
  // policies::allow_upgrade.
  std::vector<std::string_view> upgrade_offer;
  // Connection option tokens, in order. They name this connection's hop-by-hop
  // fields.
  std::vector<std::string_view> options;
  // The peer sent Expect: 100-continue and is waiting before sending its body.
  bool expects_continue = false;
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
