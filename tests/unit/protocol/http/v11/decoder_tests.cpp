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
#include <array>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <stdexcept>
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
using martianlabs::doba::protocol::deserialization_status;
using martianlabs::doba::protocol::http::target;
using martianlabs::doba::protocol::http::v11::policies;
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using decoder_type =
    martianlabs::doba::protocol::http::v11::decoder<request, response>;

constexpr std::size_t receive_capacity = 5120;
constexpr std::size_t max_query_parameters = 128;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] decoder_input                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct decoder_input {
  explicit decoder_input(policies configuration = {})
      : decoder{configuration} {}
  std::size_t append(std::string_view source) {
    const auto count = std::min(source.size(), receive_capacity - size);
    if (count) std::memcpy(buffer.data() + size, source.data(), count);
    size += count;
    return count;
  }
  auto decode() {
    std::size_t consumed = 0;
    request_seen = false;
    auto result = decoder.deserialize(
        buffer.data(), size, buffer.size(), consumed,
        [this](const request& value) {
          request_seen = true;
          if (on_request) on_request(value);
        });
    if (consumed > size) {
      throw std::runtime_error("Invalid decoder consumption");
    }
    size -= consumed;
    if (size && consumed) {
      std::memmove(buffer.data(), buffer.data() + consumed, size);
    }
    return result;
  }
  decoder_type decoder;
  std::function<void(const request&)> on_request;
  bool request_seen = false;
  std::array<char, receive_capacity> buffer{};
  std::size_t size{0};
};
std::size_t accumulate(decoder_input& value, std::string_view source) {
  return value.append(source);
}
std::vector<std::filesystem::path> body_temp_files() {
  namespace fs = std::filesystem;
  std::vector<fs::path> paths;
  std::error_code error;
  const auto directory = fs::temp_directory_path(error);
  if (error) return paths;
  for (fs::directory_iterator it(directory, error), end;
       !error && it != end; it.increment(error)) {
    if (it->path().filename().string().starts_with("doba_bytes_")) {
      paths.push_back(it->path());
    }
  }
  return paths;
}
std::filesystem::path new_body_temp_file(
    const std::vector<std::filesystem::path>& before) {
  for (const auto& path : body_temp_files()) {
    if (std::find(before.begin(), before.end(), path) == before.end()) {
      return path;
    }
  }
  return {};
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
      decoder_input value;
      if (accepted) {
        value.on_request = [&](const request& decoded) {
          DOBA_EXPECT_EQUAL(decoded.get_header(name).second, field_value);
          if (decoded.has_body_reader()) {
            std::array<std::byte, 1> output{};
            const auto state = decoded.get_body_reader()->read(output);
            DOBA_EXPECT(!state.has_error);
            DOBA_EXPECT(state.complete);
            DOBA_EXPECT_EQUAL(state.produced, 0);
          }
        };
      }
      DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
      const auto result = value.decode();
      if (accepted) {
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
        DOBA_EXPECT(value.request_seen);
      } else {
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
        DOBA_EXPECT(!value.request_seen);
      }
    }
  }
}
}  // namespace

