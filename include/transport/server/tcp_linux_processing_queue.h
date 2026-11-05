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

#ifndef martianlabs_doba_transport_server_tcp_linux_processing_queue_h
#define martianlabs_doba_transport_server_tcp_linux_processing_queue_h

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>

#include "protocol/processing_result.h"

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] linux_processing_queue                                      ( class ) |
// +---------------------------------------------------------------------------+
// | Worker-owned registrations; only stable connection identities are stored. |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class linux_processing_queue {
 public:
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using clock = protocol::processing_result::clock;
  using identity_type = std::uint64_t;
  // +=========================================================================+
  // | [>] reconcile                                                ( public ) |
  // +-------------------------------------------------------------------------+
  void reconcile(identity_type identity, protocol::processing_result result,
                 bool output_idle, clock::time_point now = clock::now()) {
    const bool due = result.turn_at && *result.turn_at <= now;
    const bool ready = result.turn_requested || due ||
        (result.turn_after_output_drained && output_idle);
    if (!ready && !result.turn_at) {
      cancel(identity);
      return;
    }
    auto& entry = registrations_[identity];
    if (entry.deadline && (due || !result.turn_at ||
        (*entry.deadline)->first != *result.turn_at)) {
      deadlines_.erase(*entry.deadline);
      entry.deadline.reset();
    }
    if (result.turn_at && !due && !entry.deadline) {
      entry.deadline = deadlines_.emplace(*result.turn_at, identity);
    }
    if (ready && !entry.ready) {
      ready_.push_back(identity);
      entry.ready = std::prev(ready_.end());
    } else if (!ready && entry.ready) {
      ready_.erase(*entry.ready);
      entry.ready.reset();
    }
  }
  // +=========================================================================+
  // | [>] promote                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  void promote(clock::time_point now, std::size_t limit = 64) {
    while (limit && !deadlines_.empty() && deadlines_.begin()->first <= now) {
      const auto timer = deadlines_.begin();
      auto& entry = registrations_.at(timer->second);
      if (!entry.ready) {
        ready_.push_back(timer->second);
        entry.ready = std::prev(ready_.end());
      }
      entry.deadline.reset();
      deadlines_.erase(timer);
      --limit;
    }
  }
  // +=========================================================================+
  // | [>] take_ready                                               ( public ) |
  // +-------------------------------------------------------------------------+
  std::optional<identity_type> take_ready() noexcept {
    if (ready_.empty()) return {};
    const auto identity = ready_.front();
    auto found = registrations_.find(identity);
    ready_.pop_front();
    found->second.ready.reset();
    if (!found->second.deadline) registrations_.erase(found);
    return identity;
  }
  // +=========================================================================+
  // | [>] ready_count                                              ( public ) |
  // +-------------------------------------------------------------------------+
  std::size_t ready_count() const noexcept { return ready_.size(); }
  // +=========================================================================+
  // | [>] next_deadline                                            ( public ) |
  // +-------------------------------------------------------------------------+
  std::optional<clock::time_point> next_deadline() const noexcept {
    if (deadlines_.empty()) return {};
    return deadlines_.begin()->first;
  }
  // +=========================================================================+
  // | [>] cancel                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  void cancel(identity_type identity) noexcept {
    auto found = registrations_.find(identity);
    if (found == registrations_.end()) return;
    if (found->second.ready) ready_.erase(*found->second.ready);
    if (found->second.deadline) deadlines_.erase(*found->second.deadline);
    registrations_.erase(found);
  }
  // +=========================================================================+
  // | [>] clear                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  void clear() noexcept {
    registrations_.clear();
    ready_.clear();
    deadlines_.clear();
  }

 private:
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  using ready_type = std::list<identity_type>;
  using deadline_type = std::multimap<clock::time_point, identity_type>;
  struct registration {
    // Present only while this identity is in the coalesced ready list.
    std::optional<ready_type::iterator> ready;
    // Its one cancellable connection deadline, independent of readiness.
    std::optional<deadline_type::iterator> deadline;
  };
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  ready_type ready_;         // Coalesced, fair connection turns.
  deadline_type deadlines_;  // One deadline registration per connection.
  std::unordered_map<identity_type, registration> registrations_;
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
