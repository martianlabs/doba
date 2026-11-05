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

#ifndef martianlabs_doba_protocol_processing_result_h
#define martianlabs_doba_protocol_processing_result_h

#include <chrono>
#include <optional>

namespace martianlabs::doba::protocol {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] processing_result                                          ( struct ) |
// +---------------------------------------------------------------------------+
// | Describes processing turns requested by an engine, independently of I/O.  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct processing_result {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using clock = std::chrono::steady_clock;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Requests a later serialized turn, never recursive execution.
  bool turn_requested{false};
  // Requests a turn at or after this monotonic deadline, if present.
  std::optional<clock::time_point> turn_at{};
  // Requests a later turn once all accepted output, including readers, drains.
  bool turn_after_output_drained{false};
};
}  // namespace martianlabs::doba::protocol

#endif
