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

#ifndef martianlabs_doba_tests_integration_test_helper_h
#define martianlabs_doba_tests_integration_test_helper_h

#include <string_view>

namespace martianlabs::doba::tests::integration {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] test_helper                                                 ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class test_helper {
 public:
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +=========================================================================+
  using test_t = void (*)();
  // +=========================================================================+
  // | [>] add                                                      ( public ) |
  // +=========================================================================+
  static bool add(std::string_view, int, std::string_view, test_t);
  // +=========================================================================+
  // | [>] expect                                                   ( public ) |
  // +=========================================================================+
  static bool expect(bool, std::string_view, std::string_view, int);
  // +=========================================================================+
  // | [>] run                                                      ( public ) |
  // +=========================================================================+
  static int run();
};
}  // namespace martianlabs::doba::tests::integration

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] MACROs                                                     ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
#define DOBA_TEST_JOIN_INNER(left, right) left##right
#define DOBA_TEST_JOIN(left, right) DOBA_TEST_JOIN_INNER(left, right)
#define DOBA_TEST(name) DOBA_TEST_IMPL(name, __LINE__)
#define DOBA_TEST_IMPL(name, line)                                  \
  static void DOBA_TEST_JOIN(doba_test_, line)();                   \
  namespace {                                                       \
  const bool DOBA_TEST_JOIN(doba_test_registered_, line) =          \
      martianlabs::doba::tests::integration::test_helper::add(     \
          __FILE__, line, name, &DOBA_TEST_JOIN(doba_test_, line)); \
  }                                                                 \
  static void DOBA_TEST_JOIN(doba_test_, line)()
#define DOBA_EXPECT(expression)                                      \
  do {                                                               \
    if (!martianlabs::doba::tests::integration::test_helper::expect( \
            expression, #expression, __FILE__, __LINE__)) {         \
      return;                                                        \
    }                                                                \
  } while (false)
#define DOBA_EXPECT_EQUAL(actual, expected) DOBA_EXPECT((actual) == (expected))

#endif
