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

#ifndef martianlabs_doba_contracts_h
#define martianlabs_doba_contracts_h

#include <concepts>

#include "protocol/http/v11/contracts.h"
#include "transport/server/contracts.h"

namespace martianlabs::doba::contracts {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] http_server                                                 (concept) |
// +---------------------------------------------------------------------------+
// | HTTP/1.1 server composition contract.                                     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, typename ROty, typename ENty,
          template <typename, typename> class TRty>
concept http_server =
    protocol::http::v11::contracts::request<RQty> &&
    protocol::http::v11::contracts::response<RSty> &&
    protocol::http::v11::contracts::router<ROty, RQty, RSty> &&
    std::constructible_from<ENty, const typename ENty::policies_type&,
                            const ROty&> &&
    transport::server::contracts::transport<
        TRty<ENty, protocol::http::v11::detail::engine_factory<ENty, ROty>>, ENty,
        protocol::http::v11::detail::engine_factory<ENty, ROty>>;
}  // namespace martianlabs::doba::contracts

#endif
