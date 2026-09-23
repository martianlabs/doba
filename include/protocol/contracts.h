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

#ifndef martianlabs_doba_protocol_contracts_h
#define martianlabs_doba_protocol_contracts_h

#include <concepts>
#include <cstddef>
#include <functional>
#include <utility>

#include "common/output.h"

namespace martianlabs::doba::protocol::contracts {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine                                                      (concept) |
// +---------------------------------------------------------------------------+
// | Protocol engine contract.                                                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
// Input callbacks are serialized per connection and stop when closing begins.
// The output delegate accepts concurrent submissions from any thread.
// The transport owns and drains submitted output independently of the engine.
template <typename ENty>
concept engine = requires(ENty& engine, const char* buf, std::size_t sze,
                          std::size_t capacity, common::send_delegate output,
                          std::function<void()> close) {
  typename ENty::policies_type;
  { engine.set_on_send(std::move(output)) } -> std::same_as<void>;
  { engine.set_on_close(std::move(close)) } -> std::same_as<void>;
  { engine.on_bytes_received(buf, sze, capacity) } -> std::same_as<std::size_t>;
};
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine_factory                                              (concept) |
// +---------------------------------------------------------------------------+
// | Protocol engine factory contract.                                         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename FNty, typename ENty>
concept engine_factory = engine<ENty> && std::move_constructible<FNty> &&
                         requires(const FNty& create_engine) {
                           { create_engine() } -> std::same_as<ENty>;
                         };
}  // namespace martianlabs::doba::protocol::contracts

#endif
