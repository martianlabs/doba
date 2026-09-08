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
#include <charconv>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

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
// +===========================================================================+
// | [>] check_header                                               ( method ) |
// +===========================================================================+
void check_header(std::string_view name, std::string_view field_value,
                  bool accepted,
                  std::string_view request_line = "GET / HTTP/1.1\r\n",
                  std::string_view extra_headers = {},
                  std::string_view body = {}) {
  std::array<std::string, 3> names = {
      std::string(name), std::string(name), std::string(name)};
  for (char& value : names[1]) {
    if (value >= 'A' && value <= 'Z') value += 'a' - 'A';
  }
  for (char& value : names[2]) {
    if (value >= 'a' && value <= 'z') value -= 'a' - 'A';
  }
  for (const auto& field_name : names) {
    for (bool ows : {false, true}) {
      martianlabs::doba::tests::unit::test_helper::set_context(
          field_name + ": " + std::string(field_value) +
          (ows ? ", with OWS" : ", without OWS"));
      const std::string source =
          std::string(request_line) + "Host: example.com\r\n" +
          std::string(extra_headers) + field_name + ":" +
          (ows ? "\t " : "") + std::string(field_value) +
          (ows ? " \t" : "") + "\r\n\r\n" + std::string(body);
      test_decoder value;
      DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
      const auto result = value.deserialize();
      if (accepted) {
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
        DOBA_EXPECT(result.request != nullptr);
        DOBA_EXPECT_EQUAL(result.request->get_header(name).second, field_value);
        if (result.request->has_body_reader()) {
          std::array<std::byte, 1> output{};
          const auto state = result.request->get_body_reader()->read(output);
          DOBA_EXPECT(!state.has_error);
          DOBA_EXPECT(state.complete);
          DOBA_EXPECT_EQUAL(state.produced, 0);
        }
      } else {
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
        DOBA_EXPECT(result.request == nullptr);
      }
    }
  }
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
  for (std::size_t i = 0; i < 8; i++) {
    const std::string next =
        "GET /reload-" + std::to_string(i) + " HTTP/1.1\r\n"
        "Host: reload.example\r\nX-Id: " + std::string(400, 'x') + "\r\n\r\n";
    DOBA_EXPECT_EQUAL(accumulate(value, next), next.size());
    const auto result = value.deserialize();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
    DOBA_EXPECT(result.request != nullptr);
    DOBA_EXPECT_EQUAL(result.request->get_absolute_path(),
                      "/reload-" + std::to_string(i));
    DOBA_EXPECT_EQUAL(first_result.request->get_absolute_path(), "/one");
    DOBA_EXPECT_EQUAL(first_result.request->get_header("Host").second,
                      "one.example");
    DOBA_EXPECT_EQUAL(first_result.request->get_header("X-Id").second, "first");
  }
}
// +===========================================================================+
// | [>] invalid conditional dates are ignored                  ( test-case )  |
// +===========================================================================+
DOBA_TEST("invalid conditional dates do not reject requests") {
  struct test_case {
    std::string_view name;
    std::string_view value;
  };
  constexpr test_case cases[] = {
      {"If-Modified-Since", "not-a-date"},
      {"If-Unmodified-Since", "not-a-date"},
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
    for (std::size_t split = 0; split <= source.size() + 1; split++) {
      for (bool triple : {false, true}) {
        if (split == source.size() + 1 && triple) continue;
        std::vector<std::size_t> ends;
        if (split == source.size() + 1) {
          for (std::size_t i = 1; i <= source.size(); i++) ends.push_back(i);
        } else {
          ends.push_back(split);
          if (triple) ends.push_back(std::min(split + 1, source.size()));
          ends.push_back(source.size());
        }
        martianlabs::doba::tests::unit::test_helper::set_context(
            "input " + std::string(source) + ", split "
                + std::to_string(split) +
            (triple ? ", three fragments" : ", two fragments or bytewise"));
        test_decoder value;
        auto result = value.deserialize();
        std::size_t offset = 0;
        for (std::size_t end : ends) {
          DOBA_EXPECT_EQUAL(
              accumulate(
                  value, std::string_view(source).substr(offset, end - offset)),
              end - offset);
          offset = end;
          result = value.deserialize();
          DOBA_EXPECT(result.request == nullptr);
          if (result.code == deserialization_status::kInvalidSource) break;
          DOBA_EXPECT_EQUAL(result.code,
                            deserialization_status::kMoreBytesNeeded);
        }
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
        DOBA_EXPECT(result.request == nullptr);
      }
    }
  }
}
// +===========================================================================+
// | [>] classifies incomplete and terminal request lines        ( test-case ) |
// +===========================================================================+
DOBA_TEST("distinguishes incomplete and terminal invalid request lines") {
  struct test_case {
    std::string_view source;
    deserialization_status expected;
    std::size_t invalid_at;
  };
  constexpr test_case cases[] = {
      {"GET", deserialization_status::kMoreBytesNeeded, 0},
      {"GET ", deserialization_status::kMoreBytesNeeded, 0},
      {"GET /", deserialization_status::kMoreBytesNeeded, 0},
      {"GET / ", deserialization_status::kMoreBytesNeeded, 0},
      {"GET / H", deserialization_status::kMoreBytesNeeded, 0},
      {"GET / HTTP/1.", deserialization_status::kMoreBytesNeeded, 0},
      {"GET / HTTP/1.1", deserialization_status::kMoreBytesNeeded, 0},
      {"GET / HTTP/1.1\r", deserialization_status::kMoreBytesNeeded, 0},
      {"GET / HTTP/1.1\r\n", deserialization_status::kMoreBytesNeeded, 0},
      {"GET\r\n", deserialization_status::kInvalidSource, 4},
      {"GET /\r\n", deserialization_status::kInvalidSource, 6},
      {"GET / X", deserialization_status::kInvalidSource, 7},
      {"GET / \r\n", deserialization_status::kInvalidSource, 7},
      {"GET / H\r\n", deserialization_status::kInvalidSource, 8},
      {"GET / HT\r\n", deserialization_status::kInvalidSource, 9},
      {"GET / HTT\r\n", deserialization_status::kInvalidSource, 10},
      {"GET / HTTP\r\n", deserialization_status::kInvalidSource, 11},
      {"GET / HTTP/\r\n", deserialization_status::kInvalidSource, 12},
      {"GET / HTTP/1\r\n", deserialization_status::kInvalidSource, 13},
      {"GET / HTTP/1.\r\n", deserialization_status::kInvalidSource, 14},
      {"GET / HTTP/1.1\n", deserialization_status::kInvalidSource, 15},
      {"GET / HTTP/1.1\rX", deserialization_status::kInvalidSource, 16},
      {"GET / \r\n\r\n", deserialization_status::kInvalidSource, 7},
  };
  for (const auto& test : cases) {
    for (std::size_t split = 0; split <= test.source.size(); split++) {
      test_decoder value;
      DOBA_EXPECT_EQUAL(accumulate(value, test.source.substr(0, split)), split);
      auto result = value.deserialize();
      const auto prefix_expected = test.invalid_at != 0 &&
                                   split >= test.invalid_at
          ? deserialization_status::kInvalidSource
          : deserialization_status::kMoreBytesNeeded;
      martianlabs::doba::tests::unit::test_helper::set_context(
          std::string(test.source) + ", prefix " + std::to_string(split));
      DOBA_EXPECT_EQUAL(result.code, prefix_expected);
      DOBA_EXPECT(result.request == nullptr);
      if (result.code != deserialization_status::kInvalidSource) {
        DOBA_EXPECT_EQUAL(accumulate(value, test.source.substr(split)),
                          test.source.size() - split);
        result = value.deserialize();
      }
      DOBA_EXPECT_EQUAL(result.code, test.expected);
      DOBA_EXPECT(result.request == nullptr);
    }
    test_decoder value;
    for (std::size_t end = 1; end <= test.source.size(); end++) {
      DOBA_EXPECT_EQUAL(accumulate(value, test.source.substr(end - 1, 1)), 1);
      const auto result = value.deserialize();
      const auto expected = test.invalid_at != 0 && end >= test.invalid_at
          ? deserialization_status::kInvalidSource
          : deserialization_status::kMoreBytesNeeded;
      DOBA_EXPECT_EQUAL(result.code, expected);
      DOBA_EXPECT(result.request == nullptr);
      if (result.code == deserialization_status::kInvalidSource) break;
    }
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
    if (source.starts_with("GET http://a/")) {
      test_decoder value;
      DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
      DOBA_EXPECT_EQUAL(value.deserialize().code,
                        deserialization_status::kInvalidSource);
      continue;
    }
    for (std::size_t split = 0; split <= source.size() + 1; split++) {
      for (bool triple : {false, true}) {
        if (split == source.size() + 1 && triple) continue;
        std::vector<std::size_t> ends;
        if (split == source.size() + 1) {
          for (std::size_t i = 1; i <= source.size(); i++) ends.push_back(i);
        } else {
          ends.push_back(split);
          if (triple) ends.push_back(std::min(split + 1, source.size()));
          ends.push_back(source.size());
        }
        martianlabs::doba::tests::unit::test_helper::set_context(
            "input " + std::string(source) + ", split "
                + std::to_string(split) +
            (triple ? ", three fragments" : ", two fragments or bytewise"));
        test_decoder value;
        auto result = value.deserialize();
        std::size_t offset = 0;
        for (std::size_t end : ends) {
          DOBA_EXPECT_EQUAL(
              accumulate(
                  value, std::string_view(source).substr(offset, end - offset)),
              end - offset);
          offset = end;
          result = value.deserialize();
          DOBA_EXPECT(result.request == nullptr);
          if (result.code == deserialization_status::kInvalidSource) break;
          DOBA_EXPECT_EQUAL(result.code,
                            deserialization_status::kMoreBytesNeeded);
        }
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
        DOBA_EXPECT(result.request == nullptr);
      }
    }
  }
}
// +===========================================================================+
// | [>] rejects transfer coding without final chunked           ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects transfer coding without final chunked") {
  constexpr std::string_view source =
      "POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: gzip\r\n\r\n";
  for (std::size_t split = 0; split <= source.size() + 1; split++) {
    for (bool triple : {false, true}) {
      if (split == source.size() + 1 && triple) continue;
      std::vector<std::size_t> ends;
      if (split == source.size() + 1) {
        for (std::size_t i = 1; i <= source.size(); i++) ends.push_back(i);
      } else {
        ends.push_back(split);
        if (triple) ends.push_back(std::min(split + 1, source.size()));
        ends.push_back(source.size());
      }
      martianlabs::doba::tests::unit::test_helper::set_context(
          "input " + std::string(source) + ", split " + std::to_string(split) +
          (triple ? ", three fragments" : ", two fragments or bytewise"));
      test_decoder value;
      auto result = value.deserialize();
      std::size_t offset = 0;
      for (std::size_t end : ends) {
        DOBA_EXPECT_EQUAL(
            accumulate(
                value, std::string_view(source).substr(offset, end - offset)),
            end - offset);
        offset = end;
        result = value.deserialize();
        DOBA_EXPECT(result.request == nullptr);
        if (result.code == deserialization_status::kInvalidSource) break;
        DOBA_EXPECT_EQUAL(result.code,
                          deserialization_status::kMoreBytesNeeded);
      }
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
      DOBA_EXPECT(result.request == nullptr);
    }
  }
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
    for (std::size_t split = 0; split <= source.size() + 1; split++) {
      for (bool triple : {false, true}) {
        if (split == source.size() + 1 && triple) continue;
        std::vector<std::size_t> ends;
        if (split == source.size() + 1) {
          for (std::size_t i = 1; i <= source.size(); i++) ends.push_back(i);
        } else {
          ends.push_back(split);
          if (triple) ends.push_back(std::min(split + 1, source.size()));
          ends.push_back(source.size());
        }
        martianlabs::doba::tests::unit::test_helper::set_context(
            "input " + std::string(source) + ", split "
                + std::to_string(split) +
            (triple ? ", three fragments" : ", two fragments or bytewise"));
        test_decoder value;
        auto result = value.deserialize();
        std::size_t offset = 0;
        for (std::size_t end : ends) {
          DOBA_EXPECT_EQUAL(
              accumulate(
                  value, std::string_view(source).substr(offset, end - offset)),
              end - offset);
          offset = end;
          result = value.deserialize();
          DOBA_EXPECT(result.request == nullptr);
          if (result.code == deserialization_status::kInvalidSource) break;
          DOBA_EXPECT_EQUAL(result.code,
                            deserialization_status::kMoreBytesNeeded);
        }
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
        DOBA_EXPECT(result.request == nullptr);
      }
    }
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
    for (std::size_t split = 0; split <= source.size() + 1; split++) {
      for (bool triple : {false, true}) {
        if (split == source.size() + 1 && triple) continue;
        std::vector<std::size_t> ends;
        if (split == source.size() + 1) {
          for (std::size_t i = 1; i <= source.size(); i++) ends.push_back(i);
        } else {
          ends.push_back(split);
          if (triple) ends.push_back(std::min(split + 1, source.size()));
          ends.push_back(source.size());
        }
        martianlabs::doba::tests::unit::test_helper::set_context(
            "input " + std::string(source) + ", split "
                + std::to_string(split) +
            (triple ? ", three fragments" : ", two fragments or bytewise"));
        test_decoder value;
        auto result = value.deserialize();
        std::size_t offset = 0;
        for (std::size_t end : ends) {
          DOBA_EXPECT_EQUAL(
              accumulate(
                  value, std::string_view(source).substr(offset, end - offset)),
              end - offset);
          offset = end;
          result = value.deserialize();
          DOBA_EXPECT(result.request == nullptr);
          if (result.code == deserialization_status::kInvalidSource) break;
          DOBA_EXPECT_EQUAL(result.code,
                            deserialization_status::kMoreBytesNeeded);
        }
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
        DOBA_EXPECT(result.request == nullptr);
      }
    }
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
  for (std::size_t split = 0; split <= source.size() + 1; split++) {
    for (bool triple : {false, true}) {
      if (split == source.size() + 1 && triple) continue;
      std::vector<std::size_t> ends;
      if (split == source.size() + 1) {
        for (std::size_t i = 1; i <= source.size(); i++) ends.push_back(i);
      } else {
        ends.push_back(split);
        if (triple) ends.push_back(std::min(split + 1, source.size()));
        ends.push_back(source.size());
      }
      martianlabs::doba::tests::unit::test_helper::set_context(
          "input " + std::string(source) + ", split " + std::to_string(split) +
          (triple ? ", three fragments" : ", two fragments or bytewise"));
      test_decoder value;
      auto result = value.deserialize();
      std::size_t offset = 0;
      for (std::size_t end : ends) {
        DOBA_EXPECT_EQUAL(
            accumulate(
                value, std::string_view(source).substr(offset, end - offset)),
            end - offset);
        offset = end;
        result = value.deserialize();
        DOBA_EXPECT(result.request == nullptr);
        if (result.code == deserialization_status::kInvalidSource) break;
        DOBA_EXPECT_EQUAL(result.code,
                          deserialization_status::kMoreBytesNeeded);
      }
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
      DOBA_EXPECT(result.request == nullptr);
    }
  }
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
    for (std::size_t split = 0; split <= source.size() + 1; split++) {
      for (bool triple : {false, true}) {
        if (split == source.size() + 1 && triple) continue;
        std::vector<std::size_t> ends;
        if (split == source.size() + 1) {
          for (std::size_t i = 1; i <= source.size(); i++) ends.push_back(i);
        } else {
          ends.push_back(split);
          if (triple) ends.push_back(std::min(split + 1, source.size()));
          ends.push_back(source.size());
        }
        martianlabs::doba::tests::unit::test_helper::set_context(
            "input " + std::string(source) + ", split "
                + std::to_string(split) +
            (triple ? ", three fragments" : ", two fragments or bytewise"));
        test_decoder value;
        auto result = value.deserialize();
        std::size_t offset = 0;
        for (std::size_t end : ends) {
          DOBA_EXPECT_EQUAL(
              accumulate(
                  value, std::string_view(source).substr(offset, end - offset)),
              end - offset);
          offset = end;
          result = value.deserialize();
          DOBA_EXPECT(result.request == nullptr);
          if (result.code == deserialization_status::kInvalidSource) break;
          DOBA_EXPECT_EQUAL(result.code,
                            deserialization_status::kMoreBytesNeeded);
        }
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
        DOBA_EXPECT(result.request == nullptr);
      }
    }
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
    for (std::size_t split = 0; split <= source.size() + 1; split++) {
      for (bool triple : {false, true}) {
        if (split == source.size() + 1 && triple) continue;
        std::vector<std::size_t> ends;
        if (split == source.size() + 1) {
          for (std::size_t i = 1; i <= source.size(); i++) ends.push_back(i);
        } else {
          ends.push_back(split);
          if (triple) ends.push_back(std::min(split + 1, source.size()));
          ends.push_back(source.size());
        }
        martianlabs::doba::tests::unit::test_helper::set_context(
            "input " + std::string(source) + ", split "
                + std::to_string(split) +
            (triple ? ", three fragments" : ", two fragments or bytewise"));
        test_decoder value;
        auto result = value.deserialize();
        std::size_t offset = 0;
        for (std::size_t end : ends) {
          DOBA_EXPECT_EQUAL(
              accumulate(
                  value, std::string_view(source).substr(offset, end - offset)),
              end - offset);
          offset = end;
          result = value.deserialize();
          DOBA_EXPECT(result.request == nullptr);
          if (result.code == deserialization_status::kInvalidSource) break;
          DOBA_EXPECT_EQUAL(result.code,
                            deserialization_status::kMoreBytesNeeded);
        }
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
        DOBA_EXPECT(result.request == nullptr);
        DOBA_EXPECT_EQUAL(result.reason, static_cast<int>(test.expected));
      }
    }
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

