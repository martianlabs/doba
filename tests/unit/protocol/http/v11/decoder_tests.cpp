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
#include <type_traits>

#include "common/byte_storage.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::channel_intent;
using martianlabs::doba::protocol::deserialization_status;
using martianlabs::doba::protocol::http::target;
using martianlabs::doba::protocol::http::v11::limits;
using martianlabs::doba::protocol::http::v11::rejection_reason;
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using test_decoder =
    martianlabs::doba::protocol::http::v11::decoder<request, response>;

std::size_t accumulate(test_decoder& value, std::string_view source) {
  std::string mutable_source(source);
  return value.accumulate(mutable_source.data(), mutable_source.size());
}
}  // namespace

// +===========================================================================+
// | [>] decoder is neither copyable nor movable                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder is neither copyable nor movable") {
  static_assert(!std::is_copy_constructible_v<test_decoder>);
  static_assert(!std::is_copy_assignable_v<test_decoder>);
  static_assert(!std::is_move_constructible_v<test_decoder>);
  static_assert(!std::is_move_assignable_v<test_decoder>);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] empty input and bounded accumulation                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("empty input needs more bytes and accumulation is bounded") {
  test_decoder value;
  DOBA_EXPECT_EQUAL(value.deserialize().code,
                    deserialization_status::kMoreBytesNeeded);
  std::string empty;
  DOBA_EXPECT_EQUAL(value.accumulate(empty.data(), 0), 0);
  DOBA_EXPECT_EQUAL(value.accumulate(nullptr, 0), 0);
  DOBA_EXPECT_EQUAL(value.accumulate(nullptr, 1), 0);
  std::string oversized(limits::kDecodingBufferSize + 1, 'x');
  DOBA_EXPECT_EQUAL(value.accumulate(oversized.data(), oversized.size()),
                    limits::kDecodingBufferSize);
  DOBA_EXPECT_EQUAL(value.accumulate(oversized.data(), 1), 0);
}
// +===========================================================================+
// | [>] parses every supported request target form              ( test-case ) |
// +===========================================================================+
DOBA_TEST("parses every supported request target form") {
  struct test_case {
    std::string_view source;
    std::string_view method;
    target target_form;
    std::string_view path;
  };
  constexpr test_case cases[] = {
      {"GET /path?q=1 HTTP/1.1\r\nHost: example.com\r\n\r\n", "GET",
       target::kOriginForm, "/path"},
      {"GET http://example.com/path HTTP/1.1\r\nHost: example.com\r\n\r\n",
       "GET", target::kAbsoluteForm, "/path"},
      {"CONNECT example.com:443 HTTP/1.1\r\nHost: example.com:443\r\n\r\n",
       "CONNECT", target::kAuthorityForm, ""},
      {"OPTIONS * HTTP/1.1\r\nHost: example.com\r\n\r\n", "OPTIONS",
       target::kAsteriskForm, ""},
  };
  for (const auto& test : cases) {
  test_decoder value;
    DOBA_EXPECT_EQUAL(accumulate(value, test.source), test.source.size());
    const auto result = value.deserialize();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
    DOBA_EXPECT(result.request != nullptr);
    DOBA_EXPECT_EQUAL(result.request->get_method(), test.method);
    DOBA_EXPECT_EQUAL(result.request->get_target(), test.target_form);
    DOBA_EXPECT_EQUAL(result.request->get_absolute_path(), test.path);
    DOBA_EXPECT_EQUAL(result.channel, channel_intent::kKeep);
  }
}
// +===========================================================================+
// | [>] parses a valid request at every transport split         ( test-case ) |
// +===========================================================================+
DOBA_TEST("parses a valid request at every transport split") {
  constexpr std::string_view source =
      "GET /a%20b?x=1&empty HTTP/1.1\r\n"
      "Host: example.com\r\nX-Test: value\r\n\r\n";
  for (std::size_t split = 0; split <= source.size(); split++) {
  test_decoder value;
    DOBA_EXPECT_EQUAL(accumulate(value, source.substr(0, split)), split);
    auto result = value.deserialize();
    if (split < source.size()) {
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
      DOBA_EXPECT_EQUAL(accumulate(value, source.substr(split)),
                        source.size() - split);
      result = value.deserialize();
    }
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
    DOBA_EXPECT_EQUAL(result.request->get_absolute_path(), "/a b");
    DOBA_EXPECT_EQUAL(result.request->get_query_parameters_length(), 2);
    DOBA_EXPECT_EQUAL(result.request->get_query_parameter("x")->second, "1");
    DOBA_EXPECT(result.request->get_query_parameter("empty")->second.empty());
    DOBA_EXPECT_EQUAL(result.request->get_header("X-Test").second, "value");
  }
}
// +===========================================================================+
// | [>] parses each target form at every transport split        ( test-case ) |
// +===========================================================================+
DOBA_TEST("parses every request target form at every transport split") {
  struct test_case {
    std::string_view source;
    target target_form;
  };
  constexpr test_case cases[] = {
      {"GET /path HTTP/1.1\r\nHost: example.com\r\n\r\n",
       target::kOriginForm},
      {"GET http://example.com/path HTTP/1.1\r\nHost: example.com\r\n\r\n",
       target::kAbsoluteForm},
      {"CONNECT example.com:443 HTTP/1.1\r\nHost: example.com:443\r\n\r\n",
       target::kAuthorityForm},
      {"OPTIONS * HTTP/1.1\r\nHost: example.com\r\n\r\n",
       target::kAsteriskForm},
  };
  for (const auto& test : cases) {
    for (std::size_t split = 0; split <= test.source.size(); split++) {
      test_decoder value;
      DOBA_EXPECT_EQUAL(accumulate(value, test.source.substr(0, split)), split);
      auto result = value.deserialize();
      if (split < test.source.size()) {
        DOBA_EXPECT_EQUAL(result.code,
                          deserialization_status::kMoreBytesNeeded);
        DOBA_EXPECT_EQUAL(accumulate(value, test.source.substr(split)),
                          test.source.size() - split);
        result = value.deserialize();
      }
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
      DOBA_EXPECT_EQUAL(result.request->get_target(), test.target_form);
    }
  }
}
// +===========================================================================+
// | [>] content length body handles every transport split       ( test-case ) |
// +===========================================================================+
DOBA_TEST("content length body consumes exact bytes across every split") {
  const std::string head =
      "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 7\r\n\r\n";
  const std::string source = head + "payload";
  for (std::size_t split = 0; split <= source.size(); split++) {
  test_decoder value;
    DOBA_EXPECT_EQUAL(
        accumulate(value, std::string_view(source).substr(0, split)), split);
    auto result = value.deserialize();
    if (split < source.size()) {
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
      DOBA_EXPECT_EQUAL(
          accumulate(value, std::string_view(source).substr(split)),
          source.size() - split);
      result = value.deserialize();
    }
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
    DOBA_EXPECT(result.request->has_body_reader());
    std::array<std::byte, 8> output{};
    const auto state = result.request->get_body_reader()->read(output);
    DOBA_EXPECT(state.complete);
    DOBA_EXPECT_EQUAL(state.produced, 7);
    DOBA_EXPECT_EQUAL(
        std::string_view(reinterpret_cast<const char*>(output.data()), 7),
        "payload");
  }
}
// +===========================================================================+
// | [>] chunked body is preserved then exposed decoded          ( test-case ) |
// +===========================================================================+
DOBA_TEST("chunked body is preserved then exposed decoded") {
  constexpr std::string_view source =
      "POST / HTTP/1.1\r\nHost: example.com\r\n"
      "Transfer-Encoding: chunked\r\n\r\n"
      "3;ext=yes\r\nabc\r\n2\r\nde\r\n0\r\nX: y\r\n\r\n";
  test_decoder value;
  DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
  const auto result = value.deserialize();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
  std::array<std::byte, 8> output{};
  const auto state = result.request->get_body_reader()->read(output);
  DOBA_EXPECT(state.complete);
  DOBA_EXPECT_EQUAL(state.produced, 5);
  DOBA_EXPECT_EQUAL(
      std::string_view(reinterpret_cast<const char*>(output.data()), 5),
      "abcde");
}
// +===========================================================================+
// | [>] chunked body handles every transport split              ( test-case ) |
// +===========================================================================+
DOBA_TEST("chunked body consumes exact bytes across every split") {
  constexpr std::string_view source =
      "POST / HTTP/1.1\r\nHost: example.com\r\n"
      "Transfer-Encoding: chunked\r\n\r\n"
      "3;ext=yes\r\nabc\r\n2\r\nde\r\n0\r\nX: y\r\n\r\n";
  for (std::size_t split = 0; split <= source.size(); split++) {
    test_decoder value;
    DOBA_EXPECT_EQUAL(accumulate(value, source.substr(0, split)), split);
    auto result = value.deserialize();
    if (split < source.size()) {
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
      DOBA_EXPECT_EQUAL(accumulate(value, source.substr(split)),
                        source.size() - split);
      result = value.deserialize();
    }
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
    std::array<std::byte, 8> output{};
    const auto state = result.request->get_body_reader()->read(output);
    DOBA_EXPECT(state.complete);
    DOBA_EXPECT_EQUAL(state.produced, 5);
    DOBA_EXPECT_EQUAL(
        std::string_view(reinterpret_cast<const char*>(output.data()), 5),
        "abcde");
  }
}
// +===========================================================================+
// | [>] expect continue returns interim while body is pending   ( test-case ) |
// +===========================================================================+
DOBA_TEST("expect continue returns interim only while body is pending") {
  constexpr std::string_view head =
      "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 1\r\n"
      "Expect: 100-continue\r\n\r\n";
  test_decoder value;
  DOBA_EXPECT_EQUAL(accumulate(value, head), head.size());
  auto result = value.deserialize();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
  DOBA_EXPECT_EQUAL(result.interim, "HTTP/1.1 100 Continue\r\n\r\n");
  DOBA_EXPECT_EQUAL(accumulate(value, "x"), 1);
  result = value.deserialize();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
  DOBA_EXPECT(result.interim.empty());
}
// +===========================================================================+
// | [>] connection close controls result channel intent         ( test-case ) |
// +===========================================================================+
DOBA_TEST("connection close controls result channel intent") {
  constexpr std::string_view source =
      "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
  test_decoder value;
  DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
  const auto result = value.deserialize();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
  DOBA_EXPECT_EQUAL(result.channel, channel_intent::kClose);
  DOBA_EXPECT(result.request->wants_connection_close());
}
// +===========================================================================+
// | [>] pipelined requests remain available after dispatch      ( test-case ) |
// +===========================================================================+
DOBA_TEST("pipelined requests remain available after first dispatch") {
  constexpr std::string_view first =
      "GET /one HTTP/1.1\r\nHost: one.example\r\nX-Id: first\r\n\r\n";
  constexpr std::string_view second =
      "GET /two HTTP/1.1\r\nHost: two.example\r\nX-Id: second\r\n\r\n";
  const std::string source = std::string(first) + std::string(second);
  test_decoder value;
  DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
  const auto first_result = value.deserialize();
  DOBA_EXPECT_EQUAL(first_result.code, deserialization_status::kSucceeded);
  const auto second_result = value.deserialize();
  DOBA_EXPECT_EQUAL(second_result.code, deserialization_status::kSucceeded);
  DOBA_EXPECT_EQUAL(first_result.request->get_absolute_path(), "/one");
  DOBA_EXPECT_EQUAL(first_result.request->get_header("Host").second,
                    "one.example");
  DOBA_EXPECT_EQUAL(first_result.request->get_header("X-Id").second, "first");
  DOBA_EXPECT_EQUAL(second_result.request->get_absolute_path(), "/two");
  DOBA_EXPECT_EQUAL(second_result.request->get_header("Host").second,
                    "two.example");
}
// +===========================================================================+
// | [>] invalid conditional dates are ignored                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("invalid conditional dates do not reject requests") {
  struct test_case {
    std::string_view name;
    std::string_view value;
  };
  constexpr test_case cases[] = {
      {"If-Modified-Since", "not-a-date"},
      {"If-Unmodified-Since", "Sunday, 06-Nov-94 08:49:37 GMT"},
  };
  for (const auto& test : cases) {
    const std::string source =
        "GET / HTTP/1.1\r\nHost: example.com\r\n" +
        std::string(test.name) + ": " + std::string(test.value) +
        "\r\n\r\n";
    test_decoder value;
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    const auto result = value.deserialize();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
    DOBA_EXPECT_EQUAL(result.request->get_header(test.name).second,
                      test.value);
  }
}
// +===========================================================================+
// | [>] rejects malformed request line and header syntax        ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects malformed request line and header syntax") {
  constexpr std::string_view cases[] = {
      " GET / HTTP/1.1\r\nHost: example.com\r\n\r\n",
      "GET\t/ HTTP/1.1\r\nHost: example.com\r\n\r\n",
      "GET /\tHTTP/1.1\r\nHost: example.com\r\n\r\n",
      "GET / http/1.1\r\nHost: example.com\r\n\r\n",
      "GET / HTTP/1.x\r\nHost: example.com\r\n\r\n",
      "GET / HTTP/1.1\nHost: example.com\r\n\r\n",
      "GET / HTTP/1.1\r\n: value\r\n\r\n",
      "GET / HTTP/1.1\r\nBad Name: value\r\n\r\n",
      "GET / HTTP/1.1\r\nHost value\r\n\r\n",
      "GET / HTTP/1.1\r\nHost: value\n\r\n",
      "GET / HTTP/1.1\r\nHost: value\r\n folded\r\n\r\n",
      "GET /%00 HTTP/1.1\r\nHost: example.com\r\n\r\n",
      "GET / HTTP/1.1\r\nHost : example.com\r\n\r\n",
      "GET / HTTP/1.1\r\nHost:\vexample.com\r\n\r\n",
      "GET / HTTP/1.1\r\nHost: example.com\rX-Test: x\r\n\r\n",
  };
  for (const auto source : cases) {
  test_decoder value;
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    DOBA_EXPECT_EQUAL(value.deserialize().code,
                      deserialization_status::kInvalidSource);
  }
}
// +===========================================================================+
// | [>] classifies incomplete and terminal request lines        ( test-case ) |
// +===========================================================================+
DOBA_TEST("distinguishes incomplete and terminal invalid request lines") {
  struct test_case {
    std::string_view source;
    deserialization_status expected;
  };
  constexpr test_case cases[] = {
      {"GET", deserialization_status::kMoreBytesNeeded},
      {"GET ", deserialization_status::kMoreBytesNeeded},
      {"GET /", deserialization_status::kMoreBytesNeeded},
      {"GET / ", deserialization_status::kMoreBytesNeeded},
      {"GET / H", deserialization_status::kMoreBytesNeeded},
      {"GET / HTTP/1.", deserialization_status::kMoreBytesNeeded},
      {"GET / HTTP/1.1", deserialization_status::kMoreBytesNeeded},
      {"GET / HTTP/1.1\r", deserialization_status::kMoreBytesNeeded},
      {"GET / HTTP/1.1\r\n", deserialization_status::kMoreBytesNeeded},
      {"GET\r\n", deserialization_status::kInvalidSource},
      {"GET /\r\n", deserialization_status::kInvalidSource},
      {"GET / X", deserialization_status::kInvalidSource},
      {"GET / \r\n", deserialization_status::kInvalidSource},
      {"GET / H\r\n", deserialization_status::kInvalidSource},
      {"GET / HT\r\n", deserialization_status::kInvalidSource},
      {"GET / HTT\r\n", deserialization_status::kInvalidSource},
      {"GET / HTTP\r\n", deserialization_status::kInvalidSource},
      {"GET / HTTP/\r\n", deserialization_status::kInvalidSource},
      {"GET / HTTP/1\r\n", deserialization_status::kInvalidSource},
      {"GET / HTTP/1.\r\n", deserialization_status::kInvalidSource},
      {"GET / HTTP/1.1\n", deserialization_status::kInvalidSource},
      {"GET / HTTP/1.1\rX", deserialization_status::kInvalidSource},
      {"GET / \r\n\r\n", deserialization_status::kInvalidSource},
  };
  for (const auto& test : cases) {
    test_decoder value;
    DOBA_EXPECT_EQUAL(accumulate(value, test.source), test.source.size());
    DOBA_EXPECT_EQUAL(value.deserialize().code, test.expected);
  }
}
// +===========================================================================+
// | [>] rejects invalid cross header combinations               ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects invalid cross header combinations") {
  constexpr std::string_view cases[] = {
      "GET / HTTP/1.1\r\n\r\n",
      "GET / HTTP/1.1\r\nHost: a\r\nHost: a\r\n\r\n",
      "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 0\r\n"
      "Content-Length: 0\r\n\r\n",
      "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 0, 0\r\n\r\n",
      "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: +1\r\n\r\nx",
      "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\n"
      "Transfer-Encoding: chunked\r\n\r\n",
      "POST / HTTP/1.1\r\nHost: a\r\n"
      "Transfer-Encoding: chunked, gzip\r\n\r\n",
      "GET http://a/ HTTP/1.1\r\nHost: b\r\n\r\n",
      "GET / HTTP/1.1\r\nHost: a\r\nConnection: host\r\n\r\n",
      "GET / HTTP/1.1\r\nHost: a\r\nConnection: upgrade\r\n\r\n",
  };
  for (const auto source : cases) {
  test_decoder value;
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    DOBA_EXPECT_EQUAL(value.deserialize().code,
                      deserialization_status::kInvalidSource);
  }
}
// +===========================================================================+
// | [>] rejects transfer coding without final chunked           ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects transfer coding without final chunked") {
  constexpr std::string_view source =
      "POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: gzip\r\n\r\n";
  test_decoder value;
  DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
  DOBA_EXPECT_EQUAL(value.deserialize().code,
                    deserialization_status::kInvalidSource);
}
// +===========================================================================+
// | [>] content length permits leading zeroes                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("content length permits leading zeroes") {
  constexpr std::string_view source =
      "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 01\r\n\r\nx";
  test_decoder value;
  DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
  const auto result = value.deserialize();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
  std::byte output;
  const auto state =
      result.request->get_body_reader()->read(std::span<std::byte>(&output, 1));
  DOBA_EXPECT(state.complete);
  DOBA_EXPECT_EQUAL(static_cast<char>(output), 'x');
}
// +===========================================================================+
// | [>] rejects embedded null and control bytes                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects embedded null and control bytes") {
  std::array<std::string, 4> cases = {
      std::string("GE") + '\0' + "T / HTTP/1.1\r\nHost: a\r\n\r\n",
      std::string("GET /") + '\0' + "x HTTP/1.1\r\nHost: a\r\n\r\n",
      std::string("GET / HTTP/1.1\r\nHo") + '\0' + "st: a\r\n\r\n",
      std::string("GET / HTTP/1.1\r\nHost: a") + '\0' + "b\r\n\r\n",
  };
  for (const auto& source : cases) {
  test_decoder value;
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    DOBA_EXPECT_EQUAL(value.deserialize().code,
                      deserialization_status::kInvalidSource);
  }
}
// +===========================================================================+
// | [>] rejects malformed chunked framing                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects malformed chunked framing") {
  constexpr std::string_view bodies[] = {
      "\r\n", ";x\r\n", "g\r\n", "1\rX", "1\r\naX", "1\r\na\rX",
  };
  constexpr std::string_view head =
      "POST / HTTP/1.1\r\nHost: a\r\n"
      "Transfer-Encoding: chunked\r\n\r\n";
  for (const auto body : bodies) {
    const std::string source = std::string(head) + std::string(body);
  test_decoder value;
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    DOBA_EXPECT_EQUAL(value.deserialize().code,
                      deserialization_status::kInvalidSource);
  }
}
// +===========================================================================+
// | [>] rejects malformed chunk extensions                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects malformed chunk extensions") {
  constexpr std::string_view body = "1;bad extension\r\na\r\n0\r\n\r\n";
  constexpr std::string_view head =
      "POST / HTTP/1.1\r\nHost: a\r\n"
      "Transfer-Encoding: chunked\r\n\r\n";
  const std::string source = std::string(head) + std::string(body);
  test_decoder value;
  DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
  DOBA_EXPECT_EQUAL(value.deserialize().code,
                    deserialization_status::kInvalidSource);
}
// +===========================================================================+
// | [>] rejects malformed chunk trailers                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects malformed chunk trailers") {
  constexpr std::string_view bodies[] = {
      "0\r\nInvalid-Trailer\r\n\r\n",
      "0\r\nX: y\n\r\n",
  };
  constexpr std::string_view head =
      "POST / HTTP/1.1\r\nHost: a\r\n"
      "Transfer-Encoding: chunked\r\n\r\n";
  for (const auto body : bodies) {
    const std::string source = std::string(head) + std::string(body);
  test_decoder value;
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    DOBA_EXPECT_EQUAL(value.deserialize().code,
                      deserialization_status::kInvalidSource);
  }
}
// +===========================================================================+
// | [>] reports version specific rejection reasons              ( test-case ) |
// +===========================================================================+
DOBA_TEST("reports version specific rejection reasons") {
  struct test_case {
    std::string_view version;
    rejection_reason expected;
  };
  constexpr test_case cases[] = {
      {"HTTP/0.9", rejection_reason::kNone},
      {"HTTP/1.0", rejection_reason::kNone},
      {"HTTP/1.2", rejection_reason::kVersionNotSupported},
      {"HTTP/2.0", rejection_reason::kVersionNotSupported},
      {"HTTP/9.9", rejection_reason::kVersionNotSupported},
  };
  for (const auto& test : cases) {
    const std::string source =
        "GET / " + std::string(test.version) + "\r\nHost: example.com\r\n\r\n";
  test_decoder value;
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    const auto result = value.deserialize();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
    DOBA_EXPECT_EQUAL(result.reason, static_cast<int>(test.expected));
  }
}
// +===========================================================================+
// | [>] incomplete prefixes request more bytes                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("incomplete prefixes request more bytes") {
  constexpr std::string_view source =
      "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
  for (std::size_t size = 0; size < source.size(); size++) {
  test_decoder value;
    DOBA_EXPECT_EQUAL(accumulate(value, source.substr(0, size)), size);
    DOBA_EXPECT_EQUAL(value.deserialize().code,
                      deserialization_status::kMoreBytesNeeded);
  }
}
