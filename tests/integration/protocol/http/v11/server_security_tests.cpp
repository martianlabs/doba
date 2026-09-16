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
#include <atomic>
#include <string>
#include <string_view>
#include <vector>

#include "protocol/http/v11/server.h"
#include "http_test_helper.h"
#include "tcpip_client.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::request;
using martianlabs::doba::protocol::http::v11::response;
using martianlabs::doba::protocol::http::v11::server;
using martianlabs::doba::tests::integration::receive_http_response;
using martianlabs::doba::tests::integration::tcpip_client;

response echo_body(const request& req) {
  if (!req.has_body_reader()) return response::bad_request_400();
  std::array<std::byte, 1024> buffer{};
  std::string body;
  for (;;) {
    const auto state = req.get_body_reader()->read(buffer);
    if (state.has_error) return response::bad_request_400();
    body.append(reinterpret_cast<const char*>(buffer.data()), state.produced);
    if (state.complete) {
      response result = response::ok_200();
      result.set_body(body);
      return result;
    }
  }
}
}  // namespace

// +===========================================================================+
// | [>] hostile requests reject successors                      ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 rejects hostile requests without dispatching successors") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  std::atomic<std::size_t> dispatched = 0;
  server<> http_server;
  http_server.add_route("GET", "/sentinel", [&](const request&) {
    dispatched.fetch_add(1);
    response res = response::ok_200();
    res.set_body("sentinel");
    return res;
  });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

  // +=========================================================================+
// | [>] test_case                                                  ( struct ) |
  // +=========================================================================+
  struct test_case {
    // +=======================================================================+
    // | [>] ATTRIBUTEs                                             ( public ) |
    // +=======================================================================+
    std::string name;
    std::string request;
    std::string_view status;
  };
  std::vector<test_case> cases = {
      {"leading request whitespace",
       " GET /sentinel HTTP/1.1\r\nHost: a\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"tab after method",
       "GET\t/sentinel HTTP/1.1\r\nHost: a\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"tab before version",
       "GET /sentinel\tHTTP/1.1\r\nHost: a\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"lowercase protocol",
       "GET /sentinel http/1.1\r\nHost: a\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"malformed version",
       "GET /sentinel HTTP/1.x\r\nHost: a\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"missing Host", "GET /sentinel HTTP/1.1\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"duplicate Host",
       "GET /sentinel HTTP/1.1\r\nHost: a\r\nHost: a\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"duplicate Content-Length",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\n"
       "Content-Length: 1\r\n\r\nx",
       "HTTP/1.1 400 Bad Request"},
      {"comma joined Content-Length",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\n"
       "Content-Length: 1, 1\r\n\r\nx",
       "HTTP/1.1 400 Bad Request"},
      {"signed Content-Length",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\nContent-Length: +1\r\n\r\nx",
       "HTTP/1.1 400 Bad Request"},
      {"negative Content-Length",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\nContent-Length: -1\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"overflowing Content-Length",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\n"
       "Content-Length: 999999999999999999999999999999999999\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"Content-Length with Transfer-Encoding",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\n"
       "Transfer-Encoding: chunked\r\n\r\n0\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"coding without final chunked",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\n"
       "Transfer-Encoding: gzip\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"repeated chunked",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\n"
       "Transfer-Encoding: chunked, chunked\r\n\r\n0\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"chunked not final",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\n"
       "Transfer-Encoding: chunked, gzip\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"Host nominated as hop by hop",
       "GET /sentinel HTTP/1.1\r\nHost: a\r\nConnection: host\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"upgrade without protocol",
       "GET /sentinel HTTP/1.1\r\nHost: a\r\nConnection: upgrade\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"whitespace before colon",
       "GET /sentinel HTTP/1.1\r\nHost : a\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"empty field name",
       "GET /sentinel HTTP/1.1\r\nHost: a\r\n: value\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"missing field colon",
       "GET /sentinel HTTP/1.1\r\nHost: a\r\nX-A value\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"vertical tab field whitespace",
       "GET /sentinel HTTP/1.1\r\nHost: a\r\nX-A:\vvalue\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"carriage return without line feed",
       "GET /sentinel HTTP/1.1\r\nHost: a\rX-A: value\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"bare line feeds", "GET /sentinel HTTP/1.1\nHost: a\n\n",
       "HTTP/1.1 400 Bad Request"},
      {"obsolete folded field",
       "GET /sentinel HTTP/1.1\r\nHost: a\r\nX-A: one\r\n two\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"invalid chunk size",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\n"
       "Transfer-Encoding: chunked\r\n\r\nz\r\nx\r\n0\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"empty chunk size",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\n"
       "Transfer-Encoding: chunked\r\n\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"overflowing chunk size",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\n"
       "Transfer-Encoding: chunked\r\n\r\n"
       "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"chunk data without final carriage return",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\n"
       "Transfer-Encoding: chunked\r\n\r\n1\r\naX",
       "HTTP/1.1 400 Bad Request"},
      {"invalid chunk extension",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\n"
       "Transfer-Encoding: chunked\r\n\r\n1;=x\r\na\r\n0\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"invalid trailer",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\n"
       "Transfer-Encoding: chunked\r\n\r\n0\r\nBad trailer\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"unsupported expectation",
       "POST /sentinel HTTP/1.1\r\nHost: a\r\nContent-Length: 1\r\n"
       "Expect: feature=on\r\n\r\n",
       "HTTP/1.1 417 Expectation Failed"},
      {"newer HTTP version", "GET /sentinel HTTP/1.2\r\nHost: a\r\n\r\n",
       "HTTP/1.1 505 HTTP Version Not Supported"},
      {"encoded null path", "GET /%00 HTTP/1.1\r\nHost: a\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
      {"invalid percent escape", "GET /%GG HTTP/1.1\r\nHost: a\r\n\r\n",
       "HTTP/1.1 400 Bad Request"},
  };
  std::string embedded_null =
      "GET /sentinel HTTP/1.1\r\nHost: a\r\nX-A: ok";
  embedded_null.push_back('\0');
  embedded_null += "bad\r\n\r\n";
  cases.push_back({"embedded null field", std::move(embedded_null),
                   "HTTP/1.1 400 Bad Request"});
  const std::string sentinel =
      "GET /sentinel HTTP/1.1\r\nHost: a\r\n\r\n";
  for (const auto& test : cases) {
    martianlabs::doba::tests::integration::test_helper::set_context(test.name);
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all(test.request + sentinel));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    if (result.has_value()) DOBA_EXPECT_EQUAL(result->status, test.status);
    DOBA_EXPECT(client.wait_for_close(std::chrono::seconds(3)));
    DOBA_EXPECT_EQUAL(dispatched.load(), 0);
    client.close();
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] case insensitive framing and target forms               ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 accepts case insensitive framing and valid target forms") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  http_server.add_route("POST", "/echo", [](const request& req) {
    return echo_body(req);
  });
  http_server.add_route("GET", "/ok", [](const request&) {
    response res = response::ok_200();
    res.set_body("ok");
    return res;
  });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

  // +=========================================================================+
