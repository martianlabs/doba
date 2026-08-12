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

#include <functional>
#include <memory>
#include <optional>
#include <type_traits>

#include "common/byte_storage.h"
#include "protocol/http/common/request_getter.h"
#include "test_helper.h"

namespace {
struct request {
  bool has_storage = false;
};
using martianlabs::doba::common::byte_storage;
using martianlabs::doba::protocol::http::request_getter;
}  // namespace

// +===========================================================================+
// | [>] alias accepts empty and populated optional storage      ( test-case ) |
// +===========================================================================+
DOBA_TEST("alias accepts empty and populated optional storage") {
  using expected =
      std::function<std::shared_ptr<request>(std::optional<byte_storage>)>;
  static_assert(std::same_as<request_getter<request>, expected>);
  request_getter<request> getter = [](std::optional<byte_storage> storage) {
    return std::make_shared<request>(request{storage.has_value()});
  };
  auto without_storage = getter(std::nullopt);
  DOBA_EXPECT(without_storage != nullptr);
  DOBA_EXPECT(!without_storage->has_storage);
  auto with_storage = getter(byte_storage{});
  DOBA_EXPECT(with_storage != nullptr);
  DOBA_EXPECT(with_storage->has_storage);
}
