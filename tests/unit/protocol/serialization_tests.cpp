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

#include <cstring>
#include <string>
#include <string_view>
#include <utility>

#include "protocol/serialization.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::byte_storage;
using martianlabs::doba::protocol::serialization_result;
}  // namespace

// +===========================================================================+
// | [>] serialization defaults contain no owned output          ( test-case ) |
// +===========================================================================+
DOBA_TEST("serialization defaults contain no owned output") {
  serialization_result value;
  DOBA_EXPECT(!value.prefix);
  DOBA_EXPECT_EQUAL(value.prefix_size, 0);
  DOBA_EXPECT(!value.source.has_value());
}
// +===========================================================================+
// | [>] serialization moves retain prefix and reader cursor     ( test-case ) |
// +===========================================================================+
DOBA_TEST("serialization moves retain prefix and reader cursor") {
  byte_storage storage;
  DOBA_EXPECT(storage.write("body", 4));
  storage.finish(4);
  serialization_result value;
  value.prefix = std::make_unique<char[]>(4);
  std::memcpy(value.prefix.get(), "HEAD", 4);
  value.prefix_size = 4;
  value.source.emplace(std::move(storage));
  std::byte byte{};
  DOBA_EXPECT(value.source->fetch(byte));
  auto* prefix = value.prefix.get();
  serialization_result moved(std::move(value));
  serialization_result target;
  target.prefix = std::make_unique<char[]>(1);
  target.prefix[0] = '!';
  target = std::move(moved);
  DOBA_EXPECT_EQUAL(target.prefix.get(), prefix);
  DOBA_EXPECT_EQUAL(std::string_view(target.prefix.get(), target.prefix_size),
                    "HEAD");
  DOBA_EXPECT(!value.prefix);
  DOBA_EXPECT(!moved.prefix);
  std::string output;
  DOBA_EXPECT_EQUAL(target.source->read_all(output), 3);
  DOBA_EXPECT_EQUAL(output, "ody");
}
