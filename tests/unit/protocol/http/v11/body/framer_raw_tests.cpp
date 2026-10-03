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

#include "protocol/http/v11/body/framer_raw.h"
#include "body_bytes.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::body::framer_error;
using martianlabs::doba::protocol::http::v11::body::framer_raw;
using martianlabs::doba::tests::unit::bytes;
}  // namespace

// +===========================================================================+
// | [>] zero length completes without consuming input           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("zero length completes without consuming input") {
  framer_raw value(0);
  auto state = value.consume(bytes("next request"));
  DOBA_EXPECT_EQUAL(state.consumed, 0);
  DOBA_EXPECT(state.complete);
  DOBA_EXPECT(!state.has_error);
}

// +===========================================================================+
// | [>] consumes exactly content length across every split      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("consumes exactly content length across every split") {
  constexpr std::string_view source = "payloadNEXT";
  constexpr std::size_t length = 7;
  for (std::size_t split = 0; split <= source.size(); split++) {
    framer_raw value(length);
    const auto first = value.consume(bytes(source.substr(0, split)));
    const auto second = value.consume(bytes(source.substr(split)));
    DOBA_EXPECT_EQUAL(first.consumed + second.consumed, length);
    DOBA_EXPECT(second.complete || first.complete);
    DOBA_EXPECT(!first.has_error);
    DOBA_EXPECT(!second.has_error);
  }
}

// +===========================================================================+
// | [>] empty writes preserve incomplete state                  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("empty writes preserve incomplete state") {
  framer_raw value(1);
  for (int i = 0; i < 2; i++) {
    const auto state = value.consume({});
    DOBA_EXPECT_EQUAL(state.consumed, 0);
    DOBA_EXPECT(!state.complete);
    DOBA_EXPECT(!state.has_error);
  }
}
