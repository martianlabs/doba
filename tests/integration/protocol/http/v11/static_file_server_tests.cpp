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

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "common/filesystem.h"
#include "common/reader.h"
#include "protocol/http/v11/server.h"
#include "protocol/http/v11/static_file_server.h"
#include "http_test_helper.h"
#include "tcpip_client.h"
#include "test_helper.h"

namespace {
namespace fs = std::filesystem;
using martianlabs::doba::common::filesystem_file;
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] file_directory                                              ( class ) |
// +---------------------------------------------------------------------------+
// | Internal implementation detail.                                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class file_directory {
 public:
  // +=========================================================================+
  // | [>] METHODs                                                  ( public ) |
  // +=========================================================================+
  file_directory() {
    static std::atomic<unsigned int> counter{0};
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    do {
      path_ = fs::temp_directory_path() /
          ("doba_files_" + std::to_string(stamp) + "_" +
           std::to_string(counter.fetch_add(1)));
    } while (!fs::create_directory(path_));
  }
  ~file_directory() {
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

namespace {
using namespace martianlabs::doba::protocol::http::v11;
using martianlabs::doba::tests::integration::tcpip_client;
using martianlabs::doba::tests::integration::receive_http_response;
}  // namespace

// +===========================================================================+
// | [>] HTTP static file framing                                ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP static files preserve binary framing HEAD and pipelining") {
  file_directory first;
  file_directory second;
  std::string body(131073, '\0');
  for (std::size_t i = 0; i < body.size(); i++) body[i] = char(i % 256);
  first.write("file.bin", body);
  second.write("hello.txt", "other");
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> value({.ip = "127.0.0.1", .port = std::to_string(port)});
  value.add_controller<static_file_server>("/assets", first.path());
  value.add_controller<static_file_server>("/other", second.path());
  value.add_route("GET", "/assets/override", [](const request&) {
    auto result = response::ok_200();
    result.set_body("application");
    return result;
  });
  value.start();
  DOBA_EXPECT(client.connect(port));
  DOBA_EXPECT(client.send_all(
      "HEAD /assets/file.bin HTTP/1.1\r\nHost: a\r\n\r\n"
      "GET /assets/file.bin HTTP/1.1\r\nHost: a\r\n\r\n"
      "GET /other/hello.txt HTTP/1.1\r\nHost: a\r\n\r\n"
      "GET /assets/override HTTP/1.1\r\nHost: a\r\n\r\n"));
  auto head = receive_http_response(client, true);
  DOBA_EXPECT(head.has_value());
  DOBA_EXPECT_EQUAL(head->header("Content-Length").value(),
                    std::to_string(body.size()));
  DOBA_EXPECT(head->body.empty());
  auto data = receive_http_response(client);
  DOBA_EXPECT(data.has_value());
  DOBA_EXPECT_EQUAL(data->body, body);
  auto other = receive_http_response(client);
  DOBA_EXPECT(other.has_value());
  DOBA_EXPECT_EQUAL(other->body, "other");
  auto application = receive_http_response(client);
  DOBA_EXPECT(application.has_value());
  DOBA_EXPECT_EQUAL(application->body, "application");
  DOBA_EXPECT(!client.has_data(std::chrono::milliseconds(50)));
  value.stop();
}

// +===========================================================================+
// | [>] HTTP static file paths and changes                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP static files reject escapes and expose current file contents") {
  file_directory directory;
  directory.write("file.txt", "old");
  directory.write("%2e%2e", "literal");
  tcpip_client client;
  const auto port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> value({.ip = "127.0.0.1", .port = std::to_string(port)});
  value.add_controller<static_file_server>("/", directory.path());
  value.start();
  DOBA_EXPECT(client.connect(port));
  struct test_case {
    std::string_view method;
    std::string_view path;
    std::string_view status;
  };
  const test_case cases[] = {
      {"GET", "/missing", "HTTP/1.1 404 Not Found"},
      {"GET", "/%2e%2e/secret", "HTTP/1.1 403 Forbidden"},
      {"GET", "/x%5cy", "HTTP/1.1 403 Forbidden"},
      {"POST", "/file.txt", "HTTP/1.1 405 Method Not Allowed"},
      {"GET", "/%252e%252e", "HTTP/1.1 200 OK"},
  };
  for (const auto& test : cases) {
    DOBA_EXPECT(client.send_all(std::string(test.method) + " " +
        std::string(test.path) + " HTTP/1.1\r\nHost: a\r\n\r\n"));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->status, test.status);
    if (test.method == "POST") {
      DOBA_EXPECT_EQUAL(result->header("Allow").value(), "GET, HEAD");
    }
    if (test.path == "/%252e%252e") DOBA_EXPECT_EQUAL(result->body, "literal");
  }
  for (std::string_view text : {"first", "second"}) {
    directory.write("file.txt", text);
    DOBA_EXPECT(client.send_all(
        "GET /file.txt HTTP/1.1\r\nHost: a\r\n\r\n"));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    DOBA_EXPECT_EQUAL(result->body, text);
  }
  value.stop();
}

// +===========================================================================+
// | [>] HTTP static file concurrent downloads                   ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP static files handle concurrent and abandoned downloads") {
  file_directory directory;
  const std::string body(262145, 'x');
  directory.write("large", body);
  tcpip_client port_probe;
  const auto port = port_probe.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> value({.ip = "127.0.0.1", .port = std::to_string(port)});
  value.add_controller<static_file_server>("/", directory.path());
  value.start();
  {
    tcpip_client abandoned;
    DOBA_EXPECT(abandoned.connect(port));
    DOBA_EXPECT(abandoned.send_all(
        "GET /large HTTP/1.1\r\nHost: a\r\n\r\n"));
    abandoned.close();
  }
  std::atomic<unsigned int> completed{0};
  {
    std::vector<std::jthread> clients;
    for (unsigned int i = 0; i < 4; i++) {
      clients.emplace_back([&]() {
        tcpip_client client;
        if (!client.connect(port) || !client.send_all(
                "GET /large HTTP/1.1\r\nHost: a\r\n\r\n")) return;
        auto result = receive_http_response(client);
        if (result && result->body == body) completed++;
      });
    }
  }
  value.stop();
  DOBA_EXPECT_EQUAL(completed.load(), 4);
  fs::remove(directory.path() / "large");
  DOBA_EXPECT(!fs::exists(directory.path() / "large"));
}
