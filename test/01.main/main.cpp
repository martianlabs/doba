//      _       _           
//   __| | ___ | |__   __ _ 
//  / _` |/ _ \| '_ \ / _` |
// | (_| | (_) | |_) | (_| |
//  \__,_|\___/|_.__/ \__,_|
// 
// Copyright 2025 martianLabs
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "string/utils.h"
#include "server/transport/tcpip.h"

using namespace martianlabs::doba;

template <typename RQty, typename RSty>
class protocol_base {
 public:
  using req = RQty;
  using res = RSty;
};

class test_request {
  static constexpr uint32_t kMaxHeaderLength = 1024;
  static constexpr char kEndOfHeaders[] = "\r\n\r\n";
  static constexpr uint32_t kEndOfHeadersLen = sizeof(kEndOfHeaders) - 1;
  char buffer_[kMaxHeaderLength] = {0};
  uint32_t buffer_cur_ = 0;
  std::optional<uint32_t> end_of_headers_;

 public:
  void reset() {
    buffer_cur_ = 0;
    end_of_headers_.reset();
  }
  protocol::result deserialize(void* buffer, uint32_t length) {
    if ((kMaxHeaderLength - buffer_cur_) >= length) {
      memcpy(&buffer_[buffer_cur_], buffer, length);
      buffer_cur_ += length;
    } else {
      return protocol::result::kError;
    }
    end_of_headers_ = string::utils::find_exact_match(
        buffer_, buffer_cur_, kEndOfHeaders, kEndOfHeadersLen);
    if (!end_of_headers_.has_value()) {
      return protocol::result::kMoreBytesNeeded;
    }
    
    // printf("%.*s", int(buffer_cur_), buffer_);
    
    return protocol::result::kCompleted;
  }
};

class test_response {
 public:
  void reset() {}
  std::shared_ptr<std::istream> serialize() {
    return std::make_shared<std::istringstream>(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "Hello, World!");
  }
};

class test_protocol : public protocol_base<test_request, test_response> {
 public:
  protocol::result process(const test_request& req, test_response& res) {
    return protocol::result::kCompleted;
  }
};

int main(int argc, char* argv[]) {
  server::transport::tcpip<test_protocol> test_server;
  test_server.start("10001", 8);
  return getchar();
}