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

#ifndef martianlabs_doba_transport_tcp_policies_h
#define martianlabs_doba_transport_tcp_policies_h

#include <cstddef>
#include <string>

namespace martianlabs::doba::transport {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcp_policies                                               ( struct ) |
// +---------------------------------------------------------------------------+
// | TCP bind address, worker settings and buffer limits.                      |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct tcp_policies {
  // +=========================================================================+
  // | [>] CONSTANTs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  static constexpr std::size_t kDefaultReceiveBufferCapacity = 4 * 1024;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Receive bytes per socket read; positive, separate from message limits.
  // At most SSIZE_MAX on Linux or ULONG_MAX on Windows.
  std::size_t receive_buffer_capacity = kDefaultReceiveBufferCapacity;
  // 0 selects one worker on Linux or hardware concurrency on Windows.
  // Windows always uses at least one, and the count must fit DWORD.
  std::size_t worker_count = 0;
  // Pending connection queue request; nonnegative. 0 uses SOMAXCONN.
  // The OS may lower it. This does not limit live connections.
  int listen_backlog = 0;
  // Buffered send bytes per connection; positive. Does not preallocate.
  // Counts active and pending bytes, not allocated capacity or reader storage.
  // Partial sends count in full. Exceeding the limit closes the connection.
  // At most SSIZE_MAX on Linux; Windows sends ULONG-sized blocks.
  std::size_t max_buffered_send_bytes = 1024 * 1024;
  // Live connection limit; 0 means unlimited.
  // Linux splits the limit between workers; each needs at least one slot.
  // Windows releases slots after pending I/O and context cleanup.
  std::size_t max_connections = 65535;
  // IPv4 address to bind at start. No hostnames or embedded NUL.
  std::string ip;
  // Decimal port (1-65535), required at start; no NUL or trailing data.
  std::string port;
};
}  // namespace martianlabs::doba::transport

#endif
