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
#include "protocol/http/v11/body/reader.h"
#include "protocol/http/v11/response.h"
#include "response_wire.h"
#include "test_helper.h"

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] usings                                                     ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using martianlabs::doba::common::reader;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::tests::unit::wire_prefix;
using martianlabs::doba::protocol::http::v11::body::body_writer;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] constants                                                  ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
constexpr std::size_t max_response_size_in_memory =
    policies::kMaxResponseHeadSizeInMemory +
    policies::kMaxResponseBodySizeInMemory;
constexpr std::size_t max_response_body_size_in_memory =
    policies::kMaxResponseBodySizeInMemory;


// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] read_source                                              ( function ) |
// +---------------------------------------------------------------------------+
// | This function reads the entire content from a reader source and returns   |
// | it as a string. It is used for testing purposes to verify the             |
// | correctness of the serialized body.                                       |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
std::string read_source(reader& source) {
  std::string output;
  source.read_all(output);
  return output;
}
}  // namespace

// +===========================================================================+
// | [>] response is movable but not copyable                    ( test-case ) |
// +---------------------------------------------------------------------------+
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
// | [>] response identifies 100 Continue                        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("response identifies 100 Continue") {
  std::array<char, max_response_size_in_memory> storage{};
  response value(storage);
  DOBA_EXPECT(value.continue_100().is_continue_100());
  DOBA_EXPECT(!value.switching_protocols_101().is_continue_100());
  DOBA_EXPECT(!value.ok_200().is_continue_100());
}

// +===========================================================================+
// | [>] moving preserves response state and owned body writers  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("moving preserves response state and owned body writers") {
  std::array<char, max_response_size_in_memory> storage_1{};
  response source(storage_1);
  source.created_201();
  auto writer = body_writer::chunked();
  DOBA_EXPECT(writer.write("body"));
  source.set_header("Date", "fixed")
      .add_header("X-Test", "value")
      .set_body(std::move(writer));
  response constructed(std::move(source));
  DOBA_EXPECT_EQUAL(constructed.get_header("X-Test").second, "value");
  std::array<char, max_response_size_in_memory> storage_2{};
  response assigned(storage_2);
  assigned.bad_request_400();
  assigned.set_body("replaced");
  assigned = std::move(constructed);
  auto serialized = assigned.serialize();
  const std::string serialized_prefix((wire_prefix(serialized)));
  DOBA_EXPECT(serialized_prefix.starts_with("HTTP/1.1 201 Created\r\n"));
  DOBA_EXPECT_EQUAL(read_source(*serialized.source), "4\r\nbody\r\n0\r\n\r\n");
}

// +===========================================================================+
// | [>] moves preserve the full streamed body                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("moves preserve the full streamed body") {
  std::string body(max_response_body_size_in_memory + 1, '\0');
  for (std::size_t i = 0; i < body.size(); i++) {
    body[i] = static_cast<char>(i % 256);
  }
  std::array<char, max_response_size_in_memory> storage_3{};
  response source(storage_3);
  source.created_201();
  source.set_header("Date", "fixed").set_body(body);
  response constructed(std::move(source));
  std::array<char, max_response_size_in_memory> storage_4{};
  response assigned(storage_4);
  assigned.ok_200();
  assigned.set_body("replaced");
  assigned = std::move(constructed);
  response& alias = assigned;
  assigned = std::move(alias);
  DOBA_EXPECT(!assigned.has_header("Content-Length"));
  auto serialized = assigned.serialize();
  const std::string serialized_prefix((wire_prefix(serialized)));
  DOBA_EXPECT(serialized_prefix.starts_with("HTTP/1.1 201 Created\r\n"));
  DOBA_EXPECT(serialized_prefix.find(
                  "Content-Length: " + std::to_string(body.size()) + "\r\n") !=
              std::string::npos);
  DOBA_EXPECT(serialized.source != nullptr);
  DOBA_EXPECT_EQUAL(read_source(*serialized.source), body);
}

// +===========================================================================+
// | [>] moved responses require factory reassignment            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("moved responses are reassigned through status factories") {
  std::array<char, max_response_size_in_memory> storage_5{};
  response source(storage_5);
  source.ok_200();
  source.set_header("Date", "fixed")
      .add_header("X-Original", "kept")
      .set_body("original");
  response constructed(std::move(source));
  std::array<char, max_response_size_in_memory> new_storage{};
  response new_response(new_storage);
  new_response.accepted_202();
  source = std::move(new_response);
  source.set_header("Date", "fixed").set_body("new");
  auto source_serialized = source.serialize();
  DOBA_EXPECT((wire_prefix(source_serialized))
                  .starts_with("HTTP/1.1 202 Accepted\r\n"));
  source = std::move(constructed);
  std::array<char, max_response_size_in_memory> other_storage{};
  response other_response(other_storage);
  other_response.no_content_204();
  constructed = std::move(other_response);
  constructed.set_header("Date", "fixed");
  DOBA_EXPECT_EQUAL(source.get_header("X-Original").second, "kept");
  auto serialized = source.serialize();
  const std::string serialized_prefix((wire_prefix(serialized)));
  DOBA_EXPECT(serialized_prefix.starts_with("HTTP/1.1 200 OK\r\n"));
  DOBA_EXPECT(serialized_prefix.ends_with("original"));
  DOBA_EXPECT(!constructed.has_header("X-Original"));
}

// +===========================================================================+
// | [>] bodyless responses survive chained moves                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("bodyless responses survive chained moves") {
  std::array<char, max_response_size_in_memory> storage_6{};
  response source(storage_6);
  source.ok_200();
  response constructed(std::move(source));
  std::array<char, max_response_size_in_memory> storage_7{};
  response assigned(storage_7);
  assigned.bad_request_400();
  assigned.set_body("replaced");
  assigned = std::move(constructed);
  std::array<char, max_response_size_in_memory> new_storage{};
  response new_source(new_storage);
  new_source.created_201();
  source = std::move(new_source);
  std::array<char, max_response_size_in_memory> other_storage{};
  response new_constructed(other_storage);
  new_constructed.accepted_202();
  constructed = std::move(new_constructed);
  for (response* value : {&source, &constructed, &assigned}) {
    value->set_body("").set_header("Date", "fixed");
    auto serialized = value->serialize();
    const std::string serialized_prefix((wire_prefix(serialized)));
    DOBA_EXPECT(serialized_prefix.find("Content-Length: 0\r\n") !=
                std::string::npos);
    DOBA_EXPECT(!serialized.source);
  }
}

