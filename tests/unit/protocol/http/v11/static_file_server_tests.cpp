//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
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
#include <atomic>
#include <chrono>
#include <deque>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>

#include "common/filesystem.h"
#include "common/reader.h"
#include "file_directory.h"
#include "test_helper.h"

#include "protocol/http/v11/static_file_server.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/common/router.h"

namespace {
using martianlabs::doba::tests::file_directory;
namespace fs = std::filesystem;
using martianlabs::doba::common::filesystem_file;
}  // namespace

namespace {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] usings                                                     ( public ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
using namespace martianlabs::doba::protocol::http::v11;
using file_router =
    martianlabs::doba::protocol::http::router<request, response>;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] file_request                                             ( function ) |
// +---------------------------------------------------------------------------+
// | This function deserializes a request and invokes the provided callback    |
// | with the decoded request. It asserts that the deserialization succeeds    |
// | and that the callback is called.                                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
response file_request(file_router& routes, std::string_view method,
                      std::string_view path, std::string_view headers = {}) {
  std::string wire = std::string(method) + " " + std::string(path) +
                     " HTTP/1.1\r\nHost: example.com\r\n" +
                     std::string(headers) + "\r\n";
  decoder<request, response> decoder;
  std::size_t consumed = 0;
  static thread_local std::deque<std::array<char, 20480>> storage;
  storage.emplace_back();
  std::optional<response> result;
  result.emplace(storage.back());
  auto decoded =
      decoder.deserialize(wire.data(), wire.size(), 8192, consumed);
  if (decoded.code !=
          martianlabs::doba::protocol::deserialization_status::kSucceeded ||
      !decoded.request) {
    throw std::runtime_error("Invalid test request");
  }
  const auto& value = *decoded.request;
  const auto match = routes.match(method, value.get_absolute_path());
  if (!match.handler) {
    result->not_found_404();
  } else {
    (*match.handler)(value, *result);
  }
  return std::move(*result);
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] file_body                                                ( function ) |
// +---------------------------------------------------------------------------+
// | This function serializes a response and returns the body as a string.     |
// | It reads all data from the source if available.                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
std::string file_body(response& value) {
  auto serialized = value.serialize();
  std::string body;
  if (serialized.source) serialized.source->read_all(body);
  return body;
}

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] file_status                                              ( function ) |
// +---------------------------------------------------------------------------+
// | This function serializes a response and returns the status line           |
// | and headers as a string.                                                  |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
std::string file_status(response& value) {
  auto serialized = value.serialize();
  return std::string(serialized.head) + std::string(serialized.body);
}
}  // namespace

// +===========================================================================+
// | [>] static files GET and HEAD                               ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("static file server serves types binary empty files and HEAD") {
  file_directory directory;
  directory.write("hello.TXT", "hello");
  directory.write("data.unknown", std::string_view("a\0b", 3));
  directory.write("empty", "");
  file_router routes;
  routes.add_controller<static_file_server>("/assets/", directory.path());
  auto text = file_request(routes, "GET", "/assets/hello.TXT?version=1");
  DOBA_EXPECT_EQUAL(text.get_header("Content-Type").second, "text/plain");
  DOBA_EXPECT_EQUAL(file_body(text), "hello");
  auto binary = file_request(routes, "GET", "/assets/data.unknown");
  DOBA_EXPECT_EQUAL(binary.get_header("Content-Type").second,
                    "application/octet-stream");
  DOBA_EXPECT_EQUAL(file_body(binary), std::string("a\0b", 3));
  auto head = file_request(routes, "HEAD", "/assets/hello.TXT");
  DOBA_EXPECT_EQUAL(head.get_header("Content-Length").second, "5");
  auto serialized = head.serialize();
  DOBA_EXPECT(!serialized.source);
  auto empty = file_request(routes, "GET", "/assets/empty");
  DOBA_EXPECT_EQUAL(file_body(empty), "");
}

// +===========================================================================+
// | [>] static files configuration and errors                   ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("static file server rejects invalid prefixes roots and directories") {
  file_directory directory;
  directory.write("file", "data");
  fs::create_directory(directory.path() / "sub");
  for (std::string_view prefix :
       {"", "assets", "/a/*", "/a/:id", "/a/../b", "/a//b", "/a?b"}) {
    bool threw = false;
    try {
      static_file_server value(prefix, directory.path());
    } catch (const std::invalid_argument&) {
      threw = true;
    }
    DOBA_EXPECT(threw);
  }
  bool threw = false;
  try {
    static_file_server value("/", directory.path() / "file");
  } catch (const fs::filesystem_error&) {
    threw = true;
  }
  DOBA_EXPECT(threw);
  file_router routes;
  routes.add_controller<static_file_server>("/assets", directory.path());
  for (std::string_view path :
       {"/assets/missing", "/assets/sub", "/assets/sub/", "/assets/", "/assets",
        "/assets2/file"}) {
    auto result = file_request(routes, "GET", path);
    DOBA_EXPECT(file_status(result).starts_with("HTTP/1.1 404"));
  }
}

