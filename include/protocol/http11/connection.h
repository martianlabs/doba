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

#ifndef martianlabs_doba_protocol_http11_connection_h
#define martianlabs_doba_protocol_http11_connection_h

#include <string_view>
#include <vector>

namespace martianlabs::doba::protocol::http11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] connection                                                 ( struct ) |
// +---------------------------------------------------------------------------+
// | The hop-by-hop transport state that the semantic layer derives from a     |
// | request. Framing and connection interpreters/rules read the parsed values |
// | of Connection, Transfer-Encoding, TE, Trailer, and Upgrade and mutate     |
// | this state accordingly. It is deliberately separate from both the         |
// | agnostic deserialization result and the request itself, because it        |
// | outlives a single request (it describes the connection) and is HTTP/1.1-  |
// | specific.                                                                 |
// |                                                                           |
// | The std::string_view members are zero-copy and point back into the        |
// | request's field-value buffer; that buffer MUST outlive this state.        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct connection {
  // Whether the connection is to be kept alive after the current message.
  // HTTP/1.1 defaults to persistent unless "Connection: close" is present.
  bool persistent = true;
  // Whether "Connection: close" was requested for this connection.
  bool close_requested = false;
  // The transfer-coding names of Transfer-Encoding, in order (outermost last).
  std::vector<std::string_view> transfer_codings;
  // Whether the final transfer-coding is "chunked" (message framing is then
  // delimited by the chunked coding rather than by Content-Length).
  bool chunked = false;
  // The transfer-coding names the client is willing to accept (TE), in order.
  std::vector<std::string_view> te_codings;
  // Whether the client signalled acceptance of the "trailers" TE token.
  bool accepts_trailers = false;
  // The header field names announced in the Trailer header, in order.
  std::vector<std::string_view> trailer_names;
  // The protocols offered in the Upgrade header, in order (empty when absent).
  std::vector<std::string_view> upgrade_offer;
  // Every connection-option token named by Connection, in order. These name
  // header fields that are hop-by-hop for this connection.
  std::vector<std::string_view> options;
};
}  // namespace martianlabs::doba::protocol::http11

#endif