// +===========================================================================+
// | [>] headers support mutation lookup removal and indexes     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("headers support append replace lookup removal and indexes") {
  std::array<char, max_response_size_in_memory> storage_8{};
  response value(storage_8);
  value.ok_200();
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
// | [>] connection close state follows header mutations         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("connection close state follows header mutations") {
  std::array<char, max_response_size_in_memory> storage_9{};
  response value(storage_9);
  value.ok_200();
  DOBA_EXPECT(!value.wants_connection_close());
  value.add_header("Connection", "keep-alive, x-close");
  DOBA_EXPECT(!value.wants_connection_close());
  value.add_header("connection", "upgrade, ClOsE ");
  DOBA_EXPECT(value.wants_connection_close());
  value.set_header("Connection", "close-later");
  DOBA_EXPECT(value.wants_connection_close());
  value.remove_header("CONNECTION");
  DOBA_EXPECT(value.wants_connection_close());
  value.set_header("Connection", "keep-alive");
  DOBA_EXPECT(!value.wants_connection_close());
  value.set_header("Connection", "close");
  DOBA_EXPECT(value.wants_connection_close());
  response moved(std::move(value));
  DOBA_EXPECT(moved.wants_connection_close());
  std::array<char, max_response_size_in_memory> storage_10{};
  response assigned(storage_10);
  assigned.ok_200();
  assigned = std::move(moved);
  DOBA_EXPECT(assigned.wants_connection_close());
  assigned.remove_header("Connection");
  DOBA_EXPECT(!assigned.wants_connection_close());
}

