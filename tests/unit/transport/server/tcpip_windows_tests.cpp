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

#if defined(_WIN32)
#include "transport/server/tcpip.h"
#include "transport/server/tcpip_windows.h"
#include "test_helper.h"
using namespace martianlabs::doba::transport::server;

// +===========================================================================+
// | [>] overlapped accept initializes its native operation      ( test-case ) |
// +===========================================================================+
DOBA_TEST("overlapped accept initializes its native operation") {
  overlapped_accept value{INVALID_SOCKET};
  DOBA_EXPECT_EQUAL(value.get_type(), io_type::kAccept);
  DOBA_EXPECT_EQUAL(value.socket, INVALID_SOCKET);
  DOBA_EXPECT_EQUAL(value.Internal, 0);
  DOBA_EXPECT_EQUAL(value.InternalHigh, 0);
  DOBA_EXPECT_EQUAL(value.Offset, 0);
  DOBA_EXPECT_EQUAL(value.OffsetHigh, 0);
  DOBA_EXPECT(value.hEvent == nullptr);
  for (char byte : value.addresses) DOBA_EXPECT_EQUAL(byte, 0);
}

#endif
