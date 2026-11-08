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

#ifndef martianlabs_doba_protocol_http_v11_body_writer_chunked_h
#define martianlabs_doba_protocol_http_v11_body_writer_chunked_h

#include <algorithm>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "common/writer.h"

namespace martianlabs::doba::protocol::http::v11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] writer_chunked                                              ( class ) |
// +---------------------------------------------------------------------------+
// | Wrap each nonempty write in one chunk: <hex-size>\r\n<payload>\r\n.       |
// | Ignore empty writes; only end() emits the final 0\r\n\r\n. No trailers    |
// | here.                                                                     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class writer_chunked {
 public:
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Wrap this payload in one chunk and write it to dst.                     |
  // +-------------------------------------------------------------------------+
  bool write(std::span<const std::byte> payload, common::writer& dst) {
    if (ended_) return false;
    if (payload.empty()) return true;
    if (!write_chunk_size(payload.size(), dst)) return false;
    if (!dst.write(payload)) return false;
    return dst.write(std::string_view("\r\n"));
  }
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  bool write(std::string_view payload, common::writer& dst) {
    return write(
        std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(payload.data()), payload.size()),
        dst);
  }
  // +=========================================================================+
  // | [>] end                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  // | Emit the final chunk once, without trailers. Later calls do nothing.    |
  // +-------------------------------------------------------------------------+
  bool end(common::writer& dst) {
    if (ended_) return true;
    if (!dst.write(std::string_view("0\r\n\r\n"))) return false;
    ended_ = true;
    return true;
  }

 private:
  // +=========================================================================+
  // | [>] write_chunk_size                                        ( private ) |
  // +-------------------------------------------------------------------------+
  static bool write_chunk_size(std::size_t size, common::writer& dst) {
    static constexpr char kHexDigits[] = "0123456789abcdef";
    std::string hex;
    do {
      hex.push_back(kHexDigits[size & 0xF]);
      size >>= 4;
    } while (size != 0);
    std::reverse(hex.begin(), hex.end());
    hex.append("\r\n");
    return dst.write(hex);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  bool ended_{false};
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
