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
#include <span>
#include <string>
#include "protocol/http/v11/engine_async.h"
#include "test_helper.h"

namespace {
using namespace martianlabs::doba::protocol::http::v11;
using martianlabs::doba::common::reader;
}  // namespace

DOBA_TEST("pending request credit uses exact limits and its own hysteresis") {
  for (std::size_t maximum : {0u, 1u, 2u, 3u, 4u, 128u}) {
    detail::engine_async pending(maximum, {}, {}, 0);
    int closes = 0;
    pending.set_on_close([&] { ++closes; });
    pending.set_on_send([](auto, auto, std::unique_ptr<reader>) {});
    const auto count = maximum ? maximum : 256;
    for (std::size_t i = 0; i < count; ++i) {
      DOBA_EXPECT(pending.reserve().has_value());
      if (maximum && i + 1 < maximum - maximum / 4)
        DOBA_EXPECT(!pending.pressured());
    }
    if (maximum) {
      DOBA_EXPECT(pending.pressured());
      for (std::size_t i = 0; i < count - maximum / 2; ++i)
        pending.submit(i, {{}, {}, nullptr}, true, false);
      DOBA_EXPECT(!pending.pressured());
      for (std::size_t i = 0; i < count - maximum / 2; ++i)
        DOBA_EXPECT(pending.reserve().has_value());
      DOBA_EXPECT(!pending.reserve().has_value());
      DOBA_EXPECT_EQUAL(closes, 1);
    } else {
      DOBA_EXPECT(!pending.pressured());
      DOBA_EXPECT_EQUAL(closes, 0);
    }
  }
}

DOBA_TEST(
    "pending response bytes use hysteresis independently of request "
    "positions") {
  detail::engine_async pending(128, {}, {}, 16);
  int closes = 0;
  std::string wire;
  pending.set_on_close([&] { ++closes; });
  pending.set_on_send([&](auto h, auto b, std::unique_ptr<reader>) {
    wire += h;
    wire += b;
  });
  for (int i = 0; i < 4; ++i) DOBA_EXPECT(pending.reserve().has_value());
  pending.submit(1, {"abc", {}, nullptr}, false, false);
  pending.submit(2, {"D", {}, nullptr}, false, false);
  DOBA_EXPECT(!pending.pressured());
  pending.submit(3, {"12345678", {}, nullptr}, false, false);
  DOBA_EXPECT(pending.pressured());  // 12 bytes, not four request positions.
  pending.submit(0, {"zero", {}, nullptr}, true, false);
  DOBA_EXPECT(pending.pressured());  // 9 bytes: below high but still above low.
  pending.submit(1, {{}, {}, nullptr}, true, false);
  DOBA_EXPECT(!pending.pressured());  // Exactly 8 bytes: low watermark.
  pending.submit(3, {"z", {}, nullptr}, false, false);
  DOBA_EXPECT(!pending.pressured());  // 9 bytes do not reenter pressure.
  pending.submit(3, {"yyy", {}, nullptr}, false, false);
  DOBA_EXPECT(pending.pressured());  // 12 bytes reenter pressure.
  pending.submit(2, {{}, {}, nullptr}, true, false);
  DOBA_EXPECT(!pending.pressured());
  pending.submit(3, {{}, {}, nullptr}, true, false);
  DOBA_EXPECT_EQUAL(wire, "zeroabcD12345678zyyy");
  DOBA_EXPECT_EQUAL(closes, 0);
}

DOBA_TEST(
    "pending sources are counted by positions and never by logical file "
    "length") {
  detail::engine_async pending(4, {}, {}, 1);
  int closes = 0, sources = 0;
  auto owner = std::make_shared<const std::string>(4096, 'x');
  std::weak_ptr<const std::string> retained = owner;
  pending.set_on_close([&] { ++closes; });
  pending.set_on_send([&](auto, auto, std::unique_ptr<reader> source) {
    if (source) ++sources;
  });
  pending.reserve();
  pending.reserve();
  auto source = std::make_unique<reader>(
      reader::shared(owner, std::as_bytes(std::span(*owner))));
  pending.submit(1, {{}, {}, std::move(source)}, true, false);
  owner.reset();
  DOBA_EXPECT(!retained.expired());
  DOBA_EXPECT(!pending.pressured());
  pending.submit(0, {{}, {}, nullptr}, true, false);
  DOBA_EXPECT_EQUAL(sources, 1);
  DOBA_EXPECT(retained.expired());
  DOBA_EXPECT_EQUAL(closes, 0);
}

DOBA_TEST(
    "pending overflow discards readers and closes once while later submissions "
    "are ignored") {
  detail::engine_async pending(0, {}, {}, 4);
  int closes = 0, sends = 0;
  pending.set_on_close([&] { ++closes; });
  pending.set_on_send([&](auto, auto, std::unique_ptr<reader>) { ++sends; });
  pending.reserve();
  pending.reserve();
  pending.reserve();
  auto owner = std::make_shared<const std::string>("source");
  std::weak_ptr<const std::string> retained = owner;
  pending.submit(2,
                 {"1234",
                  {},
                  std::make_unique<reader>(
                      reader::shared(owner, std::as_bytes(std::span(*owner))))},
                 true, false);
  owner.reset();
  DOBA_EXPECT(!retained.expired());
  pending.submit(1, {"5", {}, nullptr}, true, false);
  DOBA_EXPECT(retained.expired());
  pending.submit(0, {"ignored", {}, nullptr}, true, false);
  pending.close_now();
  DOBA_EXPECT_EQUAL(closes, 1);
  DOBA_EXPECT_EQUAL(sends, 0);
}