// +===========================================================================+
// | [>] decoder is neither copyable nor movable                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder is neither copyable nor movable") {
  static_assert(!std::is_copy_constructible_v<decoder_type>);
  static_assert(!std::is_copy_assignable_v<decoder_type>);
  static_assert(!std::is_move_constructible_v<decoder_type>);
  static_assert(!std::is_move_assignable_v<decoder_type>);
  DOBA_EXPECT(true);
}
// +===========================================================================+
// | [>] empty input and full receive buffer                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("empty input needs more bytes and full incomplete cores fail") {
  decoder_type value;
  std::size_t consumed = 42;
  const std::string source(receive_capacity, 'x');
  DOBA_EXPECT_EQUAL(value.deserialize(source.data(), 0, receive_capacity,
                                      consumed, [](const request&) {}).code,
                    deserialization_status::kMoreBytesNeeded);
  DOBA_EXPECT_EQUAL(consumed, 0);
  DOBA_EXPECT_EQUAL(value.deserialize(source.data(), source.size(),
                                      receive_capacity, consumed,
                                      [](const request&) {}).code,
                    deserialization_status::kInvalidSource);
  DOBA_EXPECT_EQUAL(consumed, 0);
}
// +===========================================================================+
// | [>] request head fills default receive buffer               ( test-case ) |
// +===========================================================================+
DOBA_TEST("request head may fill default receive buffer") {
  constexpr std::size_t capacity = policies::kMaxRequestHeadSizeInMemory;
  constexpr std::string_view prefix = "GET / HTTP/1.1\r\nHost: a\r\nX: ";
  constexpr std::string_view suffix = "\r\n\r\n";
  const std::string head =
      std::string(prefix) +
      std::string(capacity - prefix.size() - suffix.size(), 'x') +
      std::string(suffix);
  decoder_type value;
  std::size_t consumed = 0;
  bool request_seen = false;
  const auto result = value.deserialize(head.data(), head.size(), capacity,
                                        consumed, [&](const request&) {
                                          request_seen = true;
                                        });
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
  DOBA_EXPECT_EQUAL(consumed, capacity);
  DOBA_EXPECT(request_seen);
}
// +===========================================================================+
// | [>] incomplete head fills default receive buffer            ( test-case ) |
// +===========================================================================+
DOBA_TEST("incomplete head fails at default receive capacity") {
  constexpr std::size_t capacity = policies::kMaxRequestHeadSizeInMemory;
  constexpr std::string_view prefix = "GET / HTTP/1.1\r\nHost: a\r\nX: ";
  const std::string head =
      std::string(prefix) + std::string(capacity - prefix.size(), 'x');
  decoder_type value;
  std::size_t consumed = 0;
  const auto result = value.deserialize(head.data(), head.size(), capacity,
                                        consumed, [](const request&) {});
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
  DOBA_EXPECT_EQUAL(consumed, 0);
}
// +===========================================================================+
// | [>] request head size is bounded by receive capacity        ( test-case ) |
// +===========================================================================+
DOBA_TEST("request head size is bounded by receive capacity") {
  constexpr std::size_t capacity = policies::kMaxRequestHeadSizeInMemory;
  struct shape {
    std::string_view name;
    std::string_view prefix;
    std::string_view tail;
    char fill;
  };
  constexpr shape shapes[] = {
      {"single oversized field", "GET / HTTP/1.1\r\nHost: a\r\nX: ", "\r\n\r\n",
       'x'},
      {"long request target", "GET /", " HTTP/1.1\r\nHost: a\r\n\r\n", 'a'},
  };
  for (const auto& test : shapes) {
    for (std::size_t size : {capacity - 1, capacity, capacity + 1}) {
      martianlabs::doba::tests::unit::test_helper::set_context(
          std::string(test.name) + ", head size " + std::to_string(size));
      const std::string head =
          std::string(test.prefix) +
          std::string(size - test.prefix.size() - test.tail.size(),
                      test.fill) +
          std::string(test.tail);
      DOBA_EXPECT_EQUAL(head.size(), size);
      decoder_type value;
      std::size_t consumed = 0;
      bool request_seen = false;
      const std::size_t received = std::min(size, capacity);
      auto result = value.deserialize(head.data(), received, capacity,
                                      consumed, [&](const request&) {
                                        request_seen = true;
                                      });
      if (size <= capacity) {
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
        DOBA_EXPECT_EQUAL(consumed, size);
        DOBA_EXPECT(request_seen);
        continue;
      }
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
      DOBA_EXPECT(!request_seen);
      DOBA_EXPECT(result.response.has_value());
      if (!result.response.has_value()) continue;
      auto output = result.response->serialize();
      DOBA_EXPECT((output->head + output->body)
                      .starts_with("HTTP/1.1 400 "));
    }
  }
}
// +===========================================================================+
// | [>] header field count is bounded only by head size         ( test-case ) |
// +===========================================================================+
DOBA_TEST("header field count is bounded only by head size") {
  constexpr std::size_t capacity = policies::kMaxRequestHeadSizeInMemory;
  std::string head = "GET / HTTP/1.1\r\nHost: a\r\n";
  std::size_t fields = 0;
  while (head.size() + 8 + 2 <= capacity) {
    head += "X" + std::to_string(fields % 1000);
    head += std::string(4 - std::to_string(fields % 1000).size(), 'x');
    head += ":\r\n";
    fields++;
  }
  head += "\r\n";
  DOBA_EXPECT(head.size() <= capacity);
  DOBA_EXPECT(fields > 100);
  decoder_type value;
  std::size_t consumed = 0;
  const auto result = value.deserialize(head.data(), head.size(), capacity,
                                        consumed, [](const request&) {});
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
  DOBA_EXPECT_EQUAL(consumed, head.size());
}
// +===========================================================================+
// | [>] request body spans default receive buffers              ( test-case ) |
// +===========================================================================+
DOBA_TEST("request body spans default receive buffers") {
  constexpr std::size_t capacity = policies::kMaxRequestHeadSizeInMemory;
  const std::string head =
      "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 4097\r\n\r\n";
  const std::string body(capacity + 1, 'x');
  const std::size_t first_body_size = capacity - head.size();
  const std::string first = head + body.substr(0, first_body_size);
  decoder_type value;
  std::size_t consumed = 0;
  bool request_seen = false;
  auto result = value.deserialize(first.data(), first.size(), capacity,
                                  consumed, [&](const request&) {
                                    request_seen = true;
                                  });
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
  DOBA_EXPECT_EQUAL(consumed, capacity);
  const std::string_view remaining(body.data() + first_body_size,
                                   body.size() - first_body_size);
  result = value.deserialize(remaining.data(), remaining.size(), capacity,
                             consumed, [&](const request&) {
                               request_seen = true;
                             });
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
  DOBA_EXPECT_EQUAL(consumed, remaining.size());
  DOBA_EXPECT(request_seen);
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
  decoder_input value;
    value.on_request = [&](const request& decoded) {
      DOBA_EXPECT_EQUAL(decoded.get_method(), test.method);
      DOBA_EXPECT_EQUAL(decoded.get_target(), test.target_form);
      DOBA_EXPECT_EQUAL(decoded.get_absolute_path(), test.path);
    };
    DOBA_EXPECT_EQUAL(accumulate(value, test.source), test.source.size());
    const auto result = value.decode();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
    DOBA_EXPECT(value.request_seen);
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
  decoder_input value;
    value.on_request = [&](const request& decoded) {
      DOBA_EXPECT_EQUAL(decoded.get_absolute_path(), "/a b");
      DOBA_EXPECT_EQUAL(decoded.get_query_parameters_length(), 2);
      DOBA_EXPECT_EQUAL(decoded.get_query_parameter("x")->second, "1");
      DOBA_EXPECT(decoded.get_query_parameter("empty")->second.empty());
      DOBA_EXPECT_EQUAL(decoded.get_header("X-Test").second, "value");
    };
    DOBA_EXPECT_EQUAL(accumulate(value, source.substr(0, split)), split);
    auto result = value.decode();
    if (split < source.size()) {
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
      DOBA_EXPECT_EQUAL(accumulate(value, source.substr(split)),
                        source.size() - split);
      result = value.decode();
    }
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
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
      decoder_input value;
      value.on_request = [&](const request& decoded) {
        DOBA_EXPECT_EQUAL(decoded.get_target(), test.target_form);
      };
      DOBA_EXPECT_EQUAL(accumulate(value, test.source.substr(0, split)), split);
      auto result = value.decode();
      if (split < test.source.size()) {
        DOBA_EXPECT_EQUAL(result.code,
                          deserialization_status::kMoreBytesNeeded);
        DOBA_EXPECT_EQUAL(accumulate(value, test.source.substr(split)),
                          test.source.size() - split);
        result = value.decode();
      }
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
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
  decoder_input value;
    value.on_request = [&](const request& decoded) {
      DOBA_EXPECT(decoded.has_body_reader());
      std::array<std::byte, 8> output{};
      const auto state = decoded.get_body_reader()->read(output);
      DOBA_EXPECT(state.complete);
      DOBA_EXPECT_EQUAL(state.produced, 7);
      DOBA_EXPECT_EQUAL(
          std::string_view(reinterpret_cast<const char*>(output.data()), 7),
          "payload");
    };
    DOBA_EXPECT_EQUAL(
        accumulate(value, std::string_view(source).substr(0, split)), split);
    auto result = value.decode();
    if (split < source.size()) {
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
      DOBA_EXPECT_EQUAL(
          accumulate(value, std::string_view(source).substr(split)),
          source.size() - split);
      result = value.decode();
    }
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
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
  decoder_input value;
  value.on_request = [&](const request& decoded) {
    std::array<std::byte, 8> output{};
    const auto state = decoded.get_body_reader()->read(output);
    DOBA_EXPECT(state.complete);
    DOBA_EXPECT_EQUAL(state.produced, 5);
    DOBA_EXPECT_EQUAL(
        std::string_view(reinterpret_cast<const char*>(output.data()), 5),
        "abcde");
  };
  DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
  const auto result = value.decode();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
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
    decoder_input value;
    value.on_request = [&](const request& decoded) {
      std::array<std::byte, 8> output{};
      const auto state = decoded.get_body_reader()->read(output);
      DOBA_EXPECT(state.complete);
      DOBA_EXPECT_EQUAL(state.produced, 5);
      DOBA_EXPECT_EQUAL(
          std::string_view(reinterpret_cast<const char*>(output.data()), 5),
          "abcde");
    };
    DOBA_EXPECT_EQUAL(accumulate(value, source.substr(0, split)), split);
    auto result = value.decode();
    if (split < source.size()) {
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
      DOBA_EXPECT_EQUAL(accumulate(value, source.substr(split)),
                        source.size() - split);
      result = value.decode();
    }
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
  }
}
// +===========================================================================+
// | [>] expect continue waits for the complete body             ( test-case ) |
// +===========================================================================+
DOBA_TEST("expect continue decoding waits for the complete body") {
  constexpr std::string_view head =
      "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 1\r\n"
      "Expect: 100-continue\r\n\r\n";
  decoder_input value;
  DOBA_EXPECT_EQUAL(accumulate(value, head), head.size());
  auto result = value.decode();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
  DOBA_EXPECT_EQUAL(accumulate(value, "x"), 1);
  result = value.decode();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
}
// +===========================================================================+
// | [>] connection close is retained on the decoded request     ( test-case ) |
// +===========================================================================+
DOBA_TEST("connection close is retained on the decoded request") {
  constexpr std::string_view source =
      "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
  decoder_input value;
  value.on_request = [&](const request& decoded) {
    DOBA_EXPECT(decoded.wants_connection_close());
  };
  DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
  const auto result = value.decode();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
}
// +===========================================================================+
// | [>] decoder reuses storage after sequential dispatch        ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder reuses storage after sequential dispatch") {
  constexpr std::string_view first =
      "GET /one HTTP/1.1\r\nHost: one.example\r\nX-Id: first\r\n\r\n";
  constexpr std::string_view second =
      "GET /two HTTP/1.1\r\nHost: two.example\r\nX-Id: second\r\n\r\n";
  const std::string source = std::string(first) + std::string(second);
  decoder_input value;
  std::string first_path;
  std::string first_host;
  std::string first_id;
  value.on_request = [&](const request& decoded) {
    first_path = decoded.get_absolute_path();
    first_host = decoded.get_header("Host").second;
    first_id = decoded.get_header("X-Id").second;
  };
  DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
  const auto first_result = value.decode();
  DOBA_EXPECT_EQUAL(first_result.code, deserialization_status::kSucceeded);
  value.on_request = [&](const request& decoded) {
    DOBA_EXPECT_EQUAL(decoded.get_absolute_path(), "/two");
    DOBA_EXPECT_EQUAL(decoded.get_header("Host").second, "two.example");
  };
  const auto second_result = value.decode();
  DOBA_EXPECT_EQUAL(second_result.code, deserialization_status::kSucceeded);
  DOBA_EXPECT_EQUAL(first_path, "/one");
  DOBA_EXPECT_EQUAL(first_host, "one.example");
  DOBA_EXPECT_EQUAL(first_id, "first");
  for (std::size_t i = 0; i < 8; i++) {
    const std::string next =
        "GET /reload-" + std::to_string(i) + " HTTP/1.1\r\n"
        "Host: reload.example\r\nX-Id: " + std::string(400, 'x') + "\r\n\r\n";
    DOBA_EXPECT_EQUAL(accumulate(value, next), next.size());
    value.on_request = [&](const request& decoded) {
      DOBA_EXPECT_EQUAL(decoded.get_absolute_path(),
                        "/reload-" + std::to_string(i));
    };
    const auto result = value.decode();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
    DOBA_EXPECT(value.request_seen);
    DOBA_EXPECT_EQUAL(first_path, "/one");
    DOBA_EXPECT_EQUAL(first_host, "one.example");
    DOBA_EXPECT_EQUAL(first_id, "first");
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
    decoder_input value;
    value.on_request = [&](const request& decoded) {
      DOBA_EXPECT_EQUAL(decoded.get_header(test.name).second, test.value);
    };
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    const auto result = value.decode();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
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
        decoder_input value;
        auto result = value.decode();
        std::size_t offset = 0;
        for (std::size_t end : ends) {
          DOBA_EXPECT_EQUAL(
              accumulate(
                  value, std::string_view(source).substr(offset, end - offset)),
              end - offset);
          offset = end;
          result = value.decode();
          DOBA_EXPECT(!value.request_seen);
          if (result.code == deserialization_status::kInvalidSource) break;
          DOBA_EXPECT_EQUAL(result.code,
                            deserialization_status::kMoreBytesNeeded);
        }
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
        DOBA_EXPECT(!value.request_seen);
        DOBA_EXPECT(result.response.has_value());
        auto output = result.response->serialize();
        DOBA_EXPECT((output->head + output->body)
                        .starts_with("HTTP/1.1 400 "));
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
      decoder_input value;
      DOBA_EXPECT_EQUAL(accumulate(value, test.source.substr(0, split)), split);
      auto result = value.decode();
      const auto prefix_expected = test.invalid_at != 0 &&
                                   split >= test.invalid_at
          ? deserialization_status::kInvalidSource
          : deserialization_status::kMoreBytesNeeded;
      martianlabs::doba::tests::unit::test_helper::set_context(
          std::string(test.source) + ", prefix " + std::to_string(split));
      DOBA_EXPECT_EQUAL(result.code, prefix_expected);
      DOBA_EXPECT(!value.request_seen);
      if (result.code != deserialization_status::kInvalidSource) {
        DOBA_EXPECT_EQUAL(accumulate(value, test.source.substr(split)),
                          test.source.size() - split);
        result = value.decode();
      }
      DOBA_EXPECT_EQUAL(result.code, test.expected);
      DOBA_EXPECT(!value.request_seen);
    }
    decoder_input value;
    for (std::size_t end = 1; end <= test.source.size(); end++) {
      DOBA_EXPECT_EQUAL(accumulate(value, test.source.substr(end - 1, 1)), 1);
      const auto result = value.decode();
      const auto expected = test.invalid_at != 0 && end >= test.invalid_at
          ? deserialization_status::kInvalidSource
          : deserialization_status::kMoreBytesNeeded;
      DOBA_EXPECT_EQUAL(result.code, expected);
      DOBA_EXPECT(!value.request_seen);
      if (result.code == deserialization_status::kInvalidSource) break;
    }
  }
}
// +===========================================================================+
// | [>] rejects invalid cross header combinations               ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects invalid cross header combinations") {
  struct test_case {
    std::string_view invalid;
    std::string_view valid;
  };
  constexpr test_case cases[] = {
      {"GET / HTTP/1.1\r\n\r\n", "GET / HTTP/1.1\r\nHost: a\r\n\r\n"},
      {"GET / HTTP/1.1\r\nHost: a\r\nHost: a\r\n\r\n",
       "GET / HTTP/1.1\r\nHost: a\r\n\r\n"},
      {"POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 0\r\n"
       "Content-Length: 0\r\n\r\n",
       "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 0\r\n\r\n"},
      {"POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 0, 0\r\n\r\n",
       "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 0\r\n\r\n"},
      {"POST / HTTP/1.1\r\nHost: a\r\nContent-Length: +1\r\n\r\nx",
       "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\n\r\nx"},
      {"POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\n"
       "Transfer-Encoding: chunked\r\n\r\n0\r\n\r\n",
       "POST / HTTP/1.1\r\nHost: a\r\n"
       "Transfer-Encoding: chunked\r\n\r\n0\r\n\r\n"},
      {"POST / HTTP/1.1\r\nHost: a\r\n"
       "Transfer-Encoding: chunked, gzip\r\n\r\n0\r\n\r\n",
       "POST / HTTP/1.1\r\nHost: a\r\n"
       "Transfer-Encoding: gzip, chunked\r\n\r\n0\r\n\r\n"},
      {"GET / HTTP/1.1\r\nHost: a\r\nConnection: host\r\n\r\n",
       "GET / HTTP/1.1\r\nHost: a\r\nConnection: close\r\n\r\n"},
      {"GET / HTTP/1.1\r\nHost: a\r\nConnection: upgrade\r\n\r\n",
       "GET / HTTP/1.1\r\nHost: a\r\nConnection: keep-alive\r\n\r\n"},
  };
  for (const auto& test : cases) {
    martianlabs::doba::tests::unit::test_helper::set_context(
        "control " + std::string(test.valid));
    decoder_input control;
    DOBA_EXPECT_EQUAL(accumulate(control, test.valid), test.valid.size());
    DOBA_EXPECT_EQUAL(control.decode().code,
                      deserialization_status::kSucceeded);
    const std::string_view source = test.invalid;
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
        decoder_input value;
        auto result = value.decode();
        std::size_t offset = 0;
        for (std::size_t end : ends) {
          DOBA_EXPECT_EQUAL(
              accumulate(
                  value, std::string_view(source).substr(offset, end - offset)),
              end - offset);
          offset = end;
          result = value.decode();
          DOBA_EXPECT(!value.request_seen);
          if (result.code == deserialization_status::kInvalidSource) break;
          DOBA_EXPECT_EQUAL(result.code,
                            deserialization_status::kMoreBytesNeeded);
        }
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
        DOBA_EXPECT(!value.request_seen);
        DOBA_EXPECT(result.response.has_value());
        auto output = result.response->serialize();
        DOBA_EXPECT((output->head + output->body)
                        .starts_with("HTTP/1.1 400 "));
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
      decoder_input value;
      auto result = value.decode();
      std::size_t offset = 0;
      for (std::size_t end : ends) {
        DOBA_EXPECT_EQUAL(
            accumulate(
                value, std::string_view(source).substr(offset, end - offset)),
            end - offset);
        offset = end;
        result = value.decode();
        DOBA_EXPECT(!value.request_seen);
        if (result.code == deserialization_status::kInvalidSource) break;
        DOBA_EXPECT_EQUAL(result.code,
                          deserialization_status::kMoreBytesNeeded);
      }
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
      DOBA_EXPECT(!value.request_seen);
      DOBA_EXPECT(result.response.has_value());
      auto output = result.response->serialize();
      DOBA_EXPECT((output->head + output->body)
                      .starts_with("HTTP/1.1 400 "));
    }
  }
}
// +===========================================================================+
// | [>] content length permits leading zeroes                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("content length permits leading zeroes") {
  constexpr std::string_view source =
      "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 01\r\n\r\nx";
  decoder_input value;
  value.on_request = [&](const request& decoded) {
    std::byte output;
    const auto state = decoded.get_body_reader()->read(
        std::span<std::byte>(&output, 1));
    DOBA_EXPECT(state.complete);
    DOBA_EXPECT_EQUAL(static_cast<char>(output), 'x');
  };
  DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
  const auto result = value.decode();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
}
// +===========================================================================+
// | [>] rejects request smuggling vectors                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects request smuggling vectors") {
  struct test_case {
    std::string_view invalid;
    std::string_view valid;
    std::string_view status;
  };
  constexpr test_case cases[] = {
      {"POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 1 0\r\n\r\nx",
       "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\n\r\nx",
       "HTTP/1.1 400 "},
      {"POST / HTTP/1.1\r\nHost: a\r\nContent-Length: -1\r\n\r\nx",
       "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\n\r\nx",
       "HTTP/1.1 400 "},
      {"POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 1x\r\n\r\nx",
       "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\n\r\nx",
       "HTTP/1.1 400 "},
      {"POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\n"
       "Content-Length: 2\r\n\r\nxx",
       "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 2\r\n\r\nxx",
       "HTTP/1.1 400 "},
      {"POST / HTTP/1.1\r\nHost: a\r\n"
       "Content-Length: 99999999999999999999999\r\n\r\n",
       "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 0\r\n\r\n",
       "HTTP/1.1 400 "},
      {"POST / HTTP/1.1\r\nHost: a\r\nContent-Length : 1\r\n\r\nx",
       "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\n\r\nx",
       "HTTP/1.1 400 "},
      {"POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding : chunked\r\n\r\n"
       "0\r\n\r\n",
       "POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n"
       "0\r\n\r\n",
       "HTTP/1.1 400 "},
      {"POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding:\x0b"
       "chunked\r\n\r\n0\r\n\r\n",
       "POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding:\t"
       "chunked\r\n\r\n0\r\n\r\n",
       "HTTP/1.1 400 "},
      {"POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: xchunked\r\n\r\n"
       "0\r\n\r\n",
       "POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n"
       "0\r\n\r\n",
       "HTTP/1.1 400 "},
      {"POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\n 0\r\n\r\nx",
       "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\n\r\nx",
       "HTTP/1.1 400 "},
      {"POST / HTTP/1.1\r\nHost: a\nContent-Length: 1\r\n\r\nx",
       "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\n\r\nx",
       "HTTP/1.1 400 "},
      {"POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n"
       "Content-Length: 3\r\n\r\n0\r\n\r\n",
       "POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n"
       "0\r\n\r\n",
       "HTTP/1.1 400 "},
  };
  for (const auto& test : cases) {
    martianlabs::doba::tests::unit::test_helper::set_context(
        "control " + std::string(test.valid));
    decoder_input control;
    DOBA_EXPECT_EQUAL(accumulate(control, test.valid), test.valid.size());
    DOBA_EXPECT_EQUAL(control.decode().code,
                      deserialization_status::kSucceeded);
    martianlabs::doba::tests::unit::test_helper::set_context(
        "vector " + std::string(test.invalid));
    decoder_input value;
    DOBA_EXPECT_EQUAL(accumulate(value, test.invalid), test.invalid.size());
    auto result = value.decode();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
    DOBA_EXPECT(!value.request_seen);
    DOBA_EXPECT(result.response.has_value());
    if (!result.response.has_value()) continue;
    auto output = result.response->serialize();
    DOBA_EXPECT((output->head + output->body)
                    .starts_with(test.status));
  }
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
        decoder_input value;
        auto result = value.decode();
        std::size_t offset = 0;
        for (std::size_t end : ends) {
          DOBA_EXPECT_EQUAL(
              accumulate(
                  value, std::string_view(source).substr(offset, end - offset)),
              end - offset);
          offset = end;
          result = value.decode();
          DOBA_EXPECT(!value.request_seen);
          if (result.code == deserialization_status::kInvalidSource) break;
          DOBA_EXPECT_EQUAL(result.code,
                            deserialization_status::kMoreBytesNeeded);
        }
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
        DOBA_EXPECT(!value.request_seen);
        DOBA_EXPECT(result.response.has_value());
        auto output = result.response->serialize();
        DOBA_EXPECT((output->head + output->body)
                        .starts_with("HTTP/1.1 400 "));
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
        decoder_input value;
        auto result = value.decode();
        std::size_t offset = 0;
        for (std::size_t end : ends) {
          DOBA_EXPECT_EQUAL(
              accumulate(
                  value, std::string_view(source).substr(offset, end - offset)),
              end - offset);
          offset = end;
          result = value.decode();
          DOBA_EXPECT(!value.request_seen);
          if (result.code == deserialization_status::kInvalidSource) break;
          DOBA_EXPECT_EQUAL(result.code,
                            deserialization_status::kMoreBytesNeeded);
        }
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
        DOBA_EXPECT(!value.request_seen);
        DOBA_EXPECT(result.response.has_value());
        auto output = result.response->serialize();
        DOBA_EXPECT((output->head + output->body)
                        .starts_with("HTTP/1.1 400 "));
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
      decoder_input value;
      auto result = value.decode();
      std::size_t offset = 0;
      for (std::size_t end : ends) {
        DOBA_EXPECT_EQUAL(
            accumulate(
                value, std::string_view(source).substr(offset, end - offset)),
            end - offset);
        offset = end;
        result = value.decode();
        DOBA_EXPECT(!value.request_seen);
        if (result.code == deserialization_status::kInvalidSource) break;
        DOBA_EXPECT_EQUAL(result.code,
                          deserialization_status::kMoreBytesNeeded);
      }
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
      DOBA_EXPECT(!value.request_seen);
      DOBA_EXPECT(result.response.has_value());
      auto output = result.response->serialize();
      DOBA_EXPECT((output->head + output->body)
                      .starts_with("HTTP/1.1 400 "));
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
        decoder_input value;
        auto result = value.decode();
        std::size_t offset = 0;
        for (std::size_t end : ends) {
          DOBA_EXPECT_EQUAL(
              accumulate(
                  value, std::string_view(source).substr(offset, end - offset)),
              end - offset);
          offset = end;
          result = value.decode();
          DOBA_EXPECT(!value.request_seen);
          if (result.code == deserialization_status::kInvalidSource) break;
          DOBA_EXPECT_EQUAL(result.code,
                            deserialization_status::kMoreBytesNeeded);
        }
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
        DOBA_EXPECT(!value.request_seen);
        DOBA_EXPECT(result.response.has_value());
        auto output = result.response->serialize();
        DOBA_EXPECT((output->head + output->body)
                        .starts_with("HTTP/1.1 400 "));
      }
    }
  }
}
// +===========================================================================+
// | [>] discards syntactic trailer fields                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("discards syntactic trailer fields") {
  constexpr std::string_view head =
      "POST / HTTP/1.1\r\nHost: a\r\n"
      "Transfer-Encoding: chunked\r\n\r\n1\r\nx\r\n0\r\n";
  {
    const std::string control = std::string(head) + "X-Checksum: 1\r\n\r\n";
    martianlabs::doba::tests::unit::test_helper::set_context(
        "control " + control);
    decoder_input value;
    DOBA_EXPECT_EQUAL(accumulate(value, control), control.size());
    DOBA_EXPECT_EQUAL(value.decode().code, deserialization_status::kSucceeded);
  }
  constexpr std::string_view trailers[] = {
      "Content-Length: 1\r\n",       "content-length: 1\r\n",
      "Transfer-Encoding: chunked\r\n", "TRANSFER-ENCODING: chunked\r\n",
      "Host: b\r\n",                 "host: b\r\n",
  };
  for (const auto trailer : trailers) {
    const std::string source = std::string(head) + std::string(trailer) +
                               "\r\n";
    martianlabs::doba::tests::unit::test_helper::set_context(
        "trailer " + std::string(trailer));
    decoder_input value;
    value.on_request = [&](const request& decoded) {
      DOBA_EXPECT_EQUAL(decoded.get_headers_length(), 2);
      DOBA_EXPECT_EQUAL(decoded.get_header("Host").second, "a");
      DOBA_EXPECT(!decoded.exist_header("Content-Length"));
      std::byte output;
      const auto state = decoded.get_body_reader()->read(
          std::span<std::byte>(&output, 1));
      DOBA_EXPECT(!state.has_error);
      DOBA_EXPECT(state.complete);
      DOBA_EXPECT_EQUAL(state.produced, 1);
      DOBA_EXPECT_EQUAL(static_cast<char>(output), 'x');
    };
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    auto result = value.decode();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
    DOBA_EXPECT(value.request_seen);
  }
}
// +===========================================================================+
// | [>] rejects unsupported HTTP versions                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects unsupported HTTP versions") {
  struct test_case {
    std::string_view version;
    std::string_view status;
  };
  constexpr test_case cases[] = {
      {"HTTP/0.9", "400"},
      {"HTTP/1.0", "400"},
      {"HTTP/1.2", "505"},
      {"HTTP/2.0", "505"},
      {"HTTP/9.9", "505"},
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
        decoder_input value;
        auto result = value.decode();
        std::size_t offset = 0;
        for (std::size_t end : ends) {
          DOBA_EXPECT_EQUAL(
              accumulate(
                  value, std::string_view(source).substr(offset, end - offset)),
              end - offset);
          offset = end;
          result = value.decode();
          DOBA_EXPECT(!value.request_seen);
          if (result.code == deserialization_status::kInvalidSource) break;
          DOBA_EXPECT_EQUAL(result.code,
                            deserialization_status::kMoreBytesNeeded);
        }
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
        DOBA_EXPECT(!value.request_seen);
        DOBA_EXPECT(result.response.has_value());
        auto output = result.response->serialize();
        DOBA_EXPECT((output->head + output->body)
                        .starts_with("HTTP/1.1 " + std::string(test.status) +
                                     " "));
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
  decoder_input value;
    DOBA_EXPECT_EQUAL(accumulate(value, source.substr(0, size)), size);
    DOBA_EXPECT_EQUAL(value.decode().code,
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
  check_header("Max-Forwards", "33", true,
               "OPTIONS * HTTP/1.1\r\n", "", "");
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
// | [>] unsupported expectation rejection                       ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder reports unsupported expectations before body arrival") {
  constexpr std::string_view source =
      "POST / HTTP/1.1\r\nHost: example.com\r\n"
      "Content-Length: 1\r\nExpect: feature=on\r\n\r\n";
  decoder_input value;
  DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
  const auto result = value.decode();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
  DOBA_EXPECT(!value.request_seen);
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
      decoder_input value;
      value.on_request = [&](const request& decoded) {
        DOBA_EXPECT_EQUAL(decoded.get_header(name).second, date);
      };
      DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
      const auto result = value.decode();
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
      DOBA_EXPECT(value.request_seen);
    }
  }
}

