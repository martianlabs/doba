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

#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include "common/reader.h"
#include "protocol/http/v11/response.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::reader;
using martianlabs::doba::protocol::http::v11::limits;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::protocol::http::v11::body::body_writer;

std::string read_source(reader& source) {
  std::string output;
  source.read_all(output);
  return output;
}
}  // namespace

// +===========================================================================+
// | [>] response is movable but not copyable                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("response is movable but not copyable") {
  static_assert(!std::is_default_constructible_v<response>);
  static_assert(!std::is_constructible_v<response, std::string_view, int>);
  static_assert(!std::is_copy_constructible_v<response>);
  static_assert(!std::is_copy_assignable_v<response>);
  static_assert(std::is_nothrow_move_constructible_v<response>);
  static_assert(std::is_nothrow_move_assignable_v<response>);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] moving preserves response state and owned body writers  ( test-case ) |
// +===========================================================================+
DOBA_TEST("moving preserves response state and owned body writers") {
  response source = response::created_201();
  auto writer = body_writer::chunked();
  DOBA_EXPECT(writer.write("body"));
  source.set_header("Date", "fixed")
      .add_header("X-Test", "value")
      .set_body(std::move(writer));
  response constructed(std::move(source));
  DOBA_EXPECT_EQUAL(constructed.get_header("X-Test").second, "value");
  response assigned = response::bad_request_400();
  assigned.set_body("replaced");
  assigned = std::move(constructed);
  auto serialized = assigned.serialize();
  const std::string serialized_prefix(serialized->prefix.get(),
                                      serialized->prefix_size);
  DOBA_EXPECT(serialized_prefix.starts_with("HTTP/1.1 201 Created\r\n"));
  DOBA_EXPECT_EQUAL(read_source(*serialized->source),
                    "4\r\nbody\r\n0\r\n\r\n");
}
// +===========================================================================+
// | [>] moves preserve the full in-memory body                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("moves preserve the full in-memory body") {
  std::string body(limits::kMaxResponseBodySizeInMemory, '\0');
  for (std::size_t i = 0; i < body.size(); i++) {
    body[i] = static_cast<char>(i % 256);
  }
  response source = response::created_201();
  source.set_header("Date", "fixed").set_body(body);
  response constructed(std::move(source));
  response assigned = response::ok_200();
  assigned.set_body("replaced");
  assigned = std::move(constructed);
  response& alias = assigned;
  assigned = std::move(alias);
  DOBA_EXPECT(!assigned.has_header("Content-Length"));
  auto serialized = assigned.serialize();
  const std::string serialized_prefix(serialized->prefix.get(),
                                      serialized->prefix_size);
  DOBA_EXPECT(serialized_prefix.starts_with("HTTP/1.1 201 Created\r\n"));
  DOBA_EXPECT(serialized_prefix.find("Content-Length: " +
                                     std::to_string(body.size()) + "\r\n") !=
              std::string::npos);
  DOBA_EXPECT(serialized_prefix.ends_with(body));
  DOBA_EXPECT(!serialized->source.has_value());

}
// +===========================================================================+
// | [>] moved responses require factory reassignment            ( test-case ) |
// +===========================================================================+
DOBA_TEST("moved responses are reassigned through status factories") {
  response source = response::ok_200();
  source.set_header("Date", "fixed")
      .add_header("X-Original", "kept").set_body("original");
  response constructed(std::move(source));
  source = response::accepted_202();
  source.set_header("Date", "fixed").set_body("new");
  auto source_serialized = source.serialize();
  DOBA_EXPECT(std::string_view(source_serialized->prefix.get(),
                               source_serialized->prefix_size)
                  .starts_with("HTTP/1.1 202 Accepted\r\n"));
  source = std::move(constructed);
  constructed = response::no_content_204();
  constructed.set_header("Date", "fixed");
  DOBA_EXPECT_EQUAL(source.get_header("X-Original").second, "kept");
  auto serialized = source.serialize();
  const std::string serialized_prefix(serialized->prefix.get(),
                                      serialized->prefix_size);
  DOBA_EXPECT(serialized_prefix.starts_with("HTTP/1.1 200 OK\r\n"));
  DOBA_EXPECT(serialized_prefix.ends_with("original"));
  DOBA_EXPECT(!constructed.has_header("X-Original"));
}
// +===========================================================================+
// | [>] bodyless responses survive chained moves                ( test-case ) |
// +===========================================================================+
DOBA_TEST("bodyless responses survive chained moves") {
  response source = response::ok_200();
  response constructed(std::move(source));
  response assigned = response::bad_request_400();
  assigned.set_body("replaced");
  assigned = std::move(constructed);
  source = response::created_201();
  constructed = response::accepted_202();
  for (response* value : {&source, &constructed, &assigned}) {
    value->set_body("").set_header("Date", "fixed");
    auto serialized = value->serialize();
    const std::string serialized_prefix(serialized->prefix.get(),
                                        serialized->prefix_size);
    DOBA_EXPECT(serialized_prefix.find("Content-Length: 0\r\n") !=
                std::string::npos);
    DOBA_EXPECT(!serialized->source.has_value());
  }
}
// +===========================================================================+
// | [>] headers support mutation lookup removal and indexes     ( test-case ) |
// +===========================================================================+
DOBA_TEST("headers support append replace lookup removal and indexes") {
  response value = response::ok_200();
  value.add_header("X-Test", "a").add_header("X-Test", "b");
  value.add_header("X-Number", 42);
  DOBA_EXPECT(value.has_header("x-test"));
  DOBA_EXPECT_EQUAL(value.get_headers_length(), 3);
  DOBA_EXPECT_EQUAL(value.get_header("X-TEST").second, "a");
  DOBA_EXPECT_EQUAL(value.get_header(0).first, "X-Test");
  DOBA_EXPECT_EQUAL(value.get_header(0).second, "a");
  value.set_header("x-test", "longer value");
  DOBA_EXPECT_EQUAL(value.get_header("X-Test").second, "longer value");
  value.set_header("X-Test", "x");
  DOBA_EXPECT_EQUAL(value.get_header("X-Test").second, "x");
  value.set_header("X-New", 7);
  DOBA_EXPECT_EQUAL(value.get_header("x-new").second, "7");
  value.remove_header("X-TEST");
  DOBA_EXPECT_EQUAL(value.get_header("x-test").second, "b");
  value.remove_header("missing");
  DOBA_EXPECT_EQUAL(value.get_headers_length(), 3);
}
// +===========================================================================+
// | [>] missing and out of range header lookups throw           ( test-case ) |
// +===========================================================================+
DOBA_TEST("missing and out of range header lookups throw") {
  response value = response::ok_200();
  bool missing_threw = false;
  try {
    value.get_header("Missing");
  } catch (const std::runtime_error&) {
    missing_threw = true;
  }
  DOBA_EXPECT(missing_threw);
  bool index_threw = false;
  try {
    value.get_header(value.get_headers_length());
  } catch (const std::out_of_range&) {
    index_threw = true;
  }
  DOBA_EXPECT(index_threw);
}
// +===========================================================================+
// | [>] oversized header additions and growth throw             ( test-case ) |
// +===========================================================================+
DOBA_TEST("oversized header additions and growth throw") {
  response value = response::ok_200();
  const std::string huge(limits::kMaxResponseSizeInMemory, 'x');
  bool add_threw = false;
  try {
    value.add_header("X", huge);
  } catch (const std::out_of_range&) {
    add_threw = true;
  }
  DOBA_EXPECT(add_threw);
  value.add_header("X", "a");
  bool set_threw = false;
  try {
    value.set_header("X", huge);
  } catch (const std::out_of_range&) {
    set_threw = true;
  }
  DOBA_EXPECT(set_threw);
  DOBA_EXPECT_EQUAL(value.get_header("X").second, "a");
}
// +===========================================================================+
// | [>] invalid response header names are rejected              ( test-case ) |
// +===========================================================================+
DOBA_TEST("invalid response header names are rejected") {
  constexpr std::string_view cases[] = {
      "", "Bad Name", "Bad:Name", "Bad\tName", "Bad\r\nX-Test",
  };
  for (const auto name : cases) {
    response value = response::ok_200();
    bool threw = false;
    try {
      value.add_header(name, "value");
    } catch (const std::invalid_argument&) {
      threw = true;
    }
    DOBA_EXPECT(threw);
  }
}
// +===========================================================================+
// | [>] invalid response header values are rejected             ( test-case ) |
// +===========================================================================+
DOBA_TEST("invalid response header values are rejected") {
  const std::string cases[] = {
      "value\rnext",
      "value\nnext",
      "value\r\nX-Test: injected",
      std::string("value\0next", 10),
  };
  for (const auto& field_value : cases) {
    response value = response::ok_200();
    bool threw = false;
    try {
      value.add_header("X-Test", field_value);
    } catch (const std::invalid_argument&) {
      threw = true;
    }
    DOBA_EXPECT(threw);
  }
}
// +===========================================================================+
// | [>] rejected replacement preserves the original header      ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejected replacement preserves the original header") {
  response value = response::ok_200();
  value.add_header("X-Test", "original");
  bool threw = false;
  try {
    value.set_header("X-Test", "replacement\r\nX-Injected: value");
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  DOBA_EXPECT_EQUAL(value.get_header("X-Test").second, "original");
}
// +===========================================================================+
// | [>] bounded header views do not copy adjacent bytes         ( test-case ) |
// +===========================================================================+
DOBA_TEST("bounded header views do not copy adjacent bytes") {
  const std::string source = "xxX-Testyyvaluezz";
  response value = response::ok_200();
  value.add_header(std::string_view(source).substr(2, 6),
                  std::string_view(source).substr(10, 5))
      .set_header("Date", "fixed");
  const auto serialized = value.serialize();
  const std::string serialized_prefix(serialized->prefix.get(),
                                      serialized->prefix_size);
  DOBA_EXPECT(serialized_prefix.find("X-Test: value\r\n") !=
              std::string::npos);
  DOBA_EXPECT(serialized_prefix.find("xx") == std::string::npos);
  DOBA_EXPECT(serialized_prefix.find("zz") == std::string::npos);
}
// +===========================================================================+
// | [>] small bodies serialize inline including binary bytes    ( test-case ) |
// +===========================================================================+
DOBA_TEST("small bodies serialize inline including binary bytes") {
  response value = response::ok_200();
  value.set_header("Date", "fixed")
      .set_body(std::string_view("a\0b", 3));
  DOBA_EXPECT(!value.has_header("Content-Length"));
  const auto serialized = value.serialize();
  const std::string serialized_prefix(serialized->prefix.get(),
                                      serialized->prefix_size);
  DOBA_EXPECT(!serialized->source.has_value());
  DOBA_EXPECT(serialized_prefix.starts_with("HTTP/1.1 200 OK\r\n"));
  DOBA_EXPECT(serialized_prefix.find("Content-Length: 3\r\n") !=
              std::string::npos);
  DOBA_EXPECT_EQUAL(std::string_view(serialized_prefix)
                        .substr(serialized_prefix.size() - 3),
                    std::string_view("a\0b", 3));
}
// +===========================================================================+
// | [>] large bodies serialize through an owned source          ( test-case ) |
// +===========================================================================+
DOBA_TEST("large bodies serialize through an owned source") {
  for (std::size_t size : {limits::kMaxResponseBodySizeInMemory - 1,
                           limits::kMaxResponseBodySizeInMemory,
                           limits::kMaxResponseBodySizeInMemory + 1}) {
    std::string payload(size, '\0');
    for (std::size_t i = 0; i < payload.size(); i++) {
      payload[i] = static_cast<char>((i * 31 + i / 127) % 256);
    }
    response value = response::ok_200();
    value.set_header("Date", "fixed").set_body(payload);
    DOBA_EXPECT(!value.has_header("Content-Length"));
    auto serialized = value.serialize();
    const std::string serialized_prefix(serialized->prefix.get(),
                                        serialized->prefix_size);
    DOBA_EXPECT(serialized_prefix.find("Content-Length: " +
                                       std::to_string(payload.size()) + "\r\n") !=
                std::string::npos);
    const auto boundary = serialized_prefix.find("\r\n\r\n");
    DOBA_EXPECT(boundary != std::string::npos);
    DOBA_EXPECT_EQUAL(serialized->source.has_value(),
                      size > limits::kMaxResponseBodySizeInMemory);
    std::string actual = serialized_prefix.substr(boundary + 4);
    if (serialized->source.has_value()) {
      DOBA_EXPECT(actual.empty());
      actual += read_source(*serialized->source);
      DOBA_EXPECT(serialized->source->eof());
      DOBA_EXPECT(!serialized->source->failed());
    }
    DOBA_EXPECT_EQUAL(actual, payload);
  }
}
// +===========================================================================+
// | [>] adopted raw and chunked writers set matching framing    ( test-case ) |
// +===========================================================================+
DOBA_TEST("adopted raw and chunked writers set matching framing") {
  auto raw = body_writer::raw();
  DOBA_EXPECT(raw.write("abc"));
  response raw_response = response::ok_200();
  raw_response.set_header("Date", "fixed").set_body(std::move(raw));
  DOBA_EXPECT(!raw_response.has_header("Content-Length"));
  auto raw_serialized = raw_response.serialize();
  DOBA_EXPECT(std::string_view(raw_serialized->prefix.get(),
                               raw_serialized->prefix_size)
                  .find("Content-Length: 3\r\n") != std::string_view::npos);
  DOBA_EXPECT_EQUAL(read_source(*raw_serialized->source), "abc");
  auto chunked = body_writer::chunked();
  DOBA_EXPECT(chunked.write("abc"));
  response chunked_response = response::ok_200();
  chunked_response.set_header("Date", "fixed")
      .set_body(std::move(chunked));
  DOBA_EXPECT(!chunked_response.has_header("Transfer-Encoding"));
  DOBA_EXPECT(!chunked_response.has_header("Content-Length"));
  auto chunked_serialized = chunked_response.serialize();
  const std::string_view chunked_prefix(chunked_serialized->prefix.get(),
                                         chunked_serialized->prefix_size);
  DOBA_EXPECT(chunked_prefix.find("Transfer-Encoding: chunked\r\n") !=
              std::string_view::npos);
  DOBA_EXPECT(chunked_prefix.find("Content-Length:") == std::string_view::npos);
  DOBA_EXPECT_EQUAL(read_source(*chunked_serialized->source),
                    "3\r\nabc\r\n0\r\n\r\n");
}
// +===========================================================================+
// | [>] replacing and clearing bodies removes stale framing     ( test-case ) |
// +===========================================================================+
DOBA_TEST("replacing and clearing bodies removes stale framing") {
  response value = response::ok_200();
  auto chunked = body_writer::chunked();
  DOBA_EXPECT(chunked.write("abc"));
  value.set_body(std::move(chunked));
  DOBA_EXPECT(!value.has_header("Transfer-Encoding"));
  value.set_body("xy");
  DOBA_EXPECT(!value.has_header("Transfer-Encoding"));
  DOBA_EXPECT(!value.has_header("Content-Length"));
  value.clear_body();
  DOBA_EXPECT(!value.has_header("Content-Length"));
  DOBA_EXPECT(!value.has_header("Transfer-Encoding"));
}
// +===========================================================================+
// | [>] every status method emits its registered status line    ( test-case ) |
// +===========================================================================+
DOBA_TEST("every status method emits its registered status line") {
  using setter = response (*)();
  struct test_case {
    setter set;
    std::string_view line;
    bool content_length;
  };
  constexpr test_case cases[] = {
      {&response::continue_100, "HTTP/1.1 100 Continue\r\n", false},
      {&response::switching_protocols_101,
       "HTTP/1.1 101 Switching Protocols\r\n", false},
      {&response::ok_200, "HTTP/1.1 200 OK\r\n", true},
      {&response::created_201, "HTTP/1.1 201 Created\r\n", true},
      {&response::accepted_202, "HTTP/1.1 202 Accepted\r\n", true},
      {&response::non_authoritative_info_203,
       "HTTP/1.1 203 Non-Authoritative Information\r\n", true},
      {&response::no_content_204, "HTTP/1.1 204 No Content\r\n", false},
      {&response::reset_content_205, "HTTP/1.1 205 Reset Content\r\n", true},
      {&response::partial_content_206, "HTTP/1.1 206 Partial Content\r\n",
       true},
      {&response::multiple_choices_300, "HTTP/1.1 300 Multiple Choices\r\n",
       true},
      {&response::moved_permanently_301, "HTTP/1.1 301 Moved Permanently\r\n",
       true},
      {&response::found_302, "HTTP/1.1 302 Found\r\n", true},
      {&response::see_other_303, "HTTP/1.1 303 See Other\r\n", true},
      {&response::not_modified_304, "HTTP/1.1 304 Not Modified\r\n", false},
      {&response::use_proxy_305, "HTTP/1.1 305 Use Proxy\r\n", true},
      {&response::unused_306, "HTTP/1.1 306 Unused\r\n", true},
      {&response::temporary_redirect_307, "HTTP/1.1 307 Temporary Redirect\r\n",
       true},
      {&response::permanent_redirect_308, "HTTP/1.1 308 Permanent Redirect\r\n",
       true},
      {&response::bad_request_400, "HTTP/1.1 400 Bad Request\r\n", true},
      {&response::unauthorized_401, "HTTP/1.1 401 Unauthorized\r\n", true},
      {&response::payment_required_402, "HTTP/1.1 402 Payment Required\r\n",
       true},
      {&response::forbidden_403, "HTTP/1.1 403 Forbidden\r\n", true},
      {&response::not_found_404, "HTTP/1.1 404 Not Found\r\n", true},
      {&response::method_not_allowed_405, "HTTP/1.1 405 Method Not Allowed\r\n",
       true},
      {&response::not_acceptable_406, "HTTP/1.1 406 Not Acceptable\r\n", true},
      {&response::proxy_auth_required_407,
       "HTTP/1.1 407 Proxy Authentication Required\r\n", true},
      {&response::request_timeout_408, "HTTP/1.1 408 Request Timeout\r\n",
       true},
      {&response::conflict_409, "HTTP/1.1 409 Conflict\r\n", true},
      {&response::gone_410, "HTTP/1.1 410 Gone\r\n", true},
      {&response::length_required_411, "HTTP/1.1 411 Length Required\r\n",
       true},
      {&response::precondition_failed_412,
       "HTTP/1.1 412 Precondition Failed\r\n", true},
      {&response::content_too_large_413, "HTTP/1.1 413 Content Too Large\r\n",
       true},
      {&response::uri_too_long_414, "HTTP/1.1 414 URI Too Long\r\n", true},
      {&response::unsupported_media_type_415,
       "HTTP/1.1 415 Unsupported Media Type\r\n", true},
      {&response::range_not_satisfiable_416,
       "HTTP/1.1 416 Range Not Satisfiable\r\n", true},
      {&response::expectation_failed_417, "HTTP/1.1 417 Expectation Failed\r\n",
       true},
      {&response::unused_418, "HTTP/1.1 418 Im a teapot\r\n", true},
      {&response::misdirected_request_421,
       "HTTP/1.1 421 Misdirected Request\r\n", true},
      {&response::unprocessable_content_422,
       "HTTP/1.1 422 Unprocessable Content\r\n", true},
      {&response::upgrade_required_426, "HTTP/1.1 426 Upgrade Required\r\n",
       true},
      {&response::request_header_fields_too_large_431,
       "HTTP/1.1 431 Request Header Fields Too Large\r\n", true},
      {&response::internal_server_error_500,
       "HTTP/1.1 500 Internal Server Error\r\n", true},
      {&response::not_implemented_501, "HTTP/1.1 501 Not Implemented\r\n",
       true},
      {&response::bad_gateway_502, "HTTP/1.1 502 Bad Gateway\r\n", true},
      {&response::service_unavailable_503,
       "HTTP/1.1 503 Service Unavailable\r\n", true},
      {&response::gateway_timeout_504, "HTTP/1.1 504 Gateway Timeout\r\n",
       true},
      {&response::http_version_not_supported_505,
       "HTTP/1.1 505 HTTP Version Not Supported\r\n", true},
  };
  for (const auto& test : cases) {
    response value = test.set();
    value.set_header("Date", "fixed");
    DOBA_EXPECT(!value.has_header("Content-Length"));
    const auto serialized = value.serialize();
    const std::string serialized_prefix(serialized->prefix.get(),
                                        serialized->prefix_size);
    DOBA_EXPECT(serialized_prefix.starts_with(test.line));
    DOBA_EXPECT_EQUAL(
        serialized_prefix.find("Content-Length: 0\r\n") != std::string::npos,
        test.content_length);
  }
}
// +===========================================================================+
// | [>] bodyless statuses never serialize response bodies       ( test-case ) |
// +===========================================================================+
DOBA_TEST("informational 204 205 and 304 responses never serialize bodies") {
  using setter = response (*)();
  constexpr setter cases[] = {
      &response::continue_100,
      &response::switching_protocols_101,
      &response::no_content_204,
      &response::reset_content_205,
      &response::not_modified_304,
  };
  for (const auto set : cases) {
    response value = set();
    value.set_header("Date", "fixed").set_body("body");
    auto serialized = value.serialize();
    const std::string serialized_prefix(serialized->prefix.get(),
                                        serialized->prefix_size);
    const auto boundary = serialized_prefix.find("\r\n\r\n");
    DOBA_EXPECT(boundary != std::string::npos);
    DOBA_EXPECT_EQUAL(serialized_prefix.substr(boundary + 4), "");
    DOBA_EXPECT(!serialized->source.has_value());
    DOBA_EXPECT(serialized_prefix.find("Transfer-Encoding:") ==
                std::string::npos);
    if (set == &response::reset_content_205) {
      DOBA_EXPECT(serialized_prefix.find("Content-Length: 0\r\n") !=
                  std::string::npos);
    } else if (set == &response::not_modified_304) {
      DOBA_EXPECT(serialized_prefix.find("Content-Length: 4\r\n") !=
                  std::string::npos);
    } else {
      DOBA_EXPECT(serialized_prefix.find("Content-Length:") ==
                  std::string::npos);
    }
  }
  auto writer = body_writer::chunked();
  DOBA_EXPECT(writer.write("body"));
  response streamed = response::reset_content_205();
  streamed.set_header("Date", "fixed")
      .set_body(std::move(writer));
  const auto serialized = streamed.serialize();
  const std::string serialized_prefix(serialized->prefix.get(),
                                      serialized->prefix_size);
  const auto boundary = serialized_prefix.find("\r\n\r\n");
  DOBA_EXPECT(boundary != std::string::npos);
  DOBA_EXPECT_EQUAL(serialized_prefix.substr(boundary + 4), "");
  DOBA_EXPECT(!serialized->source.has_value());
  DOBA_EXPECT(serialized_prefix.find("Transfer-Encoding:") ==
              std::string::npos);
  DOBA_EXPECT(serialized_prefix.find("Content-Length: 0\r\n") !=
              std::string::npos);
}
// +===========================================================================+
// | [>] serialized bytes outlive and detach from the response   ( test-case ) |
// +===========================================================================+
DOBA_TEST("serialized bytes outlive and detach from the response") {
  std::unique_ptr<response::serialized_type> serialized;
  {
    response value = response::created_201();
    value.set_header("Date", "fixed").set_body("original");
    serialized = value.serialize();
    value = response::internal_server_error_500();
    value.set_body("replacement");
  }
  const std::string prefix(serialized->prefix.get(), serialized->prefix_size);
  DOBA_EXPECT_EQUAL(prefix,
                    "HTTP/1.1 201 Created\r\nDate: fixed\r\n"
                    "Content-Length: 8\r\n\r\noriginal");
}
// +===========================================================================+
// | [>] inline body compaction handles overlapping regions     ( test-case ) |
// +===========================================================================+
DOBA_TEST("inline body compaction handles overlapping regions") {
  std::string body(limits::kMaxResponseBodySizeInMemory, '\0');
  for (std::size_t i = 0; i < body.size(); i++) {
    body[i] = static_cast<char>(i % 256);
  }
  const std::size_t body_begin = limits::kMaxResponseSizeInMemory - body.size();
  const std::string head = "HTTP/1.1 200 OK\r\nDate: fixed\r\nX-Pad: ";
  const std::string framing =
      "Content-Length: " + std::to_string(body.size()) + "\r\n";
  const std::string padding(
      body_begin - 32 - head.size() - framing.size() - 4, 'x');
  response value = response::ok_200();
  value.set_body(body).set_header("Date", "fixed").add_header("X-Pad", padding);
  const auto serialized = value.serialize();
  const std::string serialized_prefix(serialized->prefix.get(),
                                      serialized->prefix_size);
  DOBA_EXPECT_EQUAL(serialized_prefix,
                    head + padding + "\r\n" + framing + "\r\n" + body);
  DOBA_EXPECT(!serialized->source.has_value());
}
// +===========================================================================+
// | [>] automatic framing stays deferred until serialization   ( test-case ) |
// +===========================================================================+
DOBA_TEST("automatic framing stays deferred until serialization") {
  response value = response::ok_200();
  DOBA_EXPECT_EQUAL(value.get_headers_length(), 0);
  value.add_header("Content-Type", "text/plain").set_body("ok");
  DOBA_EXPECT_EQUAL(value.get_headers_length(), 1);
  DOBA_EXPECT_EQUAL(value.get_header(0).first, "Content-Type");
  DOBA_EXPECT(!value.has_header("Content-Length"));
  bool threw = false;
  try {
    value.get_header("Content-Length");
  } catch (const std::runtime_error&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  value.remove_header("Content-Length");
  value.set_body("replacement").set_body("ok");
  auto serialized = value.serialize();
  const std::string_view prefix(serialized->prefix.get(),
                                serialized->prefix_size);
  DOBA_EXPECT(prefix.find("Content-Length: 2\r\n") != std::string_view::npos);
  DOBA_EXPECT(prefix.find("Content-Length:") == prefix.rfind("Content-Length:"));
  DOBA_EXPECT(prefix.ends_with("\r\n\r\nok"));
}
// +===========================================================================+
// | [>] body replacement clears explicit framing duplicates    ( test-case ) |
// +===========================================================================+
DOBA_TEST("body replacement clears explicit framing duplicates") {
  response value = response::ok_200();
  value.add_header("content-length", "12")
      .add_header("Content-Length", "12")
      .add_header("Transfer-Encoding", "chunked")
      .add_header("transfer-encoding", "chunked");
  response moved(std::move(value));
  value = response::ok_200();
  value = std::move(moved);
  value.set_body("xy");
  DOBA_EXPECT_EQUAL(value.get_headers_length(), 0);
  auto serialized = value.serialize();
  const std::string_view prefix(serialized->prefix.get(),
                                serialized->prefix_size);
  DOBA_EXPECT(prefix.find("Content-Length: 2\r\n") != std::string_view::npos);
  DOBA_EXPECT(prefix.find("Transfer-Encoding:") == std::string_view::npos);
  DOBA_EXPECT(prefix.ends_with("\r\n\r\nxy"));
}
// +===========================================================================+
// | [>] explicit framing overrides deferred framing            ( test-case ) |
// +===========================================================================+
DOBA_TEST("explicit framing overrides deferred framing") {
  response value = response::ok_200();
  value.set_body("abc").set_header("content-length", "3");
  DOBA_EXPECT_EQUAL(value.get_header("Content-Length").second, "3");
  auto serialized = value.serialize();
  const std::string_view prefix(serialized->prefix.get(),
                                serialized->prefix_size);
  DOBA_EXPECT(prefix.find("content-length: 3\r\n") != std::string_view::npos);
  DOBA_EXPECT(prefix.find("Content-Length:") == std::string_view::npos);
  response head = response::ok_200();
  head.set_header("Transfer-Encoding", "chunked").clear_body(true);
  auto head_serialized = head.serialize();
  const std::string_view head_prefix(head_serialized->prefix.get(),
                                     head_serialized->prefix_size);
  DOBA_EXPECT(head_prefix.find("Transfer-Encoding: chunked\r\n") !=
              std::string_view::npos);
  DOBA_EXPECT(head_prefix.find("Content-Length:") == std::string_view::npos);
}
// +===========================================================================+
// | [>] conflicting framing is rejected before transmission     ( test-case ) |
// +===========================================================================+
DOBA_TEST("conflicting response framing is rejected before transmission") {
  for (bool automatic : {false, true}) {
    response value = response::ok_200();
    if (automatic) {
      auto writer = body_writer::chunked();
      DOBA_EXPECT(writer.write("abc"));
      value.set_body(std::move(writer));
    } else {
      value.set_header("Transfer-Encoding", "chunked");
    }
    value.set_header("Content-Length", "3");
    bool threw = false;
    try {
      (void)value.serialize();
    } catch (const std::invalid_argument&) {
      threw = true;
    }
    DOBA_EXPECT(threw);
  }
}
// +===========================================================================+
// | [>] HEAD preserves deferred framing through moves          ( test-case ) |
// +===========================================================================+
DOBA_TEST("HEAD preserves deferred framing through moves") {
  for (bool chunked : {false, true}) {
    auto writer = chunked ? body_writer::chunked() : body_writer::raw();
    DOBA_EXPECT(writer.write("resource"));
    response value = response::ok_200();
    value.set_body(std::move(writer)).clear_body(true);
    response moved(std::move(value));
    value = response::created_201();
    value = std::move(moved);
    auto serialized = value.serialize();
    const std::string_view prefix(serialized->prefix.get(),
                                  serialized->prefix_size);
    DOBA_EXPECT(!serialized->source.has_value());
    DOBA_EXPECT(prefix.ends_with("\r\n\r\n"));
    DOBA_EXPECT_EQUAL(prefix.find("Content-Length: 8\r\n") !=
                          std::string_view::npos,
                      !chunked);
    DOBA_EXPECT_EQUAL(prefix.find("Transfer-Encoding: chunked\r\n") !=
                          std::string_view::npos,
                      chunked);
  }
}
// +===========================================================================+
// | [>] deferred length respects the exact header boundary     ( test-case ) |
// +===========================================================================+
DOBA_TEST("deferred length respects the exact header boundary") {
  for (std::size_t size : {0, 9, 10, 99, 100}) {
    for (std::size_t excess : {0, 1}) {
      response value = response::ok_200();
      const std::string body(size, 'a');
      const std::string head = "HTTP/1.1 200 OK\r\nDate: fixed\r\nX-Pad: ";
      const std::string framing =
          "Content-Length: " + std::to_string(size) + "\r\n";
      const std::size_t body_begin = limits::kMaxResponseSizeInMemory -
                                     limits::kMaxResponseBodySizeInMemory;
      const std::string padding(
          body_begin - head.size() - framing.size() - 4 + excess, 'x');
      value.set_body(body).set_header("Date", "fixed")
          .add_header("X-Pad", padding);
      bool threw = false;
      try {
        auto serialized = value.serialize();
        DOBA_EXPECT_EQUAL(
            std::string_view(serialized->prefix.get(), serialized->prefix_size),
            head + padding + "\r\n" + framing + "\r\n" + body);
      } catch (const std::out_of_range&) {
        threw = true;
      }
      DOBA_EXPECT_EQUAL(threw, excess != 0);
    }
  }
}