// +===========================================================================+
// | [>] valid Accept dispatch                                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Accept") {
  check_header("Accept",
               "text/plain", true);
}

// +===========================================================================+
// | [>] invalid Accept dispatch                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Accept") {
  check_header("Accept",
               "text/", false);
}

// +===========================================================================+
// | [>] valid Accept-Charset dispatch                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Accept-Charset") {
  check_header("Accept-Charset",
               "utf-8", true);
}

// +===========================================================================+
// | [>] invalid Accept-Charset dispatch                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Accept-Charset") {
  check_header("Accept-Charset",
               "utf-8;q=1.001", false);
}

// +===========================================================================+
// | [>] valid Accept-Encoding dispatch                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Accept-Encoding") {
  check_header("Accept-Encoding",
               "gzip", true);
}

// +===========================================================================+
// | [>] invalid Accept-Encoding dispatch                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Accept-Encoding") {
  check_header("Accept-Encoding",
               "gzip;q=1.1", false);
}

// +===========================================================================+
// | [>] valid Accept-Language dispatch                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Accept-Language") {
  check_header("Accept-Language",
               "en-US", true);
}

// +===========================================================================+
// | [>] invalid Accept-Language dispatch                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Accept-Language") {
  check_header("Accept-Language",
               "en--US", false);
}

