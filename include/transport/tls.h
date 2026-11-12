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

#ifndef martianlabs_doba_transport_tls_h
#define martianlabs_doba_transport_tls_h

#include "transport/tcp.h"
#include "transport/tls_engine.h"
#include "transport/tls_factory.h"
#include "transport/tls_policies.h"

namespace martianlabs::doba::transport {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls                                                         ( class ) |
// +---------------------------------------------------------------------------+
// | Runs the protocol engine over TLS and TCP.                                |
// | Requires the martianlabs::doba_tls CMake target.                          |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   ENty - engine type                                                      |
// |   FAty - engine factory type                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
class tls
    : public tcp<detail::tls_engine<ENty>, detail::tls_factory<ENty, FAty>> {
  // +=========================================================================+
  // | [>] USINGs                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  using base = tcp<detail::tls_engine<ENty>, detail::tls_factory<ENty, FAty>>;

 public:
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = tls_policies;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  tls(tls_policies configuration, FAty factory)
      : tls(std::make_shared<const detail::tls_configuration>(
                std::move(configuration)),
            std::move(factory)) {}

 private:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                ( private ) |
  // +-------------------------------------------------------------------------+
  tls(std::shared_ptr<const detail::tls_configuration> configuration,
      FAty factory)
      : base(tcp_configuration(configuration->policies),
             {configuration, std::move(factory)}) {}
  // +=========================================================================+
  // | [>] tcp_configuration                                       ( private ) |
  // +-------------------------------------------------------------------------+
  // | Uses TLS ciphertext limits for TCP receive and send buffers.            |
  // +-------------------------------------------------------------------------+
  static tcp_policies tcp_configuration(const tls_policies& policies) {
    auto result = static_cast<const tcp_policies&>(policies);
    result.receive_buffer_capacity = policies.encrypted_receive_buffer_capacity;
    result.max_buffered_send_bytes = policies.max_buffered_ciphertext_bytes;
    return result;
  }
};
}  // namespace martianlabs::doba::transport

#endif
