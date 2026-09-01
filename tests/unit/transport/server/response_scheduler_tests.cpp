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

#include <cstdint>
#include <memory>

#include "test_helper.h"
#include "transport/server/response_scheduler.h"

namespace {
using martianlabs::doba::protocol::serialization_result;
using martianlabs::doba::transport::server::detail::response_scheduler;
}  // namespace

// +===========================================================================+
// | [>] ready responses preserve position and order             ( test-case ) |
// +===========================================================================+
DOBA_TEST("ready responses preserve position and order") {
  response_scheduler value;
  DOBA_EXPECT(value.empty());

  auto first = std::make_unique<serialization_result>();
  first->prefix = "first";
  auto second = std::make_unique<serialization_result>();
  second->prefix = "second";

  DOBA_EXPECT_EQUAL(value.push_ready(std::move(first)), 0);
  DOBA_EXPECT_EQUAL(value.push_ready(std::move(second)), 1);
  DOBA_EXPECT_EQUAL(value.size(), 2);
  DOBA_EXPECT_EQUAL(value.front().position, 0);
  DOBA_EXPECT(value.front().ready());
  DOBA_EXPECT_EQUAL(value.front().response->prefix, "first");
  DOBA_EXPECT(!value.front().prefix_written);

  value.front().prefix_written = true;
  value.pop_front();
  DOBA_EXPECT_EQUAL(value.front().position, 1);
  DOBA_EXPECT_EQUAL(value.front().response->prefix, "second");
}
// +===========================================================================+
// | [>] clear removes responses without reusing positions        ( test-case ) |
// +===========================================================================+
DOBA_TEST("clear removes responses without reusing positions") {
  response_scheduler value;
  value.push_ready(std::make_unique<serialization_result>());
  value.clear();
  DOBA_EXPECT(value.empty());
  DOBA_EXPECT_EQUAL(
      value.push_ready(std::make_unique<serialization_result>()), 1);
}
// +===========================================================================+
// | [>] pending response blocks later ready responses            ( test-case ) |
// +===========================================================================+
DOBA_TEST("pending response blocks later ready responses") {
  response_scheduler value;
  uint64_t pending = value.reserve();
  auto second = std::make_unique<serialization_result>();
  second->prefix = "second";
  value.push_ready(std::move(second));

  DOBA_EXPECT_EQUAL(pending, 0);
  DOBA_EXPECT_EQUAL(value.size(), 2);
  DOBA_EXPECT(!value.front().ready());

  auto first = std::make_unique<serialization_result>();
  first->prefix = "first";
  DOBA_EXPECT(value.complete(pending, std::move(first)));
  DOBA_EXPECT(value.front().ready());
  DOBA_EXPECT_EQUAL(value.front().response->prefix, "first");
  value.pop_front();
  DOBA_EXPECT_EQUAL(value.front().response->prefix, "second");
}
// +===========================================================================+
// | [>] invalid and duplicate completions are rejected           ( test-case ) |
// +===========================================================================+
DOBA_TEST("invalid and duplicate completions are rejected") {
  response_scheduler value;
  uint64_t pending = value.reserve();
  DOBA_EXPECT(!value.complete(pending, nullptr));
  DOBA_EXPECT(!value.complete(
      pending + 1, std::make_unique<serialization_result>()));
  DOBA_EXPECT(value.complete(
      pending, std::make_unique<serialization_result>()));
  DOBA_EXPECT(!value.complete(
      pending, std::make_unique<serialization_result>()));
  value.pop_front();
  DOBA_EXPECT(!value.complete(
      pending, std::make_unique<serialization_result>()));
}
// +===========================================================================+
// | [>] response high watermark reports saturation              ( test-case ) |
// +===========================================================================+
DOBA_TEST("response high watermark reports saturation") {
  response_scheduler value;
  for (std::size_t i = 0; i < 63; i++) {
    value.push_ready(std::make_unique<serialization_result>());
  }
  DOBA_EXPECT(!value.saturated());
  value.reserve();
  DOBA_EXPECT(value.saturated());
  value.pop_front();
  DOBA_EXPECT(!value.saturated());
  value.reserve();
  DOBA_EXPECT(value.saturated());
  value.clear();
  DOBA_EXPECT(!value.saturated());
}