// +===========================================================================+
// | [>] missing and out of range header lookups throw           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("missing and out of range header lookups throw") {
  std::array<char, max_response_size_in_memory> storage_11{};
  response value(storage_11);
  value.ok_200();
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
// +---------------------------------------------------------------------------+
DOBA_TEST("oversized header additions and growth throw") {
  std::array<char, max_response_size_in_memory> storage_12{};
  response value(storage_12);
  value.ok_200();
  const std::string huge(max_response_size_in_memory, 'x');
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
// +---------------------------------------------------------------------------+
DOBA_TEST("invalid response header names are rejected") {
  constexpr std::string_view cases[] = {
      "", "Bad Name", "Bad:Name", "Bad\tName", "Bad\r\nX-Test",
  };
  for (const auto name : cases) {
    std::array<char, max_response_size_in_memory> storage_13{};
    response value(storage_13);
    value.ok_200();
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
// +---------------------------------------------------------------------------+
DOBA_TEST("invalid response header values are rejected") {
  const std::string cases[] = {
      "value\rnext",
      "value\nnext",
      "value\r\nX-Test: injected",
      std::string("value\0next", 10),
  };
  for (const auto& field_value : cases) {
    std::array<char, max_response_size_in_memory> storage_14{};
    response value(storage_14);
    value.ok_200();
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
// +---------------------------------------------------------------------------+
DOBA_TEST("rejected replacement preserves the original header") {
  std::array<char, max_response_size_in_memory> storage_15{};
  response value(storage_15);
  value.ok_200();
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
// +---------------------------------------------------------------------------+
DOBA_TEST("bounded header views do not copy adjacent bytes") {
  const std::string source = "xxX-Testyyvaluezz";
  std::array<char, max_response_size_in_memory> storage_16{};
  response value(storage_16);
  value.ok_200();
  value
      .add_header(std::string_view(source).substr(2, 6),
                  std::string_view(source).substr(10, 5))
      .set_header("Date", "fixed");
  const auto serialized = value.serialize();
  const std::string serialized_prefix((wire_prefix(serialized)));
  DOBA_EXPECT(serialized_prefix.find("X-Test: value\r\n") != std::string::npos);
  DOBA_EXPECT(serialized_prefix.find("xx") == std::string::npos);
  DOBA_EXPECT(serialized_prefix.find("zz") == std::string::npos);
}

// +===========================================================================+
// | [>] small bodies serialize inline including binary bytes    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("small bodies serialize inline including binary bytes") {
  std::array<char, max_response_size_in_memory> storage_17{};
  response value(storage_17);
  value.ok_200();
  value.set_header("Date", "fixed").set_body(std::string_view("a\0b", 3));
  DOBA_EXPECT(!value.has_header("Content-Length"));
  const auto serialized = value.serialize();
  const std::string serialized_prefix((wire_prefix(serialized)));
  DOBA_EXPECT(!serialized.source);
  DOBA_EXPECT(serialized_prefix.starts_with("HTTP/1.1 200 OK\r\n"));
  DOBA_EXPECT(serialized_prefix.find("Content-Length: 3\r\n") !=
              std::string::npos);
  DOBA_EXPECT_EQUAL(
      std::string_view(serialized_prefix).substr(serialized_prefix.size() - 3),
      std::string_view("a\0b", 3));
}

// +===========================================================================+
// | [>] request reader echoes into inline response body         ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("request reader echoes into inline response body") {
  std::string payload(10240, '\0');
  for (std::size_t i = 0; i < payload.size(); i++) {
    payload[i] = static_cast<char>((i * 31 + i / 127) % 256);
  }
  auto source = martianlabs::doba::protocol::http::v11::body::reader::raw(
      std::as_bytes(std::span(payload)), payload.size());
  std::array<char, max_response_size_in_memory> storage{};
  response value(storage);
  value.ok_200().set_header("Date", "fixed").set_body(&source);
  auto serialized = value.serialize();
  DOBA_EXPECT(!serialized.source);
  DOBA_EXPECT(wire_prefix(serialized).find("Content-Length: 10240\r\n") !=
              std::string_view::npos);
  DOBA_EXPECT_EQUAL(serialized.body, payload);
  DOBA_EXPECT_EQUAL(serialized.body.data(), payload.data());
  std::array<std::byte, 1> remaining{};
  const auto state = source.read(remaining);
  DOBA_EXPECT(state.complete);
  DOBA_EXPECT_EQUAL(state.produced, 0);
}

// +===========================================================================+
// | [>] partially read raw body uses remaining borrowed bytes ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("partially read raw body uses remaining borrowed bytes") {
  std::string payload = "abcdef";
  auto source = martianlabs::doba::protocol::http::v11::body::reader::raw(
      std::as_bytes(std::span(payload)), payload.size());
  std::array<std::byte, 2> prefix{};
  const auto first = source.read(prefix);
  DOBA_EXPECT_EQUAL(first.produced, 2);
  std::array<char, max_response_size_in_memory> storage{};
  response value(storage);
  value.ok_200().set_header("Date", "fixed").set_body(&source);
  const auto serialized = value.serialize();
  DOBA_EXPECT_EQUAL(serialized.body, "cdef");
  DOBA_EXPECT_EQUAL(serialized.body.data(), payload.data() + 2);
  DOBA_EXPECT(wire_prefix(serialized).find("Content-Length: 4\r\n") !=
              std::string_view::npos);
}

// +===========================================================================+
// | [>] borrowed raw body survives moves and replacement        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("borrowed raw body survives moves and replacement") {
  std::string payload(10240, 'x');
  auto source = martianlabs::doba::protocol::http::v11::body::reader::raw(
      std::as_bytes(std::span(payload)), payload.size());
  std::array<char, max_response_size_in_memory> storage{};
  response value(storage);
  value.ok_200().set_header("Date", "fixed").set_body(&source);
  response moved(std::move(value));
  const auto borrowed = moved.serialize();
  DOBA_EXPECT_EQUAL(borrowed.body.data(), payload.data());
  moved.ok_200().set_header("Date", "fixed").set_body("new");
  const auto replaced = moved.serialize();
  DOBA_EXPECT_EQUAL(replaced.body, "new");
  DOBA_EXPECT(replaced.body.data() != payload.data());
}

// +===========================================================================+
// | [>] request reader crosses inline response limit            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("request reader crosses inline response limit") {
  for (std::size_t size : {max_response_body_size_in_memory,
                           max_response_body_size_in_memory + 1}) {
    std::string payload(size, '\0');
    for (std::size_t i = 0; i < payload.size(); i++) {
      payload[i] = static_cast<char>((i * 31 + i / 127) % 256);
    }
    auto source = martianlabs::doba::protocol::http::v11::body::reader::raw(
        std::as_bytes(std::span(payload)), payload.size());
    std::array<char, max_response_size_in_memory> storage{};
    response value(storage);
    value.ok_200().set_header("Date", "fixed").set_body(&source);
    auto serialized = value.serialize();
    const std::string prefix(wire_prefix(serialized));
    DOBA_EXPECT(prefix.find("Content-Length: " + std::to_string(size) +
                            "\r\n") != std::string::npos);
    DOBA_EXPECT(prefix.find("Transfer-Encoding:") == std::string::npos);
    DOBA_EXPECT_EQUAL(serialized.source != nullptr,
                      size > max_response_body_size_in_memory);
    if (size <= max_response_body_size_in_memory) {
      DOBA_EXPECT_EQUAL(serialized.body.data(), payload.data());
    }
    std::string actual(serialized.body);
    if (serialized.source) actual += read_source(*serialized.source);
    DOBA_EXPECT_EQUAL(actual, payload);
  }
}

// +===========================================================================+
// | [>] chunked request reader emits decoded raw response       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("chunked request reader emits decoded raw response") {
  for (std::size_t size : {max_response_body_size_in_memory,
                           max_response_body_size_in_memory + 1}) {
    const std::string payload(size, 'x');
    const std::string encoded =
        (size == max_response_body_size_in_memory ? "4000" : "4001") +
        std::string("\r\n") + payload + "\r\n0\r\nX-Test: a\r\n\r\n";
    auto source = martianlabs::doba::protocol::http::v11::body::reader::chunked(
        std::as_bytes(std::span(encoded)));
    std::array<char, max_response_size_in_memory> storage{};
    response value(storage);
    value.ok_200().set_header("Date", "fixed").set_body(&source);
    auto serialized = value.serialize();
    const std::string prefix(wire_prefix(serialized));
    DOBA_EXPECT(prefix.find("Content-Length: " + std::to_string(size) +
                            "\r\n") != std::string::npos);
    DOBA_EXPECT(prefix.find("Transfer-Encoding:") == std::string::npos);
    DOBA_EXPECT_EQUAL(serialized.source != nullptr,
                      size > max_response_body_size_in_memory);
    std::string actual(serialized.body);
    if (serialized.source) actual += read_source(*serialized.source);
    DOBA_EXPECT_EQUAL(actual, payload);
  }
}

// +===========================================================================+
// | [>] request reader errors do not emit partial bodies        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("request reader errors do not emit partial bodies") {
  std::array<char, max_response_size_in_memory> storage{};
  response value(storage);
  value.ok_200().set_header("Date", "fixed").set_body("old");
  auto source = martianlabs::doba::protocol::http::v11::body::reader::raw(
      std::as_bytes(std::span("abc", 3)), 4);
  bool failed = false;
  try {
    value.set_body(&source);
  } catch (const std::runtime_error&) {
    failed = true;
  }
  DOBA_EXPECT(failed);
  auto serialized = value.serialize();
  DOBA_EXPECT(serialized.body.empty());
  DOBA_EXPECT(!serialized.source);

  std::array<char, max_response_size_in_memory> empty_storage{};
  response empty(empty_storage);
  empty.ok_200().set_header("Date", "fixed").set_body("old");
  empty.set_body(nullptr);
  auto empty_serialized = empty.serialize();
  DOBA_EXPECT(empty_serialized.body.empty());
  DOBA_EXPECT(!empty_serialized.source);
}

