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
#include <thread>

#include "common/filesystem.h"
#include "common/reader.h"
#include "directory_link.h"
#include "file_directory.h"
#include "test_helper.h"

#include "protocol/http/v11/static_file_server.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/common/router.h"

namespace {
using martianlabs::doba::protocol::http::router;
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

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] file_request                                             ( function ) |
// +---------------------------------------------------------------------------+
// | This function deserializes a request and invokes the provided callback    |
// | with the decoded request. It asserts that the deserialization succeeds    |
// | and that the callback is called.                                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
response file_request(router<request, response>& routes,
                      std::string_view method, std::string_view path,
                      std::string_view headers = {}) {
  std::string wire = std::string(method) + " " + std::string(path) +
                     " HTTP/1.1\r\nHost: example.com\r\n" +
                     std::string(headers) + "\r\n";
  decoder<request, response> decoder;
  std::string_view decoder_input;
  std::optional<response> result;
  result.emplace();
  decoder_input = std::string_view(wire.data(), wire.size());
  auto decoded =
      decoder.deserialize(decoder_input);
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
  router<request, response> routes;
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
  router<request, response> routes;
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
  router<request, response> routes;
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
  router<request, response> routes;
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
    std::string_view value_input;
    value_input = std::string_view(wire.data(), wire.size());
    auto decoded =
        value.deserialize(value_input);
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
  router<request, response> routes;
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
  router<request, response> routes;
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
      {"Range: bytes=0-1\r\n", "HTTP/1.1 206"},
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

// +===========================================================================+
// | [>] static file cache freshness                             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("static file cache revalidates every request and retains old snapshots") {
  file_directory directory;
  directory.write("file.txt", "old");
  auto cached = std::make_shared<static_file_cache>(
      martianlabs::doba::common::cache_policies{
          .ttl = std::chrono::hours(24)});
  router<request, response> routes;
  routes.add_controller<static_file_server>("/", directory.path(), static_file_policies{.cache = cached});
  auto first = file_request(routes, "GET", "/file.txt");
  const auto tag = std::string(first.get_header("ETag").second);
  const auto timestamp = fs::last_write_time(directory.path() / "file.txt");
  directory.write("file.txt", "new");
  fs::last_write_time(directory.path() / "file.txt", timestamp);
  auto fresh = file_request(routes, "GET", "/file.txt");
  DOBA_EXPECT(tag != fresh.get_header("ETag").second);
  DOBA_EXPECT_EQUAL(file_body(first), "old");
  DOBA_EXPECT_EQUAL(file_body(fresh), "new");
  fs::remove(directory.path() / "file.txt");
  auto missing = file_request(routes, "GET", "/file.txt");
  DOBA_EXPECT(file_status(missing).starts_with("HTTP/1.1 404"));
}

// +===========================================================================+
// | [>] static file replacement and conditional requests        ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("static file cache observes replacements before conditional evaluation") {
  file_directory directory;
  directory.write("file.txt", "old");
  auto cache = std::make_shared<static_file_cache>(
      martianlabs::doba::common::cache_policies{
          .ttl = std::chrono::hours(24)});
  router<request, response> routes;
  routes.add_controller<static_file_server>(
      "/", directory.path(), static_file_policies{.cache = cache});
  auto first = file_request(routes, "GET", "/file.txt");
  const auto tag = first.get_header("ETag").second;
  const auto timestamp = fs::last_write_time(directory.path() / "file.txt");
  fs::rename(directory.path() / "file.txt", directory.path() / "previous");
  directory.write("file.txt", "new");
  fs::last_write_time(directory.path() / "file.txt", timestamp);
  auto next = file_request(routes, "GET", "/file.txt",
                           "If-None-Match: " + tag + "\r\n");
  DOBA_EXPECT(next.get_header("ETag").second != tag);
  auto wire = next.serialize();
  DOBA_EXPECT(wire.head.starts_with("HTTP/1.1 200"));
  std::string bytes;
  if (wire.source) wire.source->read_all(bytes);
  DOBA_EXPECT_EQUAL(bytes, "new");
  DOBA_EXPECT_EQUAL(file_body(first), "old");
  auto head = file_request(routes, "HEAD", "/file.txt");
  DOBA_EXPECT(head.get_header("ETag").second != tag);
  auto old_match = file_request(routes, "GET", "/file.txt",
                                "If-Match: " + tag + "\r\n");
  DOBA_EXPECT(file_status(old_match).starts_with("HTTP/1.1 412"));
  auto old_range = file_request(routes, "GET", "/file.txt",
                                "Range: bytes=0-0\r\nIf-Range: " + tag + "\r\n");
  DOBA_EXPECT_EQUAL(file_body(old_range), "new");
}

