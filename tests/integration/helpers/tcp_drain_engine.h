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

#ifndef martianlabs_doba_tests_integration_helpers_tcp_drain_engine_h
#define martianlabs_doba_tests_integration_helpers_tcp_drain_engine_h

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include "protocol/contracts.h"

namespace martianlabs::doba::tests::integration {
// +---------------------------------------------------------------------------+
// | [>] general                                                    ( usings ) |
// +---------------------------------------------------------------------------+
using martianlabs::doba::protocol::send_delegate;
using namespace std::chrono_literals;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] drain_state                                                ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct drain_state {
  std::atomic<int> entered{0};
  std::atomic<int> queued{0};
  std::atomic<int> destroyed{0};
  std::atomic<bool> release{true};
};

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] drain_engine                                               ( struct ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
struct drain_engine {
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = int;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit drain_engine(std::shared_ptr<drain_state> state)
      : state_(std::move(state)) {}
  drain_engine(const drain_engine&) = delete;
  drain_engine(drain_engine&&) = delete;
  ~drain_engine() { state_->destroyed++; }
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(send_delegate output) { output_ = std::move(output); }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()>) {}
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  void on_bytes_received(std::string_view input) {
    const auto* bytes = input.data();
    state_->entered++;
    const auto deadline = std::chrono::steady_clock::now() + 5s;
    while (!state_->release.load()) {
      if (std::chrono::steady_clock::now() >= deadline) {
        throw std::runtime_error("Test callback was not released");
      }
      std::this_thread::yield();
    }
    for (int i = 0; i < 8; i++) {
      output_(std::string(1024 * 1024, bytes[0]), {}, nullptr);
    }
    state_->queued++;
    return;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                               ( public ) |
  // +-------------------------------------------------------------------------+
  std::shared_ptr<drain_state> state_;
  send_delegate output_;
};

}  // namespace martianlabs::doba::tests::integration

#endif