// +===========================================================================+
// | [>] complete head buffer boundaries                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts complete heads at the buffer boundary") {
  const std::string prefix =
      "GET / HTTP/1.1\r\nHost: example.com\r\nX-Pad: ";
  for (std::size_t size : {policies::kMaxRequestHeadSizeInMemory - 1,
                           policies::kMaxRequestHeadSizeInMemory}) {
    const std::string padding(size - prefix.size() - 4, 'x');
    const std::string source = prefix + padding + "\r\n\r\n";
    for (const std::size_t split : {std::size_t{0}, size / 2, size - 1}) {
      decoder_input value;
      value.on_request = [&](const request& decoded) {
        DOBA_EXPECT_EQUAL(decoded.get_header("X-Pad").second, padding);
      };
      DOBA_EXPECT_EQUAL(source.size(), size);
      DOBA_EXPECT_EQUAL(
          accumulate(value, std::string_view(source).substr(0, split)), split);
      DOBA_EXPECT_EQUAL(value.decode().code,
                        deserialization_status::kMoreBytesNeeded);
      DOBA_EXPECT_EQUAL(
          accumulate(value, std::string_view(source).substr(split)),
          size - split);
      const auto result = value.decode();
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
      DOBA_EXPECT(value.request_seen);
      DOBA_EXPECT_EQUAL(value.decode().code,
                        deserialization_status::kMoreBytesNeeded);
    }
  }
}

