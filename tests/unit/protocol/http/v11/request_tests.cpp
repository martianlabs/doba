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
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "common/writer.h"
#include "protocol/http/v11/request.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::writer;
using martianlabs::doba::protocol::http::header_view;
using martianlabs::doba::protocol::http::helpers;
using martianlabs::doba::protocol::http::query_parameter_view;
using martianlabs::doba::protocol::http::target;
using martianlabs::doba::protocol::http::v11::request;

std::string_view part(const std::string& source, std::string_view value) {
  const auto offset = source.find(value);
  return std::string_view(source).substr(offset, value.size());
}
}  // namespace

// +===========================================================================+
// | [>] request is neither copyable nor movable                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("request is neither copyable nor movable") {
  static_assert(!std::is_copy_constructible_v<request>);
  static_assert(!std::is_copy_assignable_v<request>);
  static_assert(!std::is_move_constructible_v<request>);
  static_assert(!std::is_move_assignable_v<request>);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] factory exposes every request component                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("factory exposes every request component") {
  const std::string source =
      "GET /a%20b?x=1 HTTP/1.1\r\nHost: example.com:8080\r\n"
      "X-Test: value\r\nCookie: a=1; b=two=2\r\n\r\n";
  const auto getter = request::from(
      source, part(source, "GET"), part(source, "/a%20b"), target::kOriginForm,
      std::vector<header_view>{
          {part(source, "Host"), part(source, "example.com:8080")},
          {part(source, "X-Test"), part(source, "value")},
          {part(source, "Cookie"), part(source, "a=1; b=two=2")}},
      std::vector<query_parameter_view>{{part(source, "x"), part(source, "1")}},
      part(source, "example.com"), part(source, "8080"),
      helpers::host_type::kRegName, std::nullopt, std::nullopt, std::nullopt,
      false, 0, true);
  const auto value = getter(std::nullopt);
  DOBA_EXPECT_EQUAL(value->get_method(), "GET");
  DOBA_EXPECT_EQUAL(value->get_target(), target::kOriginForm);
  DOBA_EXPECT_EQUAL(value->get_absolute_path(), "/a b");
  DOBA_EXPECT_EQUAL(value->get_headers_length(), 3);
  DOBA_EXPECT_EQUAL(value->get_header(0).first, "Host");
  DOBA_EXPECT_EQUAL(value->get_header("host").second, "example.com:8080");
  DOBA_EXPECT(value->exist_header("X-TEST"));
  DOBA_EXPECT(!value->exist_header("Missing"));
  DOBA_EXPECT_EQUAL(value->get_query_parameters_length(), 1);
  DOBA_EXPECT_EQUAL(value->get_query_parameter(0).second, "1");
  DOBA_EXPECT(value->get_query_parameter("x").has_value());
  DOBA_EXPECT(!value->get_query_parameter("X").has_value());
  DOBA_EXPECT(value->has_host());
  DOBA_EXPECT_EQUAL(value->get_host(), "example.com");
  DOBA_EXPECT_EQUAL(value->get_host_port(), "8080");
  DOBA_EXPECT_EQUAL(value->get_host_type(), helpers::host_type::kRegName);
  DOBA_EXPECT(!value->has_target_authority());
  DOBA_EXPECT(!value->has_body_reader());
  DOBA_EXPECT(value->wants_connection_close());
}
// +===========================================================================+
// | [>] missing named header throws out of range                ( test-case ) |
// +===========================================================================+
DOBA_TEST("missing named header throws out of range") {
  const std::string source = "GET / HTTP/1.1\r\n\r\n";
  const auto value = request::from(
      source, part(source, "GET"), part(source, "/"), target::kOriginForm, {},
      {}, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt,
      std::nullopt)(std::nullopt);
  bool threw = false;
  try {
    value->get_header("Missing");
  } catch (const std::out_of_range&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
}
// +===========================================================================+
// | [>] cookie accessors preserve order values and exact names  ( test-case ) |
// +===========================================================================+
DOBA_TEST("cookie accessors preserve order values and exact names") {
  const std::string source =
      "GET / HTTP/1.1\r\nCookie: a=1; b=two=2; empty=; ignored\r\n\r\n";
  const auto value = request::from(
      source, part(source, "GET"), part(source, "/"), target::kOriginForm,
      {{part(source, "Cookie"), part(source, "a=1; b=two=2; empty=; ignored")}},
      {}, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt,
      std::nullopt)(std::nullopt);
  DOBA_EXPECT_EQUAL(*value->get_cookie("a"), "1");
  DOBA_EXPECT_EQUAL(*value->get_cookie("b"), "two=2");
  DOBA_EXPECT(value->get_cookie("empty").has_value());
  DOBA_EXPECT(value->get_cookie("empty")->empty());
  DOBA_EXPECT(!value->get_cookie("A").has_value());
  DOBA_EXPECT(!value->get_cookie("missing").has_value());
  const auto cookies = value->get_cookies();
  DOBA_EXPECT_EQUAL(cookies.size(), 3);
  DOBA_EXPECT_EQUAL(cookies[0].first, "a");
  DOBA_EXPECT_EQUAL(cookies[1].second, "two=2");
  DOBA_EXPECT_EQUAL(cookies[2].first, "empty");
}
// +===========================================================================+
// | [>] absent cookie header returns empty results              ( test-case ) |
// +===========================================================================+
DOBA_TEST("absent cookie header returns empty results") {
  const std::string source = "GET / HTTP/1.1\r\n\r\n";
  const auto value = request::from(
      source, part(source, "GET"), part(source, "/"), target::kOriginForm, {},
      {}, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt,
      std::nullopt)(std::nullopt);
  DOBA_EXPECT(!value->get_cookie("a").has_value());
  DOBA_EXPECT(value->get_cookies().empty());
}
// +===========================================================================+
// | [>] empty components remain valid empty views               ( test-case ) |
// +===========================================================================+
DOBA_TEST("empty components remain valid empty views") {
  const std::string source;
  const auto value = request::from(
      source, {}, {}, target::kUnknown, {}, {}, std::nullopt, std::nullopt,
      std::nullopt, std::nullopt, std::nullopt, std::nullopt)(std::nullopt);
  DOBA_EXPECT(value->get_method().empty());
  DOBA_EXPECT(value->get_absolute_path().empty());
  DOBA_EXPECT_EQUAL(value->get_headers_length(), 0);
  DOBA_EXPECT_EQUAL(value->get_query_parameters_length(), 0);
  DOBA_EXPECT(!value->has_host());
  DOBA_EXPECT(!value->has_target_authority());
  DOBA_EXPECT(!value->has_body_reader());
}
// +===========================================================================+
// | [>] cookie parsing remains bounded on malformed input       ( test-case ) |
// +===========================================================================+
DOBA_TEST("cookie parsing remains bounded on malformed input") {
  const std::string source =
      "GET / HTTP/1.1\r\nCookie: a=1;bad; b=2; =empty; tail=3\r\n\r\n";
  const auto value = request::from(
      source, part(source, "GET"), part(source, "/"), target::kOriginForm,
      {{part(source, "Cookie"), part(source, "a=1;bad; b=2; =empty; tail=3")}},
      {}, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt,
      std::nullopt)(std::nullopt);
  const auto cookies = value->get_cookies();
  DOBA_EXPECT_EQUAL(cookies.size(), 4);
  DOBA_EXPECT_EQUAL(cookies[0].first, "a");
  DOBA_EXPECT_EQUAL(cookies[0].second, "1;bad");
  DOBA_EXPECT_EQUAL(cookies[1].first, "b");
  DOBA_EXPECT(cookies[2].first.empty());
  DOBA_EXPECT_EQUAL(cookies[3].first, "tail");
  DOBA_EXPECT_EQUAL(*value->get_cookie("tail"), "3");
}
// +===========================================================================+
// | [>] raw and chunked storage mount matching body readers     ( test-case ) |
// +===========================================================================+
DOBA_TEST("raw and chunked storage mount matching body readers") {
  const std::string source = "POST / HTTP/1.1\r\n\r\n";
  auto raw_getter = request::from(
      source, part(source, "POST"), part(source, "/"), target::kOriginForm, {},
      {}, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt,
      std::nullopt, false, 3);
  writer raw_storage;
  DOBA_EXPECT(raw_storage.write("abc"));
  const auto raw = raw_getter(raw_storage.release());
  DOBA_EXPECT(raw->has_body_reader());
  std::array<std::byte, 8> output{};
  auto state = raw->get_body_reader()->read(output);
  DOBA_EXPECT(state.complete);
  DOBA_EXPECT_EQUAL(state.produced, 3);
  DOBA_EXPECT_EQUAL(
      std::string_view(reinterpret_cast<const char*>(output.data()), 3), "abc");
  auto chunked_getter = request::from(
      source, part(source, "POST"), part(source, "/"), target::kOriginForm, {},
      {}, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt,
      std::nullopt, true);
  writer chunked_storage;
  DOBA_EXPECT(chunked_storage.write("3\r\nabc\r\n0\r\n\r\n"));
  const auto chunked = chunked_getter(chunked_storage.release());
  state = chunked->get_body_reader()->read(output);
  DOBA_EXPECT(state.complete);
  DOBA_EXPECT_EQUAL(state.produced, 3);
}
// +===========================================================================+
// | [>] request views remain independent from the source buffer ( test-case ) |
// +===========================================================================+
DOBA_TEST("request views remain independent from the source buffer") {
  std::string source =
      "GET /?name=value HTTP/1.1\r\nHost: example.com\r\nX: y\r\n\r\n";
  const auto value = request::from(
      source, part(source, "GET"), part(source, "/"), target::kOriginForm,
      {{part(source, "Host"), part(source, "example.com")},
       {part(source, "X"), part(source, "y")}},
      {{part(source, "name"), part(source, "value")}},
      part(source, "example.com"), std::nullopt, helpers::host_type::kRegName,
      std::nullopt, std::nullopt, std::nullopt)(std::nullopt);
  source.assign(source.size(), 'x');
  DOBA_EXPECT_EQUAL(value->get_method(), "GET");
  DOBA_EXPECT_EQUAL(value->get_absolute_path(), "/");
  DOBA_EXPECT(value->exist_header("Host"));
  DOBA_EXPECT_EQUAL(value->get_header("Host").second, "example.com");
  DOBA_EXPECT_EQUAL(value->get_query_parameter("name")->second, "value");
  DOBA_EXPECT_EQUAL(value->get_host(), "example.com");
}
// +===========================================================================+
// | [>] factory rejects components outside source buffer         ( test-case ) |
// +===========================================================================+
DOBA_TEST("factory rejects components outside the source buffer") {
  const std::string source = "GET / HTTP/1.1\r\n\r\n";
  const std::string foreign = "GET";
  bool threw = false;
  try {
    request::from(source, foreign, part(source, "/"), target::kOriginForm, {},
                  {}, std::nullopt, std::nullopt, std::nullopt, std::nullopt,
                  std::nullopt, std::nullopt);
  } catch (const std::invalid_argument& error) {
    threw = std::string_view(error.what()) ==
            "request component is outside full buffer";
  }
  DOBA_EXPECT(threw);
}
