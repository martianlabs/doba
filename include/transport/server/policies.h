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

#ifndef martianlabs_doba_transport_server_policies_h
#define martianlabs_doba_transport_server_policies_h

#include <cstddef>
#include <string>

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] policies                                                   ( struct ) |
// +---------------------------------------------------------------------------+
// | TCP/IP operating policies.                                                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct policies {
  // Receive capacity per connection, in bytes (must be positive).
  // Engines retain incomplete cores here; bodies may span multiple receives.
  // A full buffer with no consumption closes the connection.
  std::size_t recv_buffer_size = 8192;
  // Number of workers (0 selects automatically).
  std::size_t worker_count = 0;
  // Requested pending connection queue size (0 uses the system default).
  int listen_backlog = 0;
  // Retained send bytes per connection, including the entire active block.
  // Must be positive; exceeding it closes the connection. No preallocation.
  std::size_t send_buffer_size = 1024 * 1024;
  // IPv4 bind address and port (1-65535); both must be set.
  std::string ip;
  std::string port;
};
}  // namespace martianlabs::doba::transport::server

#endif
