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

#ifndef martianlabs_doba_protocol_http_v11_contracts_h
#define martianlabs_doba_protocol_http_v11_contracts_h

#include <concepts>
#include <utility>

#include "protocol/contracts.h"
#include "transport/server/contracts.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] make_engine_factory                                      ( function ) |
// +---------------------------------------------------------------------------+
// | Creates a factory function for the engine.                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty, typename ROty>
auto make_engine_factory(typename ENty::policies_type configuration,
                         const ROty& routes) {
  return [configuration = std::move(configuration), &routes]() -> ENty {
    return ENty{configuration, routes};
  };
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine_factory                                               ( type ) |
// +---------------------------------------------------------------------------+
// | Type of the engine factory function.                                      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty, typename ROty>
using engine_factory = decltype(make_engine_factory<ENty>(
    std::declval<typename ENty::policies_type>(), std::declval<const ROty&>()));

namespace contracts {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] server                                                      (concept) |
// +---------------------------------------------------------------------------+
// | HTTP/1.1 server composition contract.                                     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename ROty, typename ENty,
          template <typename, typename> class TRty>
concept server =
    std::default_initializable<ROty> &&
    std::constructible_from<ENty, const typename ENty::policies_type&,
                            const ROty&> &&
    transport::server::contracts::transport<
        TRty<ENty, engine_factory<ENty, ROty>>, ENty,
        engine_factory<ENty, ROty>>;
}  // namespace contracts
}  // namespace martianlabs::doba::protocol::http::v11

#endif
