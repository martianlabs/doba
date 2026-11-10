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
// | Send head, body, then the optional reader.                                |
// | TCP copies the bytes before returning and takes the reader.               |
// | Call only during an engine turn.                                          |
// | After a reader, do not send more bytes.                                   |
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
// | Run one input or processing call at a time per engine.                    |
// | Input is read-only and expires when the call returns. Copy what you keep. |
// | TCP consumes all input; the result only asks for more processing.         |
// | Call send/close only on the thread running the current turn.              |
// | output_idle says whether output was idle when the turn began.             |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ENty>
concept engine = requires(ENty& engine, std::string_view input,
                          protocol::send_delegate output,
                          std::function<void()> close, bool output_idle) {
  typename ENty::policies_type;
  { engine.set_on_send(std::move(output)) } -> std::same_as<void>;
  { engine.set_on_close(std::move(close)) } -> std::same_as<void>;
  {
    engine.on_bytes_received(input)
  } -> std::same_as<protocol::processing_result>;
  {
    engine.on_processing_turn(output_idle)
  } -> std::same_as<protocol::processing_result>;
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine_factory                                              (concept) |
// +---------------------------------------------------------------------------+
// | Create a separate engine for each connection.                             |
// | Workers can call it at the same time. Keep shared data safe.              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename FNty, typename ENty>
concept engine_factory = engine<ENty> && std::move_constructible<FNty> &&
                         requires(const FNty& create_engine) {
                           { create_engine() } -> std::same_as<ENty>;
                         };
}  // namespace martianlabs::doba::protocol::contracts

#endif
