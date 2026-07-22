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

#ifndef martianlabs_doba_transport_server_tcpip_h
#define martianlabs_doba_transport_server_tcpip_h

#include <functional>

#include "platform.h"

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] PLATFORM-INDEPENDENT-TYPEs                                 ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
namespace martianlabs::doba::transport::server {
struct types {
  template <typename RSty>
  using on_send_delegate = std::function<void(std::shared_ptr<RSty>)>;
  template <typename RQty, typename RSty>
  using on_request_delegate =
      std::function<void(std::shared_ptr<const RQty>, std::shared_ptr<RSty>,
                         on_send_delegate<RSty>)>;
  template <typename RSty>
  using on_bad_request_delegate =
      std::function<void(std::string_view, std::shared_ptr<RSty>)>;
  using on_client_connected_delegate = std::function<void()>;
  using on_client_disconnected_delegate = std::function<void()>;
};
}  // namespace martianlabs::doba::transport::server

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] PLATFORM-DEPENDENT-INCLUDEs                               ( section ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include "transport/server/tcpip_windows.h"
#elif __linux__
#include "transport/server/tcpip_linux.h"
#endif

#endif