// +===========================================================================+
// | [>] compressed file replacement                             ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("static file cache revalidates encoded variants and SPA fallback") {
  file_directory directory;
  directory.write("app.js", "identity");
  directory.write("app.js.br", "old");
  directory.write("fallback.html", "first");
  auto cache = std::make_shared<static_file_cache>(
      martianlabs::doba::common::cache_policies{
          .ttl = std::chrono::hours(24)});
  router<request, response> routes;
  routes.add_controller<static_file_server>(
      "/", directory.path(), static_file_policies{
          .cache = cache, .precompressed = true,
          .spa_fallback = "fallback.html"});
  auto encoded = file_request(routes, "GET", "/app.js",
                               "Accept-Encoding: br\r\n");
  auto fallback = file_request(routes, "GET", "/missing");
  directory.write("app.js.br", "new");
  directory.write("fallback.html", "other");
  auto next = file_request(routes, "GET", "/app.js",
                            "Accept-Encoding: br\r\n");
  auto next_fallback = file_request(routes, "GET", "/missing");
  DOBA_EXPECT_EQUAL(file_body(next), "new");
  DOBA_EXPECT_EQUAL(file_body(next_fallback), "other");
  DOBA_EXPECT_EQUAL(file_body(encoded), "old");
  DOBA_EXPECT_EQUAL(file_body(fallback), "first");
  fs::remove(directory.path() / "app.js.br");
  auto identity = file_request(routes, "GET", "/app.js",
                                "Accept-Encoding: br\r\n");
  DOBA_EXPECT_EQUAL(file_body(identity), "identity");
}

// +===========================================================================+
// | [>] cached file resize                                     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("static file cache observes in-place size changes immediately") {
  file_directory directory;
  directory.write("file", "old");
  auto cache = std::make_shared<static_file_cache>(
      martianlabs::doba::common::cache_policies{
          .ttl = std::chrono::hours(24)});
  router<request, response> routes;
  routes.add_controller<static_file_server>(
      "/", directory.path(), static_file_policies{.cache = cache});
  auto first = file_request(routes, "GET", "/file");
  directory.write("file", "longer");
  auto next = file_request(routes, "GET", "/file");
  DOBA_EXPECT_EQUAL(file_body(first), "old");
  DOBA_EXPECT_EQUAL(file_body(next), "longer");
}

// +===========================================================================+
// | [>] cached bytes beyond the first validation block          ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("static file cache checks all bytes when size and mtime are unchanged") {
  file_directory directory;
  std::string body(16385, 'x');
  directory.write("file", body);
  const auto timestamp = fs::last_write_time(directory.path() / "file");
  auto cache = std::make_shared<static_file_cache>();
  router<request, response> routes;
  routes.add_controller<static_file_server>(
      "/", directory.path(), static_file_policies{.cache = cache});
  auto first = file_request(routes, "GET", "/file");
  const auto tag = first.get_header("ETag").second;
  body.back() = 'z';
  directory.write("file", body);
  fs::last_write_time(directory.path() / "file", timestamp);
  auto next = file_request(routes, "GET", "/file",
                            "If-None-Match: " + tag + "\r\n");
  DOBA_EXPECT(next.get_header("ETag").second != tag);
  DOBA_EXPECT_EQUAL(file_body(next), body);
  auto range = file_request(routes, "GET", "/file", "Range: bytes=-1\r\n");
  DOBA_EXPECT_EQUAL(file_body(range), "z");
  body.back() = 'x';
  DOBA_EXPECT_EQUAL(file_body(first), body);
}