// +===========================================================================+
// | [>] valid Accept-Ranges dispatch                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Accept-Ranges") {
  check_header("Accept-Ranges",
               "bytes", true);
}

// +===========================================================================+
// | [>] invalid Accept-Ranges dispatch                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Accept-Ranges") {
  check_header("Accept-Ranges",
               "byte range", false);
}

// +===========================================================================+
// | [>] valid Access-Control-Allow-Credentials dispatch         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Access-Control-Allow-Credentials") {
  check_header("Access-Control-Allow-Credentials",
               "true", true);
}

// +===========================================================================+
// | [>] invalid Access-Control-Allow-Credentials dispatch       ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Access-Control-Allow-Credentials") {
  check_header("Access-Control-Allow-Credentials",
               "false", false);
}

// +===========================================================================+
// | [>] valid Access-Control-Allow-Headers dispatch             ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Access-Control-Allow-Headers") {
  check_header("Access-Control-Allow-Headers",
               "Content-Type", true);
}

// +===========================================================================+
// | [>] invalid Access-Control-Allow-Headers dispatch           ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Access-Control-Allow-Headers") {
  check_header("Access-Control-Allow-Headers",
               "Content Type", false);
}

// +===========================================================================+
// | [>] valid Access-Control-Allow-Methods dispatch             ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Access-Control-Allow-Methods") {
  check_header("Access-Control-Allow-Methods",
               "GET", true);
}

