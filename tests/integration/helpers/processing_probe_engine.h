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

#ifndef martianlabs_doba_tests_integration_helpers_processing_probe_engine_h
#define martianlabs_doba_tests_integration_helpers_processing_probe_engine_h

#include <atomic>
#include <functional>
#include <memory>
#include <string_view>
#include <utility>

#include "protocol/contracts.h"

namespace martianlabs::doba::tests::integration {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] processing_probe_engine                                    ( struct ) |
// +---------------------------------------------------------------------------+
// | Probe processing-result behavior at the real socket boundary.             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct processing_probe_engine {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(protocol::send_delegate value) {
    output = std::move(value);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> value) {
    close = std::move(value);
  }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  protocol::processing_result on_bytes_received(std::string_view) {
    if (close_after_output) {
      output("ok", {}, nullptr);
      close();
    }
    return progress;
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  protocol::processing_result on_processing_turn(bool) {
    ++*turns;
    return {};
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Fixed scheduling result returned by input.
  protocol::processing_result progress{};
  // When true, send output and ask to close gracefully.
  bool close_after_output{false};
  // Count processing calls separately from context destruction.
  std::shared_ptr<std::atomic<unsigned>> turns;
  // Use transport callbacks only during this engine turn.
  protocol::send_delegate output;
  std::function<void()> close;
};
}  // namespace martianlabs::doba::tests::integration

#endif