// +===========================================================================+
// | [>] query pair count boundaries                             ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder preserves every query pair at the supported limit") {
  for (std::size_t count : {127, 128}) {
    for (bool empty_pairs : {false, true}) {
      std::string source = empty_pairs ? "GET /?&&" : "GET /?";
      for (std::size_t i = 0; i < count; i++) {
        if (i != 0) source += empty_pairs ? "&&" : "&";
        source += "k" + std::to_string(i) + "=v" + std::to_string(i);
      }
      if (empty_pairs) source += "&&";
      source += " HTTP/1.1\r\nHost: example.com\r\n\r\n";
      DOBA_EXPECT(source.size() < receive_capacity);
      decoder_input value;
      value.on_request = [&](const request& decoded) {
        DOBA_EXPECT_EQUAL(decoded.get_query_parameters_length(), count);
        for (std::size_t i = 0; i < count; i++) {
          const auto parameter =
              decoded.get_query_parameter("k" + std::to_string(i));
          DOBA_EXPECT(parameter.has_value());
          DOBA_EXPECT_EQUAL(parameter->second, "v" + std::to_string(i));
        }
      };
      DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
      const auto result = value.decode();
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
      DOBA_EXPECT(value.request_seen);
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
    decoder_input value;
    std::size_t requests = 0;
    value.on_request = [&](const request& decoded) {
      if (requests == 0) {
        DOBA_EXPECT_EQUAL(decoded.get_absolute_path(), "/first");
        DOBA_EXPECT(decoded.has_body_reader());
        std::array<std::byte, 8> output{};
        const auto state = decoded.get_body_reader()->read(output);
        DOBA_EXPECT(!state.has_error);
        DOBA_EXPECT(state.complete);
        DOBA_EXPECT_EQUAL(state.produced, 7);
        DOBA_EXPECT_EQUAL(
            std::string_view(reinterpret_cast<const char*>(output.data()),
                             state.produced), "payload");
      } else {
        DOBA_EXPECT_EQUAL(decoded.get_absolute_path(), "/next");
        DOBA_EXPECT_EQUAL(decoded.get_header("Host").second, "next.example");
        DOBA_EXPECT(!decoded.has_body_reader());
      }
      requests++;
    };
    std::size_t offset = 0;
    for (std::size_t end : {split, source.size()}) {
      DOBA_EXPECT_EQUAL(
          accumulate(
              value, std::string_view(source).substr(offset, end - offset)),
          end - offset);
      offset = end;
      for (std::size_t dispatch = 0; dispatch < 3; dispatch++) {
        const auto result = value.decode();
        if (result.code == deserialization_status::kMoreBytesNeeded) break;
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
        DOBA_EXPECT(value.request_seen);
      }
    }
    DOBA_EXPECT_EQUAL(requests, 2);
    DOBA_EXPECT_EQUAL(value.decode().code,
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
    decoder_input value;
    std::size_t requests = 0;
    value.on_request = [&](const request& decoded) {
      if (requests == 0) {
        DOBA_EXPECT_EQUAL(decoded.get_absolute_path(), "/first");
        DOBA_EXPECT(decoded.has_body_reader());
        std::array<std::byte, 8> output{};
        const auto state = decoded.get_body_reader()->read(output);
        DOBA_EXPECT(!state.has_error);
        DOBA_EXPECT(state.complete);
        DOBA_EXPECT_EQUAL(state.produced, 3);
        DOBA_EXPECT_EQUAL(
            std::string_view(reinterpret_cast<const char*>(output.data()),
                             state.produced), "abc");
      } else {
        DOBA_EXPECT_EQUAL(decoded.get_absolute_path(), "/next");
        DOBA_EXPECT_EQUAL(decoded.get_header("Host").second, "next.example");
        DOBA_EXPECT(!decoded.has_body_reader());
      }
      requests++;
    };
    std::size_t offset = 0;
    for (std::size_t end : {split, source.size()}) {
      DOBA_EXPECT_EQUAL(
          accumulate(
              value, std::string_view(source).substr(offset, end - offset)),
          end - offset);
      offset = end;
      for (std::size_t dispatch = 0; dispatch < 3; dispatch++) {
        const auto result = value.decode();
        if (result.code == deserialization_status::kMoreBytesNeeded) break;
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
        DOBA_EXPECT(value.request_seen);
      }
    }
    DOBA_EXPECT_EQUAL(requests, 2);
    DOBA_EXPECT_EQUAL(value.decode().code,
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
    decoder_input value;
    value.on_request = [&](const request& decoded) {
      DOBA_EXPECT_EQUAL(decoded.get_method(), test.method);
      DOBA_EXPECT_EQUAL(decoded.get_target(), test.target_form);
      DOBA_EXPECT_EQUAL(decoded.get_absolute_path(), test.path);
    };
    for (std::size_t i = 0; i < test.source.size(); i++) {
      DOBA_EXPECT_EQUAL(accumulate(value, test.source.substr(i, 1)), 1);
      const auto result = value.decode();
      if (i + 1 < test.source.size()) {
        DOBA_EXPECT_EQUAL(result.code,
                          deserialization_status::kMoreBytesNeeded);
        DOBA_EXPECT(!value.request_seen);
      } else {
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
        DOBA_EXPECT(value.request_seen);
      }
    }
  }
}

// +===========================================================================+
// | [>] raw body remains in memory below spill threshold        ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder keeps raw body below spill threshold") {
  policies configuration;
  decoder_input value(configuration);
  const std::string payload(16000, 'x');
  const std::string source =
      "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: " +
      std::to_string(payload.size()) + "\r\n\r\n" + payload;
  std::string decoded;
  bool complete = false;
  value.on_request = [&](const request& request_value) {
    std::array<std::byte, 4096> output{};
    for (std::size_t i = 0; !complete && i <= payload.size(); i++) {
      const auto state = request_value.get_body_reader()->read(output);
      DOBA_EXPECT(!state.has_error);
      decoded.append(reinterpret_cast<const char*>(output.data()),
                     state.produced);
      complete = state.complete;
    }
  };
  for (std::size_t offset = 0; offset < source.size();) {
    const std::size_t count =
        std::min(std::size_t{4096}, source.size() - offset);
    DOBA_EXPECT_EQUAL(accumulate(
        value, std::string_view(source).substr(offset, count)), count);
    offset += count;
    const auto result = value.decode();
    if (offset == source.size()) {
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
      DOBA_EXPECT(value.request_seen);
    }
  }
  DOBA_EXPECT(complete);
  DOBA_EXPECT_EQUAL(decoded, payload);
}

// +===========================================================================+
// | [>] raw body spills past memory boundary                    ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder reads raw bodies across the memory boundary") {
  for (std::size_t size : {16383, 16384, 16385, 65536}) {
    const std::string payload(size, 'x');
    const std::string source =
        "POST / HTTP/1.1\r\nHost: example.com\r\nContent-Length: " +
        std::to_string(size) + "\r\n\r\n" + payload;
    decoder_input value;
    std::string decoded;
    bool complete = false;
    value.on_request = [&](const request& request_value) {
      DOBA_EXPECT(request_value.has_body_reader());
      std::array<std::byte, 4096> output{};
      for (std::size_t i = 0; !complete && i <= size; i++) {
        const auto state = request_value.get_body_reader()->read(output);
        DOBA_EXPECT(!state.has_error);
        decoded.append(reinterpret_cast<const char*>(output.data()),
                       state.produced);
        complete = state.complete;
      }
    };
    for (std::size_t offset = 0; offset < source.size();) {
      const auto count = accumulate(
          value, std::string_view(source).substr(offset, 4096));
      DOBA_EXPECT(count > 0);
      offset += count;
      auto result = value.decode();
      DOBA_EXPECT(result.code != deserialization_status::kInvalidSource);
    }
    DOBA_EXPECT(complete);
    DOBA_EXPECT_EQUAL(decoded, payload);
  }
}