// +===========================================================================+
// | [>] large bodies serialize through an owned source          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("large bodies serialize through an owned source") {
  for (std::size_t size :
       {max_response_body_size_in_memory - 1, max_response_body_size_in_memory,
        max_response_body_size_in_memory + 1}) {
    std::string payload(size, '\0');
    for (std::size_t i = 0; i < payload.size(); i++) {
      payload[i] = static_cast<char>((i * 31 + i / 127) % 256);
    }
    std::array<char, max_response_size_in_memory> storage_18{};
    response value(storage_18);
    value.ok_200();
    value.set_header("Date", "fixed").set_body(payload);
    DOBA_EXPECT(!value.has_header("Content-Length"));
    auto serialized = value.serialize();
    const std::string serialized_prefix((wire_prefix(serialized)));
    DOBA_EXPECT(serialized_prefix.find(
                    "Content-Length: " + std::to_string(payload.size()) +
                    "\r\n") != std::string::npos);
    const auto boundary = serialized_prefix.find("\r\n\r\n");
    DOBA_EXPECT(boundary != std::string::npos);
    DOBA_EXPECT_EQUAL(serialized.source != nullptr,
                      size > max_response_body_size_in_memory);
    std::string actual = serialized_prefix.substr(boundary + 4);
    if (serialized.source != nullptr) {
      DOBA_EXPECT(actual.empty());
      actual += read_source(*serialized.source);
      DOBA_EXPECT(serialized.source->eof());
      DOBA_EXPECT(!serialized.source->failed());
    }
    DOBA_EXPECT_EQUAL(actual, payload);
  }
}

// +===========================================================================+
// | [>] response prefix ends at the exact 4 KiB boundary        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("response head and body limits are independent") {
  const std::string body(max_response_body_size_in_memory, 'x');
  const std::string head = "HTTP/1.1 200 OK\r\nDate: fixed\r\nX-Pad: ";
  const std::string framing =
      "Content-Length: " + std::to_string(body.size()) + "\r\n";
  const std::string padding(
      policies::kMaxResponseHeadSizeInMemory - head.size() - framing.size() - 4,
      'p');
  for (std::size_t excess : {0, 1}) {
    const std::string payload(body.size() + excess, 'x');
    std::array<char, max_response_size_in_memory> storage_19{};
    response value(storage_19);
    value.ok_200();
    value.set_header("Date", "fixed").add_header("X-Pad", padding);
    value.set_body(payload);
    auto serialized = value.serialize();
    std::string actual((wire_prefix(serialized)));
    if (serialized.source) actual += read_source(*serialized.source);
    const std::string expected_head =
        head + padding +
        "\r\nContent-Length: " + std::to_string(payload.size()) + "\r\n\r\n";
    DOBA_EXPECT_EQUAL(actual, expected_head + payload);
    DOBA_EXPECT_EQUAL(serialized.head.size(),
                      policies::kMaxResponseHeadSizeInMemory);
    DOBA_EXPECT_EQUAL(serialized.body.size(), excess ? 0 : body.size());
    DOBA_EXPECT_EQUAL(serialized.source != nullptr, excess != 0);
  }
}

// +===========================================================================+
// | [>] adopted raw and chunked writers set matching framing    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("adopted raw and chunked writers set matching framing") {
  auto raw = body_writer::raw();
  DOBA_EXPECT(raw.write("abc"));
  std::array<char, max_response_size_in_memory> storage_20{};
  response raw_response(storage_20);
  raw_response.ok_200();
  raw_response.set_header("Date", "fixed").set_body(std::move(raw));
  DOBA_EXPECT(!raw_response.has_header("Content-Length"));
  auto raw_serialized = raw_response.serialize();
  DOBA_EXPECT((wire_prefix(raw_serialized)).find("Content-Length: 3\r\n") !=
              std::string_view::npos);
  DOBA_EXPECT_EQUAL(read_source(*raw_serialized.source), "abc");
  auto chunked = body_writer::chunked();
  DOBA_EXPECT(chunked.write("abc"));
  std::array<char, max_response_size_in_memory> storage_21{};
  response chunked_response(storage_21);
  chunked_response.ok_200();
  chunked_response.set_header("Date", "fixed").set_body(std::move(chunked));
  DOBA_EXPECT(!chunked_response.has_header("Transfer-Encoding"));
  DOBA_EXPECT(!chunked_response.has_header("Content-Length"));
  auto chunked_serialized = chunked_response.serialize();
  const std::string chunked_prefix(wire_prefix(chunked_serialized));
  DOBA_EXPECT(chunked_prefix.find("Transfer-Encoding: chunked\r\n") !=
              std::string_view::npos);
  DOBA_EXPECT(chunked_prefix.find("Content-Length:") == std::string_view::npos);
  DOBA_EXPECT_EQUAL(read_source(*chunked_serialized.source),
                    "3\r\nabc\r\n0\r\n\r\n");
}

// +===========================================================================+
// | [>] replacing and clearing bodies removes stale framing     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("replacing and clearing bodies removes stale framing") {
  std::array<char, max_response_size_in_memory> storage_22{};
  response value(storage_22);
  value.ok_200();
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
// | [>] cleared bodies retain zero length across storage modes  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("cleared bodies retain zero length across storage modes") {
  const std::string bytes = "hidden";
  for (int mode : {1, 2, 3, 4, 5, 0}) {
    for (bool repeated : {false, true}) {
      std::array<char, max_response_size_in_memory> storage_23{};
      response value(storage_23);
      value.ok_200();
      value.set_header("Date", "fixed");
      if (mode == 1) {
        value.set_body(bytes);
      } else if (mode == 2) {
        value.set_body(std::string(max_response_body_size_in_memory + 1, 'x'));
      } else if (mode == 3 || mode == 4) {
        auto writer = mode == 3 ? body_writer::raw() : body_writer::chunked();
        DOBA_EXPECT(writer.write(bytes));
        value.set_body(std::move(writer));
      } else if (mode == 5) {
        value.set_body(reader::borrowed(std::as_bytes(std::span(bytes))),
                       bytes.size());
      }
      DOBA_EXPECT_EQUAL(&value.clear_body(), &value);
      if (repeated) value.clear_body();
      DOBA_EXPECT(!value.has_header("Content-Length"));
      DOBA_EXPECT(!value.has_header("Transfer-Encoding"));
      auto serialized = value.serialize();
      DOBA_EXPECT(!serialized.source);
      DOBA_EXPECT_EQUAL(
          (wire_prefix(serialized)),
          "HTTP/1.1 200 OK\r\nDate: fixed\r\nContent-Length: 0\r\n\r\n");
    }
  }
}

