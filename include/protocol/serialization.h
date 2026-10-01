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

#ifndef martianlabs_doba_protocol_serialization_h
#define martianlabs_doba_protocol_serialization_h

#include <cstddef>
#include <memory>
#include <string_view>

#include "common/reader.h"

namespace martianlabs::doba::protocol {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] serialization_result                                       ( struct ) |
// +---------------------------------------------------------------------------+
// | Borrows head and body bytes and owns an optional source.                  |
// | The transport copies both views before the call returns.                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct serialization_result {
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +=========================================================================+
  std::string_view head;
  std::string_view body;
  std::unique_ptr<common::reader> source;
};
}  // namespace martianlabs::doba::protocol

#endif