// +===========================================================================+
// | [>] chunked body spills past memory boundary                ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder reads chunked bodies across the wire memory boundary") {
  for (std::size_t size : {16383, 16384, 16385, 65536}) {
    const std::string payload(size - 13, 'x');
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
    decoder_input value;
    std::string decoded;
    bool complete = false;
    value.on_request = [&](const request& request_value) {
      DOBA_EXPECT(request_value.has_body_reader());
      std::array<std::byte, 4096> output{};
      for (std::size_t i = 0; !complete && i <= size; i++) {
        const auto state = request_value.get_body_reader()->read(output);
        DOBA_EXPECT(!state.has_error);
        decoded.append(reinterpret_cast<const char*>(output.data()),
                       state.produced);
        complete = state.complete;
      }
    };
    for (std::size_t offset = 0; offset < source.size();) {
      const auto count = accumulate(
          value, std::string_view(source).substr(offset, 4096));
      DOBA_EXPECT(count > 0);
      offset += count;
      auto result = value.decode();
      DOBA_EXPECT(result.code != deserialization_status::kInvalidSource);
    }
    DOBA_EXPECT(complete);
    DOBA_EXPECT_EQUAL(decoded, payload);
  }
}

// +===========================================================================+
// | [>] temporary body survives an unread handler               ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder removes an unread temporary body after dispatch") {
  const auto before = body_temp_files();
  const std::string payload(16385, 'x');
  const std::string source =
      "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 16385\r\n\r\n" +
      payload;
  decoder_input value;
  std::filesystem::path path;
  value.on_request = [&](const request& request_value) {
    DOBA_EXPECT(request_value.has_body_reader());
    path = new_body_temp_file(before);
    DOBA_EXPECT(!path.empty());
    DOBA_EXPECT(std::filesystem::exists(path));
  };
  for (std::size_t offset = 0; offset < source.size();) {
    const auto count = accumulate(
        value, std::string_view(source).substr(offset, 4096));
    DOBA_EXPECT(count > 0);
    offset += count;
    const auto result = value.decode();
    DOBA_EXPECT_EQUAL(result.code,
                      offset == source.size()
                          ? deserialization_status::kSucceeded
                          : deserialization_status::kMoreBytesNeeded);
  }
  DOBA_EXPECT(!path.empty());
  DOBA_EXPECT(!std::filesystem::exists(path));
}

// +===========================================================================+
// | [>] invalid chunk after spill removes temporary body        ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder removes a temporary body after invalid framing") {
  const auto before = body_temp_files();
  const std::string source =
      "POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n"
      "4268\r\n" + std::string(17000, 'x') + "\r\n";
  decoder_input value;
  for (std::size_t offset = 0; offset < source.size();) {
    const auto count = accumulate(
        value, std::string_view(source).substr(offset, 4096));
    DOBA_EXPECT(count > 0);
    offset += count;
    DOBA_EXPECT_EQUAL(value.decode().code,
                      deserialization_status::kMoreBytesNeeded);
  }
  const auto path = new_body_temp_file(before);
  DOBA_EXPECT(!path.empty());
  DOBA_EXPECT(std::filesystem::exists(path));
  DOBA_EXPECT_EQUAL(accumulate(value, "Z\r\n"), 3);
  auto result = value.decode();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
  DOBA_EXPECT(result.response.has_value());
  auto response_wire = result.response->serialize();
  DOBA_EXPECT(response_wire->head.starts_with("HTTP/1.1 400 "));
  DOBA_EXPECT(!std::filesystem::exists(path));
}

// +===========================================================================+
// | [>] handler failure removes temporary body                  ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder removes a temporary body after handler failure") {
  const auto before = body_temp_files();
  const std::string source =
      "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 16385\r\n\r\n" +
      std::string(16385, 'x');
  decoder_input value;
  std::filesystem::path path;
  value.on_request = [&](const request&) {
    path = new_body_temp_file(before);
    DOBA_EXPECT(!path.empty());
    throw std::runtime_error("handler failure");
  };
  bool threw = false;
  try {
    for (std::size_t offset = 0; offset < source.size();) {
      const auto count = accumulate(
          value, std::string_view(source).substr(offset, 4096));
      DOBA_EXPECT(count > 0);
      offset += count;
      value.decode();
    }
  } catch (const std::runtime_error&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  DOBA_EXPECT(!path.empty());
  DOBA_EXPECT(!std::filesystem::exists(path));
}

// +===========================================================================+
// | [>] temporary body creation failure returns 500             ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder returns 500 when temporary body creation fails") {
  const auto blocked = std::filesystem::absolute(__FILE__).string();
#ifdef _WIN32
  char* previous_value = nullptr;
  std::size_t previous_size = 0;
  const int saved = _dupenv_s(&previous_value, &previous_size, "TMP");
  const std::string previous = previous_value ? previous_value : "";
  std::free(previous_value);
  DOBA_EXPECT_EQUAL(saved, 0);
  const int changed = _putenv_s("TMP", blocked.c_str());
#else
  const char* previous_value = std::getenv("TMPDIR");
  const bool had_previous = previous_value != nullptr;
  const std::string previous = had_previous ? previous_value : "";
  const int changed = setenv("TMPDIR", blocked.c_str(), 1);
#endif
  if (changed != 0) {
    DOBA_EXPECT_EQUAL(changed, 0);
    return;
  }
  decoder_input value;
  bool called = false;
  value.on_request = [&](const request&) { called = true; };
  const std::string source =
      "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 16385\r\n\r\n";
  value.append(source);
  auto result = value.decode();