// +===========================================================================+
// | [>] clearing bodies removes explicit framing                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("clearing bodies removes explicit framing") {
  std::array<char, max_response_size_in_memory> storage_24{};
  response value(storage_24);
  value.ok_200();
  value.set_header("Date", "fixed").set_body("hidden");
  value.add_header("Content-Length", "6")
      .add_header("content-length", "6")
      .add_header("Transfer-Encoding", "chunked")
      .add_header("transfer-encoding", "chunked");
  value.clear_body();
  DOBA_EXPECT_EQUAL(value.get_headers_length(), 1);
  auto serialized = value.serialize();
  DOBA_EXPECT(!serialized.source);
  DOBA_EXPECT_EQUAL(
      (wire_prefix(serialized)),
      "HTTP/1.1 200 OK\r\nDate: fixed\r\nContent-Length: 0\r\n\r\n");
}

// +===========================================================================+
// | [>] clearing bodies preserves status specific framing       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("clearing bodies preserves status specific framing") {
  using setter = response& (response::*)();
  constexpr setter cases[] = {
      &response::continue_100,     &response::switching_protocols_101,
      &response::no_content_204,   &response::reset_content_205,
      &response::not_modified_304,
  };
  for (const auto set : cases) {
    std::array<char, max_response_size_in_memory> storage{};
    response value(storage);
    (value.*set)();
    value.set_header("Date", "fixed").set_body("hidden");
    value.clear_body().clear_body();
    auto serialized = value.serialize();
    const std::string prefix(wire_prefix(serialized));
    DOBA_EXPECT(!serialized.source);
    DOBA_EXPECT(prefix.ends_with("\r\n\r\n"));
    DOBA_EXPECT(prefix.find("Transfer-Encoding:") == std::string_view::npos);
    DOBA_EXPECT_EQUAL(
        prefix.find("Content-Length: 0\r\n") != std::string_view::npos,
        set == &response::reset_content_205);
    if (set != &response::reset_content_205) {
      DOBA_EXPECT(prefix.find("Content-Length:") == std::string_view::npos);
    }
  }
  auto writer = body_writer::chunked();
  DOBA_EXPECT(writer.write("hidden"));
  std::array<char, max_response_size_in_memory> storage_25{};
  response cached(storage_25);
  cached.not_modified_304();
  cached.set_body("old").set_body(std::move(writer));
  auto serialized = cached.serialize();
  const std::string prefix(wire_prefix(serialized));
  DOBA_EXPECT(!serialized.source);
  DOBA_EXPECT(prefix.ends_with("\r\n\r\n"));
  DOBA_EXPECT(prefix.find("Content-Length:") == std::string_view::npos);
  DOBA_EXPECT(prefix.find("Transfer-Encoding:") == std::string_view::npos);
}

// +===========================================================================+
// | [>] suppression preserves metadata and permits replacement  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("suppression preserves metadata and permits replacement") {
  for (bool explicit_length : {false, true}) {
    for (int operation : {0, 1, 2}) {
      std::array<char, max_response_size_in_memory> storage_26{};
      response value(storage_26);
      value.ok_200();
      value.set_header("Date", "fixed").set_body("hidden");
      if (explicit_length) value.set_header("Content-Length", "6");
      DOBA_EXPECT_EQUAL(&value.suppress_body(), &value);
      value.suppress_body();
      if (operation == 1) value.clear_body();
      if (operation == 2) value.set_body("next");
      auto serialized = value.serialize();
      DOBA_EXPECT(!serialized.source);
      const std::string_view expected =
          operation == 0
              ? "HTTP/1.1 200 OK\r\nDate: fixed\r\nContent-Length: 6\r\n\r\n"
          : operation == 1
              ? "HTTP/1.1 200 OK\r\nDate: fixed\r\nContent-Length: 0\r\n\r\n"
              : "HTTP/1.1 200 OK\r\nDate: fixed\r\nContent-Length: "
                "4\r\n\r\nnext";
      DOBA_EXPECT_EQUAL((wire_prefix(serialized)), expected);
    }
  }
}

// +===========================================================================+
// | [>] every status method emits its registered status line    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("every status method emits its registered status line") {
  using setter = response& (response::*)();
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
    std::array<char, max_response_size_in_memory> storage{};
    response value(storage);
    (value.*test.set)();
    value.set_header("Date", "fixed");
    DOBA_EXPECT(!value.has_header("Content-Length"));
    const auto serialized = value.serialize();
    const std::string serialized_prefix((wire_prefix(serialized)));
    DOBA_EXPECT(serialized_prefix.starts_with(test.line));
    DOBA_EXPECT_EQUAL(
        serialized_prefix.find("Content-Length: 0\r\n") != std::string::npos,
        test.content_length);
  }
}

