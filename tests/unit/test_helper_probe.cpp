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

#include <chrono>
#include <cstdio>
#include <stdexcept>
#include <thread>

#include "test_helper.h"

// +===========================================================================+
// | [>] probe passes                                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe passes") {
  std::puts("probe body");
  DOBA_EXPECT(true);
}

// +===========================================================================+
// | [>] probe assertion fails                                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe assertion fails") {
  DOBA_EXPECT(2 + 2 == 5);
}

// +===========================================================================+
// | [>] probe standard exception                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe standard exception") {
  throw std::runtime_error("probe standard error");
}

// +===========================================================================+
// | [>] probe unknown exception                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe unknown exception") {
  throw 7;
}

// +===========================================================================+
// | [>] probe continues after failures                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe continues after failures") {
  std::puts("probe continued");
  DOBA_EXPECT(true);
}

// +===========================================================================+
// | [>] probe blocks                                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe blocks") {
  std::this_thread::sleep_for(std::chrono::seconds(30));
}

// +===========================================================================+
// | [>] probe row context                                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("probe row context") {
  martianlabs::doba::tests::unit::test_helper::set_context(
      "row 7: input=invalid");
  DOBA_EXPECT(false);
}
