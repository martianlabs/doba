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

#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <string_view>

#include "protocol/http/v11/body/writer.h"
#include "protocol/http/v11/limits.h"
#include "protocol/http/v11/server.h"
#include "http_test_helper.h"
#include "tcpip_client.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::common::task;
using martianlabs::doba::protocol::http::v11::body::body_writer;
using martianlabs::doba::protocol::http::v11::limits;
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::protocol::http::v11::server;
using martianlabs::doba::tests::integration::http_test_signal;
using martianlabs::doba::tests::integration::receive_http_response;
using martianlabs::doba::tests::integration::tcpip_client;
}  // namespace

// +===========================================================================+
// | [>] outgoing chunk boundaries survive the transport         ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 transmits and terminates every outgoing chunk") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  http_server.add_route("GET", "/chunks", [](const request&) {
    auto writer = body_writer::chunked();
    if (!writer.write("a") || !writer.write("bc") ||
        !writer.write("def")) {
      return response::internal_server_error_500();
    }
    response result = response::ok_200();
    result.set_body(std::move(writer));
    return result;
  });
  http_server.add_route("GET", "/next", [](const request&) {
    response result = response::ok_200();
    result.set_body("next");
    return result;
  });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /chunks HTTP/1.1\r\nHost: a\r\n\r\n"
      "GET /next HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto chunks = receive_http_response(client);
  DOBA_EXPECT(chunks.has_value());
  if (chunks.has_value()) {
    DOBA_EXPECT_EQUAL(chunks->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(chunks->header("Transfer-Encoding").value(), "chunked");
    DOBA_EXPECT_EQUAL(chunks->body, "abcdef");
    DOBA_EXPECT_EQUAL(chunks->wire_body,
                      "1\r\na\r\n2\r\nbc\r\n3\r\ndef\r\n0\r\n\r\n");
  }
  const auto next = receive_http_response(client);
  DOBA_EXPECT(next.has_value());
  if (next.has_value()) DOBA_EXPECT_EQUAL(next->body, "next");
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  http_server.stop();
}

// +===========================================================================+
// | [>] inline and streamed response body boundary              ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 preserves binary responses across the spill boundary") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  http_server.add_route("GET", "/body/:size",
                        [](const request&, std::size_t size) {
    std::string body(size, '\0');
    for (std::size_t index = 0; index < body.size(); index++) {
      body[index] = static_cast<char>((index * 71 + index / 17) % 256);
    }
    response result = response::ok_200();
    result.set_body(body);
    return result;
  });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

  for (const std::size_t size : {limits::kMaxResponseBodySizeInMemory - 1,
                                 limits::kMaxResponseBodySizeInMemory,
                                 limits::kMaxResponseBodySizeInMemory + 1}) {
    std::string expected(size, '\0');
    for (std::size_t index = 0; index < expected.size(); index++) {
      expected[index] = static_cast<char>((index * 71 + index / 17) % 256);
    }
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("GET /body/" + std::to_string(size) +
                                " HTTP/1.1\r\nHost: a\r\n\r\n"));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    if (result.has_value()) {
      DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
      DOBA_EXPECT_EQUAL(result->body, expected);
    }
    client.close();
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] cleared responses delimit persistent connections        ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 delimits cleared bodies on persistent connections") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  http_server.add_route("GET", "/clear", [](const request&) {
    response result = response::ok_200();
    result.set_body("x").clear_body().clear_body();
    return result;
  });
  http_server.add_route("GET", "/next", [](const request&) {
    response result = response::ok_200();
    result.set_body("next");
    return result;
  });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

  for (bool pipelined : {false, true}) {
    DOBA_EXPECT(client.connect(port));
    std::string requests = "GET /clear HTTP/1.1\r\nHost: a\r\n\r\n";
    if (pipelined) requests += "GET /next HTTP/1.1\r\nHost: a\r\n\r\n";
    DOBA_EXPECT(client.send_all(requests));
    const auto cleared = receive_http_response(client);
    DOBA_EXPECT(cleared.has_value());
    DOBA_EXPECT_EQUAL(cleared->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(cleared->header("Content-Length").value(), "0");
    DOBA_EXPECT(!cleared->header("Transfer-Encoding").has_value());
    DOBA_EXPECT(cleared->body.empty());
    DOBA_EXPECT(cleared->wire_body.empty());
    if (!pipelined) {
      DOBA_EXPECT(client.send_all("GET /next HTTP/1.1\r\nHost: a\r\n\r\n"));
    }
    const auto next = receive_http_response(client);
    DOBA_EXPECT(next.has_value());
    DOBA_EXPECT_EQUAL(next->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(next->body, "next");
    DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
    client.close();
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] all bodyless final statuses delimit a pipeline          ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 suppresses 205 and 304 bodies before successors") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  http_server.add_route("GET", "/reset", [](const request&) {
    response result = response::reset_content_205();
    result.set_body("hidden");
    return result;
  });
  http_server.add_route("GET", "/cached", [](const request&) {
    response result = response::not_modified_304();
    result.set_body("hidden");
    return result;
  });
  http_server.add_route("GET", "/next", [](const request&) {
    response result = response::ok_200();
    result.set_body("next");
    return result;
  });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /reset HTTP/1.1\r\nHost: a\r\n\r\n"
      "GET /cached HTTP/1.1\r\nHost: a\r\n\r\n"
      "GET /next HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto reset = receive_http_response(client);
  DOBA_EXPECT(reset.has_value());
  if (reset.has_value()) {
    DOBA_EXPECT_EQUAL(reset->status, "HTTP/1.1 205 Reset Content");
    DOBA_EXPECT_EQUAL(reset->body, "");
    DOBA_EXPECT_EQUAL(reset->header("Content-Length").value(), "0");
  }
  const auto cached = receive_http_response(client);
  DOBA_EXPECT(cached.has_value());
  if (cached.has_value()) {
    DOBA_EXPECT_EQUAL(cached->status, "HTTP/1.1 304 Not Modified");
    DOBA_EXPECT_EQUAL(cached->body, "");
    DOBA_EXPECT_EQUAL(cached->header("Content-Length").value(), "6");
  }
  const auto next = receive_http_response(client);
  DOBA_EXPECT(next.has_value());
  if (next.has_value()) DOBA_EXPECT_EQUAL(next->body, "next");
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(100)));
  http_server.stop();
}

