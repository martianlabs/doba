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

#include <fstream>

#include "string/utils.h"
#include "server.h"
#include "transport/server/tcpip.h"

using namespace martianlabs::doba;

class test_request {
  static constexpr uint32_t kMaxRequestLength = 4096;
  static constexpr char kEndOfHeaders[] = "\r\n\r\n";
  static constexpr uint32_t kEndOfHeadersLen = sizeof(kEndOfHeaders) - 1;
  char* buffer_ = nullptr;
  uint32_t buffer_cur_ = 0;
  std::optional<uint32_t> end_of_headers_;

 public:
  test_request() { buffer_ = new char[kMaxRequestLength]; }
  ~test_request() { delete[] buffer_; }
  void reset() {
    buffer_cur_ = 0;
    end_of_headers_.reset();
  }
  protocol::result deserialize(void* buffer, uint32_t length) {
    if ((kMaxRequestLength - buffer_cur_) < length) {
      return protocol::result::kError;
    }
    memcpy(&buffer_[buffer_cur_], buffer, length);
    buffer_cur_ += length;
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
    return std::make_shared<std::stringstream>(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "Hello, World!");
  }
};

int main(int argc, char* argv[]) {
  server<transport::server::tcpip, test_request, test_response> test_server;
  test_server.set_buffer_size(1024);
  test_server.set_port("10001");
  test_server.set_number_of_workers(8);
  test_server.set_on_connection([](socket_type id) {});
  test_server.set_on_disconnection([](socket_type id) {});
  test_server.set_on_bytes_received([](socket_type id, unsigned long bytes) {});
  test_server.set_on_bytes_sent([](socket_type id, unsigned long bytes) {});
  test_server.start();
  return getchar();
}


/*
class my_class {
 public:
  my_class(const int&) {}
};
class my_builder : public builder::base_builder<my_builder, my_class> {
 public:
  std::shared_ptr<my_class> build() {
    return std::make_shared<my_class>(number_of_connections());
  }
  builder::property<my_builder, int> number_of_connections{this};
};

int main(int argc, char* argv[]) {
  my_builder builder;
  auto pepe = builder.number_of_connections(12).build();
  return 0;
}
*/