#ifdef _WIN32
  const int restored = _putenv_s("TMP", previous.c_str());
#else
  const int restored = had_previous
                           ? setenv("TMPDIR", previous.c_str(), 1)
                           : unsetenv("TMPDIR");
#endif
  DOBA_EXPECT_EQUAL(restored, 0);
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
  DOBA_EXPECT(!called);
  DOBA_EXPECT(result.response.has_value());
  auto wire = result.response->serialize();
  DOBA_EXPECT(wire->head.starts_with("HTTP/1.1 500 "));
}

// +===========================================================================+
// | [>] temporary body read-open failure returns 500            ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder returns 500 when temporary body cannot reopen") {
  const auto before = body_temp_files();
  decoder_input value;
  bool called = false;
  value.on_request = [&](const request&) { called = true; };
  const std::string head =
      "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 16385\r\n\r\n";
  value.append(head);
  DOBA_EXPECT_EQUAL(value.decode().code,
                    deserialization_status::kMoreBytesNeeded);
  const auto path = new_body_temp_file(before);
  DOBA_EXPECT(!path.empty());
  DOBA_EXPECT(std::filesystem::exists(path));
  std::error_code error;
  DOBA_EXPECT(std::filesystem::remove(path, error));
  DOBA_EXPECT(!error);
  const std::string body(16385, 'x');
  martianlabs::doba::protocol::deserialization_result<request, response>
      result;
  for (std::size_t offset = 0; offset < body.size();) {
    const auto count = accumulate(
        value, std::string_view(body).substr(offset, 4096));
    DOBA_EXPECT(count > 0);
    offset += count;
    result = value.decode();
  }
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
  DOBA_EXPECT(!called);
  DOBA_EXPECT(result.response.has_value());
  auto wire = result.response->serialize();
  DOBA_EXPECT(wire->head.starts_with("HTTP/1.1 500 "));
  DOBA_EXPECT(!std::filesystem::exists(path));
}

// +===========================================================================+
// | [>] absolute form uses its authority when Host differs      ( test-case ) |
// +===========================================================================+
DOBA_TEST("absolute form uses its authority when Host differs") {
  // RFC 9112 S3.2.2: absolute-form authority overrides Host.
  constexpr std::string_view cases[] = {
      "GET http://a/path HTTP/1.1\r\nHost: b\r\n\r\n",
      "GET http://a:8080/path HTTP/1.1\r\nHost: a:80\r\n\r\n",
      "GET https://a/path HTTP/1.1\r\nHost: b:443\r\n\r\n",
  };
  for (const auto source : cases) {
    decoder_input value;
    martianlabs::doba::tests::unit::test_helper::set_context(source);
    value.on_request = [&](const request& decoded) {
      DOBA_EXPECT_EQUAL(decoded.get_target(), target::kAbsoluteForm);
      DOBA_EXPECT_EQUAL(decoded.get_absolute_path(), "/path");
      DOBA_EXPECT(decoded.has_target_authority());
      DOBA_EXPECT_EQUAL(decoded.get_target_authority_host(), "a");
      DOBA_EXPECT_EQUAL(decoded.get_target_authority_port(),
                        source.find(":8080") != std::string_view::npos ?
                            "8080" : "");
      const auto host_start = source.find("Host: ") + 6;
      const auto host = source.substr(
          host_start, source.find("\r\n", host_start) - host_start);
      const auto colon = host.find(':');
      DOBA_EXPECT_EQUAL(decoded.get_header("Host").second, host);
      DOBA_EXPECT_EQUAL(decoded.get_host(), host.substr(0, colon));
      DOBA_EXPECT_EQUAL(decoded.get_host_port(),
                        colon == std::string_view::npos ? "" :
                            host.substr(colon + 1));
    };
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    const auto result = value.decode();
    if (!martianlabs::doba::tests::unit::test_helper::expect(
            result.code == deserialization_status::kSucceeded,
            "absolute authority accepted", __FILE__, __LINE__)) continue;
    DOBA_EXPECT(value.request_seen);
  }
}
// +===========================================================================+
// | [>] absolute form retains Host validation                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("absolute form retains Host validation") {
  for (const auto headers : {"", "Host: a\r\nHost: b\r\n", "Host: [bad\r\n"}) {
    const std::string source =
        std::string("GET http://a/path HTTP/1.1\r\n") + headers + "\r\n";
    decoder_input value;
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    const auto result = value.decode();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
    DOBA_EXPECT(!value.request_seen);
  }
}
// +===========================================================================+
// | [>] accepts TE with its required connection option          ( test-case ) |
// +===========================================================================+
DOBA_TEST("accepts TE with its required connection option") {
  for (const auto connection : {"Connection: TE\r\n",
                                "connection:\t te \t\r\n",
                                "CONNECTION: Te\r\n"}) {
    check_header("TE", "trailers", true, "GET / HTTP/1.1\r\n", connection);
  }
}
// +===========================================================================+
// | [>] rejects query parameter overflow without truncating     ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects query parameter overflow without truncating") {
  for (bool empty_pairs : {false, true}) {
    for (bool body : {false, true}) {
      std::string source = empty_pairs ? "GET /?&&" : "GET /?";
      for (std::size_t index = 0; index <= max_query_parameters;
           ++index) {
        if (index != 0) source += empty_pairs ? "&&" : "&";
        source += "p" + std::to_string(index) + "=v";
      }
      if (empty_pairs) source += "&&";
      source += " HTTP/1.1\r\nHost: example.com\r\n";
      if (body) source += "Content-Length: 1\r\n";
      source += "\r\n";
      if (body) source += 'x';
      decoder_input value;
      DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
      const auto result = value.decode();
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
      DOBA_EXPECT(!value.request_seen);
    }
  }
}
// +===========================================================================+
// | [>] a full incomplete head terminates instead of stalling   ( test-case ) |
// +===========================================================================+
DOBA_TEST("a full incomplete head terminates instead of stalling") {
  std::string source = "GET / HTTP/1.1\r\nHost: example.com\r\nX-Pad: ";
  const auto limit = policies::kMaxRequestHeadSizeInMemory;
  source.append(limit + 1 - source.size() - 4, 'a');
  source += "\r\n\r\n";
  for (const std::size_t split : {std::size_t{0},
                                   limit / 2,
                                   limit - 1}) {
    decoder_input value;
    DOBA_EXPECT_EQUAL(
        accumulate(value, std::string_view(source).substr(0, split)), split);
    DOBA_EXPECT_EQUAL(value.decode().code,
                      deserialization_status::kMoreBytesNeeded);
    const auto consumed =
        accumulate(value, std::string_view(source).substr(split));
    DOBA_EXPECT_EQUAL(consumed, source.size() - split);
    const auto result = value.decode();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
    DOBA_EXPECT(!value.request_seen);
    DOBA_EXPECT_EQUAL(value.decode().code,
                      deserialization_status::kInvalidSource);
  }
}
// +===========================================================================+
// | [>] full body buffers remain consumable                     ( test-case ) |
// +===========================================================================+
DOBA_TEST("full body buffers remain consumable") {
  const std::string payload(receive_capacity * 2, 'x');
  for (bool chunked : {false, true}) {
    const std::string source =
        std::string("POST / HTTP/1.1\r\nHost: a\r\n") +
        (chunked ? "Transfer-Encoding: chunked\r\n\r\n2800\r\n" :
                   "Content-Length: " + std::to_string(payload.size()) +
                       "\r\n\r\n") +
        payload + (chunked ? "\r\n0\r\n\r\n" : "");
    decoder_input value;
    std::string decoded;
    bool complete = false;
    value.on_request = [&](const request& request_value) {
      std::array<std::byte, receive_capacity> output{};
      for (std::size_t i = 0; !complete && i <= payload.size(); i++) {
        const auto state = request_value.get_body_reader()->read(output);
        DOBA_EXPECT(!state.has_error);
        decoded.append(reinterpret_cast<const char*>(output.data()),
                       state.produced);
        complete = state.complete;
      }
    };
    for (std::size_t offset = 0; offset < source.size();) {
      const auto count =
          accumulate(value, std::string_view(source).substr(offset));
      DOBA_EXPECT(count > 0);
      offset += count;
      const auto result = value.decode();
      if (offset < source.size()) {
        DOBA_EXPECT_EQUAL(result.code,
                          deserialization_status::kMoreBytesNeeded);
        DOBA_EXPECT(!value.request_seen);
      } else {
        DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
        DOBA_EXPECT(value.request_seen);
      }
    }
    DOBA_EXPECT(complete);
    DOBA_EXPECT_EQUAL(decoded, payload);
  }
}
// +===========================================================================+
// | [>] unknown headers retain their complete normalized values ( test-case ) |
// +===========================================================================+
DOBA_TEST("unknown headers retain their complete normalized values") {
  check_header("Warning", "199 example \"diagnostic\"", true);
  check_header("X-Extension", "a,b;c=\"x\"", true);
}
// +===========================================================================+
// | [>] rejects invalid TE quality with its connection option   ( test-case ) |
// +===========================================================================+
DOBA_TEST("rejects invalid TE quality with its connection option") {
  for (const auto connection : {"Connection: TE\r\n",
                                "connection:\t te \t\r\n",
                                "CONNECTION: Te\r\n"}) {
    check_header("TE", "gzip;q=1.001", false, "GET / HTTP/1.1\r\n",
                 connection);
  }
}

