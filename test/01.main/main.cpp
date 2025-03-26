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

#include "buffer.h"
#include "server/server_tcpip.h"

struct request {
  std::string content;
  static std::optional<request> deserialize(const martianlabs::doba::buffer& buf) {
    request out;
    out.content.assign((const char* const)buf.data(), buf.used());
    return out;
  };
};

struct response {
  static martianlabs::doba::buffer serialize(const response& res) {
    martianlabs::doba::buffer out;
    out.append("HTTP/1.1 200 OK\nContent-Length: 0\n\n");
    return out;
  };
};

template<typename RQty, typename RSty>
class protocol_base {
public:
	using req = RQty;
	using res = RSty;
};

class my_protocol : public protocol_base<request, response> {
public:
  static std::optional<response> process(const request&) { return response(); }
};

int
main(int argc, char* argv[]) {
  martianlabs::doba::server::server_tcpip<my_protocol> my_server;
	my_server.start("10001", 8);
	return getchar();
}