// +===========================================================================+
// | [>] invalid Access-Control-Allow-Methods dispatch           ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Access-Control-Allow-Methods") {
  check_header("Access-Control-Allow-Methods",
               "GET POST", false);
}

// +===========================================================================+
// | [>] valid Access-Control-Allow-Origin dispatch              ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Access-Control-Allow-Origin") {
  check_header("Access-Control-Allow-Origin",
               "https://example.com", true);
}

// +===========================================================================+
// | [>] invalid Access-Control-Allow-Origin dispatch            ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Access-Control-Allow-Origin") {
  check_header("Access-Control-Allow-Origin",
               "https://example.com/path", false);
}

// +===========================================================================+
// | [>] valid Access-Control-Expose-Headers dispatch            ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Access-Control-Expose-Headers") {
  check_header("Access-Control-Expose-Headers",
               "ETag", true);
}

// +===========================================================================+
// | [>] invalid Access-Control-Expose-Headers dispatch          ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Access-Control-Expose-Headers") {
  check_header("Access-Control-Expose-Headers",
               "Content Length", false);
}

// +===========================================================================+
// | [>] valid Access-Control-Max-Age dispatch                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Access-Control-Max-Age") {
  check_header("Access-Control-Max-Age",
               "600", true);
}

// +===========================================================================+
// | [>] invalid Access-Control-Max-Age dispatch                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Access-Control-Max-Age") {
  check_header("Access-Control-Max-Age",
               "600s", false);
}

// +===========================================================================+
// | [>] valid Access-Control-Request-Headers dispatch           ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Access-Control-Request-Headers") {
  check_header("Access-Control-Request-Headers",
               "Content-Type", true);
}

// +===========================================================================+
// | [>] invalid Access-Control-Request-Headers dispatch         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Access-Control-Request-Headers") {
  check_header("Access-Control-Request-Headers",
               "Content Type", false);
}

// +===========================================================================+
// | [>] valid Access-Control-Request-Method dispatch            ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Access-Control-Request-Method") {
  check_header("Access-Control-Request-Method",
               "GET", true);
}

// +===========================================================================+
// | [>] invalid Access-Control-Request-Method dispatch          ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Access-Control-Request-Method") {
  check_header("Access-Control-Request-Method",
               "GET POST", false);
}

// +===========================================================================+
// | [>] valid Age dispatch                                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Age") {
  check_header("Age",
               "000", true);
}

// +===========================================================================+
// | [>] invalid Age dispatch                                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Age") {
  check_header("Age",
               "1s", false);
}

// +===========================================================================+
// | [>] valid Allow dispatch                                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Allow") {
  check_header("Allow",
               "GET", true);
}

// +===========================================================================+
// | [>] invalid Allow dispatch                                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Allow") {
  check_header("Allow",
               "GET POST", false);
}

// +===========================================================================+
// | [>] valid Authentication-Info dispatch                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Authentication-Info") {
  check_header("Authentication-Info",
               "nextnonce=\"abc\"", true);
}

// +===========================================================================+
// | [>] invalid Authentication-Info dispatch                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Authentication-Info") {
  check_header("Authentication-Info",
               "nextnonce=", false);
}

// +===========================================================================+
// | [>] valid Authorization dispatch                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Authorization") {
  check_header("Authorization",
               "Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==", true);
}

// +===========================================================================+
// | [>] invalid Authorization dispatch                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Authorization") {
  check_header("Authorization",
               "Basic \"abc\"", false);
}

// +===========================================================================+
// | [>] valid Cache-Control dispatch                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Cache-Control") {
  check_header("Cache-Control",
               "max-age=60", true);
}

// +===========================================================================+
// | [>] invalid Cache-Control dispatch                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Cache-Control") {
  check_header("Cache-Control",
               "max-age=", false);
}

// +===========================================================================+
// | [>] valid Content-Encoding dispatch                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Content-Encoding") {
  check_header("Content-Encoding",
               "gzip", true);
}

// +===========================================================================+
// | [>] invalid Content-Encoding dispatch                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Content-Encoding") {
  check_header("Content-Encoding",
               "g zip", false);
}

// +===========================================================================+
// | [>] valid Content-Language dispatch                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Content-Language") {
  check_header("Content-Language",
               "en-US", true);
}

// +===========================================================================+
// | [>] invalid Content-Language dispatch                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Content-Language") {
  check_header("Content-Language",
               "en--US", false);
}

// +===========================================================================+
// | [>] valid Content-Location dispatch                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Content-Location") {
  check_header("Content-Location",
               "/index.html", true);
}

// +===========================================================================+
// | [>] invalid Content-Location dispatch                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Content-Location") {
  check_header("Content-Location",
               "%GG", false);
}

// +===========================================================================+
// | [>] valid Content-Range dispatch                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Content-Range") {
  check_header("Content-Range",
               "bytes 0-0/1", true);
}

