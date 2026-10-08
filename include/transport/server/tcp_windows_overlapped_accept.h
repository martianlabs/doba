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

#ifndef martianlabs_doba_transport_server_tcp_windows_overlapped_accept_h
#define martianlabs_doba_transport_server_tcp_windows_overlapped_accept_h

#include "platform.h"
#include "transport/server/tcp_windows_overlapped_base.h"

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] CONSTANTs                                                  ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
static constexpr DWORD kAcceptAddressBytes =
    static_cast<DWORD>(sizeof(sockaddr_storage) + 16);

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] overlapped_accept                                          ( struct ) |
// +---------------------------------------------------------------------------+
// | Windows overlapped accept operation.                                      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct overlapped_accept : overlapped_base {
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  overlapped_accept(SOCKET in_socket)
      : overlapped_base(io_type::kAccept), socket{in_socket} {}
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  SOCKET socket{INVALID_SOCKET};
  CHAR addresses[(kAcceptAddressBytes * 2)]{0};
};
}  // namespace martianlabs::doba::transport::server

#endif
