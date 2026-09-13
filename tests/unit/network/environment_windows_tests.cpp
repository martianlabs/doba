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

#ifdef _WIN32

#include <type_traits>

#include "platform.h"
#include "network/environment_windows.h"
#include "test_helper.h"

// +===========================================================================+
// | [>] environment retains exclusive lifecycle ownership       ( test-case ) |
// +===========================================================================+
DOBA_TEST("environment retains exclusive lifecycle ownership") {
  using martianlabs::doba::network::detail::environment;
  static_assert(std::is_default_constructible_v<environment>);
  static_assert(std::is_nothrow_destructible_v<environment>);
  static_assert(!std::is_copy_constructible_v<environment>);
  static_assert(!std::is_copy_assignable_v<environment>);
  static_assert(!std::is_move_constructible_v<environment>);
  static_assert(!std::is_move_assignable_v<environment>);
}

#endif
