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
// Copyright 2026 martianLabs
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

#include <winsock2.h>

#include <stdexcept>
#include <string_view>

#include "test_helper.h"

namespace {
int startup_result = 0;
unsigned startups = 0, cleanups = 0;
bool valid_arguments = true;

// +---------------------------------------------------------------------------+
// | [>] startup                                                  ( function ) |
// +---------------------------------------------------------------------------+
int WSAAPI startup(WORD version, LPWSADATA data) {
  ++startups;
  valid_arguments &= version == MAKEWORD(2, 2) && data != nullptr;
  if (data) *data = {};
  return startup_result;
}

// +---------------------------------------------------------------------------+
// | [>] cleanup                                                  ( function ) |
// +---------------------------------------------------------------------------+
int WSAAPI cleanup() {
  ++cleanups;
  return 0;
}
}  // namespace

// Keep substituted and real native definitions in separate executables.
#define WSAStartup startup
#define WSACleanup cleanup
#include "network/environment_windows.h"
#undef WSAStartup
#undef WSACleanup

// +===========================================================================+
// | [>] startup failure never releases an unacquired reference  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Winsock startup failures reject exactly and permit later recovery") {
  using martianlabs::doba::network::detail::environment;
  for (const int error : {WSASYSNOTREADY, WSAVERNOTSUPPORTED, WSAEPROCLIM}) {
    startups = cleanups = 0;
    startup_result = error;
    valid_arguments = true;
    bool rejected = false;
    try {
      environment current;
    } catch (const std::runtime_error& reason) {
      rejected = std::string_view(reason.what()) ==
                 "Could not initialize network environment!";
    }
    DOBA_EXPECT(rejected);
    DOBA_EXPECT(valid_arguments);
    DOBA_EXPECT_EQUAL(startups, 1U);
    DOBA_EXPECT_EQUAL(cleanups, 0U);
    startup_result = 0;
    { environment recovered; }
    DOBA_EXPECT_EQUAL(startups, 2U);
    DOBA_EXPECT_EQUAL(cleanups, 1U);
  }
}

// +===========================================================================+
// | [>] nested owners release exactly their acquired references ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("Winsock nested owners release one reference per successful owner") {
  using martianlabs::doba::network::detail::environment;
  startups = cleanups = 0;
  startup_result = 0;
  valid_arguments = true;
  {
    environment first;
    {
      environment second;
      DOBA_EXPECT_EQUAL(startups, 2U);
      DOBA_EXPECT_EQUAL(cleanups, 0U);
    }
    DOBA_EXPECT_EQUAL(cleanups, 1U);
  }
  DOBA_EXPECT(valid_arguments);
  DOBA_EXPECT_EQUAL(cleanups, 2U);
}