// +===========================================================================+
// | [>] bodyless statuses never serialize response bodies       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("informational 204 205 and 304 responses never serialize bodies") {
  using setter = response& (response::*)();
  constexpr setter cases[] = {
      &response::continue_100,     &response::switching_protocols_101,
      &response::no_content_204,   &response::reset_content_205,
      &response::not_modified_304,
  };
  for (const auto set : cases) {
    std::array<char, max_response_size_in_memory> storage{};
    response value(storage);
    (value.*set)();
    value.set_header("Date", "fixed").set_body("body");
    auto serialized = value.serialize();
    const std::string serialized_prefix((wire_prefix(serialized)));
    const auto boundary = serialized_prefix.find("\r\n\r\n");
    DOBA_EXPECT(boundary != std::string::npos);
    DOBA_EXPECT_EQUAL(serialized_prefix.substr(boundary + 4), "");
    DOBA_EXPECT(!serialized.source);
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
  std::array<char, max_response_size_in_memory> storage_27{};
  response streamed(storage_27);
  streamed.reset_content_205();
  streamed.set_header("Date", "fixed").set_body(std::move(writer));
  const auto serialized = streamed.serialize();
  const std::string serialized_prefix((wire_prefix(serialized)));
  const auto boundary = serialized_prefix.find("\r\n\r\n");
  DOBA_EXPECT(boundary != std::string::npos);
  DOBA_EXPECT_EQUAL(serialized_prefix.substr(boundary + 4), "");
  DOBA_EXPECT(!serialized.source);
  DOBA_EXPECT(serialized_prefix.find("Transfer-Encoding:") ==
              std::string::npos);
  DOBA_EXPECT(serialized_prefix.find("Content-Length: 0\r\n") !=
              std::string::npos);
}

// +===========================================================================+
// | [>] serialized views borrow response storage                ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("serialized views borrow response storage") {
  std::array<char, max_response_size_in_memory> storage{};
  response value(storage);
  value.created_201();
  value.set_header("Date", "fixed").set_body("original");
  auto serialized = value.serialize();
  DOBA_EXPECT_EQUAL(serialized.head.data(), storage.data());
  const std::string prefix(wire_prefix(serialized));
  value.internal_server_error_500().set_body("replacement");
  DOBA_EXPECT_EQUAL(prefix,
                    "HTTP/1.1 201 Created\r\nDate: fixed\r\n"
                    "Content-Length: 8\r\n\r\noriginal");
}

// +===========================================================================+
// | [>] serializing a consumed response fails safely            ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("serializing a consumed response fails safely") {
  std::array<char, max_response_size_in_memory> storage_29{};
  response value(storage_29);
  value.ok_200();
  value.set_header("Date", "fixed").set_body("body");
  auto serialized = value.serialize();
  DOBA_EXPECT(!serialized.head.empty());
  bool threw = false;
  try {
    (void)value.serialize();
  } catch (const std::logic_error&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
}

// +===========================================================================+
// | [>] inline body survives repeated header growth             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("inline body survives repeated header growth") {
  const std::string body(1024, 'b');
  std::array<char, max_response_size_in_memory> storage_30{};
  response value(storage_30);
  value.ok_200();
  value.set_body(body).set_header("Date", "fixed");
  for (std::size_t i = 0; i < 12; i++) {
    value.add_header("X-Test", std::string(i * 17, 'x'));
  }
  value.set_header("X-Test", std::string(200, 'y'));
  value.remove_header("X-Test");
  auto serialized = value.serialize();
  const std::string prefix(wire_prefix(serialized));
  DOBA_EXPECT(prefix.find("Content-Length: 1024\r\n") !=
              std::string_view::npos);
  DOBA_EXPECT(prefix.ends_with(body));
  DOBA_EXPECT(!serialized.source);
}

// +===========================================================================+
// | [>] streamed body survives near-limit headers               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("streamed body survives near-limit headers") {
  std::string body(max_response_body_size_in_memory + 1, '\0');
  for (std::size_t i = 0; i < body.size(); i++) {
    body[i] = static_cast<char>(i % 256);
  }
  const std::size_t head_limit = policies::kMaxResponseHeadSizeInMemory;
  const std::string head = "HTTP/1.1 200 OK\r\nDate: fixed\r\nX-Pad: ";
  const std::string framing =
      "Content-Length: " + std::to_string(body.size()) + "\r\n";
  const std::string padding(head_limit - 32 - head.size() - framing.size() - 4,
                            'x');
  std::array<char, max_response_size_in_memory> storage_31{};
  response value(storage_31);
  value.ok_200();
  value.set_body(body).set_header("Date", "fixed").add_header("X-Pad", padding);
  const auto serialized = value.serialize();
  const std::string serialized_prefix((wire_prefix(serialized)));
  DOBA_EXPECT_EQUAL(serialized_prefix,
                    head + padding + "\r\n" + framing + "\r\n");
  DOBA_EXPECT(serialized.source != nullptr);
  DOBA_EXPECT_EQUAL(read_source(*serialized.source), body);
}

// +===========================================================================+
// | [>] automatic framing stays deferred until serialization    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("automatic framing stays deferred until serialization") {
  std::array<char, max_response_size_in_memory> storage_32{};
  response value(storage_32);
  value.ok_200();
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
  const std::string prefix(wire_prefix(serialized));
  DOBA_EXPECT(prefix.find("Content-Length: 2\r\n") != std::string_view::npos);
  DOBA_EXPECT(prefix.find("Content-Length:") ==
              prefix.rfind("Content-Length:"));
  DOBA_EXPECT(prefix.ends_with("\r\n\r\nok"));
}

// +===========================================================================+
// | [>] body replacement clears explicit framing duplicates     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("body replacement clears explicit framing duplicates") {
  std::array<char, max_response_size_in_memory> storage_33{};
  response value(storage_33);
  value.ok_200();
  value.add_header("content-length", "12")
      .add_header("Content-Length", "12")
      .add_header("Transfer-Encoding", "chunked")
      .add_header("transfer-encoding", "chunked");
  response moved(std::move(value));
  std::array<char, max_response_size_in_memory> new_storage{};
  response replacement(new_storage);
  replacement.ok_200();
  value = std::move(replacement);
  value = std::move(moved);
  value.set_body("xy");
  DOBA_EXPECT_EQUAL(value.get_headers_length(), 0);
  auto serialized = value.serialize();
  const std::string prefix(wire_prefix(serialized));
  DOBA_EXPECT(prefix.find("Content-Length: 2\r\n") != std::string_view::npos);
  DOBA_EXPECT(prefix.find("Transfer-Encoding:") == std::string_view::npos);
  DOBA_EXPECT(prefix.ends_with("\r\n\r\nxy"));
}