// +===========================================================================+
// | [>] static files decoded paths                              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("static file server uses the already decoded request path") {
  file_directory directory;
  fs::create_directory(directory.path() / "sub");
  directory.write("a b.txt", "space");
  directory.write("%2e%2e", "literal");
  directory.write("sub/file", "sub");
  file_router routes;
  routes.add_controller<static_file_server>("/", directory.path());
  auto space = file_request(routes, "GET", "/a%20b.txt");
  DOBA_EXPECT_EQUAL(file_body(space), "space");
  auto literal = file_request(routes, "GET", "/%252e%252e");
  DOBA_EXPECT_EQUAL(file_body(literal), "literal");
  auto separator = file_request(routes, "GET", "/sub%2ffile");
  DOBA_EXPECT_EQUAL(file_body(separator), "sub");
  for (std::string_view path : {"/../outside", "/%2e%2e/outside", "/sub%5cfile",
                                "/C%3afile", "//file"}) {
    auto result = file_request(routes, "GET", path);
    DOBA_EXPECT(file_status(result).starts_with("HTTP/1.1 403"));
  }
}

// +===========================================================================+
// | [>] static files root escape vectors                        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("static file server never escapes its root") {
  file_directory outer;
  outer.write("secret", "top-secret");
  fs::create_directory(outer.path() / "root");
  fs::create_directory(outer.path() / "root" / "sub");
  outer.write("root/sub/file", "inside");
  file_router routes;
  routes.add_controller<static_file_server>("/files/", outer.path() / "root");
  std::vector<std::string> vectors{"/files/../secret",
                                   "/files/sub/../../secret",
                                   "/files/sub/%2e%2e/%2e%2e/secret",
                                   "/files/%2E%2E/secret",
                                   "/files/sub%2f..%2f..%2fsecret",
                                   "/files/.%2fsub/file",
                                   "/files/sub/./file",
                                   "/files/sub//file",
                                   "/files/..%5csecret",
                                   "/files/sub%5c..%5c..%5csecret",
                                   "/files/%01secret",
                                   "/files/sub%7ffile",
                                   "/files/C%3a%5csecret",
                                   "/files/file%3a%3a$DATA"};
  const auto absolute = (outer.path() / "secret").generic_string();
  std::string encoded;
  for (const char c : absolute) {
    if (c == ':')
      encoded += "%3a";
    else
      encoded += c;
  }
  vectors.push_back("/files/" + encoded);
  for (const auto& path : vectors) {
    martianlabs::doba::tests::unit::test_helper::set_context(path);
    const std::string wire =
        "GET " + path + " HTTP/1.1\r\nHost: example.com\r\n\r\n";
    decoder<request, response> value;
    std::size_t consumed = 0;
    auto decoded =
        value.deserialize(wire.data(), wire.size(), 8192, consumed);
    if (!decoded.request) {
      DOBA_EXPECT_EQUAL(
          decoded.code,
          martianlabs::doba::protocol::deserialization_status::kInvalidSource);
      continue;
    }
    auto result = file_request(routes, "GET", path);
    auto serialized = result.serialize();
    const std::string status(std::string(serialized.head) +
                             std::string(serialized.body));
    DOBA_EXPECT(status.starts_with("HTTP/1.1 403") ||
                status.starts_with("HTTP/1.1 404"));
    DOBA_EXPECT(!serialized.source);
    DOBA_EXPECT(status.find("top-secret") == std::string_view::npos);
  }
  auto control = file_request(routes, "GET", "/files/sub/file");
  DOBA_EXPECT_EQUAL(file_body(control), "inside");
}

// +===========================================================================+
// | [>] static files direct access                              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("static file server opens a fresh source without preloading") {
  file_directory directory;
  directory.write("file", "old");
  file_router routes;
  routes.add_controller<static_file_server>("/", directory.path());
  auto first = file_request(routes, "GET", "/file");
  directory.write("file", "new");
  DOBA_EXPECT_EQUAL(file_body(first), "new");
  directory.write("file", "longer");
  auto second = file_request(routes, "GET", "/file");
  DOBA_EXPECT_EQUAL(file_body(second), "longer");
}

// +===========================================================================+
// | [>] static files preconditions                              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("static file server evaluates representation conditions") {
  file_directory directory;
  directory.write("file", "data");
  file_router routes;
  routes.add_controller<static_file_server>("/", directory.path());
  struct condition_case {
    std::string_view fields;
    std::string_view status;
  };
  const condition_case cases[] = {
      {"If-Match: *\r\n", "HTTP/1.1 200"},
      {"If-Match: \"unknown\"\r\n", "HTTP/1.1 412"},
      {"If-None-Match: *\r\n", "HTTP/1.1 304"},
      {"If-None-Match: \"unknown\"\r\n", "HTTP/1.1 200"},
      {"If-Match: \"unknown\"\r\nIf-None-Match: *\r\n", "HTTP/1.1 412"},
      {"Range: bytes=0-1\r\n", "HTTP/1.1 200"},
  };
  for (const auto& test : cases) {
    auto result = file_request(routes, "GET", "/file", test.fields);
    auto serialized = result.serialize();
    const std::string prefix(
        (std::string(serialized.head) + std::string(serialized.body)));
    DOBA_EXPECT(prefix.starts_with(test.status));
    if (test.status == "HTTP/1.1 304") {
      DOBA_EXPECT(!serialized.source);
      DOBA_EXPECT(prefix.find("Content-Length:") == prefix.npos);
    }
  }
  auto missing = file_request(routes, "GET", "/missing", "If-Match: *\r\n");
  DOBA_EXPECT(file_status(missing).starts_with("HTTP/1.1 404"));
}
