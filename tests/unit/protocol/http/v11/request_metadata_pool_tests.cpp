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

#include <string_view>
#include <thread>
#include <utility>

#include "protocol/http/v11/request_metadata_pool.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::detail::request_metadata_pool;
}  // namespace

// +===========================================================================+
// | [>] reuses capacity without retaining views                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("request metadata reuses capacity without retaining views") {
  auto values = request_metadata_pool::acquire();
  values.emplace_back("Name", "value");
  const auto* address = values.data();
  const auto capacity = values.capacity();
  request_metadata_pool::release(std::move(values));
  auto reused = request_metadata_pool::acquire();
  DOBA_EXPECT(reused.empty());
  DOBA_EXPECT(reused.data() == address);
  DOBA_EXPECT_EQUAL(reused.capacity(), capacity);
}

// +===========================================================================+
// | [>] keeps simultaneous loans independent                    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("request metadata keeps simultaneous loans independent") {
  auto first = request_metadata_pool::acquire();
  auto second = request_metadata_pool::acquire();
  first.emplace_back("First", "one");
  second.emplace_back("Second", "two");
  DOBA_EXPECT(first.data() != second.data());
  request_metadata_pool::release(std::move(first));
  auto reused = request_metadata_pool::acquire();
  reused.emplace_back("Third", "three");
  DOBA_EXPECT_EQUAL(second.front().second, "two");
}

// +===========================================================================+
// | [>] can move after its allocating thread exits              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("request metadata can move after its allocating thread exits") {
  request_metadata_pool::values values;
  std::jthread worker([&]() {
    values = request_metadata_pool::acquire();
    values.emplace_back("Name", "retained");
  });
  worker.join();
  DOBA_EXPECT_EQUAL(values.front().second, "retained");
  const auto* address = values.data();
  request_metadata_pool::release(std::move(values));
  auto reused = request_metadata_pool::acquire();
  DOBA_EXPECT(reused.data() == address);
  DOBA_EXPECT(reused.empty());
}

// +===========================================================================+
// | [>] returns capacity to the releasing thread                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("request metadata returns capacity to the releasing thread") {
  auto values = request_metadata_pool::acquire();
  values.emplace_back("Name", "value");
  const auto* address = values.data();
  bool reused = false;
  std::jthread worker([&]() {
    request_metadata_pool::release(std::move(values));
    auto next = request_metadata_pool::acquire();
    reused = next.data() == address && next.empty();
  });
  worker.join();
  DOBA_EXPECT(reused);
}
