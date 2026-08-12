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

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "common/reader.h"
#include "common/writer.h"
#include "protocol/http/v11/body/writer_chunked.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::reader;
using martianlabs::doba::common::writer;
using martianlabs::doba::protocol::http::v11::body::writer_chunked;

std::string release(writer& value) {
  reader source(value.release());
  std::string output;
  source.read_all(output);
  return output;
}
}  // namespace

// +===========================================================================+
// | [>] writes one RFC 9112 chunk per nonempty payload          ( test-case ) |
// +===========================================================================+
DOBA_TEST("writes one RFC 9112 chunk per nonempty payload") {
  struct test_case {
    std::size_t size;
    std::string_view prefix;
  };
  constexpr test_case cases[] = {
      {1, "1\r\n"},    {15, "f\r\n"},    {16, "10\r\n"},
      {255, "ff\r\n"}, {256, "100\r\n"},
  };
  for (const auto& test : cases) {
    writer destination;
    writer_chunked value;
    const std::string payload(test.size, 'x');
    DOBA_EXPECT(value.write(payload, destination));
    DOBA_EXPECT(value.end(destination));
    std::string expected(test.prefix);
    expected += payload;
    expected += "\r\n0\r\n\r\n";
    DOBA_EXPECT_EQUAL(release(destination), expected);
  }
}
// +===========================================================================+
// | [>] empty payloads are skipped and end is idempotent        ( test-case ) |
// +===========================================================================+
DOBA_TEST("empty payloads are skipped and end is idempotent") {
  writer destination;
  writer_chunked value;
  DOBA_EXPECT(value.write(std::string_view{}, destination));
  DOBA_EXPECT(value.write(std::span<const std::byte>{}, destination));
  DOBA_EXPECT(value.end(destination));
  DOBA_EXPECT(value.end(destination));
  DOBA_EXPECT_EQUAL(release(destination), "0\r\n\r\n");
}
// +===========================================================================+
// | [>] multiple writes preserve payload and chunk boundaries   ( test-case ) |
// +===========================================================================+
DOBA_TEST("multiple writes preserve payload and chunk boundaries") {
  writer destination;
  writer_chunked value;
  DOBA_EXPECT(value.write("hello", destination));
  const std::byte payload[] = {std::byte{0}, std::byte{0xff}};
  DOBA_EXPECT(value.write(payload, destination));
  DOBA_EXPECT(value.end(destination));
  const std::string output = release(destination);
  const std::string prefix = "5\r\nhello\r\n2\r\n";
  DOBA_EXPECT_EQUAL(output.size(), prefix.size() + 2 + 7);
  DOBA_EXPECT_EQUAL(std::string_view(output.data(), prefix.size()), prefix);
  DOBA_EXPECT_EQUAL(static_cast<unsigned char>(output[prefix.size()]), 0);
  DOBA_EXPECT_EQUAL(static_cast<unsigned char>(output[prefix.size() + 1]),
                    0xff);
  DOBA_EXPECT_EQUAL(std::string_view(output).substr(prefix.size() + 2),
                    "\r\n0\r\n\r\n");
}
// +===========================================================================+
// | [>] writes after the terminating chunk are rejected         ( test-case ) |
// +===========================================================================+
DOBA_TEST("writes after the terminating chunk are rejected") {
  writer destination;
  writer_chunked value;
  DOBA_EXPECT(value.write("before", destination));
  DOBA_EXPECT(value.end(destination));
  DOBA_EXPECT(!value.write("after", destination));
  DOBA_EXPECT(!value.write(std::span<const std::byte>{}, destination));
  DOBA_EXPECT_EQUAL(release(destination), "6\r\nbefore\r\n0\r\n\r\n");
}
