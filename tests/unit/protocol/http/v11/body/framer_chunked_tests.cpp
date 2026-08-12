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
#include <limits>
#include <span>
#include <string>
#include <string_view>

#include "common/reader.h"
#include "common/writer.h"
#include "protocol/http/v11/body/framer_chunked.h"
#include "protocol/http/v11/limits.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::byte_storage_options;
using martianlabs::doba::common::reader;
using martianlabs::doba::common::writer;
using martianlabs::doba::protocol::http::v11::limits;
using martianlabs::doba::protocol::http::v11::body::framer_chunked;
using martianlabs::doba::protocol::http::v11::body::framer_error;

std::span<const std::byte> bytes(std::string_view value) {
  return {reinterpret_cast<const std::byte*>(value.data()), value.size()};
}
std::string release(writer& value) {
  reader source(value.release());
  std::string output;
  source.read_all(output);
  return output;
}
}  // namespace

// +===========================================================================+
// | [>] accepts valid bodies and preserves every wire byte      ( test-case ) |
// +===========================================================================+
DOBA_TEST("accepts valid bodies and preserves every wire byte") {
  constexpr std::string_view cases[] = {
      "0\r\n\r\n",
      "1\r\na\r\n0\r\n\r\n",
      "A\r\n0123456789\r\n0\r\n\r\n",
      "1;name=value\r\na\r\n0;last=yes\r\n\r\n",
      "1\r\na\r\n2\r\nbc\r\n0\r\nX-Test: value\r\n\r\n",
  };
  for (const auto wire : cases) {
    framer_chunked value;
    writer destination;
    const auto state = value.write(bytes(wire), destination);
    DOBA_EXPECT_EQUAL(state.consumed, wire.size());
    DOBA_EXPECT(state.complete);
    DOBA_EXPECT(!state.has_error);
    DOBA_EXPECT_EQUAL(release(destination), wire);
  }
}
// +===========================================================================+
// | [>] accepts every possible transport split                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("accepts every possible transport split") {
  constexpr std::string_view body =
      "3;ext=value\r\nabc\r\n2\r\nde\r\n0\r\nTrailer: value\r\n\r\n";
  const std::string source = std::string(body) + "NEXT";
  for (std::size_t split = 0; split <= source.size(); split++) {
    framer_chunked value;
    writer destination;
    const auto first = value.write(
        bytes(std::string_view(source).substr(0, split)), destination);
    const auto second =
        value.write(bytes(std::string_view(source).substr(split)), destination);
    DOBA_EXPECT(!first.has_error);
    DOBA_EXPECT(!second.has_error);
    DOBA_EXPECT(first.complete || second.complete);
    DOBA_EXPECT_EQUAL(first.consumed + second.consumed, body.size());
    DOBA_EXPECT_EQUAL(release(destination), body);
  }
}
// +===========================================================================+
// | [>] empty and truncated buffers remain incomplete           ( test-case ) |
// +===========================================================================+
DOBA_TEST("empty and truncated buffers remain incomplete") {
  constexpr std::string_view wire = "1\r\na\r\n0\r\n\r\n";
  for (std::size_t size = 0; size < wire.size(); size++) {
    framer_chunked value;
    writer destination;
    const auto state = value.write(bytes(wire.substr(0, size)), destination);
    DOBA_EXPECT_EQUAL(state.consumed, size);
    DOBA_EXPECT(!state.complete);
    DOBA_EXPECT(!state.has_error);
  }
}
// +===========================================================================+
// | [>] rejects malformed chunk sizes and CRLF sequences        ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects malformed chunk sizes and CRLF sequences") {
  struct test_case {
    std::string_view source;
    framer_error expected;
  };
  constexpr test_case cases[] = {
      {"\r\n", framer_error::invalid_chunk_size},
      {";x\r\n", framer_error::invalid_chunk_size},
      {"g\r\n", framer_error::invalid_chunk_size},
      {"1 x\r\n", framer_error::invalid_chunk_size},
      {"1\rX", framer_error::invalid_chunk_crlf},
      {"1\r\naX", framer_error::invalid_chunk_crlf},
      {"1\r\na\rX", framer_error::invalid_chunk_crlf},
  };
  for (const auto& test : cases) {
    framer_chunked value;
    writer destination;
    auto state = value.write(bytes(test.source), destination);
    DOBA_EXPECT(state.has_error);
    DOBA_EXPECT_EQUAL(state.error, test.expected);
    state = value.write(bytes("0\r\n\r\n"), destination);
    DOBA_EXPECT(state.has_error);
    DOBA_EXPECT_EQUAL(state.error, test.expected);
    DOBA_EXPECT_EQUAL(state.consumed, 0);
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
  for (const auto source : cases) {
    framer_chunked value;
    writer destination;
    const auto state = value.write(bytes(source), destination);
    DOBA_EXPECT(state.has_error);
    DOBA_EXPECT_EQUAL(state.error, framer_error::invalid_chunk_size);
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
  for (const auto source : cases) {
    framer_chunked value;
    writer destination;
    const auto state = value.write(bytes(source), destination);
    DOBA_EXPECT(state.has_error);
    DOBA_EXPECT_EQUAL(state.error, framer_error::invalid_trailer);
  }
}
// +===========================================================================+
// | [>] completed framers ignore following request bytes        ( test-case ) |
// +===========================================================================+
DOBA_TEST("completed framers ignore following request bytes") {
  framer_chunked value;
  writer destination;
  auto state = value.write(bytes("0\r\n\r\nNEXT"), destination);
  DOBA_EXPECT(state.complete);
  DOBA_EXPECT_EQUAL(state.consumed, 5);
  state = value.write(bytes("NEXT"), destination);
  DOBA_EXPECT(state.complete);
  DOBA_EXPECT_EQUAL(state.consumed, 0);
  DOBA_EXPECT(!state.has_error);
  DOBA_EXPECT_EQUAL(release(destination), "0\r\n\r\n");
}
// +===========================================================================+
// | [>] rejects chunk size overflow                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects chunk size overflow") {
  const std::string source(sizeof(std::size_t) * 2 + 1, 'f');
  framer_chunked value;
  writer destination;
  const auto state = value.write(bytes(source), destination);
  DOBA_EXPECT(state.has_error);
  DOBA_EXPECT_EQUAL(state.error, framer_error::chunk_size_overflow);
}
// +===========================================================================+
// | [>] enforces extension and trailer size limits              ( test-case ) |
// +===========================================================================+
DOBA_TEST("enforces extension and trailer size limits") {
  std::string extension = "1;";
  extension.append(limits::kMaxChunkedExtensionSize + 1, 'x');
  framer_chunked extension_framer;
  writer extension_destination;
  auto state = extension_framer.write(bytes(extension), extension_destination);
  DOBA_EXPECT(state.has_error);
  DOBA_EXPECT_EQUAL(state.error,
                    framer_error::chunk_extension_size_limit_exceeded);
  std::string trailer = "0\r\n";
  trailer.append(limits::kMaxChunkedTrailerSize + 1, 'x');
  framer_chunked trailer_framer;
  writer trailer_destination;
  state = trailer_framer.write(bytes(trailer), trailer_destination);
  DOBA_EXPECT(state.has_error);
  DOBA_EXPECT_EQUAL(state.error, framer_error::trailer_size_limit_exceeded);
}
// +===========================================================================+
// | [>] destination errors are reported and latched             ( test-case ) |
// +===========================================================================+
DOBA_TEST("destination errors are reported and latched") {
  framer_chunked value;
  writer destination(
      byte_storage_options{.spill_threshold = 1, .spill_dir = "?:\\invalid"});
  auto state = value.write(bytes("1\r\na\r\n0\r\n\r\n"), destination);
  DOBA_EXPECT(state.has_error);
  DOBA_EXPECT_EQUAL(state.error, framer_error::io_error);
  state = value.write(bytes("0\r\n\r\n"), destination);
  DOBA_EXPECT(state.has_error);
  DOBA_EXPECT_EQUAL(state.error, framer_error::io_error);
  DOBA_EXPECT_EQUAL(state.consumed, 0);
}
