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

#ifndef martianlabs_doba_transport_server_tls_linux_h
#define martianlabs_doba_transport_server_tls_linux_h

#include <utility>

#include "transport/server/tcp.h"
#include "transport/server/tls_connection.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tls [linux]                                                 ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
class tls
    : public basic_transport<ENty, FAty, tls_connection<ENty>,
                             tls_policies> {
 public:
  explicit tls(tls_policies configuration, FAty create_engine)
      : basic_transport<ENty, FAty, tls_connection<ENty>, tls_policies>(
            configuration, std::move(create_engine),
            make_tls_context(configuration)) {}
};
}  // namespace martianlabs::doba::transport::server

#endif
