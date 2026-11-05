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

#include <chrono>
#include <string_view>
#include <type_traits>

#include "processing_engine.h"
#include "protocol/contracts.h"
#include "protocol/processing_result.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::processing_result;
using martianlabs::doba::tests::unit::processing_engine;
namespace contracts = martianlabs::doba::protocol::contracts;

// +===========================================================================+
// | [>] Contract signatures                                        ( static ) |
// +---------------------------------------------------------------------------+
static_assert(contracts::engine<processing_engine<>>);
static_assert(!contracts::engine<processing_engine<void>>);
static_assert(!contracts::engine<processing_engine<int>>);
static_assert(!contracts::engine<processing_engine<processing_result&>>);
static_assert(!contracts::engine<processing_engine<processing_result, void>>);
static_assert(!contracts::engine<processing_engine<processing_result, int>>);
static_assert(!contracts::engine<
              processing_engine<processing_result, processing_result&>>);
static_assert(!contracts::engine<processing_engine<
              processing_result, processing_result, std::string_view>>);
static_assert(!contracts::engine<processing_engine<
              processing_result, processing_result, bool, false>>);
static_assert(contracts::engine_factory<
              decltype([] { return processing_engine<>{}; }),
              processing_engine<>>);
static_assert(std::is_aggregate_v<processing_result>);
static_assert(std::is_trivially_copyable_v<processing_result>);
static_assert(processing_result::clock::is_steady);
}  // namespace

// +===========================================================================+
// | [>] Default requires no processing                          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("processing result defaults to no additional turns") {
  const processing_result result;
  DOBA_EXPECT(!result.turn_requested);
  DOBA_EXPECT(!result.turn_at);
  DOBA_EXPECT(!result.turn_after_output_drained);
}

// +===========================================================================+
// | [>] Independent processing conditions                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("processing result retains a deadline with other turn conditions") {
  const auto due = processing_result::clock::time_point{} +
                   std::chrono::seconds(7);
  const processing_result result{true, due, true};
  DOBA_EXPECT(result.turn_requested);
  DOBA_EXPECT(result.turn_at && *result.turn_at == due);
  DOBA_EXPECT(result.turn_after_output_drained);
}