// +===========================================================================+
// | [>] explicit framing overrides deferred framing             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("explicit framing overrides deferred framing") {
  std::array<char, max_response_size_in_memory> storage_34{};
  response value(storage_34);
  value.ok_200();
  value.set_body("abc").set_header("content-length", "3");
  DOBA_EXPECT_EQUAL(value.get_header("Content-Length").second, "3");
  auto serialized = value.serialize();
  const std::string prefix(wire_prefix(serialized));
  DOBA_EXPECT(prefix.find("content-length: 3\r\n") != std::string_view::npos);
  DOBA_EXPECT(prefix.find("Content-Length:") == std::string_view::npos);
  std::array<char, max_response_size_in_memory> storage_35{};
  response head(storage_35);
  head.ok_200();
  head.set_header("Transfer-Encoding", "chunked").suppress_body();
  auto head_serialized = head.serialize();
  const std::string head_prefix(wire_prefix(head_serialized));
  DOBA_EXPECT(head_prefix.find("Transfer-Encoding: chunked\r\n") !=
              std::string_view::npos);
  DOBA_EXPECT(head_prefix.find("Content-Length:") == std::string_view::npos);
}

// +===========================================================================+
// | [>] conflicting framing is rejected before transmission     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("conflicting response framing is rejected before transmission") {
  for (bool automatic : {false, true}) {
    std::array<char, max_response_size_in_memory> storage_36{};
    response value(storage_36);
    value.ok_200();
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
// | [>] HEAD preserves deferred framing through moves           ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("HEAD preserves deferred framing through moves") {
  for (bool chunked : {false, true}) {
    auto writer = chunked ? body_writer::chunked() : body_writer::raw();
    DOBA_EXPECT(writer.write("resource"));
    std::array<char, max_response_size_in_memory> storage_37{};
    response value(storage_37);
    value.ok_200();
    value.set_body(std::move(writer)).suppress_body();
    response moved(std::move(value));
    std::array<char, max_response_size_in_memory> new_storage{};
    response replacement(new_storage);
    replacement.created_201();
    value = std::move(replacement);
    value = std::move(moved);
    auto serialized = value.serialize();
    const std::string prefix(wire_prefix(serialized));
    DOBA_EXPECT(!serialized.source);
    DOBA_EXPECT(prefix.ends_with("\r\n\r\n"));
    DOBA_EXPECT_EQUAL(
        prefix.find("Content-Length: 8\r\n") != std::string_view::npos,
        !chunked);
    DOBA_EXPECT_EQUAL(
        prefix.find("Transfer-Encoding: chunked\r\n") != std::string_view::npos,
        chunked);
  }
}

// +===========================================================================+
// | [>] deferred length respects the exact header boundary      ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("deferred length respects the exact header boundary") {
  for (std::size_t size : {0, 9, 10, 99, 100}) {
    for (std::size_t excess : {0, 1}) {
      std::array<char, max_response_size_in_memory> storage_38{};
      response value(storage_38);
      value.ok_200();
      const std::string body(size, 'a');
      const std::string head = "HTTP/1.1 200 OK\r\nDate: fixed\r\nX-Pad: ";
      const std::string framing =
          "Content-Length: " + std::to_string(size) + "\r\n";
      const std::size_t body_begin =
          max_response_size_in_memory - max_response_body_size_in_memory;
      const std::string padding(
          body_begin - head.size() - framing.size() - 4 + excess, 'x');
      value.set_body(body)
          .set_header("Date", "fixed")
          .add_header("X-Pad", padding);
      bool threw = false;
      try {
        auto serialized = value.serialize();
        std::string actual((wire_prefix(serialized)));
        if (serialized.source) actual += read_source(*serialized.source);
        DOBA_EXPECT_EQUAL(actual,
                          head + padding + "\r\n" + framing + "\r\n" + body);
      } catch (const std::out_of_range&) {
        threw = true;
      }
      DOBA_EXPECT_EQUAL(threw, excess != 0);
    }
  }
}

// +===========================================================================+
// | [>] removing one Date preserves the remaining explicit Date ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("removing one Date preserves the remaining explicit Date") {
  std::array<char, max_response_size_in_memory> storage_39{};
  response value(storage_39);
  value.ok_200();
  value.add_header("Date", "first").add_header("Date", "remaining");
  value.remove_header("date");
  DOBA_EXPECT_EQUAL(value.get_header("Date").second, "remaining");
  const auto serialized = value.serialize();
  const std::string prefix(wire_prefix(serialized));
  DOBA_EXPECT_EQUAL(prefix,
                    "HTTP/1.1 200 OK\r\nDate: remaining\r\n"
                    "Content-Length: 0\r\n\r\n");
}

// +===========================================================================+
// | [>] removing the last Date restores automatic generation    ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("removing the last Date restores automatic generation") {
  std::array<char, max_response_size_in_memory> storage_40{};
  response value(storage_40);
  value.ok_200();
  value.add_header("DATE", "first").add_header("date", "remaining");
  value.add_header("X-Keep", "kept");
  value.remove_header("DaTe").remove_header("DATE");
  DOBA_EXPECT(!value.has_header("Date"));
  const auto serialized = value.serialize();
  const std::string prefix(wire_prefix(serialized));
  DOBA_EXPECT(prefix.find("\r\nX-Keep: kept\r\n") != std::string_view::npos);
  const auto date = prefix.find("\r\nDate: ");
  DOBA_EXPECT(date != std::string_view::npos);
  DOBA_EXPECT_EQUAL(prefix.find("\r\n", date + 2), date + 8 + 29);
  DOBA_EXPECT_EQUAL(prefix.find("\r\nDate: ", date + 1),
                    std::string_view::npos);
}

