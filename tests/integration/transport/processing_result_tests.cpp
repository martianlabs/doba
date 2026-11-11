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

#include <atomic>
#include <chrono>
#include <memory>
#include <string>

#include "processing_probe_engine.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "transport/tcp.h"

namespace {
using martianlabs::doba::protocol::processing_result;
using martianlabs::doba::tests::integration::processing_probe_engine;
using martianlabs::doba::tests::integration::tcpip_client;
namespace tr = martianlabs::doba::transport;
using namespace std::chrono_literals;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] exercise                                                 ( function ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
void exercise() {
  for (unsigned condition : {1u, 2u, 4u, 7u}) {
    auto turns = std::make_shared<std::atomic<unsigned>>(0);
    auto factory = [condition, turns] {
      processing_probe_engine engine;
      engine.progress.turn_requested = (condition & 1) != 0;
      if (condition & 2) {
        engine.progress.turn_at = processing_result::clock::time_point::max();
      }
      engine.progress.turn_after_output_drained = (condition & 4) != 0;
      engine.close_after_output = true;
      engine.turns = turns;
      return engine;
    };
    tcpip_client client;
    const auto port = client.find_available_port();
    DOBA_EXPECT(port != 0);
    tr::tcp<processing_probe_engine, decltype(factory)> transport(
        {.worker_count = 1, .ip = "127.0.0.1", .port = std::to_string(port)},
        factory);
    transport.set_on_connection([] {});
    transport.set_on_disconnection([] {});
    transport.start();
    DOBA_EXPECT(client.connect(port));
    DOBA_EXPECT(client.send_all("x"));
    const auto wire = client.receive(2, 5s);
    DOBA_EXPECT(wire && *wire == "ok");
    DOBA_EXPECT(client.wait_for_close(5s));
    client.close();
    transport.stop();
    DOBA_EXPECT_EQUAL(turns->load(), 0u);
  }
}
}  // namespace

// +===========================================================================+
// | [>] Close overrides scheduling                              ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("native close drains accepted output despite processing requests") {
  exercise();
}
