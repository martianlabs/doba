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

#ifndef martianlabs_doba_protocol_http_v11_engine_h
#define martianlabs_doba_protocol_http_v11_engine_h

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "protocol/contracts.h"
#include "protocol/processing_result.h"
#include "protocol/http/common/header_names.h"
#include "protocol/http/common/headers/www_authenticate.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/router.h"
#include "protocol/http/common/router_match_result.h"
#include "protocol/http/v11/contracts.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/v11/pending_response.h"
#include "protocol/http/v11/policies.h"
#include "protocol/http/v11/request_processor_async.h"
#include "protocol/http/v11/request_processor_sync.h"
#include "protocol/http/v11/results.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine                                                      ( class ) |
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
  engine(policies_type configuration, const ROty& router)
      : router_{router}, decoder_{configuration} {
    if (!configuration.max_pending_requests ||
        !configuration.max_deferred_input_size ||
        !configuration.max_pending_response_bytes ||
        !configuration.max_task_steps_per_turn) {
      throw std::invalid_argument("HTTP processing limits must be positive");
    }
  }
  engine(const engine&) = delete;
  engine(engine&&) = delete;
  ~engine() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  engine& operator=(const engine&) = delete;
  engine& operator=(engine&&) = delete;
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(protocol::send_delegate output) {
    on_send_ = std::move(output);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_close(std::function<void()> close) {
    on_close_ = std::move(close);
  }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_bytes_received(std::string_view input) {
    if (closed_) return {};
    try {
      auto budget = decoder_.configuration().max_task_steps_per_turn;
      if (deferred_.empty()) {
        process_input(input, budget);
        retain(input);
      } else {
        retain(input);
        process_deferred(budget);
      }
    } catch (...) {
      query_for_close();
    }
    return processing();
  }
  // +=========================================================================+
  // | [>] on_processing_turn                                       ( public ) |
  // +-------------------------------------------------------------------------+
  processing_result on_processing_turn(bool output_idle) {
    if (closed_) return {};
    try {
      auto budget = decoder_.configuration().max_task_steps_per_turn;
      if (output_pending_ && output_idle) {
        output_pending_ = false;
        pop_front();
      }
      // Leave an admission step for buffered input when capacity is free.
      const auto allowance = admission_ready() ? budget - 1 : budget;
      resume_pending(budget, allowance);
      flush();
      process_deferred(budget);
    } catch (...) {
      query_for_close();
    }
    return processing();
  }

 private:
  // +=========================================================================+
  // | [>] USINGs                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  using clock = processing_result::clock;
  using sync_processor = detail::request_processor_sync<RQty, RSty, ROty>;
  using async_processor = detail::request_processor_async<RQty, RSty>;
  using entry_type = detail::pending_response<RQty, RSty>;
  using queue_type = std::deque<entry_type>;
  // +=========================================================================+
  // | [>] process_input                                           ( private ) |
  // +-------------------------------------------------------------------------+
  void process_input(std::string_view& input, std::size_t& budget) {
    while (!closed_ && !admission_closed_ && !input.empty() && budget &&
           capacity()) {
      auto result = decoder_.deserialize(input);
      if (result.code != deserialization_status::kSucceeded) {
        const bool rejected =
            result.code == deserialization_status::kInvalidSource;
        if (result.response) {
          auto serialized = serialize(*result.response, rejected);
          if (!pending_ && !output_pending_) {
            send_direct(std::move(serialized));
            if (rejected) query_for_close();
          } else {
            auto& entry = decoding_ ? *decoding_ : append();
            if (rejected) {
              store(entry, std::move(serialized), true);
              decoding_ = nullptr;
              barrier(pending_->size() - 1);
            } else {
              store_interim(entry, std::move(serialized));
              decoding_ = &entry;
            }
            flush();
          }
        }
        return;
      }
      auto& request = *result.request;
      router_match_result<RQty, RSty> match{};
      if (request.get_target() == target::kOriginForm ||
          request.get_target() == target::kAbsoluteForm) {
        match = router_.match(request.get_method(),
                              request.get_absolute_path());
      }
      if (match.async_handler || match.async_parametrized_handler) {
        auto& entry = decoding_ ? *decoding_ : append();
        decoding_ = nullptr;
        --budget;
        entry.handler =
            std::make_unique<async_processor>(decoder_, std::move(request));
        bool failed = false;
        try {
          if (authenticate(match, entry.handler->request(),
                           entry.handler->response())) {
            entry.handler->start(match);
            entry.handler->resume(clock::now());
          }
        } catch (...) {
          failed = true;
        }
        if (entry.handler->done() || failed) {
          finish_async(entry, failed);
        }
        if (entry.close || entry.handler->request().wants_connection_close()) {
          barrier(pending_->size() - 1);
        }
        flush();
      } else {
        bool close = request.wants_connection_close();
        auto response = decoder_.make_response(response_head_, response_body_);
        try {
          if (authenticate(match, request, response)) {
            sync_processor::execute(router_, request, response, match);
          }
        } catch (...) {
          close = true;
          build_error_response(response);
        }
        auto serialized = serialize(request, response, close);
        if (!pending_ && !output_pending_) {
          send_direct(std::move(serialized));
          if (close) query_for_close();
        } else {
          --budget;
          auto& entry = decoding_ ? *decoding_ : append();
          decoding_ = nullptr;
          store(entry, std::move(serialized), close);
          if (close) barrier(pending_->size() - 1);
          flush();
        }
      }
    }
    if (admission_closed_) input = {};
  }
  // +=========================================================================+
  // | [>] resume_pending                                          ( private ) |
  // +-------------------------------------------------------------------------+
  void resume_pending(std::size_t& budget, std::size_t allowance) {
    if (!pending_ || !allowance) return;
    const auto count = pending_->size();
    for (std::size_t visited = 0; visited < count && allowance; ++visited) {
      const auto index = cursor_;
      auto& entry = (*pending_)[index];
      cursor_ = (cursor_ + 1) % count;
      if (entry.complete || !entry.handler) continue;
      if (!entry.handler->resume(clock::now())) continue;
      --budget;
      --allowance;
      if (entry.handler->done()) {
        finish_async(entry, false);
        if (entry.close) {
          barrier(index);
          return;
        }
      }
    }
  }
  // +=========================================================================+
  // | [>] finish_async                                            ( private ) |
  // +-------------------------------------------------------------------------+
  void finish_async(entry_type& entry, bool failed) {
    bool close = entry.handler->request().wants_connection_close();
    if (failed || entry.handler->error()) {
      close = true;
      build_error_response(entry.handler->response());
    }
    auto output = serialize(entry.handler->request(), entry.handler->response(),
                            close);
    store(entry, std::move(output), close);
  }
  // +=========================================================================+
  // | [>] store                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  void store(entry_type& entry, serialization_result output, bool close) {
    reserve_bytes(output.head.size(), output.body.size());
    entry.head.assign(output.head);
    entry.body.assign(output.body);
    entry.source = std::move(output.source);
    pending_bytes_ += entry.head.size() + entry.body.size();
    entry.close = close;
    entry.complete = true;
  }
  // +=========================================================================+
  // | [>] store_interim                                           ( private ) |
  // +-------------------------------------------------------------------------+
  void store_interim(entry_type& entry, serialization_result output) {
    if (!entry.interim.empty() || !output.body.empty() || output.source) {
      throw std::logic_error("Invalid interim response");
    }
    reserve_bytes(output.head.size(), 0);
    entry.interim.assign(output.head);
    pending_bytes_ += entry.interim.size();
  }
  // +=========================================================================+
  // | [>] reserve_bytes                                           ( private ) |
  // +-------------------------------------------------------------------------+
  void reserve_bytes(std::size_t head, std::size_t body) const {
    const auto capacity = decoder_.configuration().max_pending_response_bytes;
    const auto used = pending_bytes_;
    if (head > capacity - used || body > capacity - used - head) {
      throw std::length_error("Pending response byte limit exceeded");
    }
  }
  // +=========================================================================+
  // | [>] flush                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  void flush() {
    while (!closed_ && pending_ && !output_pending_) {
      auto& front = pending_->front();
      if (!front.interim.empty()) {
        on_send_(front.interim, {}, nullptr);
        pending_bytes_ -= front.interim.size();
        front.interim.clear();
      }
      if (!front.complete) return;
      const bool source = static_cast<bool>(front.source);
      front.source_sent = source;
      output_pending_ = source;
      on_send_(front.head, front.body, std::move(front.source));
      pending_bytes_ -= front.head.size() + front.body.size();
      front.head.clear();
      front.body.clear();
      if (front.close) {
        query_for_close();
        return;
      }
      if (!source) pop_front();
    }
  }
  // +=========================================================================+
  // | [>] send_direct                                             ( private ) |
  // +-------------------------------------------------------------------------+
  void send_direct(serialization_result output) {
    if (output.source) {
      auto& entry = append();
      entry.complete = true;
      entry.source_sent = true;
      output_pending_ = true;
      on_send_(output.head, output.body, std::move(output.source));
    } else {
      on_send_(output.head, output.body, nullptr);
    }
  }
  // +=========================================================================+
  // | [>] append                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  entry_type& append() {
    if (!pending_) pending_ = std::make_unique<queue_type>();
    pending_->emplace_back();
    return pending_->back();
  }
  // +=========================================================================+
  // | [>] pop_front                                               ( private ) |
  // +-------------------------------------------------------------------------+
  void pop_front() {
    pending_->pop_front();
    if (cursor_) --cursor_;
    if (pending_->empty()) {
      pending_.reset();
      cursor_ = 0;
    }
  }
  // +=========================================================================+
  // | [>] barrier                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  void barrier(std::size_t position) {
    admission_closed_ = true;
    deferred_.clear();
    decoding_ = nullptr;
    while (pending_->size() > position + 1) {
      auto& last = pending_->back();
      pending_bytes_ -=
          last.interim.size() + last.head.size() + last.body.size();
      pending_->pop_back();
    }
    cursor_ %= pending_->size();
  }
  // +=========================================================================+
  // | [>] capacity                                                ( private ) |
  // +-------------------------------------------------------------------------+
  bool capacity() const noexcept {
    return decoding_ || !pending_ ||
           pending_->size() < decoder_.configuration().max_pending_requests;
  }
  // +=========================================================================+
  // | [>] admission_ready                                         ( private ) |
  // +-------------------------------------------------------------------------+
  bool admission_ready() const noexcept {
    return !admission_closed_ && !deferred_.empty() && capacity();
  }
  // +=========================================================================+
  // | [>] process_deferred                                        ( private ) |
  // +-------------------------------------------------------------------------+
  void process_deferred(std::size_t& budget) {
    if (closed_ || !admission_ready()) return;
    std::string_view input(deferred_);
    process_input(input, budget);
    if (!closed_ && !admission_closed_) {
      deferred_.erase(0, deferred_.size() - input.size());
    }
  }
  // +=========================================================================+
  // | [>] retain                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  void retain(std::string_view input) {
    if (closed_ || admission_closed_ || input.empty()) return;
    const auto limit = decoder_.configuration().max_deferred_input_size;
    if (input.size() > limit - deferred_.size()) {
      throw std::length_error("Deferred input byte limit exceeded");
    }
    deferred_.append(input);
  }
  // +=========================================================================+
  // | [>] processing                                              ( private ) |
  // +-------------------------------------------------------------------------+
  processing_result processing() const noexcept {
    if (closed_) return {};
    processing_result result{admission_ready(), {}, output_pending_};
    if (!pending_) return result;
    const auto now = clock::now();
    for (const auto& entry : *pending_) {
      if (entry.complete || !entry.handler) continue;
      const auto task = entry.handler->processing();
      result.turn_requested |= task.turn_requested;
      if (task.turn_at) {
        if (*task.turn_at <= now) {
          result.turn_requested = true;
        } else if (!result.turn_at || *task.turn_at < *result.turn_at) {
          result.turn_at = task.turn_at;
        }
      }
    }
    return result;
  }
  // +=========================================================================+
  // | [>] query_for_close                                         ( private ) |
  // +-------------------------------------------------------------------------+
  void query_for_close() {
    if (closed_) return;
    closed_ = true;
    admission_closed_ = true;
    deferred_.clear();
    decoding_ = nullptr;
    pending_bytes_ = 0;
    if (pending_) {
      // An accepted reader may still borrow bytes owned by the front handler.
      if (output_pending_ && pending_->front().source_sent) {
        pending_->erase(pending_->begin() + 1, pending_->end());
      } else {
        pending_.reset();
      }
    }
    if (on_close_) on_close_();
  }
  // +=========================================================================+
  // | [>] authenticate                                            ( private ) |
  // +-------------------------------------------------------------------------+
  static bool authenticate(const router_match_result<RQty, RSty>& match,
                           const RQty& request, RSty& response) {
    if (!match.authenticate || !*match.authenticate) return true;
    const auto result = (*match.authenticate)(request);
    if (result.status == auth::authentication_status::kAccepted) return true;
    if ((!result.challenge.empty() &&
         !http::headers::www_authenticate::check(result.challenge)) ||
        (result.status == auth::authentication_status::kUnauthenticated &&
         result.challenge.find_first_not_of(" \t,") ==
             std::string::npos)) {
      throw std::invalid_argument("Invalid authentication challenge");
    }
    switch (result.status) {
      case auth::authentication_status::kInvalidRequest:
        response.bad_request_400();
        break;
      case auth::authentication_status::kUnauthenticated:
        response.unauthorized_401();
        break;
      case auth::authentication_status::kForbidden:
        response.forbidden_403();
        break;
      default:
        throw std::invalid_argument("Invalid authentication result");
    }
    if (!result.challenge.empty()) {
      response.set_header(header_names::kWwwAuthenticate, result.challenge);
    }
    return false;
  }
  // +=========================================================================+
  // | [>] build_error_response                                    ( private ) |
  // +-------------------------------------------------------------------------+
  // | Turn a handler exception into a 500 response with an Internal Server    |
  // | Error body.                                                             |
  // +-------------------------------------------------------------------------+
  static void build_error_response(RSty& response) {
    response.internal_server_error_500();
    response.set_body("Internal Server Error");
  }
  // +=========================================================================+
  // | [>] serialize                                               ( private ) |
  // +-------------------------------------------------------------------------+
  // | Serialize the response; add Connection: close when closing.             |
  // +-------------------------------------------------------------------------+
  static serialization_result serialize(RSty& response, bool close) {
    if (close) response.set_header(header_names::kConnection, "close");
    return response.serialize();
  }
  // +=========================================================================+
  // | [>] serialize                                               ( private ) |
  // +-------------------------------------------------------------------------+
  // | Serialize for this request; add Connection: close when closing.         |
  // +-------------------------------------------------------------------------+
  static serialization_result serialize(const RQty& request, RSty& response,
                                        bool& close) {
    serialization_result serialized;
    try {
      if (response.is_continue_100()) {
        close = true;
        build_error_response(response);
      }
      if (close) {
        response.set_header(header_names::kConnection, "close");
      } else if (response.wants_connection_close()) {
        close = true;
      }
      // RFC 9110 S9.3.2: HEAD preserves framing but never sends body bytes.
      if (request.get_method() == method_names::kHead) response.suppress_body();
      serialized = response.serialize();
    } catch (...) {
      close = true;
      build_error_response(response);
      response.set_header(header_names::kConnection, "close");
      if (request.get_method() == method_names::kHead) response.suppress_body();
      serialized = response.serialize();
    }
    return serialized;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  const ROty& router_;
  decoder<RQty, RSty> decoder_;
  std::string response_head_;       // Reusable sync reply storage.
  std::string response_body_;
  protocol::send_delegate on_send_;
  std::function<void()> on_close_;
  std::string deferred_;            // Bounded input awaiting admission.
  std::unique_ptr<queue_type> pending_;  // Lazy ordered engine records.
  entry_type* decoding_{nullptr};   // Reserved interim-response record.
  std::size_t pending_bytes_{0};    // Own head and inline-body bytes.
  std::size_t cursor_{0};           // Where to resume fairly in the records.
  bool output_pending_{false};      // Wait for the terminal reader to drain.
  bool admission_closed_{false};    // Ordered close point is known.
  bool closed_{false};
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
