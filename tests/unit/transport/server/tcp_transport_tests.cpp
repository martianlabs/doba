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

#include <cstddef>
#include <functional>
#include <limits>
#include <stdexcept>

#include "protocol/send_delegate.h"
#include "test_helper.h"
#include "transport/server/tcp.h"

namespace {
namespace tr = martianlabs::doba::transport::server;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] policy_engine                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct policy_engine {
  using policies_type = int;
  void set_on_send(martianlabs::doba::protocol::send_delegate) {}
  void set_on_close(std::function<void()>) {}
  std::size_t on_bytes_received(const char*, std::size_t size,
                                std::size_t) {
    return size;
  }
};
}  // namespace

// +===========================================================================+
// | [>] tcp transport rejects invalid buffer policies          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("tcp transport rejects invalid buffer policies") {
  auto factory = []() { return policy_engine{}; };
  auto rejects = [&](tr::tcp_policies configuration) {
    try {
      tr::tcp<policy_engine, decltype(factory)> transport(configuration,
                                                         factory);
    } catch (const std::runtime_error&) {
      return true;
    }
    return false;
  };
  tr::tcp_policies configuration;
  configuration.recv_buffer_size = 0;
  DOBA_EXPECT(rejects(configuration));
  configuration.recv_buffer_size = std::numeric_limits<std::size_t>::max();
  DOBA_EXPECT(rejects(configuration));
  configuration.recv_buffer_size = 1;
  configuration.max_send_buffer_size = 0;
  DOBA_EXPECT(rejects(configuration));
  configuration.max_send_buffer_size = 1;
  configuration.listen_backlog = -1;
  DOBA_EXPECT(rejects(configuration));
#ifdef _WIN32
  configuration.listen_backlog = 0;
  configuration.worker_count = std::numeric_limits<std::size_t>::max();
  DOBA_EXPECT(rejects(configuration));
#endif
}
