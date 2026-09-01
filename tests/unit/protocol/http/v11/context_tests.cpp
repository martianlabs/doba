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

#include "protocol/http/v11/context.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::helpers;
using martianlabs::doba::protocol::http::v11::context;
using martianlabs::doba::protocol::http::v11::rejection_reason;
}  // namespace

// +===========================================================================+
// | [>] default context has no request derived signals          ( test-case ) |
// +===========================================================================+
DOBA_TEST("default context has no request derived signals") {
  const context value;
  DOBA_EXPECT(!value.has_content_length);
  DOBA_EXPECT(!value.multiple_content_length);
  DOBA_EXPECT_EQUAL(value.content_length, 0);
  DOBA_EXPECT(!value.has_transfer_encoding);
  DOBA_EXPECT(!value.has_host);
  DOBA_EXPECT(!value.multiple_host);
  DOBA_EXPECT(value.host.host.empty());
  DOBA_EXPECT(value.host.port.empty());
  DOBA_EXPECT_EQUAL(value.host.type, helpers::host_type::kUnknown);
  DOBA_EXPECT(!value.has_target_authority);
  DOBA_EXPECT(value.target_authority.host.empty());
  DOBA_EXPECT_EQUAL(value.forwarding_hops, 0);
  DOBA_EXPECT_EQUAL(value.rejection_reason, rejection_reason::kNone);
  DOBA_EXPECT(value.connection.persistent);
  DOBA_EXPECT_EQUAL(value.policies.max_content_length, 0);
}