// +===========================================================================+
// | [>] handler and serializer failures isolate their channels  ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 converts response failures and recovers on new clients") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  http_server.add_route("GET", "/throw", [](const request&) -> response {
    throw std::runtime_error("doba-private-sync-token");
  });
  http_server.add_route("GET", "/unknown", [](const request&) -> response {
    throw 1;
  });
  http_server.add_route("GET", "/framing", [](const request&) {
    response result = response::ok_200();
    result.set_header("Transfer-Encoding", "chunked");
    result.set_header("Content-Length", "1");
    return result;
  });
  http_server.add_route("GET", "/ok", [](const request&) {
    response result = response::ok_200();
    result.set_body("ok");
    return result;
  });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

  // +=========================================================================+
// | [>] failure_case                                               ( struct ) |
  // +=========================================================================+
  struct failure_case {
    // +=======================================================================+
    // | [>] ATTRIBUTEs                                             ( public ) |
    // +=======================================================================+
    std::string_view path;
    std::string_view detail;
  };
  constexpr failure_case cases[] = {
      {"/throw", "doba-private-sync-token"},
      {"/unknown", "Request handler error!"},
      {"/framing", "conflicting response framing headers!"},
  };
  for (const auto& test : cases) {
    martianlabs::doba::tests::integration::test_helper::set_context(test.path);
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("GET " + std::string(test.path) +
                                " HTTP/1.1\r\nHost: a\r\n\r\n"));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    if (result.has_value()) {
      DOBA_EXPECT_EQUAL(result->status,
                        "HTTP/1.1 500 Internal Server Error");
      DOBA_EXPECT_EQUAL(result->body, "Internal Server Error");
      DOBA_EXPECT_EQUAL(result->wire_body, "Internal Server Error");
      DOBA_EXPECT_EQUAL(result->header("Content-Length").value(), "21");
      DOBA_EXPECT(result->wire_body.find(test.detail) == std::string::npos);
      for (const auto& header : result->headers) {
        DOBA_EXPECT(header.first.find(test.detail) == std::string::npos);
        DOBA_EXPECT(header.second.find(test.detail) == std::string::npos);
      }
    }
    DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
    client.close();
  }
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all("GET /ok HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto recovery = receive_http_response(client);
  DOBA_EXPECT(recovery.has_value());
  if (recovery.has_value()) DOBA_EXPECT_EQUAL(recovery->body, "ok");
  http_server.stop();
}