// +===========================================================================+
// | [>] absolute form rejects suffixes after IP literals        ( test-case ) |
// +===========================================================================+
DOBA_TEST("absolute form rejects suffixes after IP literals at every split") {
  constexpr std::string_view cases[] = {
      "http://[::1]junk/", "http://[::1]80/", "http://[::1]%3A80/",
      "http://[::1];x?y=1", "http://[::1]]", "http://[v1.name]junk/",
      "http://[v1.name]80?x=1", "https://[vF.a:b]!/",
  };
  for (const auto uri : cases) {
    const std::string source =
        "GET " + std::string(uri) + " HTTP/1.1\r\nHost: other.example\r\n\r\n";
    for (std::size_t split = 0; split <= source.size() + 1; split++) {
      martianlabs::doba::tests::unit::test_helper::set_context(
          std::string(uri) + ", split " + std::to_string(split));
      std::vector<std::size_t> ends;
      if (split == source.size() + 1) {
        for (std::size_t i = 1; i <= source.size(); i++) ends.push_back(i);
      } else {
        ends.push_back(split);
        if (split < source.size()) ends.push_back(source.size());
      }
      decoder_input value;
      auto result = value.decode();
      std::size_t offset = 0;
      for (const std::size_t end : ends) {
        DOBA_EXPECT_EQUAL(
            accumulate(
                value, std::string_view(source).substr(offset, end - offset)),
            end - offset);
        offset = end;
        result = value.decode();
        DOBA_EXPECT(!value.request_seen);
        if (result.code == deserialization_status::kInvalidSource) break;
        DOBA_EXPECT_EQUAL(result.code,
                          deserialization_status::kMoreBytesNeeded);
      }
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
      DOBA_EXPECT(!value.request_seen);
    }
  }
}
// +===========================================================================+
// | [>] absolute form preserves valid IP literal authorities    ( test-case ) |
// +===========================================================================+
DOBA_TEST("absolute form preserves valid IP literals at every split") {
  struct test_case {
    std::string_view uri;
    std::string_view host;
    std::string_view port;
  };
  constexpr test_case cases[] = {
      {"http://[::1]/path?x=1", "[::1]", ""},
      {"http://[::1]:80/path?x=1", "[::1]", "80"},
      {"http://[::1]:/path?x=1", "[::1]", ""},
      {"http://[v1.name]/path?x=1", "[v1.name]", ""},
      {"http://[vF.a:b]:009/path?x=1", "[vF.a:b]", "009"},
      {"http://[v1.name]:/path?x=1", "[v1.name]", ""},
  };
  for (const auto& test : cases) {
    const std::string source = "GET " + std::string(test.uri) +
                               " HTTP/1.1\r\nHost: other.example:8080\r\n\r\n";
    for (std::size_t split = 0; split <= source.size() + 1; split++) {
      martianlabs::doba::tests::unit::test_helper::set_context(
          std::string(test.uri) + ", split " + std::to_string(split));
      std::vector<std::size_t> ends;
      if (split == source.size() + 1) {
        for (std::size_t i = 1; i <= source.size(); i++) ends.push_back(i);
      } else {
        ends.push_back(split);
        if (split < source.size()) ends.push_back(source.size());
      }
      decoder_input value;
      value.on_request = [&](const request& decoded) {
        DOBA_EXPECT_EQUAL(decoded.get_target(), target::kAbsoluteForm);
        DOBA_EXPECT(decoded.has_target_authority());
        DOBA_EXPECT_EQUAL(decoded.get_target_authority_host(), test.host);
        DOBA_EXPECT_EQUAL(decoded.get_target_authority_port(), test.port);
        DOBA_EXPECT_EQUAL(decoded.get_absolute_path(), "/path");
        DOBA_EXPECT_EQUAL(decoded.get_query_parameter("x")->second, "1");
        DOBA_EXPECT_EQUAL(decoded.get_header("Host").second,
                          "other.example:8080");
        DOBA_EXPECT_EQUAL(decoded.get_host(), "other.example");
        DOBA_EXPECT_EQUAL(decoded.get_host_port(), "8080");
      };
      auto result = value.decode();
      std::size_t offset = 0;
      for (const std::size_t end : ends) {
        DOBA_EXPECT_EQUAL(
            accumulate(
                value, std::string_view(source).substr(offset, end - offset)),
            end - offset);
        offset = end;
        result = value.decode();
        if (end < source.size()) {
          DOBA_EXPECT_EQUAL(result.code,
                            deserialization_status::kMoreBytesNeeded);
          DOBA_EXPECT(!value.request_seen);
        }
      }
      DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
      DOBA_EXPECT(value.request_seen);
    }
  }
}

// +===========================================================================+
// | [>] decoder returns one interim with an incomplete body     ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder returns one interim with an incomplete body") {
  for (const bool chunked : {false, true}) {
    const std::string head =
        "POST / HTTP/1.1\r\nHost: localhost\r\nExpect: 100-continue\r\n" +
        std::string(chunked ? "Transfer-Encoding: chunked\r\n\r\n"
                            : "Content-Length: 2\r\n\r\n");
    decoder_input value;
    accumulate(value, head);
    auto result = value.decode();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
    DOBA_EXPECT(!value.request_seen);
    DOBA_EXPECT(result.response.has_value());
    auto wire = result.response->serialize();
    const std::string bytes(wire->head + wire->body);
    DOBA_EXPECT(bytes.starts_with("HTTP/1.1 100 Continue\r\n"));
    DOBA_EXPECT(bytes.ends_with("\r\n\r\n"));
    DOBA_EXPECT(!wire->source);
    DOBA_EXPECT(bytes.find("Content-Length:") == std::string_view::npos);
    DOBA_EXPECT(bytes.find("Transfer-Encoding:") == std::string_view::npos);
    accumulate(value, chunked ? "2\r\nx" : "x");
    result = value.decode();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
    DOBA_EXPECT(!result.response);
    accumulate(value, chunked ? "y\r\n0\r\n\r\n" : "y");
    result = value.decode();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
    DOBA_EXPECT(value.request_seen);
    DOBA_EXPECT(!result.response);
    accumulate(value, head);
    result = value.decode();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
    DOBA_EXPECT(result.response.has_value());
  }
}

// +===========================================================================+
// | [>] decoder limits interim to accepted incomplete bodies    ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder limits interim to accepted incomplete bodies") {
  const std::string prefix = "POST / HTTP/1.1\r\nHost: localhost\r\n";
  const std::string expect = "Expect: 100-continue\r\n";
  const std::vector<std::pair<std::string, deserialization_status>> cases{
      {expect + "Content-Length: 1\r\n",
       deserialization_status::kMoreBytesNeeded},
      {"Content-Length: 1\r\n\r\n", deserialization_status::kMoreBytesNeeded},
      {expect + "\r\n", deserialization_status::kSucceeded},
      {expect + "Content-Length: 0\r\n\r\n",
       deserialization_status::kSucceeded},
      {expect + "Content-Length: 1\r\n\r\nx",
       deserialization_status::kSucceeded},
      {expect + "Transfer-Encoding: chunked\r\n\r\n0\r\n\r\n",
       deserialization_status::kSucceeded},
      {expect + "Transfer-Encoding: chunked\r\n\r\n1\r\nx\r\n0\r\n\r\n",
       deserialization_status::kSucceeded},
      {expect + "Transfer-Encoding: chunked\r\n\r\nZ\r\n",
       deserialization_status::kInvalidSource},
      {expect + "Content-Length: invalid\r\n\r\n",
       deserialization_status::kInvalidSource},
      {"Expect: unsupported\r\nContent-Length: 1\r\n\r\n",
       deserialization_status::kInvalidSource},
      {expect + "Content-Length: 1\r\nHost: other\r\n\r\n",
       deserialization_status::kInvalidSource}};
  for (const auto& [fields, status] : cases) {
    decoder_input value;
    accumulate(value, prefix + fields);
    auto result = value.decode();
    DOBA_EXPECT_EQUAL(result.code, status);
    if (status == deserialization_status::kInvalidSource) {
      DOBA_EXPECT(result.response.has_value());
      auto wire = result.response->serialize();
      const std::string bytes(wire->head + wire->body);
      DOBA_EXPECT(!bytes.starts_with("HTTP/1.1 100 "));
    } else {
      DOBA_EXPECT(!result.response);
    }
  }
}

// +===========================================================================+
// | [>] decoder returns responses for rejection reasons         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder returns responses for rejection reasons") {
  martianlabs::doba::protocol::http::v11::policies configuration;
  configuration.max_content_length = 1;
  configuration.allow_chunked = false;
  const std::vector<std::pair<std::string, std::string_view>> cases{
      {"GET / HTTP/1.1\r\nHost: a\r\nContent-Length: invalid\r\n\r\n", "400"},
      {"POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 2\r\n\r\n", "413"},
      {"POST / HTTP/1.1\r\nHost: a\r\nExpect: unknown\r\n\r\n", "417"},
      {"POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n",
       "501"},
      {"GET / HTTP/1.2\r\nHost: a\r\n\r\n", "505"}};
  for (const auto& [input, status] : cases) {
    decoder_type value(configuration);
    std::size_t consumed = 0;
    bool request_seen = false;
    auto result = value.deserialize(input.data(), input.size(), 4096,
                                    consumed, [&](const request&) {
                                      request_seen = true;
                                    });
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
    DOBA_EXPECT(!request_seen);
    DOBA_EXPECT(result.response.has_value());
    auto output = result.response->serialize();
    const std::string bytes(output->head + output->body);
    DOBA_EXPECT(bytes.starts_with("HTTP/1.1 " + std::string(status) + " "));
    DOBA_EXPECT(bytes.ends_with("\r\n\r\nInvalid request content!"));
    DOBA_EXPECT(bytes.find("Content-Length: 24\r\n") != std::string_view::npos);
  }
}

// +===========================================================================+
// | [>] target and header fit within receive capacity           ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts target and headers within receive capacity") {
  for (const std::string& source : std::array<std::string, 2>{
           "GET /123456789 HTTP/1.1\r\nHost: a\r\n\r\n",
           "GET / HTTP/1.1\r\nHost: a\r\nX: " + std::string(140, 'x') +
               "\r\n\r\n"}) {
    decoder_input value;
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    DOBA_EXPECT_EQUAL(value.decode().code,
                      deserialization_status::kSucceeded);
  }
}

