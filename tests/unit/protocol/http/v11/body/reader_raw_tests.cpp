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
#include <string_view>

#include "common/reader.h"
#include "protocol/http/v11/body/reader_raw.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::reader;
using martianlabs::doba::protocol::http::v11::body::reader_error;
using martianlabs::doba::protocol::http::v11::body::reader_raw;

std::span<const std::byte> bytes(std::string_view value) {
  return {reinterpret_cast<const std::byte*>(value.data()), value.size()};
}
}  // namespace

// +===========================================================================+
// | [>] zero length completes without reading source            ( test-case ) |
// +===========================================================================+
DOBA_TEST("zero length completes without reading source") {
  reader source = reader::borrowed(bytes("next"));
  reader_raw value(0);
  auto state = value.read(source, {});
  DOBA_EXPECT_EQUAL(state.produced, 0);
  DOBA_EXPECT(state.complete);
  DOBA_EXPECT(!state.has_error);
  DOBA_EXPECT(!source.eof());
}
// +===========================================================================+
// | [>] reads exactly content length for every output size      ( test-case ) |
// +===========================================================================+
DOBA_TEST("reads exactly content length for every output size") {
  constexpr std::string_view input = "payloadNEXT";
  for (std::size_t size = 1; size <= input.size(); size++) {
    reader source = reader::borrowed(bytes(input));
    reader_raw value(7);
    std::array<std::byte, 16> output{};
    std::string decoded;
    bool complete = false;
    while (!complete) {
      auto state =
          value.read(source, std::span<std::byte>(output.data(), size));
      DOBA_EXPECT(!state.has_error);
      decoded.append(reinterpret_cast<const char*>(output.data()),
                     state.produced);
      complete = state.complete;
    }
    DOBA_EXPECT_EQUAL(decoded, "payload");
    DOBA_EXPECT(!source.eof());
  }
}
// +===========================================================================+
// | [>] empty output makes no progress                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("empty output makes no progress") {
  reader source = reader::borrowed(bytes("a"));
  reader_raw value(1);
  const auto state = value.read(source, {});
  DOBA_EXPECT_EQUAL(state.produced, 0);
  DOBA_EXPECT(!state.complete);
  DOBA_EXPECT(!state.has_error);
  DOBA_EXPECT(!source.eof());
}
// +===========================================================================+
// | [>] truncated source reports and latches raw incomplete     ( test-case ) |
// +===========================================================================+
DOBA_TEST("truncated source reports and latches raw incomplete") {
  reader source = reader::borrowed(bytes("ab"));
  reader_raw value(3);
  std::array<std::byte, 3> output{};
  auto state = value.read(source, output);
  DOBA_EXPECT_EQUAL(state.produced, 2);
  DOBA_EXPECT(!state.complete);
  DOBA_EXPECT(!state.has_error);
  state = value.read(source, output);
  DOBA_EXPECT(state.has_error);
  DOBA_EXPECT_EQUAL(state.error, reader_error::raw_incomplete);
  state = value.read(source, output);
  DOBA_EXPECT(state.has_error);
  DOBA_EXPECT_EQUAL(state.error, reader_error::raw_incomplete);
  DOBA_EXPECT_EQUAL(state.produced, 0);
}
