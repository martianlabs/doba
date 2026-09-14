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

#include <type_traits>

#include "common/date_server_helpers.h"
#include "test_helper.h"

// +===========================================================================+
// | [>] date helper selects the platform conversion             ( test-case ) |
// +===========================================================================+
DOBA_TEST("date helper selects the platform conversion") {
  using martianlabs::doba::common::gm_time;
#ifdef _WIN32
  static_assert(
      std::is_same_v<decltype(&gm_time), errno_t (*)(tm*, const time_t*)>);
#elif __linux__
  static_assert(
      std::is_same_v<decltype(&gm_time), tm* (*)(tm*, const time_t*)>);
#endif
  const auto conversion = &gm_time;
  DOBA_EXPECT(conversion != nullptr);
}
