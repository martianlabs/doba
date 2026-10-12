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

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

#include "protocol/contracts.h"
#include "transport/server/contracts.h"

namespace martianlabs::doba {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] server                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | Server class that manages the lifecycle of a transport server and its     |
// | associated protocol engine.                                               |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// +---------------------------------------------------------------------------+
// | ENty: The protocol engine type.                                           |
// | TRty: The server transport type.                                          |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty, typename TRty>
class server {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  template <protocol::contracts::engine_factory<ENty> EFy>
    requires transport::server::contracts::transport<TRty, ENty, EFy>
  explicit server(typename TRty::policies_type configuration, EFy factory) {
    auto transport_arguments =
        std::make_shared<std::pair<typename TRty::policies_type, EFy>>(
            std::move(configuration), std::move(factory));
    create_transport_ = [this, args = std::move(transport_arguments)]() {
      transport_.emplace(std::move(args->first), std::move(args->second));
    };
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
  // | Starts the server and begins accepting connections.                     |
  // +-------------------------------------------------------------------------+
  void start() {
    std::lock_guard<std::mutex> lock(locked_mutex_);
    if (locked_) return;
    if (!transport_ && !create_transport_) {
      throw std::runtime_error("Transport construction previously failed");
    }
    try {
      if (on_starting_) on_starting_();
      if (!transport_) {
        auto create_transport = std::move(create_transport_);
        create_transport_ = {};
        create_transport();
      }
      transport_->set_on_connection([]() {});
      transport_->set_on_disconnection([]() {});
      transport_->start();
    } catch (...) {
      if (on_start_failed_) on_start_failed_();
      throw;
    }
    locked_ = true;
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Stops the server and closes all active connections.                     |
  // +-------------------------------------------------------------------------+
  void stop() {
    std::lock_guard<std::mutex> lock(locked_mutex_);
    if (!locked_) return;
    if constexpr (requires { transport_->quiesce(); }) {
      transport_->quiesce();
    } else {
      transport_->stop();
    }
    if (on_stopping_) on_stopping_();
    if constexpr (requires { transport_->quiesce(); }) {
      transport_->stop();
    }
    locked_ = false;
    if (on_stopped_) on_stopped_();
  }

 protected:
  // +=========================================================================+
  // | [>] set_on_starting                                       ( protected ) |
  // +-------------------------------------------------------------------------+
  void set_on_starting(std::function<void()> callback) {
    on_starting_ = std::move(callback);
  }
  // +=========================================================================+
  // | [>] set_on_start_failed                                   ( protected ) |
  // +-------------------------------------------------------------------------+
  void set_on_start_failed(std::function<void()> callback) {
    on_start_failed_ = std::move(callback);
  }
  // +=========================================================================+
  // | [>] set_on_stopping                                       ( protected ) |
  // +-------------------------------------------------------------------------+
  void set_on_stopping(std::function<void()> callback) {
    on_stopping_ = std::move(callback);
  }
  // +=========================================================================+
  // | [>] set_on_stopped                                        ( protected ) |
  // +-------------------------------------------------------------------------+
  void set_on_stopped(std::function<void()> callback) {
    on_stopped_ = std::move(callback);
  }

 private:
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  std::function<void()> on_starting_;
  std::function<void()> on_start_failed_;
  std::function<void()> on_stopping_;
  std::function<void()> on_stopped_;
  bool locked_{false};
  std::mutex locked_mutex_;
  std::function<void()> create_transport_;
  std::optional<TRty> transport_;
};
}  // namespace martianlabs::doba

#endif
