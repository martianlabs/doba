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

#include "protocol/http/v11/connection.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::connection;
}  // namespace

// +===========================================================================+
// | [>] default state represents persistent HTTP 1.1 connection ( test-case ) |
// +===========================================================================+
DOBA_TEST("default state represents persistent HTTP 1.1 connection") {
  const connection value;
  DOBA_EXPECT(value.persistent);
  DOBA_EXPECT(!value.close_requested);
  DOBA_EXPECT(value.transfer_codings.empty());
  DOBA_EXPECT(!value.chunked);
  DOBA_EXPECT(value.te_codings.empty());
  DOBA_EXPECT(!value.accepts_trailers);
  DOBA_EXPECT(value.trailer_names.empty());
  DOBA_EXPECT(value.upgrade_offer.empty());
  DOBA_EXPECT(value.options.empty());
  DOBA_EXPECT(!value.expects_continue);
}
// +===========================================================================+
// | [>] state retains zero copy parsed values                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("state retains zero copy parsed values") {
  connection value;
  value.transfer_codings = {"gzip", "chunked"};
  value.te_codings = {"trailers"};
  value.trailer_names = {"Digest"};
  value.upgrade_offer = {"websocket"};
  value.options = {"close", "upgrade"};
  DOBA_EXPECT_EQUAL(value.transfer_codings[0], "gzip");
  DOBA_EXPECT_EQUAL(value.transfer_codings[1], "chunked");
  DOBA_EXPECT_EQUAL(value.te_codings[0], "trailers");
  DOBA_EXPECT_EQUAL(value.trailer_names[0], "Digest");
  DOBA_EXPECT_EQUAL(value.upgrade_offer[0], "websocket");
  DOBA_EXPECT_EQUAL(value.options.size(), 2);
}
