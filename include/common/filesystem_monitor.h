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

#ifndef martianlabs_doba_common_filesystem_monitor_h
#define martianlabs_doba_common_filesystem_monitor_h

#include <cstdint>
#include <limits>
#include <mutex>
#include <shared_mutex>

#include "common/filesystem.h"
#ifdef _WIN32
#include "common/filesystem_monitor_windows.h"
#elif __linux__
#include "common/filesystem_monitor_linux.h"
#endif

namespace martianlabs::doba::common {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] filesystem_monitor                                         ( class ) |
// +---------------------------------------------------------------------------+
// | Caller-driven native change monitor. Generations cover the entire root.   |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class filesystem_monitor {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit filesystem_monitor(const std::filesystem::path& root,
                               std::size_t max_watches = 4096)
      : max_watches_(max_watches) {
    root_ = filesystem_root(root, setup_error_);
  }
  filesystem_monitor(const filesystem_monitor&) = delete;
  filesystem_monitor& operator=(const filesystem_monitor&) = delete;
  // +=========================================================================+
  // | [>] observe                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // Establish coverage BEFORE reading a value. Zero means no usable coverage.
  [[nodiscard]] std::uint64_t observe(std::string_view relative, std::error_code& error) {
    std::lock_guard lock(mutex_);
    error.clear();
    if (!detail::filesystem_relative_path(relative, error)) return 0;
    if (setup_error_) { error = setup_error_; return 0; }
    if (ready_) synchronize(error);
    if (!ready_) {
      native_.close();
      advance();
      if (setup_error_) { error = setup_error_; return 0; }
      ready_ = native_.open(root_, max_watches_, error);
      if (!ready_) { native_.close(); return 0; }
    }
    // Failure to extend coverage must not revoke observations already covered.
    if (!native_.watch(root_, relative, error)) return 0;
    synchronize(error);
    return ready_ ? generation_ : 0;
  }
  // +=========================================================================+
  // | [>] poll                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // Consume pending events synchronously, without waiting for new events.
  [[nodiscard]] std::uint64_t poll(std::error_code& error) {
    {
      std::shared_lock lock(mutex_);
      error.clear();
      if (!ready_) {
        error = setup_error_ ? setup_error_ : std::make_error_code(std::errc::state_not_recoverable);
        return 0;
      }
      // The shared lock keeps descriptors alive and excludes a consumer that
      // has dequeued events but has not yet published its new generation.
      if (!native_.pending(error)) return generation_;
    }
    std::lock_guard lock(mutex_);
    if (ready_) synchronize(error);
    return ready_ ? generation_ : 0;
  }
  // +=========================================================================+
  // | [>] unchanged                                                ( public ) |
  // +-------------------------------------------------------------------------+
  [[nodiscard]] bool unchanged(std::uint64_t generation, std::error_code& error) {
    const auto current = poll(error);
    return current && generation == current;
  }
  // +=========================================================================+
  // | [>] reset                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // Invalidate every token and release coverage. observe() rebuilds it lazily.
  void reset() {
    std::lock_guard lock(mutex_);
    ready_ = false;
    advance();
    native_.close();
  }
 private:
  // +=========================================================================+
  // | [>] synchronize                                            ( private ) |
  // +-------------------------------------------------------------------------+
  void synchronize(std::error_code& error) {
    const auto status = native_.poll(error);
    if (status) advance();
    if (status == 2) ready_ = false;
  }
  // +=========================================================================+
  // | [>] advance                                                ( private ) |
  // +-------------------------------------------------------------------------+
  void advance() {
    // Never recycle a generation: saturation disables fast-path coverage.
    if (generation_ != std::numeric_limits<std::uint64_t>::max()) ++generation_;
    else { ready_ = false; setup_error_ = std::make_error_code(std::errc::value_too_large); }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Canonical directory whose generations are tracked.
  std::filesystem::path root_;
  // Bound on native watch resources; zero disables monitoring.
  std::size_t max_watches_;
  // Parallel empty-queue checks; exclusive event processing/registration/reset.
  // No content reads or sends occur under this lock.
  std::shared_mutex mutex_;
  // Native descriptors/handles; no background thread or callback.
  detail::filesystem_monitor_native native_;
  // Monotonic root-wide revision. Zero is reserved for unavailable coverage.
  std::uint64_t generation_ = 1;
  // Whether all registered observations retain valid native coverage.
  bool ready_ = false;
  // Permanent root/configuration error, if any.
  std::error_code setup_error_;
};
}  // namespace martianlabs::doba::common

#endif