// +===========================================================================+
// | [>] invalid Content-Range dispatch                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Content-Range") {
  check_header("Content-Range",
               "bytes 0-1/", false);
}

// +===========================================================================+
// | [>] valid Content-Type dispatch                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Content-Type") {
  check_header("Content-Type",
               "text/plain", true);
}

// +===========================================================================+
// | [>] invalid Content-Type dispatch                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Content-Type") {
  check_header("Content-Type",
               "text", false);
}

// +===========================================================================+
// | [>] valid Cookie dispatch                                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Cookie") {
  check_header("Cookie",
               "a=b", true);
}

// +===========================================================================+
// | [>] invalid Cookie dispatch                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Cookie") {
  check_header("Cookie",
               "=b", false);
}

// +===========================================================================+
// | [>] valid Date dispatch                                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Date") {
  check_header("Date",
               "Sun, 06 Nov 1994 08:49:37 GMT", true);
}

// +===========================================================================+
// | [>] invalid Date dispatch                                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Date") {
  check_header("Date",
               "Sun, 06 Nov 1994 08:49:37 UTC", false);
}

// +===========================================================================+
// | [>] valid ETag dispatch                                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid ETag") {
  check_header("ETag",
               "\"abc\"", true);
}

// +===========================================================================+
// | [>] invalid ETag dispatch                                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid ETag") {
  check_header("ETag",
               "abc", false);
}

// +===========================================================================+
// | [>] valid Expires dispatch                                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Expires") {
  check_header("Expires",
               "Sun, 06 Nov 1994 08:49:37 GMT", true);
}

// +===========================================================================+
// | [>] invalid Expires dispatch                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Expires") {
  check_header("Expires",
               "Sun, 06 Nov 1994 08:49:37 UTC", false);
}

// +===========================================================================+
// | [>] valid From dispatch                                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid From") {
  check_header("From",
               "user@example.com", true);
}

// +===========================================================================+
// | [>] invalid From dispatch                                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid From") {
  check_header("From",
               "user@", false);
}

// +===========================================================================+
// | [>] valid If-Match dispatch                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid If-Match") {
  check_header("If-Match",
               "\"abc\"", true);
}

// +===========================================================================+
// | [>] invalid If-Match dispatch                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid If-Match") {
  check_header("If-Match",
               "abc", false);
}

// +===========================================================================+
// | [>] valid If-None-Match dispatch                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid If-None-Match") {
  check_header("If-None-Match",
               "\"abc\"", true);
}

// +===========================================================================+
// | [>] invalid If-None-Match dispatch                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid If-None-Match") {
  check_header("If-None-Match",
               "abc", false);
}

// +===========================================================================+
// | [>] valid If-Range dispatch                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid If-Range") {
  check_header("If-Range",
               "\"abc\"", true);
}

// +===========================================================================+
// | [>] invalid If-Range dispatch                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid If-Range") {
  check_header("If-Range",
               "abc", false);
}

// +===========================================================================+
// | [>] valid Keep-Alive dispatch                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Keep-Alive") {
  check_header("Keep-Alive",
               "timeout=5", true);
}

// +===========================================================================+
// | [>] invalid Keep-Alive dispatch                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Keep-Alive") {
  check_header("Keep-Alive",
               "timeout=", false);
}

// +===========================================================================+
// | [>] valid Last-Modified dispatch                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Last-Modified") {
  check_header("Last-Modified",
               "Sun, 06 Nov 1994 08:49:37 GMT", true);
}

// +===========================================================================+
// | [>] invalid Last-Modified dispatch                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Last-Modified") {
  check_header("Last-Modified",
               "Sun, 06 Nov 1994 08:49:37 UTC", false);
}

// +===========================================================================+
// | [>] valid Location dispatch                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Location") {
  check_header("Location",
               "/index.html", true);
}

// +===========================================================================+
// | [>] invalid Location dispatch                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Location") {
  check_header("Location",
               "%GG", false);
}

// +===========================================================================+
// | [>] valid Origin dispatch                                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Origin") {
  check_header("Origin",
               "https://example.com", true);
}

// +===========================================================================+
// | [>] invalid Origin dispatch                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Origin") {
  check_header("Origin",
               "https://example.com/path", false);
}

// +===========================================================================+
// | [>] valid Pragma dispatch                                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Pragma") {
  check_header("Pragma",
               "no-cache", true);
}

// +===========================================================================+
// | [>] invalid Pragma dispatch                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Pragma") {
  check_header("Pragma",
               "foo=", false);
}

// +===========================================================================+
// | [>] valid Proxy-Connection dispatch                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Proxy-Connection") {
  check_header("Proxy-Connection",
               "close", true);
}

// +===========================================================================+
// | [>] invalid Proxy-Connection dispatch                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Proxy-Connection") {
  check_header("Proxy-Connection",
               "keep alive", false);
}

// +===========================================================================+
// | [>] valid Range dispatch                                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Range") {
  check_header("Range",
               "bytes=0-499", true);
}

// +===========================================================================+
// | [>] invalid Range dispatch                                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Range") {
  check_header("Range",
               "bytes=0--1", false);
}

// +===========================================================================+
// | [>] valid Referer dispatch                                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Referer") {
  check_header("Referer",
               "/index.html", true);
}

// +===========================================================================+
// | [>] invalid Referer dispatch                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Referer") {
  check_header("Referer",
               "%GG", false);
}

// +===========================================================================+
// | [>] valid Retry-After dispatch                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Retry-After") {
  check_header("Retry-After",
               "120", true);
}

// +===========================================================================+
// | [>] invalid Retry-After dispatch                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Retry-After") {
  check_header("Retry-After",
               "120s", false);
}

// +===========================================================================+
// | [>] valid Sec-WebSocket-Accept dispatch                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Sec-WebSocket-Accept") {
  check_header("Sec-WebSocket-Accept",
               "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=", true);
}

// +===========================================================================+
// | [>] invalid Sec-WebSocket-Accept dispatch                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Sec-WebSocket-Accept") {
  check_header("Sec-WebSocket-Accept",
               "A===", false);
}

// +===========================================================================+
// | [>] valid Sec-WebSocket-Extensions dispatch                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Sec-WebSocket-Extensions") {
  check_header("Sec-WebSocket-Extensions",
               "permessage-deflate", true);
}

