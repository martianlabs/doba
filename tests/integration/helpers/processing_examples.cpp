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
    require(argc == 4, "Expected task, memory and network example executables");
    for (int index = 1; index <= 2; ++index) {
      auto child = run({argv[index]}, 10s);
      clean_diagnostics(child.errors);
      require(child.code == 0 && trim(child.output) == (index == 1 ?
        "Completed steps: 3" :
        "Concurrent handlers: 3; ordered output: 1 2 3"),
        "Processing example output/exit mismatch");
    }
    auto port = peer::available_port();
    process child({argv[3], std::to_string(port)});
    std::unique_ptr<peer> connection;
    auto deadline = clock::now() + 10s;
    while (!connection) {
      require(!child.poll(), "Network example exited: " + child.errors());
      try { connection = std::make_unique<peer>(port); }
      catch (const std::exception&) {
        require(clock::now() < deadline, "Network example startup timeout");
        std::this_thread::sleep_for(20ms);
      }
    }
    connection->send(request("/wait/1") + request("/sync") +
      request("/wait/2"));
    for (auto expected : {"1", "sync", "2"}) {
      auto value = response(*connection, true);
      require(value.status == 200 && value.body == expected,
        "Network example response mismatch");
    }
    connection.reset();
    child.send("\n");
    require(child.wait(5s) == 0, "Network example failed to stop");
    clean_diagnostics(child.errors());
    std::cout << "Executable processing examples: PASS\n";
  });
}
