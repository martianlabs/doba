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

#ifdef __linux__

#include <type_traits>

#include "common/signaler_linux.h"
#include "test_helper.h"

// +===========================================================================+
// | [>] signaler exposes a static nonconstructible wait         ( test-case ) |
// +===========================================================================+
DOBA_TEST("signaler exposes a static nonconstructible wait") {
  using martianlabs::doba::common::signaler;
  static_assert(!std::is_default_constructible_v<signaler>);
  static_assert(std::is_same_v<decltype(&signaler::wait), void (*)()>);
}

#endif
