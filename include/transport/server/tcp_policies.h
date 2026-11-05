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
// | TCP operating policies.                                                   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct tcp_policies {
  // Default receive block capacity, in bytes.
  static constexpr std::size_t kDefaultReceiveBufferCapacity = 4 * 1024;
  // Positive RX block size, independent of protocol message limits.
  // Linux: at most SSIZE_MAX. Windows: at most ULONG_MAX.
  // The engine owns incomplete messages after the input call returns.
  std::size_t receive_buffer_capacity = kDefaultReceiveBufferCapacity;
  // Worker count. Zero selects one on Linux, hardware concurrency on Windows
  // (at least one). Windows counts must fit DWORD; startup needs resources.
  std::size_t worker_count = 0;
  // Nonnegative listen backlog request. Zero passes SOMAXCONN to the kernel;
  // the kernel can cap it. This is separate from the live connection limit.
  int listen_backlog = 0;
  // Positive logical TX byte budget per connection; no preallocation.
  // Counts the entire active batch plus accumulated inline bytes, including
  // partial sends. Allocator capacity and reader backing storage are excluded.
  // Linux: at most SSIZE_MAX. Windows submits ULONG-sized chunks as needed.
  // Exceeding the budget closes the connection; one terminal reader is fixed.
  std::size_t max_buffered_send_bytes = 1024 * 1024;
  // Maximum live contexts; zero disables admission limiting.
  // Linux partitions a finite budget, which must cover every selected worker.
  // Windows shares reservations through pending I/O and context release.
  std::size_t max_connections = 65535;
  // Numeric IPv4 bind address, required at start. No hostnames or NUL bytes.
  std::string ip;
  // Decimal port in [1, 65535], required at start. No NUL or trailing data.
  std::string port;
};
}  // namespace martianlabs::doba::transport::server

#endif
