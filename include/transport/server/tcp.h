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

#ifndef martianlabs_doba_transport_server_tcp_h
#define martianlabs_doba_transport_server_tcp_h

#include <functional>
#include <utility>

#include "platform.h"
#include "transport/server/contracts.h"
#include "transport/server/tcp_connection.h"
#include "transport/server/tcp_policies.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] types                                                      ( struct ) |
// +---------------------------------------------------------------------------+
// | Platform-independent transport delegate types.                            |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct types {
  // On client connected delegate type
  using on_client_connected_delegate = std::function<void()>;
  // On client disconnected delegate type
  using on_client_disconnected_delegate = std::function<void()>;
};
} // namespace martianlabs::doba::transport::server

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] PLATFORM-DEPENDENT-INCLUDEs                               ( section ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include "transport/server/tcp_windows_transport.h"
#elif __linux__
#include "transport/server/tcp_linux_transport.h"
#endif

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcp                                                         ( class ) |
// +---------------------------------------------------------------------------+
// | This class provides a TCP transport for an engine. It inherits from       |
// | `basic_transport` and takes a `tcp_policies` configuration and an engine  |
// | factory when it is created.                                               |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   ENty - engine type being used.                                          |
// |   FAty - engine factory type being used.                                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
class tcp
    : public basic_transport<ENty, FAty, tcp_connection<ENty>, tcp_policies> {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit tcp(tcp_policies configuration, FAty create_engine)
      : basic_transport<ENty, FAty, tcp_connection<ENty>, tcp_policies>(
            std::move(configuration), std::move(create_engine), nullptr) {}
};
}  // namespace martianlabs::doba::transport::server

#endif
