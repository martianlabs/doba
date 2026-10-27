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

#ifndef martianlabs_doba_transport_server_tcp_policies_h
#define martianlabs_doba_transport_server_tcp_policies_h

#include <chrono>
#include <cstddef>
#include <string>

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcp_policies                                               ( struct ) |
// +---------------------------------------------------------------------------+
// | TCP operating policies.                                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct tcp_policies {
  // Default receive buffer capacity, in bytes.
  static constexpr std::size_t kDefaultReceiveBufferCapacity = 4 * 1024;
  // Receive I/O block size per connection, in bytes (must be positive).
  // Engines receive only fresh bytes and own incomplete protocol messages.
  // This capacity is independent of protocol head and body limits.
  std::size_t receive_buffer_capacity = kDefaultReceiveBufferCapacity;
  // Number of workers (0 selects automatically).
  std::size_t worker_count = 0;
  // Requested pending connection queue size (0 uses the system default).
  int listen_backlog = 0;
  // Queued send reservations plus the entire active batch, per connection.
  // Also bounds each accumulated batch; source-owned storage is excluded.
  // Must be positive; exceeding it closes the connection. No preallocation.
  std::size_t max_buffered_send_bytes = 1024 * 1024;
  // Global live context reservations, including unfinished work after closure.
  // 0 disables admission limiting. No contexts are preallocated.
  std::size_t max_connections = 65535;
  // Positive socket I/O renews this period. 0 disables; negative is invalid.
  std::chrono::milliseconds inactivity_period = std::chrono::seconds{60};
  // Retained readers, including the active reader. 0 means unlimited.
  std::size_t max_pending_sources = 32;
  // IPv4 bind address and port (1-65535); both must be set.
  std::string ip;
  std::string port;
};
}  // namespace martianlabs::doba::transport::server

#endif