// +===========================================================================+
// | [>] cached directory replaced by a link                     ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("static file cache rejects a directory link after warming") {
  file_directory directory;
  file_directory outside;
  fs::create_directory(directory.path() / "sub");
  directory.write("sub/file", "inside");
  outside.write("file", "outside");
  auto cache = std::make_shared<static_file_cache>();
  router<request, response> routes;
  routes.add_controller<static_file_server>(
      "/", directory.path(), static_file_policies{.cache = cache});
  auto warm = file_request(routes, "GET", "/sub/file");
  fs::rename(directory.path() / "sub", directory.path() / "previous");
  const auto link = directory.path() / "sub";
  martianlabs::doba::tests::unit::create_directory_link(link, outside.path());
  auto denied = file_request(routes, "GET", "/sub/file");
  DOBA_EXPECT(file_status(denied).starts_with("HTTP/1.1 403"));
  DOBA_EXPECT_EQUAL(file_body(warm), "inside");
  DOBA_EXPECT_EQUAL(cache->statistics().entries, 0);
  fs::remove(link);
}

DOBA_TEST("static file cache isolates roots and bypasses exhausted budgets") {
  file_directory a, b;
  a.write("file", "first"); b.write("file", "other");
  auto cached = std::make_shared<static_file_cache>(martianlabs::doba::common::cache_policies{
      .max_cost = sizeof(static_file_representation) + 66 + 5});
  router<request, response> routes;
  routes.add_controller<static_file_server>("/a", a.path(), static_file_policies{.cache = cached});
  routes.add_controller<static_file_server>("/b", b.path(), static_file_policies{.cache = cached});
  auto first = file_request(routes, "GET", "/a/file");
  auto second = file_request(routes, "GET", "/b/file");
  DOBA_EXPECT(!first.get_header("ETag").second.starts_with("W/"));
  DOBA_EXPECT(second.get_header("ETag").second.starts_with("W/"));
  DOBA_EXPECT_EQUAL(file_body(first), "first");
  DOBA_EXPECT_EQUAL(file_body(second), "other");
  cached->clear();
  DOBA_EXPECT_EQUAL(cached->statistics().live_cost, 0);
}

DOBA_TEST("static files implement validators dates and precondition precedence") {
  file_directory directory;
  directory.write("file", "abc");
  router<request, response> routes;
  routes.add_controller<static_file_server>("/", directory.path(), static_file_policies{
      .cache = std::make_shared<static_file_cache>(), .cache_control = "public, max-age=60"});
  auto original = file_request(routes, "GET", "/file");
  const auto tag = std::string(original.get_header("ETag").second);
  DOBA_EXPECT_EQUAL(tag, "\"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\"");
  const auto date = std::string(original.get_header("Last-Modified").second);
  struct check { std::string headers; std::string_view status; };
  const check cases[] = {
      {"If-None-Match: W/" + tag + "\r\n", "304"},
      {"If-None-Match: \"a,b\", " + tag + "\r\n", "304"},
      {"If-None-Match: , , " + tag + "\r\n", "304"},
      {"If-Match: " + tag + "\r\n", "200"},
      {"If-Match: W/" + tag + "\r\n", "412"},
      {"If-Match: \"missing\"\r\nIf-None-Match: " + tag + "\r\n", "412"},
      {"If-Modified-Since: " + date + "\r\n", "304"},
      {"If-None-Match: \"missing\"\r\nIf-Modified-Since: " + date + "\r\n", "200"},
      {"If-None-Match: \r\nIf-Modified-Since: " + date + "\r\n", "200"},
      {"If-Unmodified-Since: Sun, 06 Nov 1994 08:49:37 GMT\r\n", "412"},
      {"If-Match: " + tag + "\r\nIf-Unmodified-Since: Sun, 06 Nov 1994 08:49:37 GMT\r\n", "200"},
      {"If-Modified-Since: Sun, 31 Feb 2027 08:49:37 GMT\r\n", "200"},
  };
  for (const auto& test : cases) {
    auto result = file_request(routes, "GET", "/file", test.headers);
    DOBA_EXPECT(file_status(result).starts_with("HTTP/1.1 " + std::string(test.status)));
    DOBA_EXPECT_EQUAL(result.get_header("ETag").second, tag);
    DOBA_EXPECT_EQUAL(result.get_header("Cache-Control").second, "public, max-age=60");
  }
}

