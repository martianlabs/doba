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

#include <array>
#include <cstdio>

#include "../../../../fuzz/engine_case.h"
#include "test_helper.h"

// +===========================================================================+
// | [>] mixed response streams follow an independent model      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("security model checks ordered sources close and poisoned RX") {
  unsigned checked = 0;
  for (unsigned window = 0; window < 4; ++window) {
    for (unsigned steps = 0; steps < 12; ++steps) {
      for (unsigned terminal = 0; terminal < 3; ++terminal) {
        std::array<std::uint8_t, 32> input{};
        for (std::size_t i = 0; i < input.size(); ++i)
          input[i] = static_cast<std::uint8_t>(i * 37 + steps * 11);
        input[0] = static_cast<std::uint8_t>(window + terminal * 16);
        input[1] = static_cast<std::uint8_t>(steps);
        std::printf("model window=%u requests=%u terminal=%u\n",
                     window + 1, steps + 1, terminal);
        std::fflush(stdout);
        doba_fuzz::engine_case::run(input);
        ++checked;
      }
    }
  }
  DOBA_EXPECT_EQUAL(checked, 144U);
}
