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

#include "platform.h"

#include "test_helper.h"

// +===========================================================================+
// | [>] platform exposes its native socket and event types      ( test-case ) |
// +===========================================================================+
DOBA_TEST("platform exposes its native socket and event types") {
#ifdef _WIN32
#ifndef NOMINMAX
#error platform must disable the Windows min and max macros
#endif
#ifndef _WIN32_DCOM
#error platform must enable DCOM declarations
#endif
  static_assert(sizeof(SOCKET) == sizeof(UINT_PTR));
  static_assert(sizeof(OVERLAPPED::Internal) == sizeof(ULONG_PTR));
  static_assert(sizeof(sockaddr_storage) >= sizeof(sockaddr_in6));
#elif __linux__
  static_assert(sizeof(epoll_data_t) >= sizeof(uint64_t));
  static_assert(sizeof(eventfd_t) == sizeof(uint64_t));
  static_assert(sizeof(sockaddr_storage) >= sizeof(sockaddr_in6));
#endif
}
