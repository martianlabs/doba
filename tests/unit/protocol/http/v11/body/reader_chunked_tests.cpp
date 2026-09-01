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

#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "common/reader.h"
#include "protocol/http/v11/body/reader_chunked.h"
#include "protocol/http/v11/limits.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::reader;
using martianlabs::doba::protocol::http::v11::limits;
using martianlabs::doba::protocol::http::v11::body::reader_chunked;
using martianlabs::doba::protocol::http::v11::body::reader_error;

std::span<const std::byte> bytes(std::string_view value) {
  return {reinterpret_cast<const std::byte*>(value.data()), value.size()};
}
}  // namespace

// +===========================================================================+
// | [>] decodes chunks extensions and trailers                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("decodes chunks extensions and trailers") {
  constexpr std::string_view wire =
      "5;name=value\r\nhello\r\n6\r\n world\r\n0\r\nX: y\r\n\r\n";
  for (std::size_t output_size = 1; output_size <= 12; output_size++) {
    reader source = reader::borrowed(bytes(wire));
    reader_chunked value;
    std::array<std::byte, 16> output{};
    std::string decoded;
    bool complete = false;
    while (!complete) {
      const auto state =
          value.read(source, std::span<std::byte>(output.data(), output_size));
      DOBA_EXPECT(!state.has_error);
      decoded.append(reinterpret_cast<const char*>(output.data()),
                     state.produced);
      complete = state.complete;
    }
    DOBA_EXPECT_EQUAL(decoded, "hello world");
  }
}
// +===========================================================================+
// | [>] empty output consumes framing but not payload           ( test-case ) |
// +===========================================================================+
DOBA_TEST("empty output consumes framing but not payload") {
  reader source = reader::borrowed(bytes("1\r\na\r\n0\r\n\r\n"));
  reader_chunked value;
  auto state = value.read(source, {});
  DOBA_EXPECT_EQUAL(state.produced, 0);
  DOBA_EXPECT(!state.complete);
  DOBA_EXPECT(!state.has_error);
  std::byte output;
  state = value.read(source, std::span<std::byte>(&output, 1));
  DOBA_EXPECT_EQUAL(state.produced, 1);
  DOBA_EXPECT(state.complete);
  DOBA_EXPECT_EQUAL(static_cast<char>(output), 'a');
}
// +===========================================================================+
// | [>] truncated sources report and latch incomplete           ( test-case ) |
// +===========================================================================+
DOBA_TEST("truncated sources report and latch incomplete") {
  constexpr std::string_view wire = "1\r\na\r\n0\r\n\r\n";
  for (std::size_t size = 0; size < wire.size(); size++) {
    reader source = reader::borrowed(bytes(wire.substr(0, size)));
    reader_chunked value;
    std::array<std::byte, 16> output{};
    auto state = value.read(source, output);
    DOBA_EXPECT(state.has_error);
    DOBA_EXPECT_EQUAL(state.error, reader_error::chunked_incomplete);
    state = value.read(source, output);
    DOBA_EXPECT(state.has_error);
    DOBA_EXPECT_EQUAL(state.error, reader_error::chunked_incomplete);
    DOBA_EXPECT_EQUAL(state.produced, 0);
  }
}
// +===========================================================================+
// | [>] rejects malformed chunk sizes and CRLF sequences        ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects malformed chunk sizes and CRLF sequences") {
  struct test_case {
    std::string_view source;
    reader_error expected;
  };
  constexpr test_case cases[] = {
      {"\r\n", reader_error::invalid_chunk_size},
      {";x\r\n", reader_error::invalid_chunk_size},
      {"g\r\n", reader_error::invalid_chunk_size},
      {"1 x\r\n", reader_error::invalid_chunk_size},
      {"1\rX", reader_error::invalid_chunk_crlf},
      {"1\r\naX", reader_error::invalid_chunk_crlf},
      {"1\r\na\rX", reader_error::invalid_chunk_crlf},
  };
  for (const auto& test : cases) {
    reader source = reader::borrowed(bytes(test.source));
    reader_chunked value;
    std::array<std::byte, 16> output{};
    const auto state = value.read(source, output);
    DOBA_EXPECT(state.has_error);
    DOBA_EXPECT_EQUAL(state.error, test.expected);
  }
}
// +===========================================================================+
// | [>] rejects malformed chunk extensions                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects malformed chunk extensions") {
  constexpr std::string_view cases[] = {
      "1;\r\na\r\n0\r\n\r\n",
      "1;=value\r\na\r\n0\r\n\r\n",
      "1;name=\r\na\r\n0\r\n\r\n",
      "1;bad extension\r\na\r\n0\r\n\r\n",
      "1;name=\"unterminated\r\na\r\n0\r\n\r\n",
      "1;name=\x01\r\na\r\n0\r\n\r\n",
  };
  for (const auto wire : cases) {
    reader source = reader::borrowed(bytes(wire));
    reader_chunked value;
    std::byte output;
    const auto state = value.read(source, std::span<std::byte>(&output, 1));
    DOBA_EXPECT(state.has_error);
    DOBA_EXPECT_EQUAL(state.error, reader_error::invalid_chunk_size);
  }
}
// +===========================================================================+
// | [>] rejects malformed trailer fields                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects malformed trailer fields") {
  constexpr std::string_view cases[] = {
      "0\r\nInvalid\r\n\r\n",         "0\r\n: value\r\n\r\n",
      "0\r\nBad Name: value\r\n\r\n", "0\r\nX: value\n\r\n",
      "0\r\n X: value\r\n\r\n",       "0\r\nX: value\x01\r\n\r\n",
  };
  for (const auto wire : cases) {
    reader source = reader::borrowed(bytes(wire));
    reader_chunked value;
    std::byte output;
    const auto state = value.read(source, std::span<std::byte>(&output, 1));
    DOBA_EXPECT(state.has_error);
    DOBA_EXPECT_EQUAL(state.error, reader_error::invalid_trailer);
  }
}
// +===========================================================================+
// | [>] rejects chunk size overflow                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects chunk size overflow") {
  const std::string wire(sizeof(std::size_t) * 2 + 1, 'f');
  reader source = reader::borrowed(bytes(wire));
  reader_chunked value;
  std::byte output;
  const auto state = value.read(source, std::span<std::byte>(&output, 1));
  DOBA_EXPECT(state.has_error);
  DOBA_EXPECT_EQUAL(state.error, reader_error::chunk_size_overflow);
}
// +===========================================================================+
// | [>] enforces extension and trailer size limits              ( test-case ) |
// +===========================================================================+
DOBA_TEST("enforces extension and trailer size limits") {
  std::string extension = "1;";
  extension.append(limits::kMaxChunkedExtensionSize + 1, 'x');
  reader extension_source = reader::borrowed(bytes(extension));
  reader_chunked extension_reader;
  std::byte output;
  auto state =
      extension_reader.read(extension_source, std::span<std::byte>(&output, 1));
  DOBA_EXPECT(state.has_error);
  DOBA_EXPECT_EQUAL(state.error,
                    reader_error::chunk_extension_size_limit_exceeded);
  std::string trailer = "0\r\n";
  trailer.append(limits::kMaxChunkedTrailerSize + 1, 'x');
  reader trailer_source = reader::borrowed(bytes(trailer));
  reader_chunked trailer_reader;
  state = trailer_reader.read(trailer_source, std::span<std::byte>(&output, 1));
  DOBA_EXPECT(state.has_error);
  DOBA_EXPECT_EQUAL(state.error, reader_error::trailer_size_limit_exceeded);
}
