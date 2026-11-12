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

#ifndef martianlabs_doba_transport_tls_factory_h
#define martianlabs_doba_transport_tls_factory_h

#include <memory>

#include "protocol/contracts.h"
#include "transport/tls_configuration.h"
#include "transport/tls_engine.h"

namespace martianlabs::doba::transport::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls_factory                                                ( struct ) |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   ENty - protocol engine type                                             |
// |   FAty - engine factory type                                              |
// +---------------------------------------------------------------------------+
// | Creates engines with TLS using the application's factory.                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
struct tls_factory {
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  tls_engine<ENty> operator()() const {
    return tls_engine<ENty>(configuration, factory);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  std::shared_ptr<const tls_configuration> configuration;
  FAty factory;
};
}  // namespace martianlabs::doba::transport::detail

#endif
