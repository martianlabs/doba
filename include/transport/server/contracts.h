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

#ifndef martianlabs_doba_transport_server_contracts_h
#define martianlabs_doba_transport_server_contracts_h

#include <chrono>
#include <concepts>
#include <functional>
#include <utility>

#include "protocol/contracts.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] execution_work                                             ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct execution_work {
  void* context{nullptr};
  void (*run)(void*) noexcept{nullptr};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] execution_capacity                                         ( struct ) |
// +---------------------------------------------------------------------------+
// | While active, accepted work runs once, no earlier than its threshold.     |
// | An independent execution service supplies this capability to the engine. |
// | Bundled TCP does not implement or inject an application executor.        |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct execution_capacity {
  void* owner{nullptr};
  void (*submit)(void*, std::chrono::steady_clock::time_point,
                 execution_work){nullptr};
};
}  // namespace martianlabs::doba::transport::server

namespace martianlabs::doba::transport::server::contracts {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] execution_receiver                                      ( concept ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ENty>
concept execution_receiver =
    requires(ENty& engine, execution_capacity execution) {
      { engine.set_execution_capacity(execution) } -> std::same_as<void>;
    };

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] transport                                                 ( concept ) |
// +---------------------------------------------------------------------------+
// | Structural transport interface. Runtime rules for bundled TCP:           |
// | configure and administer from one sequential external owner, outside     |
// | worker callbacks. Notify connections outside transport locks.            |
// | Serialize input per connection and copy accepted inline output before    |
// | its synchronous send callback returns. Transfer reader ownership; allow  |
// | only one terminal reader and keep socket buffers alive during I/O.       |
// | Engine close drains accepted output. Linux administrative stop cancels   |
// | pending output; Windows drains it before joining its IOCP workers.       |
// | TLS and application execution are outside the bundled TCP contract.      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename TRty, typename ENty, typename FNty>
concept transport =
    protocol::contracts::engine_factory<FNty, ENty> &&
    std::constructible_from<TRty, typename TRty::policies_type, FNty> &&
    requires(TRty& tr, std::function<void()> callback) {
      typename TRty::policies_type;
      // Set the callback for when a connection is established
      { tr.set_on_connection(std::move(callback)) } -> std::same_as<void>;
      // Set the callback for when a connection is disconnected
      { tr.set_on_disconnection(std::move(callback)) } -> std::same_as<void>;
      // Start the transport
      { tr.start() } -> std::same_as<void>;
      // Stop the transport
      { tr.stop() } -> std::same_as<void>;
    };
}  // namespace martianlabs::doba::transport::server::contracts

#endif
