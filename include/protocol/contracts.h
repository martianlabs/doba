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
// | Send head, body, then the optional reader. TCP copies inline bytes before |
// | returning and takes ownership of the reader. Call only during a           |
// | serialized input or processing turn. A reader ends output: no nonempty    |
// | sends after it.                                                           |
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
// | Give each engine one turn at a time. Input is borrowed, immutable and     |
// | discarded when the call returns, whatever the result; keep your own copy  |
// | if needed. Send/close callbacks stay inside these turns, never on other   |
// | threads. Results ask for processing, not RX control or partial            |
// | consumption. output_idle is the snapshot from before the processing turn. |
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
// | Create one engine per connection. Different workers may call different    |
// | engines at once, so protect shared state.                                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename FNty, typename ENty>
concept engine_factory = engine<ENty> && std::move_constructible<FNty> &&
                         requires(const FNty& create_engine) {
                           { create_engine() } -> std::same_as<ENty>;
                         };
}  // namespace martianlabs::doba::protocol::contracts

#endif
