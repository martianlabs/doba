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

#include <stdexcept>
#include <string>

#include "temporary_directory.h"
#include "test_helper.h"

// +===========================================================================+
// | [>] temporary overrides survive early unwinding             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("temporary directory restores the environment during unwinding") {
  using martianlabs::doba::tests::unit::temporary_directory;
  const auto before = temporary_directory::current();
  try {
    temporary_directory value("doba-temporary-override");
    DOBA_EXPECT_EQUAL(value.changed(), 0);
    DOBA_EXPECT_EQUAL(temporary_directory::current(), "doba-temporary-override");
    throw std::runtime_error("expected fixture failure");
  } catch (const std::runtime_error&) {
  }
  DOBA_EXPECT_EQUAL(temporary_directory::current(), before);
}

// +===========================================================================+
// | [>] nested overrides restore their own predecessor          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("temporary directory restores nested overrides exactly once") {
  using martianlabs::doba::tests::unit::temporary_directory;
  const auto before = temporary_directory::current();
  {
    temporary_directory outer("doba-outer-override");
    DOBA_EXPECT_EQUAL(outer.changed(), 0);
    {
      temporary_directory inner("doba-inner-override");
      DOBA_EXPECT_EQUAL(inner.changed(), 0);
      DOBA_EXPECT_EQUAL(temporary_directory::current(), "doba-inner-override");
      DOBA_EXPECT_EQUAL(inner.restore(), 0);
      DOBA_EXPECT_EQUAL(inner.restore(), 0);
      DOBA_EXPECT_EQUAL(temporary_directory::current(), "doba-outer-override");
    }
    DOBA_EXPECT_EQUAL(temporary_directory::current(), "doba-outer-override");
  }
  DOBA_EXPECT_EQUAL(temporary_directory::current(), before);
}
