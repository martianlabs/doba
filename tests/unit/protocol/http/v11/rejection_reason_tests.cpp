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

#include "protocol/http/v11/rejection_reason.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::rejection_reason;
}  // namespace

// +===========================================================================+
// | [>] reasons are stable distinct protocol codes              ( test-case ) |
// +===========================================================================+
DOBA_TEST("reasons are stable distinct protocol codes") {
  constexpr rejection_reason values[] = {
      rejection_reason::kNone,
      rejection_reason::kSyntax,
      rejection_reason::kPayloadTooLarge,
      rejection_reason::kUnsupportedFeature,
      rejection_reason::kVersionNotSupported,
      rejection_reason::kUriTooLong,
      rejection_reason::kHeaderFieldsTooLarge,
      rejection_reason::kHandlerError,
      rejection_reason::kExpectationFailed,
  };
  for (std::size_t i = 0; i < std::size(values); i++) {
    DOBA_EXPECT_EQUAL(static_cast<std::size_t>(values[i]), i);
  }
}
