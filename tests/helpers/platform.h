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

#ifndef martianlabs_doba_tests_helpers_platform_h
#define martianlabs_doba_tests_helpers_platform_h

// Platform selection belongs here. Helpers include their selected backend
// without introducing operating-system branches in shared test code.
#define DOBA_TEST_PLATFORM_STRING_IMPL(value) #value
#define DOBA_TEST_PLATFORM_STRING(value) DOBA_TEST_PLATFORM_STRING_IMPL(value)
#if defined(_WIN32)
#define DOBA_TEST_PLATFORM_HEADER(name) \
  DOBA_TEST_PLATFORM_STRING(name##_windows.h)
#elif defined(__linux__)
#define DOBA_TEST_PLATFORM_HEADER(name) \
  DOBA_TEST_PLATFORM_STRING(name##_linux.h)
#else
#error Unsupported test platform
#endif

#endif
