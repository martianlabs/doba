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

#include "common/byte_storage_helpers.h"
#include "test_helper.h"

// +===========================================================================+
// | [>] byte_storage_helpers selects its platform               ( test-case ) |
// +===========================================================================+
DOBA_TEST("byte_storage_helpers selects the platform implementation") {
#ifdef _WIN32
#ifndef martianlabs_doba_common_byte_storage_helpers_windows_h
#error Windows implementation was not selected
#endif
#elif __linux__
#ifndef martianlabs_doba_common_byte_storage_helpers_linux_h
#error Linux implementation was not selected
#endif
#endif
  static_assert(std::is_class_v<martianlabs::doba::common::byte_storage_file>);
}
