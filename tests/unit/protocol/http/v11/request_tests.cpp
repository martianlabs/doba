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
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include "protocol/http/v11/decoder.h"
#include "protocol/http/v11/request.h"
#include "protocol/http/v11/response.h"
#include "test_helper.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] usings                                                     ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using martianlabs::doba::protocol::deserialization_status;
using martianlabs::doba::protocol::http::helpers;
using martianlabs::doba::protocol::http::target;
using martianlabs::doba::protocol::http::v11::decoder;
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] with_request                                             ( function ) |
// +---------------------------------------------------------------------------+
// | This function deserializes a request and invokes the provided callback    |
// | with the decoded request. It asserts that the deserialization succeeds    |
// | and that the callback is called.                                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename FNty>
void with_request(std::string_view wire, FNty&& check) {
  decoder<request, response> value;
  std::size_t consumed = 0;
  bool called = false;
  const auto result = value.deserialize(wire.data(), wire.size(), wire.size(),
                                        consumed, [&](const request& decoded) {
                                          called = true;
                                          check(decoded);
                                        });
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
  DOBA_EXPECT_EQUAL(consumed, wire.size());
  DOBA_EXPECT(called);
}
}  // namespace

// +===========================================================================+
// | [>] request is neither copyable nor movable                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("request is neither copyable nor movable") {
  static_assert(!std::is_copy_constructible_v<request>);
  static_assert(!std::is_copy_assignable_v<request>);
  static_assert(!std::is_move_constructible_v<request>);
  static_assert(!std::is_move_assignable_v<request>);
  DOBA_EXPECT(true);
}

// +===========================================================================+
// | [>] request exposes decoded components                      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("request exposes decoded components") {
  constexpr std::string_view wire =
      "GET /a%20b?x=1 HTTP/1.1\r\nHost: example.com:8080\r\n"
      "X-Test: value\r\nCookie: a=1; b=two=2\r\n"
      "Connection: close\r\n\r\n";
  with_request(wire, [](const request& value) {
    DOBA_EXPECT_EQUAL(value.get_method(), "GET");
    DOBA_EXPECT_EQUAL(value.get_target(), target::kOriginForm);
    DOBA_EXPECT_EQUAL(value.get_absolute_path(), "/a b");
    DOBA_EXPECT_EQUAL(value.get_headers_length(), 4);
    DOBA_EXPECT_EQUAL(value.get_header(0).first, "Host");
    DOBA_EXPECT_EQUAL(value.get_header("host").second, "example.com:8080");
    DOBA_EXPECT(value.exist_header("X-TEST"));
    DOBA_EXPECT(!value.exist_header("Missing"));
    DOBA_EXPECT_EQUAL(value.get_query_parameters_length(), 1);
    DOBA_EXPECT_EQUAL(value.get_query_parameter(0).second, "1");
    DOBA_EXPECT(value.get_query_parameter("x").has_value());
    DOBA_EXPECT(!value.get_query_parameter("X").has_value());
    DOBA_EXPECT(value.has_host());
    DOBA_EXPECT_EQUAL(value.get_host(), "example.com");
    DOBA_EXPECT_EQUAL(value.get_host_port(), "8080");
    DOBA_EXPECT_EQUAL(value.get_host_type(), helpers::host_type::kRegName);
    DOBA_EXPECT(!value.has_target_authority());
    DOBA_EXPECT(!value.has_body_reader());
    DOBA_EXPECT(value.wants_connection_close());
  });
}

// +===========================================================================+
// | [>] missing named header throws out of range                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("missing named header throws out of range") {
  with_request("GET / HTTP/1.1\r\nHost: a\r\n\r\n", [](const request& value) {
    bool threw = false;
    try {
      value.get_header("Missing");
    } catch (const std::out_of_range&) {
      threw = true;
    }
    DOBA_EXPECT(threw);
  });
}

// +===========================================================================+
// | [>] cookie accessors preserve order values and exact names  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("cookie accessors preserve order values and exact names") {
  with_request(
      "GET / HTTP/1.1\r\nHost: a\r\n"
      "Cookie: a=1; b=two=2; empty=\r\n\r\n",
      [](const request& value) {
        DOBA_EXPECT_EQUAL(*value.get_cookie("a"), "1");
        DOBA_EXPECT_EQUAL(*value.get_cookie("b"), "two=2");
        DOBA_EXPECT(value.get_cookie("empty").has_value());
        DOBA_EXPECT(value.get_cookie("empty")->empty());
        DOBA_EXPECT(!value.get_cookie("A").has_value());
        DOBA_EXPECT(!value.get_cookie("missing").has_value());
        const auto cookies = value.get_cookies();
        DOBA_EXPECT_EQUAL(cookies.size(), 3);
        DOBA_EXPECT_EQUAL(cookies[0].first, "a");
        DOBA_EXPECT_EQUAL(cookies[1].second, "two=2");
        DOBA_EXPECT_EQUAL(cookies[2].first, "empty");
      });
}

// +===========================================================================+
// | [>] absent cookie header returns empty results              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("absent cookie header returns empty results") {
  with_request("GET / HTTP/1.1\r\nHost: a\r\n\r\n", [](const request& value) {
    DOBA_EXPECT(!value.get_cookie("a").has_value());
    DOBA_EXPECT(value.get_cookies().empty());
    DOBA_EXPECT_EQUAL(value.get_query_parameters_length(), 0);
    DOBA_EXPECT(!value.has_body_reader());
  });
}