// +===========================================================================+
// | [>] header names validate every byte before mutation        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("header names validate every byte before mutation") {
  for (unsigned int byte = 0; byte < 256; byte++) {
    martianlabs::doba::tests::unit::test_helper::set_context(
        std::to_string(byte));
    const char ch = static_cast<char>(byte);
    const std::string candidate = std::string("a") + ch + "b";
    const bool valid =
        (byte >= '0' && byte <= '9') || (byte >= 'A' && byte <= 'Z') ||
        (byte >= 'a' && byte <= 'z') || byte == 96 ||
        std::string_view("!#$%&'*+-.^_|~").find(ch) != std::string_view::npos;
    for (bool replace : {false, true}) {
      std::array<char, max_response_size_in_memory> storage_41{};
      response value(storage_41);
      value.ok_200();
      value.add_header("X-Keep", "original");
      bool threw = false;
      try {
        if (replace)
          value.set_header(candidate, "new");
        else
          value.add_header(candidate, "new");
      } catch (const std::invalid_argument&) {
        threw = true;
      }
      DOBA_EXPECT_EQUAL(threw, !valid);
      DOBA_EXPECT_EQUAL(value.get_header("X-Keep").second, "original");
      DOBA_EXPECT_EQUAL(value.get_headers_length(), valid ? 2 : 1);
      if (valid) DOBA_EXPECT_EQUAL(value.get_header(candidate).second, "new");
    }
  }
}

// +===========================================================================+
// | [>] header values validate every byte before mutation       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("header values validate every byte before mutation") {
  for (unsigned int byte = 0; byte < 256; byte++) {
    martianlabs::doba::tests::unit::test_helper::set_context(
        std::to_string(byte));
    const char ch = static_cast<char>(byte);
    const std::string candidate = std::string("a") + ch + "b";
    const bool valid = byte == 9 || (byte >= 32 && byte != 127);
    for (bool replace : {false, true}) {
      std::array<char, max_response_size_in_memory> storage_42{};
      response value(storage_42);
      value.ok_200();
      value.add_header("X-Keep", "original");
      bool threw = false;
      try {
        if (replace)
          value.set_header("X-Keep", candidate);
        else
          value.add_header("X-New", candidate);
      } catch (const std::invalid_argument&) {
        threw = true;
      }
      DOBA_EXPECT_EQUAL(threw, !valid);
      DOBA_EXPECT_EQUAL(value.get_header("X-Keep").second,
                        valid && replace ? candidate : "original");
      DOBA_EXPECT_EQUAL(value.get_headers_length(), valid && !replace ? 2 : 1);
      if (valid && !replace) {
        DOBA_EXPECT_EQUAL(value.get_header("X-New").second, candidate);
      }
    }
  }
}

// +===========================================================================+
// | [>] header growth at capacity preserves neighbors and body  ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("header growth at capacity preserves neighbors and body") {
  std::array<char, max_response_size_in_memory> storage_43{};
  response value(storage_43);
  value.ok_200();
  const std::string head =
      "HTTP/1.1 200 OK\r\nDate: fixed\r\nContent-Length: 4\r\nX: ";
  const std::string tail = "\r\nY: sentinel\r\n\r\n";
  const std::size_t body_begin =
      max_response_size_in_memory - max_response_body_size_in_memory;
  const std::string padding(body_begin - head.size() - tail.size(), 'x');
  value.set_body("body")
      .set_header("Date", "fixed")
      .set_header("Content-Length", "4")
      .add_header("X", "small")
      .add_header("Y", "sentinel");
  value.set_header("X", padding);
  bool threw = false;
  try {
    value.set_header("X", padding + "x");
  } catch (const std::out_of_range&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  DOBA_EXPECT_EQUAL(value.get_header("X").second, padding);
  DOBA_EXPECT_EQUAL(value.get_header("Y").second, "sentinel");
  value.set_header("X", "short").set_header("X", "equal");
  DOBA_EXPECT_EQUAL(value.get_header("Y").second, "sentinel");
  value.set_header("X", padding);
  const auto serialized = value.serialize();
  DOBA_EXPECT_EQUAL((wire_prefix(serialized)), head + padding + tail + "body");
  DOBA_EXPECT(!serialized.source);
}

// +===========================================================================+
// | [>] response adopts a reader at its current position        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("response adopts a reader at its current position") {
  const std::string bytes = "abcdef";
  auto source = reader::borrowed(std::as_bytes(std::span(bytes)));
  std::byte first{};
  DOBA_EXPECT(source.fetch(first));
  std::array<char, max_response_size_in_memory> storage_44{};
  response value(storage_44);
  value.ok_200();
  value.set_body(std::move(source), 5);
  auto moved = std::move(value);
  std::array<char, max_response_size_in_memory> storage_45{};
  response assigned(storage_45);
  assigned.ok_200();
  assigned.set_body("old");
  assigned = std::move(moved);
  auto serialized = assigned.serialize();
  DOBA_EXPECT(serialized.source != nullptr);
  const std::string prefix((wire_prefix(serialized)));
  DOBA_EXPECT(prefix.find("Content-Length: 5\r\n") != prefix.npos);
  DOBA_EXPECT_EQUAL(read_source(*serialized.source), "bcdef");
}

// +===========================================================================+
// | [>] reader body replacement and suppression                 ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("reader bodies are replaced or suppressed without leaking bytes") {
  const std::string bytes = "hidden";
  std::array<char, max_response_size_in_memory> storage_46{};
  response value(storage_46);
  value.ok_200();
  value.set_body(reader::borrowed(std::as_bytes(std::span(bytes))), 6);
  value.set_body("next");
  auto serialized = value.serialize();
  DOBA_EXPECT(!serialized.source);
  DOBA_EXPECT((wire_prefix(serialized)).ends_with("next"));
  for (auto result : {0, 1, 2, 3, 4}) {
    std::array<char, max_response_size_in_memory> next_storage{};
    response res(next_storage);
    if (result == 0) res.ok_200();
    if (result == 1) res.no_content_204();
    if (result == 2) res.reset_content_205();
    if (result == 3) res.not_modified_304();
    if (result == 4) res.continue_100();
    res.set_body(reader::borrowed(std::as_bytes(std::span(bytes))), 6);
    if (result == 0) res.suppress_body();
    auto wire = res.serialize();
    DOBA_EXPECT(!wire.source);
    const std::string prefix(wire_prefix(wire));
    DOBA_EXPECT(!prefix.ends_with(bytes));
    if (result == 0) {
      DOBA_EXPECT(prefix.find("Content-Length: 6\r\n") != prefix.npos);
    }
  }
}
