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
  for (std::size_t iteration = 0; !complete && iteration <= 11;
       iteration++) {
    const auto state = value.read(output);
    DOBA_EXPECT(!state.has_error);
    decoded.append(reinterpret_cast<const char*>(output.data()),
                   state.produced);
    complete = state.complete;
  }
  DOBA_EXPECT(complete);
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
  for (std::size_t iteration = 0; !complete && iteration <= wire.size();
       iteration++) {
    const auto state = value.read(output);
    DOBA_EXPECT(!state.has_error);
    decoded.append(reinterpret_cast<const char*>(output.data()),
                   state.produced);
    complete = state.complete;
  }
  DOBA_EXPECT(complete);
  DOBA_EXPECT_EQUAL(decoded, "hello world");
}
// +===========================================================================+
// | [>] moves preserve partially consumed body states           ( test-case ) |
// +===========================================================================+
DOBA_TEST("moves preserve partially consumed raw and chunked states") {
  for (const bool chunked : {false, true}) {
    constexpr std::string_view raw = "abc";
    constexpr std::string_view wire = "3\r\nabc\r\n0\r\n\r\n";
    auto value = chunked
                     ? reader::chunked(common_reader::borrowed(bytes(wire)))
                     : reader::raw(common_reader::borrowed(bytes(raw)), 3);
    std::array<std::byte, 2> output{};
    auto first = value.read(std::span(output).first(1));
    DOBA_EXPECT_EQUAL(first.produced, 1);
    DOBA_EXPECT(!first.complete);
    reader moved(std::move(value));
    auto target = reader::raw(common_reader::borrowed(bytes("old")), 3);
    target = std::move(moved);
    const auto state = target.read(output);
    DOBA_EXPECT(!state.has_error);
    DOBA_EXPECT(state.complete);
    DOBA_EXPECT_EQUAL(state.produced, 2);
    DOBA_EXPECT_EQUAL(output[0], std::byte{'b'});
    DOBA_EXPECT_EQUAL(output[1], std::byte{'c'});
    DOBA_EXPECT_EQUAL(target.read(output).produced, 0);
  }
}
