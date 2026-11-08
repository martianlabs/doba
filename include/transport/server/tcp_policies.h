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

#ifndef martianlabs_doba_transport_server_tcp_policies_h
#define martianlabs_doba_transport_server_tcp_policies_h

#include <cstddef>
#include <string>

namespace martianlabs::doba::transport::server {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] tcp_policies                                               ( struct ) |
// +---------------------------------------------------------------------------+
// | TCP limits and listener settings.                                         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct tcp_policies {
  // Default socket RX block size in bytes.
  static constexpr std::size_t kDefaultReceiveBufferCapacity = 4 * 1024;
  // Positive RX block size, separate from message limits. Cap it at SSIZE_MAX
  // on Linux or ULONG_MAX on Windows. The engine keeps unfinished messages
  // after input returns.
  std::size_t receive_buffer_capacity = kDefaultReceiveBufferCapacity;
  // 0 workers means one on Linux, hardware concurrency on Windows, at least
  // one. Windows counts must fit DWORD; startup still needs enough resources.
  std::size_t worker_count = 0;
  // Nonnegative listen backlog; 0 passes SOMAXCONN. The kernel may cap it. This
  // isn't the live-connection limit.
  int listen_backlog = 0;
  // Positive TX byte cap per connection, without preallocation. Count the whole
  // active batch, partial sends included, plus buffered inline bytes. Exclude
  // allocator capacity and reader backing storage. Linux caps at SSIZE_MAX;
  // Windows sends ULONG-sized pieces. Exceeding the budget closes the
  // connection. One terminal reader only.
  std::size_t max_buffered_send_bytes = 1024 * 1024;
  // Live-context cap; 0 means unlimited. Linux splits a finite budget across
  // workers, so give each one a slot. Windows holds shared credits through
  // pending I/O and context release.
  std::size_t max_connections = 65535;
  // Bind to numeric IPv4 at startup; no hostnames or NUL bytes.
  std::string ip;
  // Use a decimal port from 1 to 65535, with no NUL or trailing bytes.
  std::string port;
};
}  // namespace martianlabs::doba::transport::server

#endif