// +===========================================================================+
// | [>] decoder suppresses errors only for a known HEAD         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder suppresses errors only for a known HEAD") {
  for (const std::string_view method : {"HEAD ", "HEADX ", "HEAD\t", "HEAD@"}) {
    decoder_input value;
    accumulate(value, std::string(method) +
        "/ HTTP/1.1\r\nHost: a\r\nContent-Length: invalid\r\n\r\n");
    auto result = value.decode();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
    DOBA_EXPECT(result.response.has_value());
    auto output = result.response->serialize();
    const std::string bytes(output->head + output->body);
    DOBA_EXPECT(bytes.find("Content-Length: 24\r\n") != std::string_view::npos);
    DOBA_EXPECT(bytes.ends_with(method == "HEAD " ? "\r\n\r\n"
        : "\r\n\r\nInvalid request content!"));
  }
  decoder_input partial;
  accumulate(partial, "HEAD");
  auto result = partial.decode();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
  DOBA_EXPECT(!result.response);
  accumulate(partial, " / HTTP/1.1\r\nHost: a\r\nBad Name: x\r\n\r\n");
  result = partial.decode();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
  auto output = result.response->serialize();
  DOBA_EXPECT((output->head + output->body)
                  .ends_with("\r\n\r\n"));
}

// +===========================================================================+
// | [>] decoder retains HEAD while decoding body fragments      ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder retains HEAD while decoding body fragments") {
  decoder_input value;
  accumulate(value, "HEAD / HTTP/1.1\r\nHost: a\r\n"
                    "Expect: 100-continue\r\n"
                    "Transfer-Encoding: chunked\r\n\r\n");
  auto result = value.decode();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
  DOBA_EXPECT(result.response.has_value());
  accumulate(value, "1\r\nx");
  result = value.decode();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kMoreBytesNeeded);
  DOBA_EXPECT(!result.response);
  accumulate(value, "Z");
  result = value.decode();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
  DOBA_EXPECT(result.response.has_value());
  auto output = result.response->serialize();
  const std::string bytes(output->head + output->body);
  DOBA_EXPECT(bytes.starts_with("HTTP/1.1 400 "));
  DOBA_EXPECT(bytes.find("Content-Length: 24\r\n") != std::string_view::npos);
  DOBA_EXPECT(bytes.ends_with("\r\n\r\n"));
  decoder_input next;
  accumulate(next, "HEAD / HTTP/1.1\r\nHost: a\r\n\r\n");
  DOBA_EXPECT_EQUAL(next.decode().code, deserialization_status::kSucceeded);
  accumulate(next, "GET / HTTP/1.1\r\nBad Name: x\r\n\r\n");
  result = next.decode();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
  output = result.response->serialize();
  DOBA_EXPECT((output->head + output->body)
                  .ends_with("\r\n\r\nInvalid request content!"));
}

// +===========================================================================+
// | [>] default Content-Length limit                            ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder enforces the configured Content-Length limit") {
  DOBA_EXPECT_EQUAL(policies{}.max_content_length, 16 * 1024 * 1024);
  for (const std::size_t size : {policies::kMaxRequestBodySizeInMemory,
                                 policies::kMaxRequestBodySizeInMemory + 1,
                                 std::size_t{16 * 1024 * 1024},
                                 std::size_t{16 * 1024 * 1024 + 1}}) {
    const std::string source =
        "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: " +
        std::to_string(size) + "\r\n\r\n";
    decoder_input value;
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    auto result = value.decode();
    DOBA_EXPECT_EQUAL(result.code,
                      size <= policies{}.max_content_length
                          ? deserialization_status::kMoreBytesNeeded
                          : deserialization_status::kInvalidSource);
    if (size > policies{}.max_content_length) {
      auto wire = result.response->serialize();
      DOBA_EXPECT((wire->head + wire->body)
                      .starts_with("HTTP/1.1 413 "));
    }
  }
}

// +===========================================================================+
// | [>] chunked payload limit across receives                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder limits chunked payload across receives") {
  policies configuration;
  configuration.max_content_length = 5;
  const std::string head =
      "POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n";
  for (const bool fragmented : {false, true}) {
    decoder_input value(configuration);
    if (fragmented) {
      const std::string first = head + "3\r\nabc\r\n";
      DOBA_EXPECT_EQUAL(accumulate(value, first), first.size());
      DOBA_EXPECT_EQUAL(value.decode().code,
                        deserialization_status::kMoreBytesNeeded);
    }
    const std::string rest = "3\r\ndef\r\n0\r\n\r\n";
    const std::string input = fragmented ? rest : head + "3\r\nabc\r\n" + rest;
    DOBA_EXPECT_EQUAL(accumulate(value, input), input.size());
    auto result = value.decode();
    DOBA_EXPECT_EQUAL(result.code, deserialization_status::kInvalidSource);
    DOBA_EXPECT(result.response.has_value());
    auto wire = result.response->serialize();
    DOBA_EXPECT((wire->head + wire->body)
                    .starts_with("HTTP/1.1 413 "));
  }
  decoder_input allowed(configuration);
  const std::string exact = head + "3\r\nabc\r\n2\r\nde\r\n0\r\n\r\n";
  DOBA_EXPECT_EQUAL(accumulate(allowed, exact), exact.size());
  DOBA_EXPECT_EQUAL(allowed.decode().code,
                    deserialization_status::kSucceeded);
}

// +===========================================================================+
// | [>] zero chunked payload limit accepts larger bodies        ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder accepts chunked payload with zero length limit") {
  policies configuration;
  configuration.max_content_length = 0;
  decoder_input value(configuration);
  const std::string source =
      "POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n"
      "3\r\nabc\r\n3\r\ndef\r\n0\r\n\r\n";
  DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
  auto result = value.decode();
  DOBA_EXPECT_EQUAL(result.code, deserialization_status::kSucceeded);
  DOBA_EXPECT(value.request_seen);
}

// +===========================================================================+
// | [>] chunked wire spill across receives                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder spills chunked wire across receives") {
  constexpr std::size_t limit = policies::kMaxRequestBodySizeInMemory;
  DOBA_EXPECT_EQUAL(policies{}.max_content_length, 16 * 1024 * 1024);
  for (const std::size_t second_chunk : {8171, 8172}) {
    std::string source =
        "POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\n\r\n"
        "2000\r\n";
    source.append(8192, 'a');
    source += "\r\n";
    source += second_chunk == 8171 ? "1feb\r\n" : "1fec\r\n";
    source.append(second_chunk, 'b');
    source += "\r\n0\r\n\r\n";
    DOBA_EXPECT_EQUAL(source.size() - source.find("2000\r\n"),
                      limit + (second_chunk - 8171));
    decoder_input value;
    bool finished = false;
    for (std::size_t offset = 0; offset < source.size();) {
      const std::size_t count =
          std::min(std::size_t{4096}, source.size() - offset);
      DOBA_EXPECT_EQUAL(accumulate(
          value, std::string_view(source).substr(offset, count)), count);
      offset += count;
      auto result = value.decode();
      if (result.code == deserialization_status::kSucceeded) {
        DOBA_EXPECT_EQUAL(offset, source.size());
        DOBA_EXPECT(value.request_seen);
        finished = true;
        break;
      }
      DOBA_EXPECT_EQUAL(result.code,
                        deserialization_status::kMoreBytesNeeded);
    }
    DOBA_EXPECT(finished);
  }
}

// +===========================================================================+
// | [>] transfer coding count spans field lines                 ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder counts transfer codings across field lines") {
  const std::string head = "POST / HTTP/1.1\r\nHost: a\r\n";
  for (const std::size_t count : {std::size_t{4}, std::size_t{5}}) {
    std::string source = head;
    for (std::size_t i = 1; i < count; i++) {
      source += "Transfer-Encoding: gzip\r\n";
    }
    source += "Transfer-Encoding: chunked\r\n\r\n0\r\n\r\n";
    decoder_input value;
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    DOBA_EXPECT_EQUAL(value.decode().code,
                      count == 4 ? deserialization_status::kSucceeded
                                 : deserialization_status::kInvalidSource);
  }
}

// +===========================================================================+
// | [>] forwarding hop default boundary                         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder enforces default forwarding hop limit") {
  for (const std::size_t count : {std::size_t{32}, std::size_t{33}}) {
    std::string source = "GET / HTTP/1.1\r\nHost: a\r\nVia: ";
    for (std::size_t i = 0; i < count; i++) {
      if (i) source += ", ";
      source += "1.1 proxy";
    }
    source += "\r\n\r\n";
    decoder_input value;
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    DOBA_EXPECT_EQUAL(value.decode().code,
                      count == 32 ? deserialization_status::kSucceeded
                                  : deserialization_status::kInvalidSource);
  }
}

// +===========================================================================+
// | [>] forwarding hops accumulate across header fields         ( test-case ) |
// +===========================================================================+
DOBA_TEST("decoder counts forwarding hops across header fields") {
  for (const std::size_t count : {std::size_t{32}, std::size_t{33}}) {
    std::string source = "GET / HTTP/1.1\r\nHost: a\r\nVia: ";
    for (std::size_t i = 0; i < 10; i++) {
      if (i) source += ", ";
      source += "1.1 proxy";
    }
    source += "\r\nForwarded: ";
    for (std::size_t i = 0; i < 11; i++) {
      if (i) source += ", ";
      source += "for=192.0.2.43";
    }
    source += "\r\nX-Forwarded-For: ";
    for (std::size_t i = 0; i < count - 21; i++) {
      if (i) source += ", ";
      source += "unknown";
    }
    source += "\r\n\r\n";
    decoder_input value;
    DOBA_EXPECT_EQUAL(accumulate(value, source), source.size());
    DOBA_EXPECT_EQUAL(value.decode().code,
                      count == 32 ? deserialization_status::kSucceeded
                                  : deserialization_status::kInvalidSource);
  }
}