DOBA_TEST("static file helpers support HTTP dates and content hash boundaries") {
  using namespace martianlabs::doba::protocol::http::v11::detail;
  DOBA_EXPECT_EQUAL(static_file_time("Sun, 06 Nov 1994 08:49:37 GMT"),
                    static_file_time("Sunday, 06-Nov-94 08:49:37 GMT"));
  DOBA_EXPECT_EQUAL(static_file_time("Sun, 06 Nov 1994 08:49:37 GMT"),
                    static_file_time("Sun Nov  6 08:49:37 1994"));
  DOBA_EXPECT_EQUAL(static_file_date(784111777), "Sun, 06 Nov 1994 08:49:37 GMT");
  DOBA_EXPECT_EQUAL(static_file_etag(""), "\"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\"");
  DOBA_EXPECT_EQUAL(static_file_etag(std::string(64, 'a')),
      "\"ffe054fe7ae0cb6dc65c3af9b61d5209f439851db43d0ba5997337df154668eb\"");
}

DOBA_TEST("static file ranges work for cached and streamed representations") {
  for (bool cache_enabled : {false, true}) {
    file_directory directory;
    directory.write("file", "0123456789"); directory.write("empty", "");
    router<request, response> routes;
    routes.add_controller<static_file_server>("/", directory.path(), static_file_policies{
        .cache = cache_enabled ? std::make_shared<static_file_cache>() : nullptr});
    struct range_case { std::string_view range, status, body; };
    const range_case cases[] = {
        {"bytes=2-4", "206", "234"}, {"bytes=8-", "206", "89"},
        {"bytes=-3", "206", "789"}, {"bytes=0-999", "206", "0123456789"},
        {"BYTES=2-3", "206", "23"},
        {"bytes=10-", "416", ""}, {"bytes=-0", "416", ""},
        {"bytes=4-2", "200", "0123456789"}, {"other=0-1", "200", "0123456789"},
        {"bytes=0-1,4-5", "200", "0123456789"},
        {"bytes=999999999999999999999999-", "200", "0123456789"},
    };
    for (const auto& test : cases) {
      auto result = file_request(routes, "GET", "/file", "Range: " + std::string(test.range) + "\r\n");
      auto wire = result.serialize();
      DOBA_EXPECT(wire.head.starts_with("HTTP/1.1 " + std::string(test.status)));
      std::string body;
      if (wire.source) wire.source->read_all(body);
      DOBA_EXPECT_EQUAL(body, test.body);
    }
    auto head = file_request(routes, "HEAD", "/file", "Range: bytes=2-4\r\n");
    DOBA_EXPECT_EQUAL(head.get_header("Content-Length").second, "10");
    auto empty = file_request(routes, "GET", "/empty", "Range: bytes=0-\r\n");
    DOBA_EXPECT(file_status(empty).starts_with("HTTP/1.1 416"));
    auto original = file_request(routes, "GET", "/file");
    const auto tag = std::string(original.get_header("ETag").second);
    auto conditional = file_request(routes, "GET", "/file", "Range: bytes=1-2\r\nIf-Range: " + tag + "\r\n");
    DOBA_EXPECT_EQUAL(file_body(conditional), cache_enabled ? "12" : "0123456789");
  }
}

