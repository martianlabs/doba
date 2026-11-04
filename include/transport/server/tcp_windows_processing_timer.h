//                              _       _
//                           __| | ___ | |__   __ _
//                          / _` |/ _ \| '_ \ / _` |
//                         | (_| | (_) | |_) | (_| |
//                          \__,_|\___/|_.__/ \__,_|
//
//                              Apache License
//                        Version 2.0, January 2004
//                     http://www.apache.org/licenses/
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

#ifndef martianlabs_doba_transport_server_tcp_windows_processing_timer_h
#define martianlabs_doba_transport_server_tcp_windows_processing_timer_h

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <utility>

#include "protocol/contracts.h"
#include "protocol/processing_result.h"
#include "transport/server/tcp_windows_overlapped_base.h"

namespace martianlabs::doba::transport::server::detail {
template <protocol::contracts::engine ENty>
struct context;

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] windows_processing_timer                                    ( class ) |
// +---------------------------------------------------------------------------+
// | One lazy thread-pool timer for an IOCP worker group. Its callback only    |
// | publishes a generic wake packet. IOCP workers select due connections.     |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <protocol::contracts::engine ENty>
class windows_processing_timer {
 public:
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using clock = protocol::processing_result::clock;
  using entry = std::pair<std::shared_ptr<context<ENty>>, std::uint64_t>;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  windows_processing_timer() = default;
  windows_processing_timer(const windows_processing_timer&) = delete;
  windows_processing_timer& operator=(const windows_processing_timer&) = delete;
  ~windows_processing_timer() { stop(); reset(); }
  // +=========================================================================+
  // | [>] start                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // Administrative calls are sequential and surround the IOCP worker lifetime.
  void start(HANDLE port) {
    std::lock_guard<std::mutex> lock(mutex_);
    port_ = port;
    stopped_ = false;
  }
  // +=========================================================================+
  // | [>] replace                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  bool replace(const std::shared_ptr<context<ENty>>& owner,
               clock::time_point due, std::uint64_t version) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (stopped_) return false;
    if (!timer_) {
      timer_ = CreateThreadpoolTimer(&notify, this, nullptr);
      if (!timer_) return false;
    }
    auto old = positions_.find(owner.get());
    if (old != positions_.end()) {
      deadlines_.erase(old->second);
      positions_.erase(old);
    }
    auto added = deadlines_.emplace(due, entry{owner, version});
    try { positions_.emplace(owner.get(), added); }
    catch (...) { deadlines_.erase(added); throw; }
    arm_();
    return true;
  }
  // +=========================================================================+
  // | [>] cancel                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  void cancel(context<ENty>* owner) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto found = positions_.find(owner);
    if (found == positions_.end()) return;
    deadlines_.erase(found->second);
    positions_.erase(found);
    arm_();
  }
  // +=========================================================================+
  // | [>] take_due                                                 ( public ) |
  // +-------------------------------------------------------------------------+
  // Release the timer lock before acquiring contexts or calling engines.
  std::array<entry, 64> take_due() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::array<entry, 64> ready{};
    std::size_t count = 0;
    posted_ = false;
    const auto now = clock::now();
    while (!stopped_ && !deadlines_.empty() && count < ready.size() &&
           deadlines_.begin()->first <= now) {
      auto first = deadlines_.begin();
      positions_.erase(first->second.first.get());
      ready[count++] = std::move(first->second);
      deadlines_.erase(first);
    }
    arm_();
    return ready;
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // Disable producers before draining IOCP; callbacks never acquire contexts.
  void stop() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stopped_ = true;
      if (timer_) SetThreadpoolTimer(timer_, nullptr, 0, 0);
    }
    if (timer_) WaitForThreadpoolTimerCallbacks(timer_, TRUE);
    std::lock_guard<std::mutex> lock(mutex_);
    positions_.clear();
    deadlines_.clear();
    armed_.reset();
  }
  // +=========================================================================+
  // | [>] reset                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // Called only after IOCP workers have joined and all wake packets drained.
  void reset() {
    if (timer_) CloseThreadpoolTimer(timer_);
    timer_ = nullptr;
    port_ = nullptr;
    posted_ = false;
  }

 private:
  // +=========================================================================+
  // | [>] notify                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  static void CALLBACK notify(PTP_CALLBACK_INSTANCE, void* state, PTP_TIMER) {
    auto& self = *static_cast<windows_processing_timer*>(state);
    std::lock_guard<std::mutex> lock(self.mutex_);
    self.armed_.reset();
    if (self.stopped_ || self.posted_ || self.deadlines_.empty()) return;
    // Publish all packet state first. Dequeue may precede API return; the
    // consumer takes this same lock and the embedded packet is never deleted.
    self.posted_ = true;
    if (!PostQueuedCompletionStatus(self.port_, 0, 0, &self.wake_)) {
      self.posted_ = false;
      // Failed publication transfers no ownership. Keep registrations and
      // retry through the one native timer; stop can always cancel the retry.
      self.arm_();
    }
  }
  // +=========================================================================+
  // | [>] arm_                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  void arm_() {
    if (!timer_ || stopped_ || posted_) return;
    if (deadlines_.empty()) {
      if (armed_) SetThreadpoolTimer(timer_, nullptr, 0, 0);
      armed_.reset();
      return;
    }
    const auto due = deadlines_.begin()->first;
    if (armed_ && *armed_ == due) return;
    const auto now = clock::now();
    // Keep bounded due batches on IOCP; a timer is only for future work or
    // publication failure. Waiting for another timer tick throttles backlog.
    if (due <= now) {
      posted_ = true;
      if (PostQueuedCompletionStatus(port_, 0, 0, &wake_)) {
        if (armed_) SetThreadpoolTimer(timer_, nullptr, 0, 0);
        armed_.reset();
        return;
      }
      posted_ = false;
    }
    // Relative Windows ticks are 100 ns. Round up and use at least 1 ms for
    // a failed publication retry, avoiding callback storms under contention.
    const auto delta = due > now ? due - now : clock::duration::zero();
    const auto nanos = std::chrono::duration_cast<
        std::chrono::nanoseconds>(delta).count();
    const auto ticks = std::max<std::int64_t>(10000,
        nanos / 100 + (nanos % 100 != 0));
    const auto relative = -ticks;
    ULARGE_INTEGER native{};
    native.QuadPart = static_cast<ULONGLONG>(relative);
    FILETIME target{native.LowPart, native.HighPart};
    SetThreadpoolTimer(timer_, &target, 0, 0);
    armed_ = due;
  }
  // +=========================================================================+
  // | [>] USINGs                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  using deadlines = std::multimap<clock::time_point, entry>;
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Protects only registration/publication, never protocol or socket execution.
  std::mutex mutex_;
  // Group IOCP remains valid through producer stop, packet drain and join.
  HANDLE port_{nullptr};
  // Created only on the first future-deadline registration.
  PTP_TIMER timer_{nullptr};
  // Stable notification storage outlives every post and completion.
  overlapped_base wake_{io_type::kProcessingTimer};
  // Ordered connection deadlines and an index for eager cancellation.
  deadlines deadlines_;
  std::unordered_map<context<ENty>*, typename deadlines::iterator> positions_;
  // Cached absolute target avoids restarting an unchanged earliest deadline.
  std::optional<clock::time_point> armed_;
  // At most one timer notification is queued, independently of new deadlines.
  bool posted_{false};
  // Set before waiting for callback producers and disabling registrations.
  bool stopped_{true};
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