// +===========================================================================+
// | [>] invalid Sec-WebSocket-Extensions dispatch               ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Sec-WebSocket-Extensions") {
  check_header("Sec-WebSocket-Extensions",
               "extension;name=", false);
}

// +===========================================================================+
// | [>] valid Sec-WebSocket-Key dispatch                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Sec-WebSocket-Key") {
  check_header("Sec-WebSocket-Key",
               "dGhlIHNhbXBsZSBub25jZQ==", true);
}

// +===========================================================================+
// | [>] invalid Sec-WebSocket-Key dispatch                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Sec-WebSocket-Key") {
  check_header("Sec-WebSocket-Key",
               "A===", false);
}

// +===========================================================================+
// | [>] valid Sec-WebSocket-Protocol dispatch                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Sec-WebSocket-Protocol") {
  check_header("Sec-WebSocket-Protocol",
               "chat", true);
}

// +===========================================================================+
// | [>] invalid Sec-WebSocket-Protocol dispatch                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Sec-WebSocket-Protocol") {
  check_header("Sec-WebSocket-Protocol",
               "chat protocol", false);
}

// +===========================================================================+
// | [>] valid Sec-WebSocket-Version dispatch                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Sec-WebSocket-Version") {
  check_header("Sec-WebSocket-Version",
               "255", true);
}

// +===========================================================================+
// | [>] invalid Sec-WebSocket-Version dispatch                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Sec-WebSocket-Version") {
  check_header("Sec-WebSocket-Version",
               "256", false);
}

// +===========================================================================+
// | [>] valid Server dispatch                                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Server") {
  check_header("Server",
               "doba/1.0", true);
}

// +===========================================================================+
// | [>] invalid Server dispatch                                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Server") {
  check_header("Server",
               "doba/", false);
}

// +===========================================================================+
// | [>] valid Set-Cookie dispatch                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Set-Cookie") {
  check_header("Set-Cookie",
               "a=b", true);
}

// +===========================================================================+
// | [>] invalid Set-Cookie dispatch                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Set-Cookie") {
  check_header("Set-Cookie",
               "=b", false);
}

// +===========================================================================+
// | [>] valid User-Agent dispatch                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid User-Agent") {
  check_header("User-Agent",
               "doba/1.0", true);
}

// +===========================================================================+
// | [>] invalid User-Agent dispatch                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid User-Agent") {
  check_header("User-Agent",
               "doba/", false);
}

// +===========================================================================+
// | [>] valid Vary dispatch                                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Vary") {
  check_header("Vary",
               "Accept-Encoding", true);
}

// +===========================================================================+
// | [>] invalid Vary dispatch                                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Vary") {
  check_header("Vary",
               "Accept Encoding", false);
}

// +===========================================================================+
// | [>] valid WWW-Authenticate dispatch                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid WWW-Authenticate") {
  check_header("WWW-Authenticate",
               "Basic realm = \"x\"", true);
}

// +===========================================================================+
// | [>] invalid WWW-Authenticate dispatch                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid WWW-Authenticate") {
  check_header("WWW-Authenticate",
               "Basic =value", false);
}

// +===========================================================================+
// | [>] valid Trailer dispatch                                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Trailer") {
  check_header("Trailer",
               "X-Checksum", true,
               "POST / HTTP/1.1\r\n",
               "Transfer-Encoding: chunked\r\n",
               "0\r\nX-Checksum: abc\r\n\r\n");
}

// +===========================================================================+
// | [>] invalid Trailer dispatch                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Trailer") {
  check_header("Trailer",
               "Content Length", false,
               "POST / HTTP/1.1\r\n",
               "Transfer-Encoding: chunked\r\n",
               "0\r\nX-Checksum: abc\r\n\r\n");
}

// +===========================================================================+
// | [>] valid Upgrade dispatch                                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Upgrade") {
  check_header("Upgrade",
               "websocket", true,
               "GET / HTTP/1.1\r\n",
               "Connection: upgrade\r\n",
               "");
}

// +===========================================================================+
// | [>] invalid Upgrade dispatch                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Upgrade") {
  check_header("Upgrade",
               "HTTP/", false,
               "GET / HTTP/1.1\r\n",
               "Connection: upgrade\r\n",
               "");
}

// +===========================================================================+
// | [>] valid Max-Forwards dispatch                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Max-Forwards") {
  check_header("Max-Forwards",
               "10", true,
               "OPTIONS * HTTP/1.1\r\n",
               "",
               "");
}

// +===========================================================================+
// | [>] invalid Max-Forwards dispatch                           ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Max-Forwards") {
  check_header("Max-Forwards",
               "+1", false,
               "OPTIONS * HTTP/1.1\r\n",
               "",
               "");
}

// +===========================================================================+
// | [>] valid Via dispatch                                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Via") {
  check_header("Via",
               "1.1 proxy", true);
}

// +===========================================================================+
// | [>] invalid Via dispatch                                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Via") {
  check_header("Via",
               "1.1 proxy (unterminated", false);
}

// +===========================================================================+
// | [>] valid Forwarded dispatch                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid Forwarded") {
  check_header("Forwarded",
               "for=192.0.2.43;proto=http", true);
}

// +===========================================================================+
// | [>] invalid Forwarded dispatch                              ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid Forwarded") {
  check_header("Forwarded",
               "for=", false);
}

// +===========================================================================+
// | [>] valid X-Forwarded-For dispatch                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid X-Forwarded-For") {
  check_header("X-Forwarded-For",
               "192.0.2.1", true);
}

// +===========================================================================+
// | [>] invalid X-Forwarded-For dispatch                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid X-Forwarded-For") {
  check_header("X-Forwarded-For",
               "999.0.0.1", false);
}

// +===========================================================================+
// | [>] valid X-Forwarded-Host dispatch                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid X-Forwarded-Host") {
  check_header("X-Forwarded-Host",
               "example.com:80", true);
}

// +===========================================================================+
// | [>] invalid X-Forwarded-Host dispatch                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid X-Forwarded-Host") {
  check_header("X-Forwarded-Host",
               "example.com:http", false);
}

// +===========================================================================+
// | [>] valid X-Forwarded-Proto dispatch                        ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts valid X-Forwarded-Proto") {
  check_header("X-Forwarded-Proto",
               "https", true);
}

// +===========================================================================+
// | [>] invalid X-Forwarded-Proto dispatch                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder rejects invalid X-Forwarded-Proto") {
  check_header("X-Forwarded-Proto",
               "1http", false);
}

