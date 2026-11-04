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
// Copyright 2026 martianLabs
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

#ifndef martianlabs_doba_tests_helpers_tool_response_h
#define martianlabs_doba_tests_helpers_tool_response_h

#include "tool_support.h"

#include <cctype>
#include <set>

namespace martianlabs::doba::tests::tools {

// +---------------------------------------------------------------------------+
// | [>] wire_response                                             ( struct ) |
// +---------------------------------------------------------------------------+
struct wire_response {
  int status{};
  std::map<std::string, std::string> headers;
  std::string body;
};

// Independent final Content-Length oracle. Read only the current response;
// never consume the first bytes of its pipelined successor.
template<class Stream>
wire_response response(Stream& stream, bool unique_headers = false) {
  size_t total = 0;
  auto line = [&] {
    std::string result;
    while (!result.ends_with("\r\n")) {
      auto data = stream.receive(1);
      require(data.size() == 1, "Truncated response head");
      result += data;
      require(++total <= 16384, "Invalid or oversized response head");
    }
    return result;
  };
  auto status = line();
  require(status.size() >= 15 && status.starts_with("HTTP/1.1 ") &&
    status[12] == ' ' && status[9] >= '0' && status[9] <= '9' &&
    status[10] >= '0' && status[10] <= '9' &&
    status[11] >= '0' && status[11] <= '9', "Invalid response status syntax");
  wire_response result;
  result.status = (status[9] - '0') * 100 + (status[10] - '0') * 10 +
    status[11] - '0';
  require(result.status >= 200 && result.status <= 599,
    "Expected a final response status");
  auto printable = [](std::string_view value) {
    for (unsigned char character : value) {
      if ((character < 32 || character == 127) && character != 9) return false;
    }
    return true;
  };
  require(printable(std::string_view(status).substr(13, status.size() - 15)),
    "Control byte in response reason");
  for (;;) {
    auto field = line();
    if (field == "\r\n") break;
    field.resize(field.size() - 2);
    auto colon = field.find(':');
    require(colon != std::string::npos && colon > 0,
      "Invalid response field name");
    auto name = field.substr(0, colon);
    for (char& character : name) {
      unsigned char byte = static_cast<unsigned char>(character);
      require((byte >= 'A' && byte <= 'Z') ||
        (byte >= 'a' && byte <= 'z') || (byte >= '0' && byte <= '9') ||
        std::string_view("!#$%&'*+-.^_`|~").find(character) !=
          std::string_view::npos, "Invalid response field name");
      if (byte >= 'A' && byte <= 'Z') character += 'a' - 'A';
    }
    auto value = field.substr(colon + 1);
    require(printable(value), "Control byte in response field");
    bool singleton = name == "content-length" || name == "date" ||
      name == "transfer-encoding";
    require(!(unique_headers || singleton) || !result.headers.contains(name),
      "Duplicate response singleton");
    result.headers[name] = trim(value);
  }
  require(result.headers.contains("content-length") &&
    !result.headers.contains("transfer-encoding"),
    "Ambiguous response framing");
  auto& length = result.headers.at("content-length");
  require(!length.empty(), "Missing Content-Length");
  size_t size = 0;
  for (char digit : length) {
    require(digit >= '0' && digit <= '9', "Invalid Content-Length");
    size = size * 10 + static_cast<unsigned>(digit - '0');
    require(size <= 1024 * 1024, "Oversized response body");
  }
  while (result.body.size() < size) {
    auto data = stream.receive(size - result.body.size());
    require(!data.empty(), "Truncated response body");
    result.body += data;
  }
  return result;
}

inline std::string request(const std::string& path,
                           const std::string& extra = "") {
  return "GET " + path + " HTTP/1.1\r\nHost: localhost\r\n" + extra + "\r\n";
}

template<class Stream>
void expect_response(Stream& stream, int code, const std::string& body,
                     bool unique_headers = false) {
  auto value = response(stream, unique_headers);
  require(value.status == code && value.body == body,
    "HTTP status/body mismatch: " + std::to_string(value.status) +
    ", body bytes=" + std::to_string(value.body.size()));
}

} // namespace martianlabs::doba::tests::tools

#endif