// +===========================================================================+
// | [>] deferred HTTP errors hide internal diagnostics          ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 hides deferred handler and serializer diagnostics") {
  for (std::string_view scenario : {"throw", "unknown", "framing"}) {
    martianlabs::doba::tests::integration::test_helper::set_context(scenario);
    tcpip_client client;
    const uint16_t port = client.find_available_port();
    DOBA_EXPECT(port != 0);
    auto signal = std::make_shared<http_test_signal>();
    server<> http_server;
    http_server.add_route(
        "GET", "/fail",
        [signal, scenario](std::shared_ptr<const request>,
                            std::stop_token token) -> task<response> {
      std::stop_callback cancellation(
          token, [signal]() { signal->resume(); });
      co_await *signal;
      if (scenario == "throw") {
        throw std::runtime_error("doba-private-deferred-token");
      }
      if (scenario == "unknown") throw 1;
      response result = response::ok_200();
      result.set_header("Transfer-Encoding", "chunked");
      result.set_header("Content-Length", "1");
      co_return result;
    });
    http_server.add_route("GET", "/ok", [](const request&) {
      response result = response::ok_200();
      result.set_body("ok");
      return result;
    });
    const std::string port_text = std::to_string(port);
    http_server.start(port_text.c_str());

    DOBA_EXPECT(client.connect(port));
    const bool sent = client.send_all("GET /fail HTTP/1.1\r\nHost: a\r\n\r\n");
    const bool waiting = signal->wait();
    const bool premature = client.has_data(std::chrono::milliseconds(50));
    signal->resume();
    DOBA_EXPECT(sent);
    DOBA_EXPECT(waiting);
    DOBA_EXPECT(!premature);
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 500 Internal Server Error");
    DOBA_EXPECT_EQUAL(result->body, "Internal Server Error");
    DOBA_EXPECT_EQUAL(result->wire_body, "Internal Server Error");
    DOBA_EXPECT_EQUAL(result->header("Content-Length").value(), "21");
    const std::string_view detail = scenario == "throw"
        ? "doba-private-deferred-token" : scenario == "unknown"
        ? "Request handler error!" : "conflicting response framing headers!";
    DOBA_EXPECT(result->wire_body.find(detail) == std::string::npos);
    for (const auto& header : result->headers) {
      DOBA_EXPECT(header.first.find(detail) == std::string::npos);
      DOBA_EXPECT(header.second.find(detail) == std::string::npos);
    }
    DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
    client.close();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("GET /ok HTTP/1.1\r\nHost: a\r\n\r\n"));
    const auto recovery = receive_http_response(client);
    DOBA_EXPECT(recovery.has_value());
    DOBA_EXPECT_EQUAL(recovery->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(recovery->body, "ok");
    http_server.stop();
  }
}

namespace {
namespace fs = std::filesystem;
using martianlabs::doba::common::filesystem_file;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] response_file_directory                                     ( class ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class response_file_directory {
 public:
  // +=========================================================================+
  // | [>] METHODs                                                  ( public ) |
  // +=========================================================================+
  response_file_directory() {
    static std::atomic<unsigned int> counter{0};
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    do {
      path_ = fs::temp_directory_path() /
          ("doba_files_" + std::to_string(stamp) + "_" +
           std::to_string(counter.fetch_add(1)));
    } while (!fs::create_directory(path_));
  }
  ~response_file_directory() {
    std::error_code error;
    fs::remove_all(path_, error);
  }
  const fs::path& path() const { return path_; }
  void write(std::string_view name, std::string_view contents) {
    const fs::path relative(std::u8string(name.begin(), name.end()));
    std::ofstream output(path_ / relative, std::ios::binary);
    output.write(contents.data(),
                 static_cast<std::streamsize>(contents.size()));
    if (!output) throw std::runtime_error("Unable to write fixture");
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  fs::path path_;
};
}  // namespace

// +===========================================================================+
// | [>] incomplete file response                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP closes incomplete file responses before subsequent bytes") {
  response_file_directory directory;
  directory.write("file", "abcdef");
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> value;
  value.add_route("GET", "/file", [&](const request&) {
    filesystem_file file;
    std::error_code error;
    if (!file.open(directory.path(), "file", error)) {
      throw std::runtime_error("Unable to open file");
    }
    auto result = response::ok_200();
    const auto length = file.size();
    result.set_body(martianlabs::doba::common::reader(std::move(file)), length);
    fs::resize_file(directory.path() / "file", 1);
    return result;
  });
  value.add_route("GET", "/next", [](const request&) {
    auto result = response::ok_200();
    result.set_body("must-not-be-transmitted");
    return result;
  });
  value.start(std::to_string(port).c_str());
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "GET /file HTTP/1.1\r\nHost: a\r\n\r\n"
      "GET /next HTTP/1.1\r\nHost: a\r\n\r\n"));
  std::string wire;
  while (wire.size() < 8192) {
    auto byte = client.receive(1);
    if (!byte) break;
    wire += *byte;
  }
#ifdef _WIN32
  const int reset = WSAECONNRESET;
#else
  const int reset = ECONNRESET;
#endif
  DOBA_EXPECT(client.error() == "eof" ||
              (client.error() == "socket" && client.native_error() == reset));
  DOBA_EXPECT(wire.find("must-not-be-transmitted") == wire.npos);
  DOBA_EXPECT(wire.find("abcdef") == wire.npos);
  value.stop();
}
