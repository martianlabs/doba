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

#include <algorithm>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "protocol/http/v11/body/framer_chunked.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::body::framer_chunked;
using martianlabs::doba::protocol::http::v11::body::framer_error;

std::span<const std::byte> bytes(std::string_view value) {
  return {reinterpret_cast<const std::byte*>(value.data()), value.size()};
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

    const auto state = value.consume(bytes(wire));
    DOBA_EXPECT_EQUAL(state.consumed, wire.size());
    DOBA_EXPECT(state.complete);
    DOBA_EXPECT(!state.has_error);

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

    const auto first = value.consume(
        bytes(std::string_view(source).substr(0, split)));
    const auto second =
        value.consume(bytes(std::string_view(source).substr(split)));
    DOBA_EXPECT(!first.has_error);
    DOBA_EXPECT(!second.has_error);
    DOBA_EXPECT(first.complete || second.complete);
    DOBA_EXPECT_EQUAL(first.consumed + second.consumed, body.size());

  }
}
// +===========================================================================+
// | [>] empty and truncated buffers remain incomplete           ( test-case ) |
// +===========================================================================+
DOBA_TEST("empty and truncated buffers remain incomplete") {
  constexpr std::string_view wire = "1\r\na\r\n0\r\n\r\n";
  for (std::size_t size = 0; size < wire.size(); size++) {
    framer_chunked value;

    const auto state = value.consume(bytes(wire.substr(0, size)));
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

    auto state = value.consume(bytes(test.source));
    DOBA_EXPECT(state.has_error);
    DOBA_EXPECT_EQUAL(state.error, test.expected);
    state = value.consume(bytes("0\r\n\r\n"));
    DOBA_EXPECT(state.has_error);
    DOBA_EXPECT_EQUAL(state.error, test.expected);
    DOBA_EXPECT_EQUAL(state.consumed, 0);
  }
}
// +===========================================================================+
// | [>] rejects smuggling-prone chunk size forms                ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects smuggling-prone chunk size forms") {
  constexpr std::string_view accepted[] = {
      "A\r\n0123456789\r\n0\r\n\r\n",
      "a\r\n0123456789\r\n0\r\n\r\n",
      "0000a\r\n0123456789\r\n0\r\n\r\n",
      "000\r\n\r\n",
  };
  for (const auto wire : accepted) {
    martianlabs::doba::tests::unit::test_helper::set_context(
        "accepted " + std::string(wire));
    framer_chunked value;

    const auto state = value.consume(bytes(wire));
    DOBA_EXPECT(!state.has_error);
    DOBA_EXPECT(state.complete);
    DOBA_EXPECT_EQUAL(state.consumed, wire.size());
  }
  constexpr std::string_view rejected[] = {
      "0x1\r\na\r\n0\r\n\r\n", "-1\r\na\r\n0\r\n\r\n",
      "+1\r\na\r\n0\r\n\r\n",  " 1\r\na\r\n0\r\n\r\n",
      "1\na\r\n0\r\n\r\n",     "1\r\na\n0\r\n\r\n",
      "1\r\na\r\n0\n\r\n",
  };
  for (const auto wire : rejected) {
    martianlabs::doba::tests::unit::test_helper::set_context(
        "rejected " + std::string(wire));
    framer_chunked value;

    const auto state = value.consume(bytes(wire));
    DOBA_EXPECT(state.has_error);
    DOBA_EXPECT(!state.complete);
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

    const auto state = value.consume(bytes(source));
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

    const auto state = value.consume(bytes(source));
    DOBA_EXPECT(state.has_error);
    DOBA_EXPECT_EQUAL(state.error, framer_error::invalid_trailer);
  }
}
// +===========================================================================+
// | [>] accepts syntactic trailer fields                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("accepts syntactic trailer fields") {
  constexpr std::string_view names[] = {
      "Content-Length", "content-length", "Transfer-Encoding", "Host",
      "Content-Lengthx", "XContent-Length", "Transfer-Encodings", "X-Host",
  };
  for (const auto name : names) {
    const std::string source = "0\r\n" + std::string(name) + ": 1\r\n\r\n";
    martianlabs::doba::tests::unit::test_helper::set_context(
        std::string(name));
    for (std::size_t split = 0; split <= source.size(); split++) {
      framer_chunked value;

      auto state = value.consume(
          bytes(std::string_view(source).substr(0, split)));
      DOBA_EXPECT(!state.has_error);
      state = value.consume(bytes(std::string_view(source).substr(split)));
      DOBA_EXPECT(!state.has_error);
      DOBA_EXPECT(state.complete);

    }
  }
}
// +===========================================================================+
// | [>] completed framers ignore following request bytes        ( test-case ) |
// +===========================================================================+
DOBA_TEST("completed framers ignore following request bytes") {
  framer_chunked value;

  auto state = value.consume(bytes("0\r\n\r\nNEXT"));
  DOBA_EXPECT(state.complete);
  DOBA_EXPECT_EQUAL(state.consumed, 5);
  state = value.consume(bytes("NEXT"));
  DOBA_EXPECT(state.complete);
  DOBA_EXPECT_EQUAL(state.consumed, 0);
  DOBA_EXPECT(!state.has_error);

}
// +===========================================================================+
// | [>] rejects chunk size overflow                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects chunk size overflow") {
  const std::string source(sizeof(std::size_t) * 2 + 1, 'f');
  framer_chunked value;

  const auto state = value.consume(bytes(source));
  DOBA_EXPECT(state.has_error);
  DOBA_EXPECT_EQUAL(state.error, framer_error::chunk_size_overflow);
}
// +===========================================================================+
// | [>] enforces extension and trailer size limits              ( test-case ) |
// +===========================================================================+
DOBA_TEST("enforces extension and trailer size limits") {
  for (bool extension : {true, false}) {
    const std::size_t limit =
        extension ? framer_chunked::kMaxChunkedExtensionSize
                  : framer_chunked::kMaxChunkedTrailerSize;
    for (std::size_t length : {limit - 1, limit, limit + 1}) {
      const std::string wire = extension
          ? "1;" + std::string(length - 1, 'x') + "\r\na\r\n0\r\n\r\n"
          : "0\r\nX: " + std::string(length - 7, 'x') + "\r\n\r\n";
      const auto expected = extension
          ? framer_error::chunk_extension_size_limit_exceeded
          : framer_error::trailer_size_limit_exceeded;
      for (std::size_t split = 0; split <= wire.size(); split++) {
        martianlabs::doba::tests::unit::test_helper::set_context(
            std::string(extension ? "extension " : "trailer ") +
            std::to_string(length) + ", split " + std::to_string(split));
        framer_chunked value;

        const auto first = value.consume(
            bytes(std::string_view(wire).substr(0, split)));
        const auto second = value.consume(
            bytes(std::string_view(wire).substr(split)));
        if (length <= limit) {
          DOBA_EXPECT(!first.has_error);
          DOBA_EXPECT(!second.has_error);
          DOBA_EXPECT(first.complete || second.complete);
          DOBA_EXPECT_EQUAL(first.consumed + second.consumed, wire.size());

        } else {
          DOBA_EXPECT(second.has_error);
          DOBA_EXPECT_EQUAL(second.error, expected);
          const auto repeated = value.consume(bytes("NEXT"));
          DOBA_EXPECT(repeated.has_error);
          DOBA_EXPECT_EQUAL(repeated.error, expected);
          DOBA_EXPECT_EQUAL(repeated.consumed, 0);
        }
      }
    }
  }
}
// +===========================================================================+
// | [>] limits cumulative chunk data across writes              ( test-case ) |
// +===========================================================================+
DOBA_TEST("limits cumulative chunk data across writes") {
  const std::string first = "3\r\nabc\r\n";
  const std::string exact = "2\r\nde\r\n0\r\n\r\n";
  const std::string excess = "3\r\ndef\r\n0\r\n\r\n";
  for (const bool fragmented : {false, true}) {
    framer_chunked value(5);

    const auto initial = value.consume(bytes(first));
    DOBA_EXPECT(!initial.has_error);
    DOBA_EXPECT_EQUAL(initial.consumed, first.size());
    if (fragmented) {
      const auto partial = value.consume(bytes("3\r\nd"));
      DOBA_EXPECT(!partial.has_error);
      DOBA_EXPECT_EQUAL(partial.consumed, 4);
    }
    const auto rejected = value.consume(
        bytes(fragmented ? std::string_view("ef\r\n0\r\n\r\n")
                         : std::string_view(excess)));
    DOBA_EXPECT(rejected.has_error);
    DOBA_EXPECT_EQUAL(rejected.error,
                      framer_error::chunked_size_limit_exceeded);
    const auto repeated = value.consume(bytes(exact));
    DOBA_EXPECT(repeated.has_error);
    DOBA_EXPECT_EQUAL(repeated.error,
                      framer_error::chunked_size_limit_exceeded);
    DOBA_EXPECT_EQUAL(repeated.consumed, 0);
  }
  framer_chunked limited(5);

  const auto accepted =
      limited.consume(bytes(first + exact));
  DOBA_EXPECT(!accepted.has_error);
  DOBA_EXPECT(accepted.complete);

  framer_chunked unlimited(0);

  const auto unrestricted =
      unlimited.consume(bytes(first + excess));
  DOBA_EXPECT(!unrestricted.has_error);
  DOBA_EXPECT(unrestricted.complete);

}
// +===========================================================================+
// | [>] quoted extensions survive fragmented and empty writes   ( test-case ) |
// +===========================================================================+
DOBA_TEST("quoted extensions survive three fragments and empty writes") {
  const std::string body =
      "1 \t; flag \t; name \t= \t\"a\\\"b\\\\c\" \t"
      "; token = v \t; bare \t\r\nx\r\n"
      "0;end=\"\"\r\nX:\t v\r\nY:\r\n\r\n";
  const std::string source = body + "NEXT";
  for (std::size_t split = 0; split <= body.size(); ++split) {
    framer_chunked value;

    std::size_t offset = 0;
    std::size_t total = 0;
    bool complete = false;
    for (const std::size_t end :
         {split, std::min(split + 1, body.size()), source.size()}) {
      const auto fragment =
          std::string_view(source).substr(offset, end - offset);
      const auto state = value.consume(bytes(fragment));
      DOBA_EXPECT(!state.has_error);
      DOBA_EXPECT(state.consumed <= fragment.size());
      total += state.consumed;
      complete = state.complete;
      const auto empty = value.consume({});
      DOBA_EXPECT_EQUAL(empty.consumed, 0);
      DOBA_EXPECT_EQUAL(empty.complete, complete);
      DOBA_EXPECT(!empty.has_error);
      offset = end;
    }
    DOBA_EXPECT(complete);
    DOBA_EXPECT_EQUAL(total, body.size());

  }
}
// +===========================================================================+
// | [>] rejects invalid bytes after extension transitions       ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects invalid bytes after extension transitions") {
  constexpr std::string_view cases[] = {
      "1 ;=x\r\n",
      "1;name /\r\n",
      "1;name= /\r\n",
      "1;name=token/\r\n",
      "1;name=\"x\"/\r\n",
      "1;name=\"x\\\001\"\r\n",
      "1;name=\"x\177\"\r\n",
  };
  for (const auto wire : cases) {
    framer_chunked value;

    const auto state = value.consume(bytes(wire));
    DOBA_EXPECT(state.has_error);
    DOBA_EXPECT_EQUAL(state.error, framer_error::invalid_chunk_size);
  }
}
// +===========================================================================+
// | [>] maximum chunk size is valid until the next hex digit    ( test-case ) |
// +===========================================================================+
DOBA_TEST("maximum chunk size is valid until the next hex digit") {
  const std::string maximum(sizeof(std::size_t) * 2, 'F');
  framer_chunked value;

  auto state = value.consume(bytes(maximum));
  DOBA_EXPECT(!state.has_error);
  DOBA_EXPECT(!state.complete);
  DOBA_EXPECT_EQUAL(state.consumed, maximum.size());
  state = value.consume(bytes("0"));
  DOBA_EXPECT(state.has_error);
  DOBA_EXPECT_EQUAL(state.error, framer_error::chunk_size_overflow);
  DOBA_EXPECT_EQUAL(state.consumed, 0);
}
// +===========================================================================+
// | [>] extension budget resets for each chunk                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("extension budget resets for each chunk") {
  const std::string chunk =
      "1;" + std::string(framer_chunked::kMaxChunkedExtensionSize - 1, 'x') +
      "\r\na\r\n";
  const std::string wire = chunk + chunk + std::string(64, '0') + "\r\n\r\n";
  framer_chunked value;

  const auto state = value.consume(bytes(wire));
  DOBA_EXPECT(!state.has_error);
  DOBA_EXPECT(state.complete);
  DOBA_EXPECT_EQUAL(state.consumed, wire.size());

}
// +===========================================================================+
// | [>] trailer budget includes all fields and the final line   ( test-case ) |
// +===========================================================================+
DOBA_TEST("trailer budget includes all fields and the final line") {
  const auto limit = framer_chunked::kMaxChunkedTrailerSize;
  for (const std::size_t length : {limit - 1, limit, limit + 1}) {
    const std::string wire =
        "0\r\nA:\r\nB:" + std::string(length - 10, 'x') + "\r\n\r\n";
    framer_chunked value;

    const auto state = value.consume(bytes(wire));
    DOBA_EXPECT_EQUAL(state.has_error, length > limit);
    DOBA_EXPECT_EQUAL(state.complete, length <= limit);
    if (length > limit) {
      DOBA_EXPECT_EQUAL(state.error,
                        framer_error::trailer_size_limit_exceeded);
    }
  }
}