// +===========================================================================+
// | [>] malformed cookie syntax is rejected                     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("malformed cookie syntax is rejected") {
  const std::string wire =
      "GET / HTTP/1.1\r\nHost: a\r\n"
      "Cookie: a=1;bad; b=2; =empty; tail=3\r\n\r\n";
  decoder<request, response> value;
  std::size_t consumed = 0;
  bool called = false;
  const auto result =
      value.deserialize(wire.data(), wire.size(), wire.size(), consumed,
                        [&](const request&) { called = true; });
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
  DOBA_EXPECT(!called);
}

// +===========================================================================+
// | [>] raw and chunked requests expose decoded payload         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("raw and chunked requests expose decoded payload") {
  for (const bool chunked : {false, true}) {
    const std::string wire = std::string("POST / HTTP/1.1\r\nHost: a\r\n") +
                             (chunked ? "Transfer-Encoding: chunked\r\n\r\n"
                                      : "Content-Length: 3\r\n\r\n") +
                             (chunked ? "3\r\nabc\r\n0\r\n\r\n" : "abc");
    with_request(wire, [](const request& value) {
      DOBA_EXPECT(value.has_body_reader());
      DOBA_EXPECT(value.get_body_reader() != nullptr);
      std::array<std::byte, 8> output{};
      const auto state = value.get_body_reader()->read(output);
      DOBA_EXPECT(!state.has_error);
      DOBA_EXPECT(state.complete);
      DOBA_EXPECT_EQUAL(state.produced, 3);
      DOBA_EXPECT_EQUAL(
          std::string_view(reinterpret_cast<const char*>(output.data()), 3),
          "abc");
    });
  }
}

// +===========================================================================+
// | [>] body reader can remain unread                           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("body reader can remain unread") {
  with_request(
      "POST / HTTP/1.1\r\nHost: a\r\n"
      "Transfer-Encoding: chunked\r\n\r\n3\r\nabc\r\n0\r\n\r\n",
      [](const request& value) { DOBA_EXPECT(value.has_body_reader()); });
}

// +===========================================================================+
// | [>] views refer to decoder storage                          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("request views remain valid when transport input changes") {
  std::string wire =
      "GET /?name=value HTTP/1.1\r\nHost: example.com\r\nX: y\r\n\r\n";
  decoder<request, response> value;
  std::size_t consumed = 0;
  bool called = false;
  const auto result = value.deserialize(
      wire.data(), wire.size(), wire.size(), consumed,
      [&](const request& decoded) {
        called = true;
        wire.assign(wire.size(), 'x');
        DOBA_EXPECT_EQUAL(decoded.get_method(), "GET");
        DOBA_EXPECT_EQUAL(decoded.get_absolute_path(), "/");
        DOBA_EXPECT_EQUAL(decoded.get_header("Host").second, "example.com");
        DOBA_EXPECT_EQUAL(decoded.get_header("X").second, "y");
        DOBA_EXPECT_EQUAL(decoded.get_query_parameter("name")->second, "value");
      });
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
  DOBA_EXPECT(called);
}

// +===========================================================================+
// | [>] absolute authority views are available in callback      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("absolute authority views are available in callback") {
  with_request(
      "GET http://target.example:81/a%252F%2Fb?key=unchanged HTTP/1.1\r\n"
      "Host: source.example:82\r\nX-Tail: preserved\r\n\r\n",
      [](const request& value) {
        DOBA_EXPECT_EQUAL(value.get_absolute_path(), "/a%2F/b");
        DOBA_EXPECT_EQUAL(value.get_target(), target::kAbsoluteForm);
        DOBA_EXPECT_EQUAL(value.get_header(1).first, "X-Tail");
        DOBA_EXPECT_EQUAL(value.get_header(1).second, "preserved");
        DOBA_EXPECT_EQUAL(value.get_query_parameter("key")->second,
                          "unchanged");
        DOBA_EXPECT_EQUAL(value.get_host(), "source.example");
        DOBA_EXPECT_EQUAL(value.get_host_port(), "82");
        DOBA_EXPECT(value.has_target_authority());
        DOBA_EXPECT_EQUAL(value.get_target_authority_host(), "target.example");
        DOBA_EXPECT_EQUAL(value.get_target_authority_port(), "81");
        DOBA_EXPECT_EQUAL(value.get_target_authority_type(),
                          helpers::host_type::kRegName);
      });
}

// +===========================================================================+
// | [>] duplicate cookies retain the first exact name           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("duplicate cookies retain the first exact name") {
  with_request(
      "GET / HTTP/1.1\r\nHost: a\r\n"
      "Cookie: sid=first; SID=upper; sid=last\r\n\r\n",
      [](const request& value) {
        DOBA_EXPECT_EQUAL(*value.get_cookie("sid"), "first");
        DOBA_EXPECT_EQUAL(*value.get_cookie("SID"), "upper");
        const auto cookies = value.get_cookies();
        DOBA_EXPECT_EQUAL(cookies.size(), 3);
        DOBA_EXPECT_EQUAL(cookies[2].second, "last");
      });
}
