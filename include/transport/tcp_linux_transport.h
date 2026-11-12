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

#ifndef martianlabs_doba_transport_tcp_linux_transport_h
#define martianlabs_doba_transport_tcp_linux_transport_h

#include <arpa/inet.h>

#include <charconv>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "transport/tcp_linux_worker.h"

namespace martianlabs::doba::transport::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] linux_transport                                             ( class ) |
// +---------------------------------------------------------------------------+
// | Each worker owns its listener and connections.                            |
// | One external owner handles setup/start/stop, outside worker callbacks.    |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   ENty - engine type                                                      |
// |   FAty - engine factory type                                              |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty,
          protocol::contracts::engine_factory<ENty> FAty>
class linux_transport {
 public:
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = tcp_policies;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  linux_transport(tcp_policies configuration, FAty factory)
      : configuration_(std::move(configuration)), factory_(std::move(factory)) {
    if (!configuration_.receive_buffer_capacity ||
        configuration_.receive_buffer_capacity >
            static_cast<std::size_t>(std::numeric_limits<ssize_t>::max()) ||
        !configuration_.max_buffered_send_bytes ||
        configuration_.max_buffered_send_bytes >
            static_cast<std::size_t>(std::numeric_limits<ssize_t>::max()) ||
        configuration_.listen_backlog < 0) {
      throw std::runtime_error("Invalid Linux transport buffer or backlog");
    }
    if (configuration_.max_connections &&
        configuration_.worker_count > configuration_.max_connections) {
      throw std::runtime_error("Each Linux worker needs admission capacity");
    }
  }
  linux_transport(const linux_transport&) = delete;
  ~linux_transport() { stop(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  linux_transport& operator=(const linux_transport&) = delete;
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Starts one worker when worker_count is 0.                               |
  // | If startup fails, workers already started are stopped.                  |
  // +-------------------------------------------------------------------------+
  void start() {
    if (!workers_.empty()) return;
    in_addr ip{};
    unsigned int port = 0;
    const std::string_view text(configuration_.port);
    const auto parsed =
        std::from_chars(text.data(), text.data() + text.size(), port);
    if (configuration_.ip.find('\0') != std::string::npos ||
        ::inet_pton(AF_INET, configuration_.ip.c_str(), &ip) != 1 ||
        parsed.ec != std::errc() || parsed.ptr != text.data() + text.size() ||
        port == 0 || port > 65535) {
      throw std::runtime_error("Invalid Linux transport IPv4 address or port");
    }
    const auto count =
        configuration_.worker_count ? configuration_.worker_count : 1;
    std::vector<std::unique_ptr<linux_worker<ENty, FAty>>> workers;
    workers.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
      auto policies = configuration_;
      if (policies.max_connections) {
        policies.max_connections =
            configuration_.max_connections / count +
            (index < configuration_.max_connections % count ? 1 : 0);
      }
      auto worker = std::make_unique<linux_worker<ENty, FAty>>(
          std::move(policies), factory_, connected_, disconnected_);
      worker->start(ip, static_cast<std::uint16_t>(port));
      workers.push_back(std::move(worker));
    }
    workers_ = std::move(workers);
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Stops and joins workers, cancelling pending output.                     |
  // | Call outside worker threads.                                            |
  // +-------------------------------------------------------------------------+
  void stop() {
    for (const auto& worker : workers_) {
      if (worker->is_current_thread()) {
        throw std::runtime_error("Stop must be called outside Linux workers");
      }
    }
    workers_.clear();
  }
  // +=========================================================================+
  // | [>] set_on_connection                                        ( public ) |
  // +-------------------------------------------------------------------------+
  // | Template parameters:                                                    |
  // |   FNty - callback type                                                  |
  // +-------------------------------------------------------------------------+
  // | Set while stopped; cannot change while workers are running.             |
  // +-------------------------------------------------------------------------+
  template <typename FNty>
  void set_on_connection(FNty&& callback) {
    ensure_stopped();
    connected_ = std::forward<FNty>(callback);
  }
  // +=========================================================================+
  // | [>] set_on_disconnection                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Template parameters:                                                    |
  // |   FNty - callback type                                                  |
  // +-------------------------------------------------------------------------+
  // | Set while stopped; cannot change while workers are running.             |
  // +-------------------------------------------------------------------------+
  template <typename FNty>
  void set_on_disconnection(FNty&& callback) {
    ensure_stopped();
    disconnected_ = std::forward<FNty>(callback);
  }

 private:
  // +=========================================================================+
  // | [>] ensure_stopped                                          ( private ) |
  // +-------------------------------------------------------------------------+
  void ensure_stopped() const {
    if (!workers_.empty()) {
      throw std::runtime_error("Transport callbacks are fixed");
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  const tcp_policies configuration_;
  // Creates a separate engine for each connection.
  const FAty factory_;
  std::function<void()> connected_;
  std::function<void()> disconnected_;
  // Workers are joined before callbacks are destroyed.
  std::vector<std::unique_ptr<linux_worker<ENty, FAty>>> workers_;
};
}  // namespace martianlabs::doba::transport::detail

#endif
