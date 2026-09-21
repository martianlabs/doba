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

#include <utility>

#include "protocol/deserialization.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::deserialization_result;
using martianlabs::doba::protocol::deserialization_status;
}  // namespace

// +===========================================================================+
// | [>] deserialization errors preserve status and defaults     ( test-case ) |
// +===========================================================================+
DOBA_TEST("deserialization errors preserve status and defaults") {
  deserialization_result<int> empty;
  DOBA_EXPECT_EQUAL(empty.code, deserialization_status::kInvalidSource);
  DOBA_EXPECT(!empty.request);
  for (const auto status : {deserialization_status::kInvalidSource,
                            deserialization_status::kMoreBytesNeeded}) {
    deserialization_result<int> value(status);
    DOBA_EXPECT_EQUAL(value.code, status);
    DOBA_EXPECT(!value.request);
  }
}
// +===========================================================================+
// | [>] deserialization copies and moves preserve ownership     ( test-case ) |
// +===========================================================================+
DOBA_TEST("deserialization copies and moves preserve ownership") {
  auto owner = std::make_shared<int>(7);
  std::weak_ptr<int> lifetime = owner;
  deserialization_result<int> value(owner);
  owner.reset();
  auto copy = value;
  deserialization_result<int> moved(std::move(copy));
  deserialization_result<int> target;
  target = moved;
  value = {};
  moved = {};
  DOBA_EXPECT(!lifetime.expired());
  DOBA_EXPECT_EQUAL(target.code, deserialization_status::kSucceeded);
  DOBA_EXPECT_EQUAL(*target.request, 7);
  target = {};
  DOBA_EXPECT(lifetime.expired());
}
