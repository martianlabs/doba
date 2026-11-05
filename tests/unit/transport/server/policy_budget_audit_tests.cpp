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

#include <memory>
#include <span>
#include <string>
#include "common/reader.h"
#include "test_helper.h"
#include "transport/server/send_state.h"

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] policy_budget_audit_tests                              ( test-suite ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
namespace {
using martianlabs::doba::common::reader;
using martianlabs::doba::transport::server::detail::send_state;
std::unique_ptr<reader> byte_source() {
  static const std::string bytes = "x";
  return std::make_unique<reader>(
      reader::borrowed(std::as_bytes(std::span(bytes))));
}
}  // namespace

DOBA_TEST(
    "empty terminal readers release ownership after clear or "
    "failure") {
  send_state output(16);
  DOBA_EXPECT(output.push({}, {}, std::make_unique<reader>(
      reader::borrowed(std::span<const std::byte>{}))));
  DOBA_EXPECT(!output.push({}, {}, byte_source()));
  DOBA_EXPECT(output.fill());  // An empty reader retires without a socket send.
  DOBA_EXPECT(output.push("0123456789abcdef", {}, nullptr));
  output.clear();
  DOBA_EXPECT(output.push({}, {}, byte_source()));
  output.clear();
  DOBA_EXPECT(output.push("0123456789abcdef", {}, nullptr));
  output.clear();
  reader failed(martianlabs::doba::common::filesystem_file{});
  std::byte byte{};
  failed.read(std::span<std::byte>(&byte, 1));
  DOBA_EXPECT(failed.failed());
  DOBA_EXPECT(output.push({}, {}, std::make_unique<reader>(std::move(failed))));
  DOBA_EXPECT(!output.fill());
  output.clear();
  DOBA_EXPECT(output.push("0123456789abcdef", {}, nullptr));
}
