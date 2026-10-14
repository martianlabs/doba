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

#ifndef martianlabs_doba_protocol_http_v11_engine_h
#define martianlabs_doba_protocol_http_v11_engine_h

#include <cstddef>
#include <functional>
#include <memory>
#include <string_view>
#include <utility>

#include "protocol/contracts.h"
#include "protocol/http/common/header_names.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/router.h"
#include "protocol/http/common/router_match_result.h"
#include "protocol/http/v11/contracts.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/v11/engine_async.h"
#include "protocol/http/v11/engine_sync.h"
#include "protocol/http/v11/policies.h"
#include "protocol/http/v11/results.h"
#include "transport/server/contracts.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | HTTP/1.1 protocol engine.                                                 |
// +---------------------------------------------------------------------------+
// | Template parameters:                                                      |
// |   RQty - request being used.                                              |
// |   RSty - response being used.                                             |
// |   ROty - router being used.                                               |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <contracts::request RQty, contracts::response RSty,
          typename ROty = router<RQty, RSty>>
  requires contracts::router<ROty, RQty, RSty>
class engine {
 public:
  // +=========================================================================+
  // | [>] USINGs                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = v11::policies;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  engine(policies_type configuration, const ROty& router,
         std::function<void()> on_async_started = {},
         std::function<void()> on_async_finished = {})
      : router_{router},
        decoder_{configuration},
        on_async_started_{std::move(on_async_started)},
        on_async_finished_{std::move(on_async_finished)} {
    async_ = std::make_shared<engine_async>(
        decoder_.configuration().max_pending_requests, on_async_started_,
        on_async_finished_);
  }
  engine(const engine&) = delete;
  engine(engine&&) noexcept = delete;
  ~engine() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  engine& operator=(const engine&) = delete;
  engine& operator=(engine&&) noexcept = delete;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  // | Let's set the 'on_send' callback to send data to the transport.         |
  // +-------------------------------------------------------------------------+
  void set_on_send(protocol::send_delegate output) {
    on_send_ = std::move(output);
    async_->set_on_send(on_send_);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  // | Let's set the 'on_close' callback to close the transport.               |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> close) {
    on_close_ = std::move(close);
    async_->set_on_close(on_close_);
  }
  // +=========================================================================+
  // | [>] set_execution_capacity                                   ( public ) |
  // +-------------------------------------------------------------------------+
  // | Sets the execution capacity for the engine.                             |
  // +-------------------------------------------------------------------------+
  void set_execution_capacity(
      transport::server::execution_capacity execution) noexcept {
    execution_ = execution;
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  bool stop(std::function<void()> completed) {
    closed_ = true;
    return async_->stop(std::move(completed));
  }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  // | Handles bytes received from the transport.                              |
  // +-------------------------------------------------------------------------+
  std::size_t on_bytes_received(const char* buffer, const std::size_t size,
                                const std::size_t capacity) {
    if (closed_) return size;
    if (!size) return 0;
    std::size_t total = 0;
    while (total < size && !closed_) {
      if (!synchronous_) {
        if (!async_->accepting()) return size;
        if (async_->idle()) synchronous_ = true;
      }
      std::size_t consumed = 0;
      try {
        deserialization_result result = decoder_.deserialize(
            buffer + total, size - total, capacity, consumed);
        if (result.code != deserialization_status::kSucceeded) {
          const bool rejected =
              result.code == deserialization_status::kInvalidSource;
          if (result.response) {
            if (!synchronous_) {
              const bool submitted =
                  rejected ? async_->handle_error<RQty, RSty, ROty>(
                                 *result.response, closed_)
                           : async_->handle_interim<RQty, RSty, ROty>(
                                 *result.response);
              if (!submitted) return size;
            } else {
              engine_sync<RQty, RSty, ROty>::handle_response(
                  *result.response, rejected, on_send_);
              if (rejected) query_for_close();
            }
          }
          return rejected ? size : total + consumed;
        }
        RQty& request = *result.request;
        router_match_result<RQty, RSty> match{};
        std::string_view path;
        if (request.get_target() == target::kOriginForm ||
            request.get_target() == target::kAbsoluteForm) {
          path = request.get_absolute_path();
          match = router_.match(request.get_method(), path);
        }
        if (!synchronous_ || engine_async::handles(match)) {
          synchronous_ = false;
          if (!async_->handle_request(decoder_, router_, request, match, path,
                                      execution_, async_, closed_)) {
            return size;
          }
        } else if (engine_sync<RQty, RSty, ROty>::handle_request(
                       decoder_, router_, request, match, on_send_)) {
          query_for_close();
        }
        total += consumed;
        continue;
      } catch (...) {
        query_for_close();
        return total + consumed;
      }
    }
    return total;
  }

 private:
  // +=========================================================================+
  // | [>] query_for_close                                         ( private ) |
  // +-------------------------------------------------------------------------+
  // | If the connection isn't already marked for closure, it marks it and     |
  // | calls the `on_close_` callback.                                         |
  // +-------------------------------------------------------------------------+
  void query_for_close() {
    if (closed_) return;
    closed_ = true;
    on_close_();
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  const ROty& router_;
  decoder<RQty, RSty> decoder_;
  std::function<void()> on_async_started_;
  std::function<void()> on_async_finished_;
  std::function<void()> on_close_;
  std::shared_ptr<engine_async> async_;
  transport::server::execution_capacity execution_;
  protocol::send_delegate on_send_;
  bool closed_{false};
  bool synchronous_{true};
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
