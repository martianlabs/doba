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

#ifndef martianlabs_doba_transport_server_connection_reservation_h
#define martianlabs_doba_transport_server_connection_reservation_h

#include <cstddef>
#include <memory>
#include <optional>
#include <utility>

#include "transport/server/connection_budget.h"

namespace martianlabs::doba::transport::server::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] connection_reservation                                      ( class ) |
// +---------------------------------------------------------------------------+
// | One admission credit. Moves transfer its sole release obligation.         |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class connection_reservation {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  connection_reservation() = default;
  connection_reservation(const connection_reservation&) = delete;
  connection_reservation(connection_reservation&& other) noexcept
      : budget_{std::move(other.budget_)} {}
  ~connection_reservation() { release(); }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  connection_reservation& operator=(const connection_reservation&) = delete;
  connection_reservation& operator=(connection_reservation&& other) noexcept {
    if (this != &other) {
      release();
      budget_ = std::move(other.budget_);
    }
    return *this;
  }
  // +=========================================================================+
  // | [>] acquire                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  static std::optional<connection_reservation> acquire(
      const std::shared_ptr<connection_budget>& budget) {
    auto used = budget->used.load(std::memory_order_relaxed);
    do {
      if (budget->maximum && used >= budget->maximum) return std::nullopt;
      if (used == static_cast<std::size_t>(-1)) return std::nullopt;
    } while (!budget->used.compare_exchange_weak(used, used + 1));
    connection_reservation result;
    result.budget_ = budget;
    return result;
  }

 private:
  // +=========================================================================+
  // | [>] release                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  void release() noexcept {
    if (budget_) budget_->used.fetch_sub(1);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  // Retains the shared counter until this credit is released or transferred.
  std::shared_ptr<connection_budget> budget_;
};
}  // namespace martianlabs::doba::transport::server::detail

#endif