// +===========================================================================+
// | [>] unsupported expectation reason                          ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder reports unsupported expectations before body arrival") {
  constexpr std::string_view source =
      "POST / HTTP/1.1\r\nHost: example.com\r\n"
      "Content-Length: 1\r\nExpect: feature=on\r\n\r\n";
  test_decoder value;
  DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
  const auto result = value.deserialize();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
  DOBA_EXPECT_EQUAL(result.reason,
                    static_cast<int>(rejection_reason::kExpectationFailed));
  DOBA_EXPECT(result.request == nullptr);
  DOBA_EXPECT(result.interim.empty());
}

// +===========================================================================+
// | [>] conditional date formats                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts all conditional date formats") {
  constexpr std::string_view dates[] = {
      "Sun, 06 Nov 1994 08:49:37 GMT",
      "Sunday, 06-Nov-94 08:49:37 GMT",
      "Sun Nov  6 08:49:37 1994",
  };
  for (std::string_view name : {"If-Modified-Since", "If-Unmodified-Since"}) {
    for (std::string_view date : dates) {
      const std::string source =
          "GET / HTTP/1.1\r\nHost: example.com\r\n" +
          std::string(name) + ": " + std::string(date) + "\r\n\r\n";
      martianlabs::doba::tests::unit::test_helper::set_context(
          std::string(name) + ": " + std::string(date));
      test_decoder value;
      DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
      const auto result = value.deserialize();
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
      DOBA_EXPECT(result.request != nullptr);
      DOBA_EXPECT_EQUAL(result.request->get_header(name).second, date);
    }
  }
}

// +===========================================================================+
// | [>] complete head buffer boundaries                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts complete heads at the buffer boundary") {
  const std::string prefix =
      "GET / HTTP/1.1\r\nHost: example.com\r\nX-Pad: ";
  for (std::size_t size : {5119, 5120}) {
    const std::string padding(size - prefix.size() - 4, 'x');
    const std::string source = prefix + padding + "\r\n\r\n";
    test_decoder value;
    DOBA_EXPECT_EQUAL(source.size(), size);
    DOBA_EXPECT_EQUAL(accumulate(value, source), size);
    const auto result = value.deserialize();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
    DOBA_EXPECT(result.request != nullptr);
    DOBA_EXPECT_EQUAL(result.request->get_header("X-Pad").second, padding);
    DOBA_EXPECT_EQUAL(value.deserialize().code,
                      deserialization_status::kMoreBytesNeeded);
  }
}

// +===========================================================================+
// | [>] query pair count boundaries                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder preserves every query pair at the supported limit") {
  for (std::size_t count : {127, 128}) {
    std::string source = "GET /?";
    for (std::size_t i = 0; i < count; i++) {
      if (i != 0) source += '&';
      source += "k" + std::to_string(i) + "=v" + std::to_string(i);
    }
    source += " HTTP/1.1\r\nHost: example.com\r\n\r\n";
    DOBA_EXPECT(source.size() < 5120);
    test_decoder value;
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    const auto result = value.deserialize();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
    DOBA_EXPECT(result.request != nullptr);
    DOBA_EXPECT_EQUAL(result.request->get_query_parameters_length(), count);
    for (std::size_t i = 0; i < count; i++) {
      const auto parameter =
          result.request->get_query_parameter("k" + std::to_string(i));
      DOBA_EXPECT(parameter.has_value());
      DOBA_EXPECT_EQUAL(parameter->second, "v" + std::to_string(i));
    }
  }
}

// +===========================================================================+
// | [>] raw body before pipelined successor                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder preserves a pipelined successor after a raw body") {
  constexpr std::string_view head =
      "POST /first HTTP/1.1\r\nHost: example.com\r\n"
      "Content-Length: 7\r\n\r\n";
  constexpr std::string_view body = "payload";
  constexpr std::string_view next =
      "GET /next HTTP/1.1\r\nHost: next.example\r\n\r\n";
  const std::string source =
      std::string(head) + std::string(body) + std::string(next);
  for (std::size_t split = 0; split <= source.size(); split++) {
    test_decoder value;
    std::vector<std::shared_ptr<request>> requests;
    std::size_t offset = 0;
    for (std::size_t end : {split, source.size()}) {
      DOBA_EXPECT_EQUAL(
          accumulate(
              value, std::string_view(source).substr(offset, end - offset)),
          end - offset);
      offset = end;
      for (std::size_t dispatch = 0; dispatch < 3; dispatch++) {
        const auto result = value.deserialize();
        if (result.code == deserialization_status::kMoreBytesNeeded) break;
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
        DOBA_EXPECT(result.request != nullptr);
        requests.push_back(result.request);
      }
    }
    DOBA_EXPECT_EQUAL(requests.size(), 2);
    DOBA_EXPECT_EQUAL(requests[0]->get_absolute_path(), "/first");
    DOBA_EXPECT(requests[0]->has_body_reader());
    std::array<std::byte, 8> output{};
    const auto state = requests[0]->get_body_reader()->read(output);
    DOBA_EXPECT(!state.has_error);
    DOBA_EXPECT(state.complete);
    DOBA_EXPECT_EQUAL(state.produced, 7);
    DOBA_EXPECT_EQUAL(
        std::string_view(reinterpret_cast<const char*>(output.data()),
                         state.produced), "payload");
    DOBA_EXPECT_EQUAL(requests[1]->get_absolute_path(), "/next");
    DOBA_EXPECT_EQUAL(requests[1]->get_header("Host").second, "next.example");
    DOBA_EXPECT(!requests[1]->has_body_reader());
    DOBA_EXPECT_EQUAL(value.deserialize().code,
                      deserialization_status::kMoreBytesNeeded);
  }
}