// | [>] test_case                                                  ( struct ) |
  // +=========================================================================+
  struct test_case {
    // +=======================================================================+
    // | [>] ATTRIBUTEs                                             ( public ) |
    // +=======================================================================+
    std::string_view request;
    std::string_view status;
    std::string_view body;
  };
  constexpr test_case cases[] = {
      {"POST /echo HTTP/1.1\r\nhost: a\r\ncontent-length: 3\r\n\r\nraw",
       "HTTP/1.1 200 OK", "raw"},
      {"POST /echo HTTP/1.1\r\nhOsT: a\r\n"
       "tRaNsFeR-EnCoDiNg: chunked\r\n\r\n3\r\nhex\r\n0\r\n\r\n",
       "HTTP/1.1 200 OK", "hex"},
      {"GET http://target.example/ok HTTP/1.1\r\n"
       "Host: target.example:80\r\n\r\n",
       "HTTP/1.1 200 OK", "ok"},
      {"CONNECT target.example:443 HTTP/1.1\r\n"
       "Host: target.example:443\r\n\r\n",
       "HTTP/1.1 501 Not Implemented", ""},
      {"OPTIONS * HTTP/1.1\r\nHost: a\r\n\r\n",
       "HTTP/1.1 200 OK", ""},
  };
  for (const auto& test : cases) {
    martianlabs::doba::tests::integration::test_helper::set_context(
        test.request);
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all(test.request));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    if (result.has_value()) {
      DOBA_EXPECT_EQUAL(result->status, test.status);
      DOBA_EXPECT_EQUAL(result->body, test.body);
    }
    client.close();
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] absolute authority preserves the received Host          ( test-case ) |
// +===========================================================================+
DOBA_TEST("HTTP/1.1 absolute authority preserves the received Host") {
  tcpip_client client;
  const uint16_t port = client.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  http_server.add_route("GET", "/authority", [](const request& req) {
    response res = response::ok_200();
    res.set_body(std::string(req.get_target_authority_host()) + "|" +
                 std::string(req.get_target_authority_port()) + "|" +
                 std::string(req.get_host()) + "|" +
                 std::string(req.get_host_port()) + "|" +
                 std::string(req.get_header("Host").second));
    return res;
  });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());
  // +=========================================================================+
