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

#ifndef martianlabs_doba_protocol_http_v11_body_writer_raw_h
#define martianlabs_doba_protocol_http_v11_body_writer_raw_h

#include <span>
#include <string_view>

#include "common/writer.h"

namespace martianlabs::doba::protocol::http::v11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] writer_raw                                                  ( class ) |
// +---------------------------------------------------------------------------+
// | Writes payload bytes unchanged, without adding body framing.              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class writer_raw {
 public:
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  static bool write(std::span<const std::byte> payload, common::writer& dst) {
    return dst.write(payload);
  }
  // +=========================================================================+
  // | [>] write                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  static bool write(std::string_view payload, common::writer& dst) {
    return dst.write(payload);
  }
  // +=========================================================================+
  // | [>] end                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  // | Raw bodies need no terminator.                                          |
  // +-------------------------------------------------------------------------+
  static bool end(common::writer&) { return true; }
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
