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

#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

#include "common/reader.h"
#include "protocol/http/v11/body/reader.h"
#include "test_helper.h"

namespace {
using common_reader = martianlabs::doba::common::reader;
using martianlabs::doba::protocol::http::v11::body::reader;

std::span<const std::byte> bytes(std::string_view value) {
  return {reinterpret_cast<const std::byte*>(value.data()), value.size()};
}
}  // namespace

// +===========================================================================+
// | [>] reader is movable but not copyable                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("reader is movable but not copyable") {
  static_assert(!std::is_copy_constructible_v<reader>);
  static_assert(!std::is_copy_assignable_v<reader>);
  static_assert(std::is_move_constructible_v<reader>);
  static_assert(std::is_move_assignable_v<reader>);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] raw factory decodes only the declared body              ( test-case ) |
// +===========================================================================+
DOBA_TEST("raw factory decodes only the declared body") {
  auto value = reader::raw(common_reader::borrowed(bytes("payloadNEXT")), 7);
  std::array<std::byte, 3> output{};
  std::string decoded;
  bool complete = false;
  while (!complete) {
    const auto state = value.read(output);
    DOBA_EXPECT(!state.has_error);
    decoded.append(reinterpret_cast<const char*>(output.data()),
                   state.produced);
    complete = state.complete;
  }
  DOBA_EXPECT_EQUAL(decoded, "payload");
}
// +===========================================================================+
// | [>] chunked factory removes wire framing and trailers       ( test-case ) |
// +===========================================================================+
DOBA_TEST("chunked factory removes wire framing and trailers") {
  constexpr std::string_view wire =
      "5\r\nhello\r\n6;ext=yes\r\n world\r\n0\r\nX-Test: value\r\n\r\n";
  auto value = reader::chunked(common_reader::borrowed(bytes(wire)));
  std::array<std::byte, 2> output{};
  std::string decoded;
  bool complete = false;
  while (!complete) {
    const auto state = value.read(output);
    DOBA_EXPECT(!state.has_error);
    decoded.append(reinterpret_cast<const char*>(output.data()),
                   state.produced);
    complete = state.complete;
  }
  DOBA_EXPECT_EQUAL(decoded, "hello world");
}
