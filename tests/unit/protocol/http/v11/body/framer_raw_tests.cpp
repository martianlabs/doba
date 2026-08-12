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

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "common/reader.h"
#include "common/writer.h"
#include "protocol/http/v11/body/framer_raw.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::byte_storage_options;
using martianlabs::doba::common::reader;
using martianlabs::doba::common::writer;
using martianlabs::doba::protocol::http::v11::body::framer_error;
using martianlabs::doba::protocol::http::v11::body::framer_raw;

std::span<const std::byte> bytes(std::string_view value) {
  return {reinterpret_cast<const std::byte*>(value.data()), value.size()};
}
std::string release(writer& value) {
  reader source(value.release());
  std::string output;
  source.read_all(output);
  return output;
}
}  // namespace

// +===========================================================================+
// | [>] zero length completes without consuming input           ( test-case ) |
// +===========================================================================+
DOBA_TEST("zero length completes without consuming input") {
  framer_raw value(0);
  writer destination;
  auto state = value.write(bytes("next request"), destination);
  DOBA_EXPECT_EQUAL(state.consumed, 0);
  DOBA_EXPECT(state.complete);
  DOBA_EXPECT(!state.has_error);
  DOBA_EXPECT(release(destination).empty());
}
// +===========================================================================+
// | [>] consumes exactly content length across every split      ( test-case ) |
// +===========================================================================+
DOBA_TEST("consumes exactly content length across every split") {
  constexpr std::string_view source = "payloadNEXT";
  constexpr std::size_t length = 7;
  for (std::size_t split = 0; split <= source.size(); split++) {
    framer_raw value(length);
    writer destination;
    const auto first = value.write(bytes(source.substr(0, split)), destination);
    const auto second = value.write(bytes(source.substr(split)), destination);
    DOBA_EXPECT_EQUAL(first.consumed + second.consumed, length);
    DOBA_EXPECT(second.complete || first.complete);
    DOBA_EXPECT(!first.has_error);
    DOBA_EXPECT(!second.has_error);
    DOBA_EXPECT_EQUAL(release(destination), "payload");
  }
}
// +===========================================================================+
// | [>] empty writes preserve incomplete state                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("empty writes preserve incomplete state") {
  framer_raw value(1);
  writer destination;
  for (int i = 0; i < 2; i++) {
    const auto state = value.write({}, destination);
    DOBA_EXPECT_EQUAL(state.consumed, 0);
    DOBA_EXPECT(!state.complete);
    DOBA_EXPECT(!state.has_error);
  }
  DOBA_EXPECT(release(destination).empty());
}
// +===========================================================================+
// | [>] destination errors are reported and latched             ( test-case ) |
// +===========================================================================+
DOBA_TEST("destination errors are reported and latched") {
  framer_raw value(2);
  writer destination(
      byte_storage_options{.spill_threshold = 1, .spill_dir = "?:\\invalid"});
  auto state = value.write(bytes("ab"), destination);
  DOBA_EXPECT(state.has_error);
  DOBA_EXPECT_EQUAL(state.error, framer_error::io_error);
  DOBA_EXPECT_EQUAL(state.consumed, 0);
  state = value.write(bytes("ab"), destination);
  DOBA_EXPECT(state.has_error);
  DOBA_EXPECT_EQUAL(state.error, framer_error::io_error);
  DOBA_EXPECT_EQUAL(state.consumed, 0);
}