// +===========================================================================+
// | [>] chunked body before pipelined successor                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder preserves a pipelined successor after a chunked body") {
  constexpr std::string_view head =
      "POST /first HTTP/1.1\r\nHost: example.com\r\n"
      "Transfer-Encoding: chunked\r\n\r\n";
  constexpr std::string_view body = "3\r\nabc\r\n0\r\n\r\n";
  constexpr std::string_view next =
      "GET /next HTTP/1.1\r\nHost: next.example\r\n\r\n";
  const std::string source =
      std::string(head) + std::string(body) + std::string(next);
  for (std::size_t split = 0; split <= source.size(); split++) {
    test_decoder value;
    std::vector<std::shared_ptr<request>> requests;
    std::size_t offset = 0;
    for (std::size_t end : {split, source.size()}) {
      DOBA_EXPECT_EQUAL(
          accumulate(
              value, std::string_view(source).substr(offset, end - offset)),
          end - offset);
      offset = end;
      for (std::size_t dispatch = 0; dispatch < 3; dispatch++) {
        const auto result = value.deserialize();
        if (result.code == deserialization_status::kMoreBytesNeeded) break;
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
        DOBA_EXPECT(result.request != nullptr);
        requests.push_back(result.request);
      }
    }
    DOBA_EXPECT_EQUAL(requests.size(), 2);
    DOBA_EXPECT_EQUAL(requests[0]->get_absolute_path(), "/first");
    DOBA_EXPECT(requests[0]->has_body_reader());
    std::array<std::byte, 8> output{};
    const auto state = requests[0]->get_body_reader()->read(output);
    DOBA_EXPECT(!state.has_error);
    DOBA_EXPECT(state.complete);
    DOBA_EXPECT_EQUAL(state.produced, 3);
    DOBA_EXPECT_EQUAL(
        std::string_view(reinterpret_cast<const char*>(output.data()),
                         state.produced), "abc");
    DOBA_EXPECT_EQUAL(requests[1]->get_absolute_path(), "/next");
    DOBA_EXPECT_EQUAL(requests[1]->get_header("Host").second, "next.example");
    DOBA_EXPECT(!requests[1]->has_body_reader());
    DOBA_EXPECT_EQUAL(value.deserialize().code,
                      deserialization_status::kMoreBytesNeeded);
  }
}

// +===========================================================================+
// | [>] bytewise target forms                                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts every target form byte by byte") {
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
    for (std::size_t i = 0; i < test.source.size(); i++) {
      DOBA_EXPECT_EQUAL(accumulate(value, test.source.substr(i, 1)), 1);
      const auto result = value.deserialize();
      if (i + 1 < test.source.size()) {
        DOBA_EXPECT_EQUAL(result.code,
                          deserialization_status::kMoreBytesNeeded);
        DOBA_EXPECT(result.request == nullptr);
      } else {
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
        DOBA_EXPECT(result.request != nullptr);
        DOBA_EXPECT_EQUAL(result.request->get_method(), test.method);
        DOBA_EXPECT_EQUAL(result.request->get_target(), test.target_form);
        DOBA_EXPECT_EQUAL(result.request->get_absolute_path(), test.path);
      }
    }
  }
}

// +===========================================================================+
// | [>] raw body spill boundaries                               ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder preserves raw bodies across the spill threshold") {
  for (std::size_t size : {65534, 65535, 65536}) {
    std::string payload(size, '\0');
    for (std::size_t i = 0; i < payload.size(); i++) {
      payload[i] = static_cast<char>((i * 43 + i / 251) % 256);
    }
    const std::string source =
        "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: " +
        std::to_string(payload.size()) + "\r\n\r\n" + payload;
    test_decoder value;
    std::shared_ptr<request> request_value;
    for (std::size_t offset = 0; offset < source.size();) {
      const std::size_t count =
          std::min(std::size_t{4096}, source.size() - offset);
      DOBA_EXPECT_EQUAL(
          accumulate(
              value, std::string_view(source).substr(offset, count)), count);
      offset += count;
      const auto result = value.deserialize();
      if (offset < source.size()) {
        DOBA_EXPECT_EQUAL(result.code,
                          deserialization_status::kMoreBytesNeeded);
        DOBA_EXPECT(result.request == nullptr);
      } else {
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
        request_value = result.request;
      }
    }
    DOBA_EXPECT(request_value != nullptr);
    DOBA_EXPECT(request_value->has_body_reader());
    std::array<std::byte, 4096> output{};
    std::string decoded;
    bool complete = false;
    for (std::size_t i = 0; !complete && i <= size; i++) {
      const auto state = request_value->get_body_reader()->read(output);
      DOBA_EXPECT(!state.has_error);
      decoded.append(reinterpret_cast<const char*>(output.data()),
                     state.produced);
      complete = state.complete;
    }
    DOBA_EXPECT(complete);
    DOBA_EXPECT_EQUAL(decoded, payload);
  }
}

// +===========================================================================+
// | [>] encoded chunked body spill boundaries                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder preserves encoded chunked bodies across the spill threshold") {
  for (std::size_t size : {65534, 65535, 65536}) {
    std::string payload(size - 13, '\0');
    for (std::size_t i = 0; i < payload.size(); i++) {
      payload[i] = static_cast<char>((i * 43 + i / 251) % 256);
    }
    std::array<char, 16> chunk_size{};
    const auto encoded = std::to_chars(
        chunk_size.data(), chunk_size.data() + chunk_size.size(),
        payload.size(), 16);
    DOBA_EXPECT_EQUAL(encoded.ec, std::errc{});
    const std::string body =
        std::string(chunk_size.data(), encoded.ptr) + "\r\n" +
        payload + "\r\n0\r\n\r\n";
    DOBA_EXPECT_EQUAL(body.size(), size);
    const std::string source =
        "POST / HTTP/1.1\r\nHost: example.com\r\n"
        "Transfer-Encoding: chunked\r\n\r\n" + body;
    test_decoder value;
    std::shared_ptr<request> request_value;
    for (std::size_t offset = 0; offset < source.size();) {
      const std::size_t count =
          std::min(std::size_t{4096}, source.size() - offset);
      DOBA_EXPECT_EQUAL(
          accumulate(
              value, std::string_view(source).substr(offset, count)), count);
      offset += count;
      const auto result = value.deserialize();
      if (offset < source.size()) {
        DOBA_EXPECT_EQUAL(result.code,
                          deserialization_status::kMoreBytesNeeded);
        DOBA_EXPECT(result.request == nullptr);
      } else {
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
        request_value = result.request;
      }
    }
    DOBA_EXPECT(request_value != nullptr);
    DOBA_EXPECT(request_value->has_body_reader());
    std::array<std::byte, 4096> output{};
    std::string decoded;
    bool complete = false;
    for (std::size_t i = 0; !complete && i <= size; i++) {
      const auto state = request_value->get_body_reader()->read(output);
      DOBA_EXPECT(!state.has_error);
      decoded.append(reinterpret_cast<const char*>(output.data()),
                     state.produced);
      complete = state.complete;
    }
    DOBA_EXPECT(complete);
    DOBA_EXPECT_EQUAL(decoded, payload);
  }
}
