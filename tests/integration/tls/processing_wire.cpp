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

#include "../../helpers/tool_response.h"
#include "fixture_process.h"

using namespace martianlabs::doba::tests::tools;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] main                                                  ( entry-point ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
  return guarded([&] {
    require(argc == 4, "Expected TLS fixture, certificate and key");
    for (int version : {TLS1_2_VERSION, TLS1_3_VERSION}) {
      {
        fixture_process server(argv[1], argv[2], argv[3], "http-async", 1048576,
                               127);
        {
          peer connection(server.port());
          connection.tls(argv[2], version, "http/1.1");
          connection.send(request("/gated") + request("/wait/2") +
                          request("/source") + request("/release"));
          for (auto expected : std::vector<std::string>{
                   "gated", "2", std::string(32768, 's'), "release"}) {
            expect_response(connection, 200, expected);
          }
          connection.send(request("/state"));
          expect_response(connection, 200, "2:2:0");
          connection.send(request("/wait/3") +
                          request("/source", "Connection: close\r\n") +
                          request("/hello"));
          expect_response(connection, 200, "3");
          expect_response(connection, 200, std::string(32768, 's'));
          require(connection.receive(1).empty(), "Ordered close lost");
        }
        {
          peer connection(server.port());
          connection.tls(argv[2], version, "http/1.1");
          connection.send(request("/hello") + request("/fail"));
          expect_response(connection, 200, "hello");
          require(response(connection).status == 500, "Missing async failure");
        }
        peer slow(server.port());
        slow.tls(argv[2], version, "http/1.1");
        slow.send(request("/sleep"));
        peer fast(server.port());
        fast.tls(argv[2], version, "http/1.1");
        auto state_until = [&](const std::string& suffix) {
          auto deadline = clock::now() + 5s;
          for (;;) {
            fast.send(request("/state"));
            auto state = response(fast);
            require(state.status == 200, "Invalid async state response");
            if (state.body.ends_with(suffix)) break;
            require(clock::now() < deadline, "Async state did not progress");
          }
        };
        state_until(":1");
        fast.send(request("/hello"));
        expect_response(fast, 200, "hello");
        slow.close();
        state_until(":0");
        server.stop();
      }
      {
        fixture_process server(argv[1], argv[2], argv[3], "http-async");
        peer slow(server.port());
        slow.tls(argv[2], version, "http/1.1");
        slow.send(request("/sleep"));
        {
          peer fast(server.port());
          fast.tls(argv[2], version, "http/1.1");
          auto deadline = clock::now() + 5s;
          for (;;) {
            fast.send(request("/state"));
            if (response(fast).body.ends_with(":1")) break;
            require(clock::now() < deadline, "Sleeping frame not observed");
          }
        }
        server.stop();
        require(server.child().output() == "READY\nLIVE 0\n",
                "Stop retained live async frames");
        require(slow.receive(1).empty(), "Stop retained TLS connection");
      }
    }
    std::cout << "TLS ordered mixed/source, failure, cancel and stop: PASS\n";
  });
}
