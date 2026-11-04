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

#include "../../helpers/tool_peer.h"
#include "../../helpers/tool_process.h"
#include "../../helpers/tool_response.h"

using namespace martianlabs::doba::tests::tools;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
  return guarded([&] {
    require(argc == 5, "Expected HTTPS/echo executables, certificate and key");
    for (int index = 1; index <= 2; ++index) {
      auto port = peer::available_port();
      process child({argv[index], argv[3], argv[4], std::to_string(port)},
        {}, {}, true);
      auto deadline = clock::now() + 10s;
      std::unique_ptr<peer> connection;
      while (!connection) {
        require(!child.poll(), "TLS example exited: " + child.errors());
        try { connection = std::make_unique<peer>(port); }
        catch (const std::exception&) {
          require(clock::now() < deadline, "TLS example startup timeout");
          std::this_thread::sleep_for(20ms);
        }
      }
      connection->tls(argv[3], 0, index == 1 ? "http/1.1" : "");
      if (index == 1) {
        require(connection->alpn() == "http/1.1", "HTTPS ALPN mismatch");
        connection->send(request("/hello"));
        expect_response(*connection, 200, "hello over TLS");
        connection->send(request("/wait/7"));
        expect_response(*connection, 200, "7");
      } else {
        connection->send("hel"); connection->send("lo\n");
        connection->expect("echo: hello\n");
        connection->send(std::string(4097, 'x') + '\n');
        require(connection->receive(1).empty(), "Echo limit was ignored");
      }
      connection.reset();
      require(!child.poll(), "TLS example terminated before shutdown");
      child.terminate();
      child.wait(5s);
      clean_diagnostics(child.errors());
    }
    std::cout << "HTTPS and custom TLS executable examples: PASS\n";
  });
}