DOBA_TEST("static files negotiate compressed variants and enforce exclusions") {
  file_directory directory;
  directory.write("app.js", "identity"); directory.write("app.js.br", "brotli");
  directory.write("app.js.gz", "gzip"); directory.write(".env", "secret");
  fs::create_directory(directory.path() / "private"); directory.write("private/key", "secret");
  router<request, response> routes;
  routes.add_controller<static_file_server>("/", directory.path(), static_file_policies{
      .cache = std::make_shared<static_file_cache>(), .precompressed = true, .excluded = {"private"}});
  struct variant_case { std::string_view accept, body, coding; };
  const variant_case cases[] = {
      {"br, gzip", "brotli", "br"}, {"gzip", "gzip", "gzip"},
      {"br;q=0, gzip;q=0.5", "identity", ""},
      {"br;q=0.5, identity;q=0", "brotli", "br"},
      {"*;q=0, gzip;q=1", "gzip", "gzip"}, {"", "identity", ""}};
  for (const auto& test : cases) {
    auto result = file_request(routes, "GET", "/app.js", "Accept-Encoding: " + std::string(test.accept) + "\r\n");
    DOBA_EXPECT_EQUAL(result.get_header("Content-Type").second, "text/javascript");
    if (!test.coding.empty()) DOBA_EXPECT_EQUAL(result.get_header("Content-Encoding").second, test.coding);
    DOBA_EXPECT_EQUAL(result.get_header("Vary").second, "Accept-Encoding");
    DOBA_EXPECT_EQUAL(file_body(result), test.body);
  }
  auto unacceptable = file_request(routes, "GET", "/app.js", "Accept-Encoding: *;q=0\r\n");
  DOBA_EXPECT(file_status(unacceptable).starts_with("HTTP/1.1 406"));
  directory.write("plain.txt", "plain");
  auto unavailable = file_request(routes, "GET", "/plain.txt", "Accept-Encoding: br, identity;q=0\r\n");
  DOBA_EXPECT(file_status(unavailable).starts_with("HTTP/1.1 406"));
  directory.write("only.txt.gz", "only gzip");
  auto only_variant = file_request(routes, "GET", "/only.txt", "Accept-Encoding: gzip;q=0.5\r\n");
  DOBA_EXPECT_EQUAL(file_body(only_variant), "only gzip");
  for (auto path : {"/.env", "/private/key", "/%2eenv", "/../app.js"}) {
    auto forbidden = file_request(routes, "GET", path);
    DOBA_EXPECT(file_status(forbidden).starts_with("HTTP/1.1 403"));
  }
}

DOBA_TEST("static files offer optional index redirects SPA and MIME overrides") {
  file_directory directory;
  fs::create_directory(directory.path() / "sub");
  directory.write("sub/index.html", "index"); directory.write("fallback.html", "spa");
  directory.write("data.custom", "custom");
  router<request, response> routes;
  routes.add_controller<static_file_server>("/files", directory.path(), static_file_policies{
      .redirect_directories = true, .index_file = "index.html", .spa_fallback = "fallback.html",
      .mime_types = {{".custom", "application/x-custom"}}});
  auto redirect = file_request(routes, "GET", "/files/sub?x=hello%20world");
  DOBA_EXPECT(file_status(redirect).starts_with("HTTP/1.1 301"));
  DOBA_EXPECT_EQUAL(redirect.get_header("Location").second, "/files/sub/?x=hello%20world");
  auto mount = file_request(routes, "GET", "/files");
  DOBA_EXPECT_EQUAL(mount.get_header("Location").second, "/files/");
  auto index = file_request(routes, "GET", "/files/sub/");
  DOBA_EXPECT_EQUAL(file_body(index), "index");
  auto spa = file_request(routes, "GET", "/files/missing");
  DOBA_EXPECT_EQUAL(file_body(spa), "spa");
  auto mime = file_request(routes, "GET", "/files/data.custom");
  DOBA_EXPECT_EQUAL(mime.get_header("Content-Type").second, "application/x-custom");
}

DOBA_TEST("static file cache expires and large files preserve direct reading") {
  file_directory directory;
  directory.write("small", "old"); directory.write("large", "long");
  auto cached = std::make_shared<static_file_cache>(martianlabs::doba::common::cache_policies{
      .ttl = std::chrono::milliseconds(5)});
  router<request, response> routes;
  routes.add_controller<static_file_server>("/", directory.path(), static_file_policies{
      .cache = cached, .max_cached_file_size = 3});
  auto old = file_request(routes, "GET", "/small");
  const auto tag = old.get_header("ETag").second;
  directory.write("small", "new");
  std::this_thread::sleep_for(std::chrono::milliseconds(15));
  auto fresh = file_request(routes, "GET", "/small");
  DOBA_EXPECT(fresh.get_header("ETag").second != tag);
  DOBA_EXPECT_EQUAL(file_body(old), "old");
  DOBA_EXPECT_EQUAL(file_body(fresh), "new");
  auto large = file_request(routes, "GET", "/large");
  directory.write("large", "edit");
  DOBA_EXPECT(large.get_header("ETag").second.starts_with("W/"));
  DOBA_EXPECT_EQUAL(file_body(large), "edit");
}
