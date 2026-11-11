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
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "protocol/processing_result.h"
#include "tcpip_client.h"
#include "test_helper.h"
#include "transport/tcp.h"

namespace {
using martianlabs::doba::protocol::processing_result;
using martianlabs::doba::tests::integration::tcpip_client;
namespace tr = martianlabs::doba::transport;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] factory_probe                                              ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct factory_probe {
  // Count factory calls and disconnection callbacks.
  std::atomic<unsigned> created{0}, disconnected{0};
  // Set application state before starting the transport.
  std::string expected;
  // Every input call must see the state supplied by the factory.
  std::atomic<bool> preserved{true};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] factory_engine                                             ( struct ) |
// +---------------------------------------------------------------------------+
// | Needs only the engine contract, without execution helpers.                |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct factory_engine {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(martianlabs::doba::protocol::send_delegate value) {
    output = std::move(value);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()>) {}
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view input) {
    if (marker != probe->expected) probe->preserved = false;
    output(input, {}, nullptr);
    return {};
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool) { return {}; }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  // Keep this state alive until the transport and its callbacks finish.
  std::shared_ptr<factory_probe> probe;
  // A value supplied by the application factory.
  std::string marker;
  // Use this callback only during an engine call.
  martianlabs::doba::protocol::send_delegate output;
};
}  // namespace

// +===========================================================================+
// | [>] Factory ownership                                       ( test-case ) |
// +---------------------------------------------------------------------------+
DOBA_TEST("TCP preserves factory state with only the current engine contract") {
  for (const std::size_t workers : {1U, 2U}) {
    for (const auto marker : {"", "application-owned"}) {
      auto probe = std::make_shared<factory_probe>();
      probe->expected = marker;
      auto factory = [probe] {
        ++probe->created;
        factory_engine result;
        result.probe = probe;
        result.marker = probe->expected;
        return result;
      };
      tcpip_client client;
      const auto port = client.find_available_port();
      DOBA_EXPECT(port != 0);
      tr::tcp<factory_engine, decltype(factory)> transport(
          {.worker_count = workers, .ip = "127.0.0.1",
           .port = std::to_string(port)}, factory);
      transport.set_on_disconnection([probe] { ++probe->disconnected; });
      transport.start();
      DOBA_EXPECT(client.connect(port));
      for (const auto& batch : {std::string("first"),
                                std::string(64 * 1024, 'b')}) {
        DOBA_EXPECT(client.send_all(batch));
        const auto received = client.receive(batch.size());
        DOBA_EXPECT(received.has_value());
        DOBA_EXPECT_EQUAL(*received, batch);
      }
      transport.stop();
      DOBA_EXPECT_EQUAL(probe->created.load(), 1U);
      DOBA_EXPECT_EQUAL(probe->disconnected.load(), 1U);
      DOBA_EXPECT(probe->preserved.load());
    }
  }
}
