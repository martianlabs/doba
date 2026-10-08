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

#include <concepts>
#include <functional>
#include <utility>

#include "protocol/contracts.h"

namespace martianlabs::doba::transport::server::contracts {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] transport                                                 ( concept ) |
// +---------------------------------------------------------------------------+
// | This concept defines the requirements for a transport type. It requires   |
// | that the transport type is constructible from its policies and an engine  |
// | factory, and that it provides methods for setting callbacks for           |
// | connection and disconnection events, as well as starting and              |
// | stopping the transport.                                                   |
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
