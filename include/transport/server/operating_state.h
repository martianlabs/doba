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

#ifndef martianlabs_doba_transport_server_operating_state_h
#define martianlabs_doba_transport_server_operating_state_h

#include <atomic>
#include <chrono>
#include <cstddef>
#include <memory>
#include <optional>
#include <utility>

namespace martianlabs::doba::transport::server::detail {
// Credit follows the context, including work surviving socket closure.
struct connection_budget {
  explicit connection_budget(std::size_t maximum) : maximum{maximum} {}
  const std::size_t maximum;
  std::atomic<std::size_t> used{0};
};

class connection_reservation {
public:
  connection_reservation() = default;
  connection_reservation(const connection_reservation &) = delete;
  connection_reservation &operator=(const connection_reservation &) = delete;
  connection_reservation(connection_reservation &&other) noexcept
      : budget_{std::move(other.budget_)} {}
  connection_reservation &operator=(connection_reservation &&other) noexcept {
    if (this != &other) {
      release();
      budget_ = std::move(other.budget_);
    }
    return *this;
  }
  ~connection_reservation() { release(); }

  static std::optional<connection_reservation>
  acquire(const std::shared_ptr<connection_budget> &budget) {
    auto used = budget->used.load(std::memory_order_relaxed);
    do {
      if (budget->maximum && used >= budget->maximum)
        return std::nullopt;
      if (used == static_cast<std::size_t>(-1))
        return std::nullopt;
    } while (!budget->used.compare_exchange_weak(used, used + 1));
    connection_reservation result;
    result.budget_ = budget;
    return result;
  }

private:
  void release() noexcept {
    if (budget_)
      budget_->used.fetch_sub(1);
  }
  std::shared_ptr<connection_budget> budget_;
};

// Ceiling of 75%, without multiplying the budget. Low watermark is floor(50%).
inline bool update_pressure(bool paused, std::size_t used,
                            std::size_t maximum) noexcept {
  if (!maximum)
    return false;
  return paused ? used > maximum / 2 : used >= maximum - maximum / 4;
}

struct connection_progress {
  using clock = std::chrono::steady_clock;
  static constexpr auto maintenance_interval = std::chrono::milliseconds{250};
  // Context I/O and maintenance already hold the same context lock. Neither
  // construction nor positive I/O needs a timestamp or an additional atomic.
  void transferred(std::size_t bytes) noexcept {
    if (bytes)
      active_ = true;
  }
  bool expired(std::chrono::milliseconds period,
               clock::time_point now = clock::now()) noexcept {
    if (period.count() <= 0) return false;
    if (observed_ && now < last) return false;
    if (!observed_ || active_) {
      last = now;
      observed_ = true;
      active_ = false;
      return false;
    }
    return now >= last &&
           std::chrono::duration_cast<std::chrono::milliseconds>(now - last) >=
               period;
  }
  clock::time_point last{};
 private:
  bool observed_{false};
  bool active_{false};
};
} // namespace martianlabs::doba::transport::server::detail
#endif
