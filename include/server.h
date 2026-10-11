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

#ifndef martianlabs_doba_server_h
#define martianlabs_doba_server_h

#include <mutex>
#include <utility>

#include "protocol/contracts.h"
#include "transport/server/contracts.h"

namespace martianlabs::doba {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] server                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | Composes a protocol engine and server transport.                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty, typename TRty>
class server {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  template <protocol::contracts::engine_factory<ENty> FNty>
    requires transport::server::contracts::transport<TRty, ENty, FNty>
  explicit server(typename TRty::policies_type configuration,
                  FNty create_engine)
      : transport_{std::move(configuration), std::move(create_engine)} {
    transport_.set_on_connection([]() {});
    transport_.set_on_disconnection([]() {});
  }
  server(const server&) = delete;
  server(server&&) noexcept = delete;
  ~server() { stop(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  server& operator=(const server&) = delete;
  server& operator=(server&&) noexcept = delete;
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void start() { start([]() {}, []() {}); }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  void stop() { stop([]() {}, []() {}); }

 protected:
  // +=========================================================================+
  // | [>] start                                                 ( protected ) |
  // +-------------------------------------------------------------------------+
  template <typename BFty, typename FFty>
  void start(BFty before_start, FFty on_failure) {
    std::lock_guard<std::mutex> lock(locked_mutex_);
    if (locked_) return;
    before_start();
    try {
      transport_.start();
    } catch (...) {
      on_failure();
      throw;
    }
    locked_ = true;
  }
  // +=========================================================================+
  // | [>] stop                                                  ( protected ) |
  // +-------------------------------------------------------------------------+
  template <typename WFty, typename AFty>
  void stop(WFty wait, AFty after_stop) {
    std::lock_guard<std::mutex> lock(locked_mutex_);
    if (!locked_) return;
    if constexpr (requires { transport_.quiesce(); }) {
      transport_.quiesce();
    } else {
      transport_.stop();
    }
    wait();
    if constexpr (requires { transport_.quiesce(); }) {
      transport_.stop();
    }
    after_stop();
    locked_ = false;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                            ( protected ) |
  // +-------------------------------------------------------------------------+
  bool locked_{false};
  std::mutex locked_mutex_;

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  TRty transport_;
};
}  // namespace martianlabs::doba

#endif
