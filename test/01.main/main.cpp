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

#include "buffex.h"
#include "server/server_tcpip.h"

template<typename RQty, typename RSty>
class processor_base {
public:
  using req = RQty;
  using res = RSty;
};

class my_request {
};

class my_response {
};

class my_processor : public processor_base<my_request, my_response> {
 public:
  std::optional<my_request> decode(void* buffer, uint32_t length) {
    printf(">>>>>> %.*s", int(length), (char*)buffer);
    return {};
  }
  std::optional<my_response> decode(const my_request& req) {
    return {};
  }
};

int main(int argc, char* argv[]) {
  martianlabs::doba::server::server_tcpip<my_processor> my_server;
  my_server.start("10001", 8);

  /*
  martianlabs::doba::buffex my_buffer;
  martianlabs::doba::buffex my_copied_buffer = my_buffer;
  martianlabs::doba::buffex my_moved_buffer = std::move(my_buffer);
  */

  /*
  std::string my_str =
      "Hello World! This is just a simple, and silly, example about using "
      "buffex!";
  martianlabs::doba::buffex buf;
  buf.append(my_str.data(), my_str.size());
  constexpr std::size_t kSize = 1024;
  char bf[kSize] = {0};
  std::size_t sz = 0;
  if (!buf.read(64, bf, sz)) {
    printf(">>>>> ERROR!!!!!!\n");
  }
  if (!buf.read(64, bf, sz)) {
    printf(">>>>> ERROR!!!!!!\n");
  }

  */

  return getchar();
}