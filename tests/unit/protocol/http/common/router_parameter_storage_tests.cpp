//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
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

#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>

#include "protocol/http/common/router_parameter_storage.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::detail::router_parameter_storage;
}  // namespace

// +===========================================================================+
// | [>] reuses values and string capacity                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("parameter storage reuses values and string capacity") {
  static_assert(!std::is_copy_constructible_v<router_parameter_storage>);
  router_parameter_storage first;
  auto& values = first.acquire<std::tuple<std::string, int>>();
  std::get<0>(values).assign(256, 'x');
  const auto* address = &values;
  const auto* text = std::get<0>(values).data();
  first.reset();
  router_parameter_storage second;
  auto& reused = second.acquire<std::tuple<std::string, int>>();
  DOBA_EXPECT(&reused == address);
  std::get<0>(reused).assign(128, 'y');
  DOBA_EXPECT(std::get<0>(reused).data() == text);
}

// +===========================================================================+
// | [>] keeps simultaneous loans independent                    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("parameter storage keeps simultaneous loans independent") {
  router_parameter_storage first;
  router_parameter_storage second;
  auto& left = first.acquire<std::tuple<int>>();
  auto& right = second.acquire<std::tuple<int>>();
  std::get<0>(left) = 17;
  std::get<0>(right) = 42;
  DOBA_EXPECT(&left != &right);
  DOBA_EXPECT_EQUAL(std::get<0>(left), 17);
  DOBA_EXPECT_EQUAL(std::get<0>(right), 42);
  auto& other = first.acquire<std::tuple<std::string>>();
  std::get<0>(other) = "other type";
  DOBA_EXPECT_EQUAL(std::get<0>(right), 42);
}

// +===========================================================================+
// | [>] returns loans to the releasing thread                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("parameter storage returns loans to the releasing thread") {
  router_parameter_storage storage;
  auto& values = storage.acquire<std::tuple<long, bool>>();
  const auto* address = &values;
  std::get<0>(values) = 91;
  bool reused = false;
  std::jthread worker([&]() {
    DOBA_EXPECT_EQUAL(std::get<0>(values), 91);
    storage.reset();
    router_parameter_storage next;
    reused = &next.acquire<std::tuple<long, bool>>() == address;
  });
  worker.join();
  DOBA_EXPECT(reused);
}

// +===========================================================================+
// | [>] survives the allocating thread exit                     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("parameter storage survives the allocating thread exit") {
  router_parameter_storage storage;
  std::jthread worker([&]() {
    auto& values = storage.acquire<std::tuple<std::string, bool>>();
    std::get<0>(values) = "retained after thread exit";
  });
  worker.join();
  auto moved = std::move(storage);
  moved.reset();
  router_parameter_storage next;
  const auto& values = next.acquire<std::tuple<std::string, bool>>();
  DOBA_EXPECT_EQUAL(std::get<0>(values), "retained after thread exit");
}

// +===========================================================================+
// | [>] clears borrowed views before caching values             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("parameter storage clears borrowed views before caching values") {
  router_parameter_storage storage;
  auto& values = storage.acquire<std::tuple<std::string_view>>();
  std::get<0>(values) = "borrowed";
  const auto* address = &values;
  storage.reset();
  router_parameter_storage next;
  const auto& reused = next.acquire<std::tuple<std::string_view>>();
  DOBA_EXPECT(&reused == address);
  DOBA_EXPECT(std::get<0>(reused).empty());
  DOBA_EXPECT(std::get<0>(reused).data() == nullptr);
}
