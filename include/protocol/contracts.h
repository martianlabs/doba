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

#ifndef martianlabs_doba_protocol_contracts_h
#define martianlabs_doba_protocol_contracts_h

#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <string_view>
#include <utility>

#include "common/reader.h"
#include "protocol/processing_result.h"

namespace martianlabs::doba::protocol {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] send_delegate                                              ( public ) |
// +---------------------------------------------------------------------------+
// | Sends head, body and an optional source, in that order. Bundled TCP       |
// | copies inline bytes before returning and takes ownership of the reader.   |
// | Invoke only inside a serialized engine input or processing turn.          |
// | A reader is terminal; later nonempty output is a contract violation.      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using send_delegate = std::function<void(std::string_view, std::string_view,
                                         std::unique_ptr<common::reader>)>;
}  // namespace martianlabs::doba::protocol

namespace martianlabs::doba::protocol::contracts {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine                                                      (concept) |
// +---------------------------------------------------------------------------+
// | Input and processing turns never overlap for one engine instance.         |
// | Input bytes are immutable and borrowed only until that call returns;      |
// | they are all discarded by the transport, regardless of the result.        |
// | The engine owns any data it retains. Send/close callbacks run only        |
// | inside these turns and must not escape for cross-thread invocation.       |
// | Results describe processing needs, not RX control or input consumption.   |
// | output_idle is a snapshot taken before the processing turn begins.        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ENty>
concept engine = requires(ENty& engine, std::string_view input,
                          protocol::send_delegate output,
                          std::function<void()> close, bool output_idle) {
  typename ENty::policies_type;
  { engine.set_on_send(std::move(output)) } -> std::same_as<void>;
  { engine.set_on_close(std::move(close)) } -> std::same_as<void>;
  { engine.on_bytes_received(input) } ->
      std::same_as<protocol::processing_result>;
  { engine.on_processing_turn(output_idle) } ->
      std::same_as<protocol::processing_result>;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine_factory                                              (concept) |
// +---------------------------------------------------------------------------+
// | Creates an independent engine per connection. Calls for different         |
// | connections may overlap on different workers; shared state must be safe.  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename FNty, typename ENty>
concept engine_factory = engine<ENty> && std::move_constructible<FNty> &&
                         requires(const FNty& create_engine) {
                           { create_engine() } -> std::same_as<ENty>;
                         };
}  // namespace martianlabs::doba::protocol::contracts

#endif
