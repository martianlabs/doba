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

#ifndef martianlabs_doba_protocol_http_v11_body_writer_state_h
#define martianlabs_doba_protocol_http_v11_body_writer_state_h

#include <cstddef>

#include "protocol/http/v11/body/writer_error.h"

namespace martianlabs::doba::protocol::http::v11::body {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] writer_state                                               ( struct ) |
// +---------------------------------------------------------------------------+
// | State declared for body writing.                                          |
// | Current writers return bool and do not use this type.                     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct writer_state {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Input payload bytes consumed.
  std::size_t consumed = 0;
  // The body is finished.
  bool complete = false;
  // Writing failed.
  bool has_error = false;
  // Only meaningful when has_error is true.
  writer_error error = writer_error::none;
};
}  // namespace martianlabs::doba::protocol::http::v11::body

#endif