// | [>] test_case                                                  ( struct ) |
  // +=========================================================================+
  struct test_case {
    // +=======================================================================+
    // | [>] ATTRIBUTEs                                             ( public ) |
    // +=======================================================================+
    std::string_view target;
    std::string_view host;
    std::string_view expected;
  };
  constexpr test_case cases[] = {
      {"http://a/authority", "b", "a||b||b"},
      {"http://a:8080/authority", "a:80", "a|8080|a|80|a:80"},
      {"https://a/authority", "b:443", "a||b|443|b:443"},
      {"http://a/authority", "a:80", "a||a|80|a:80"},
  };
  for (const auto& test : cases) {
    const std::string wire = "GET " + std::string(test.target) +
                             " HTTP/1.1\r\nHost: " + std::string(test.host) +
                             "\r\n\r\n";
    martianlabs::doba::tests::integration::test_helper::set_context(wire);
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all(wire));
    const auto result = receive_http_response(client);
    DOBA_EXPECT(result.has_value());
    if (result.has_value()) {
      DOBA_EXPECT_EQUAL(result->status, "HTTP/1.1 200 OK");
      DOBA_EXPECT_EQUAL(result->body, test.expected);
    }
    client.close();
  }
  http_server.stop();
}

// +===========================================================================+
// | [>] slow inputs do not starve complete clients              ( test-case ) |
// +===========================================================================+
DOBA_TEST(
    "HTTP/1.1 isolates incomplete heads and bodies from healthy clients") {
  tcpip_client slow_head;
  tcpip_client slow_body;
  tcpip_client healthy;
  const uint16_t port = healthy.find_available_port();
  DOBA_EXPECT(port != 0);
  server<> http_server;
  http_server.add_route("GET", "/ok", [](const request&) {
    response result = response::ok_200();
    result.set_body("ok");
    return result;
  });
  http_server.add_route("POST", "/echo", [](const request& req) {
    return echo_body(req);
  });
  const std::string port_text = std::to_string(port);
  http_server.start(port_text.c_str());

  DOBA_EXPECT(slow_head.connect(port));
  DOBA_EXPECT(slow_head.send_all(
      "GET /ok HTTP/1.1\r\nHost: a\r\nX-Pad:"));
  DOBA_EXPECT(slow_body.connect(port));
  DOBA_EXPECT(slow_body.send_all(
      "POST /echo HTTP/1.1\r\nHost: a\r\nContent-Length: 4\r\n\r\nab"));
  DOBA_EXPECT(healthy.connect(port));
  DOBA_EXPECT(healthy.send_all("GET /ok HTTP/1.1\r\nHost: a\r\n\r\n"));
  const auto healthy_response = receive_http_response(healthy);
  DOBA_EXPECT(healthy_response.has_value());
  if (healthy_response.has_value()) {
    DOBA_EXPECT_EQUAL(healthy_response->status, "HTTP/1.1 200 OK");
    DOBA_EXPECT_EQUAL(healthy_response->body, "ok");
  }

  DOBA_EXPECT(slow_head.send_all(" value\r\n\r\n"));
  const auto head_response = receive_http_response(slow_head);
  DOBA_EXPECT(head_response.has_value());
  if (head_response.has_value()) DOBA_EXPECT_EQUAL(head_response->body, "ok");
  DOBA_EXPECT(slow_body.send_all("cd"));
  const auto body_response = receive_http_response(slow_body);
  DOBA_EXPECT(body_response.has_value());
  if (body_response.has_value()) {
    DOBA_EXPECT_EQUAL(body_response->body, "abcd");
  }
  http_server.stop();
}
