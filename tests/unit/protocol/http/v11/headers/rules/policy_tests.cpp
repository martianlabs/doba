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

#include <limits>

#include "protocol/http/v11/headers/rules/policy.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::context;
using martianlabs::doba::protocol::http::v11::verdict;
using martianlabs::doba::protocol::http::v11::headers::rules::policy;
}  // namespace

// +===========================================================================+
// | [>] zero limit accepts every forwarding hop count           ( test-case ) |
// +===========================================================================+
DOBA_TEST("zero limit accepts every forwarding hop count") {
  context ctx;
  ctx.forwarding_hops = std::numeric_limits<std::size_t>::max();
  DOBA_EXPECT_EQUAL(policy::apply(ctx), verdict::kAccept);
}
// +===========================================================================+
// | [>] configured limit accepts boundary and rejects excess    ( test-case ) |
// +===========================================================================+
DOBA_TEST("configured limit accepts boundary and rejects excess") {
  context ctx;
  ctx.policies.max_forwarding_hops = 10;
  for (std::size_t hops = 0; hops <= 10; hops++) {
    ctx.forwarding_hops = hops;
    DOBA_EXPECT_EQUAL(policy::apply(ctx), verdict::kAccept);
  }
  ctx.forwarding_hops = 11;
  DOBA_EXPECT_EQUAL(policy::apply(ctx), verdict::kReject);
}
