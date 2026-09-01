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

#include <string_view>

#include "protocol/http/v11/status_lines.h"
#include "test_helper.h"

namespace {
using martianlabs::doba::protocol::http::v11::status_lines;
}  // namespace

// +===========================================================================+
// | [>] literals contain complete HTTP 1.1 status lines         ( test-case ) |
// +===========================================================================+
DOBA_TEST("literals contain complete HTTP 1.1 status lines") {
  struct test_case {
    std::string_view actual;
    std::size_t size;
    std::string_view expected;
  };
  constexpr test_case cases[] = {
      {status_lines::k100, status_lines::k100Sz, "HTTP/1.1 100 Continue\r\n"},
      {status_lines::k101, status_lines::k101Sz,
       "HTTP/1.1 101 Switching Protocols\r\n"},
      {status_lines::k200, status_lines::k200Sz, "HTTP/1.1 200 OK\r\n"},
      {status_lines::k201, status_lines::k201Sz, "HTTP/1.1 201 Created\r\n"},
      {status_lines::k202, status_lines::k202Sz, "HTTP/1.1 202 Accepted\r\n"},
      {status_lines::k203, status_lines::k203Sz,
       "HTTP/1.1 203 Non-Authoritative Information\r\n"},
      {status_lines::k204, status_lines::k204Sz, "HTTP/1.1 204 No Content\r\n"},
      {status_lines::k205, status_lines::k205Sz,
       "HTTP/1.1 205 Reset Content\r\n"},
      {status_lines::k206, status_lines::k206Sz,
       "HTTP/1.1 206 Partial Content\r\n"},
      {status_lines::k300, status_lines::k300Sz,
       "HTTP/1.1 300 Multiple Choices\r\n"},
      {status_lines::k301, status_lines::k301Sz,
       "HTTP/1.1 301 Moved Permanently\r\n"},
      {status_lines::k302, status_lines::k302Sz, "HTTP/1.1 302 Found\r\n"},
      {status_lines::k303, status_lines::k303Sz, "HTTP/1.1 303 See Other\r\n"},
      {status_lines::k304, status_lines::k304Sz,
       "HTTP/1.1 304 Not Modified\r\n"},
      {status_lines::k305, status_lines::k305Sz, "HTTP/1.1 305 Use Proxy\r\n"},
      {status_lines::k306, status_lines::k306Sz, "HTTP/1.1 306 Unused\r\n"},
      {status_lines::k307, status_lines::k307Sz,
       "HTTP/1.1 307 Temporary Redirect\r\n"},
      {status_lines::k308, status_lines::k308Sz,
       "HTTP/1.1 308 Permanent Redirect\r\n"},
      {status_lines::k400, status_lines::k400Sz,
       "HTTP/1.1 400 Bad Request\r\n"},
      {status_lines::k401, status_lines::k401Sz,
       "HTTP/1.1 401 Unauthorized\r\n"},
      {status_lines::k402, status_lines::k402Sz,
       "HTTP/1.1 402 Payment Required\r\n"},
      {status_lines::k403, status_lines::k403Sz, "HTTP/1.1 403 Forbidden\r\n"},
      {status_lines::k404, status_lines::k404Sz, "HTTP/1.1 404 Not Found\r\n"},
      {status_lines::k405, status_lines::k405Sz,
       "HTTP/1.1 405 Method Not Allowed\r\n"},
      {status_lines::k406, status_lines::k406Sz,
       "HTTP/1.1 406 Not Acceptable\r\n"},
      {status_lines::k407, status_lines::k407Sz,
       "HTTP/1.1 407 Proxy Authentication Required\r\n"},
      {status_lines::k408, status_lines::k408Sz,
       "HTTP/1.1 408 Request Timeout\r\n"},
      {status_lines::k409, status_lines::k409Sz, "HTTP/1.1 409 Conflict\r\n"},
      {status_lines::k410, status_lines::k410Sz, "HTTP/1.1 410 Gone\r\n"},
      {status_lines::k411, status_lines::k411Sz,
       "HTTP/1.1 411 Length Required\r\n"},
      {status_lines::k412, status_lines::k412Sz,
       "HTTP/1.1 412 Precondition Failed\r\n"},
      {status_lines::k413, status_lines::k413Sz,
       "HTTP/1.1 413 Content Too Large\r\n"},
      {status_lines::k414, status_lines::k414Sz,
       "HTTP/1.1 414 URI Too Long\r\n"},
      {status_lines::k415, status_lines::k415Sz,
       "HTTP/1.1 415 Unsupported Media Type\r\n"},
      {status_lines::k416, status_lines::k416Sz,
       "HTTP/1.1 416 Range Not Satisfiable\r\n"},
      {status_lines::k417, status_lines::k417Sz,
       "HTTP/1.1 417 Expectation Failed\r\n"},
      {status_lines::k418, status_lines::k418Sz,
       "HTTP/1.1 418 Im a teapot\r\n"},
      {status_lines::k421, status_lines::k421Sz,
       "HTTP/1.1 421 Misdirected Request\r\n"},
      {status_lines::k422, status_lines::k422Sz,
       "HTTP/1.1 422 Unprocessable Content\r\n"},
      {status_lines::k426, status_lines::k426Sz,
       "HTTP/1.1 426 Upgrade Required\r\n"},
      {status_lines::k431, status_lines::k431Sz,
       "HTTP/1.1 431 Request Header Fields Too Large\r\n"},
      {status_lines::k500, status_lines::k500Sz,
       "HTTP/1.1 500 Internal Server Error\r\n"},
      {status_lines::k501, status_lines::k501Sz,
       "HTTP/1.1 501 Not Implemented\r\n"},
      {status_lines::k502, status_lines::k502Sz,
       "HTTP/1.1 502 Bad Gateway\r\n"},
      {status_lines::k503, status_lines::k503Sz,
       "HTTP/1.1 503 Service Unavailable\r\n"},
      {status_lines::k504, status_lines::k504Sz,
       "HTTP/1.1 504 Gateway Timeout\r\n"},
      {status_lines::k505, status_lines::k505Sz,
       "HTTP/1.1 505 HTTP Version Not Supported\r\n"},
  };
  for (const auto& test : cases) {
    DOBA_EXPECT_EQUAL(test.actual, test.expected);
    DOBA_EXPECT_EQUAL(test.size, test.actual.size());
  }
}
