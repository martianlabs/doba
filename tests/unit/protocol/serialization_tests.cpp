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
using martianlabs::doba::common::reader;
using martianlabs::doba::protocol::serialization_result;
}  // namespace

// +===========================================================================+
// | [>] serialization defaults contain no owned output          ( test-case ) |
// +===========================================================================+
DOBA_TEST("serialization defaults contain no owned output") {
  serialization_result value;
  DOBA_EXPECT(value.head.empty());
  DOBA_EXPECT(value.body.empty());
  DOBA_EXPECT(!value.source);
}
// +===========================================================================+
// | [>] serialization moves retain strings and reader cursor    ( test-case ) |
// +===========================================================================+
DOBA_TEST("serialization moves retain strings and reader cursor") {
  byte_storage storage;
  DOBA_EXPECT(storage.write("body", 4));
  storage.finish(4);
  serialization_result value;
  value.head = std::string(32, 'H');
  value.body = std::string(32, 'B');
  value.source = std::make_unique<reader>(std::move(storage));
  std::byte byte{};
  DOBA_EXPECT(value.source->fetch(byte));
  const char* head = value.head.data();
  const char* body = value.body.data();
  serialization_result moved(std::move(value));
  serialization_result target;
  target.head = "!";
  target = std::move(moved);
  DOBA_EXPECT_EQUAL(target.head.data(), head);
  DOBA_EXPECT_EQUAL(target.body.data(), body);
  DOBA_EXPECT_EQUAL(target.head, std::string(32, 'H'));
  DOBA_EXPECT_EQUAL(target.body, std::string(32, 'B'));
  DOBA_EXPECT(value.head.empty());
  DOBA_EXPECT(moved.head.empty());
  std::string output;
  DOBA_EXPECT_EQUAL(target.source->read_all(output), 3);
  DOBA_EXPECT_EQUAL(output, "ody");
}
