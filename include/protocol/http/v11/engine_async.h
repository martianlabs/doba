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

#ifndef martianlabs_doba_protocol_http_v11_engine_async_h
#define martianlabs_doba_protocol_http_v11_engine_async_h

#include <atomic>
#include <chrono>
#include <cstddef>
#include <coroutine>
#include <deque>
#include <exception>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "common/task.h"
#include "protocol/contracts.h"
#include "protocol/http/common/header_names.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/headers/www_authenticate.h"
#include "protocol/http/common/router.h"
#include "protocol/http/common/router_match_result.h"
#include "protocol/http/v11/buffer_pool.h"
#include "protocol/http/v11/contracts.h"
#include "protocol/http/v11/results.h"
#include "transport/server/contracts.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine_async                                                ( class ) |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <contracts::request RQty, contracts::response RSty, typename ROty>
class engine_async {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  struct slot {
    std::unique_ptr<detail::buffer_pool::buffer_pair> storage;
    std::unique_ptr<detail::buffer_pool::buffer_pair> interim_storage;
    std::optional<RQty> request;
    std::optional<RSty> response;
    std::optional<RSty> interim;
    std::optional<serialization_result> output;
    std::optional<serialization_result> interim_output;
    http::detail::router_parameter_storage parameters;
    std::shared_ptr<engine_async> keep_alive;
    transport::server::execution_capacity execution;
    std::exception_ptr error;
    engine_async* owner{nullptr};
    slot* next{nullptr};
    slot* free_next{nullptr};
    slot* completed_next{nullptr};
    bool close{false};
    bool starting{false};
    bool completed{false};
    bool inline_completed{false};
    bool tracked{false};
    bool ready{false};
  };
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  engine_async(std::size_t max_pending_requests,
                const std::function<void()>& on_async_started,
                const std::function<void()>& on_async_finished)
      : max_pending_requests_{max_pending_requests},
        on_async_started_{on_async_started},
        on_async_finished_{on_async_finished},
        slots_(max_pending_requests) {
    for (auto& entry : slots_) {
      entry.owner = this;
      entry.free_next = free_;
      free_ = &entry;
    }
  }
  engine_async(const engine_async&) = delete;
  engine_async(engine_async&&) noexcept = delete;
  ~engine_async() {
    for (auto& entry : slots_) clear(entry);
  }
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  engine_async& operator=(const engine_async&) = delete;
  engine_async& operator=(engine_async&&) noexcept = delete;
  // +=========================================================================+
  // | [>] reserve                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  slot* reserve() {
    {
      std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
      if (!fast()) lock.lock();
      if (!accepting()) return nullptr;
      if (!free_ && !max_pending_requests_) {
        slots_.emplace_back();
        free_ = &slots_.back();
        free_->owner = this;
      }
      if (free_) {
        auto* entry = free_;
        free_ = entry->free_next;
        entry->free_next = nullptr;
        if (tail_) {
          tail_->next = entry;
        } else {
          head_ = entry;
        }
        tail_ = entry;
        return entry;
      }
    }
    close_now();
    return nullptr;
  }
  // +=========================================================================+
  // | [>] handle_request                                           ( public ) |
  // +-------------------------------------------------------------------------+
  void handle_request(slot& entry, const ROty& router,
                       const router_match_result<RQty, RSty>& match,
                       std::string_view path,
                       transport::server::execution_capacity execution,
                       const std::shared_ptr<engine_async>& state,
                       bool& closed) {
    entry.close = entry.request->wants_connection_close();
    if (entry.close) closed = true;
    if (match.handler || match.parametrized_handler) {
      entry.execution = execution;
      dispatch(entry, match, path, state);
      return;
    }
    execute(router, *entry.request, *entry.response, entry.close, &match);
    auto output = serialize(*entry.request, *entry.response, entry.close);
    if (entry.close) closed = true;
    submit(entry, std::move(output));
  }
  // +=========================================================================+
  // | [>] handle_error                                             ( public ) |
  // +-------------------------------------------------------------------------+
  void handle_error(slot& entry) {
    entry.close = true;
    submit(entry, serialize(*entry.response, true));
  }
  // +=========================================================================+
  // | [>] handle_interim                                           ( public ) |
  // +-------------------------------------------------------------------------+
  void handle_interim(slot& entry, RSty response,
                       std::unique_ptr<detail::buffer_pool::buffer_pair>
                           storage) {
    auto output = serialize(response, false);
    bool start;
    {
      std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
      if (!fast()) lock.lock();
      entry.interim_storage = std::move(storage);
      entry.interim.emplace(std::move(response));
      entry.interim_output.emplace(std::move(output));
      start = !draining_;
      if (start) {
        draining_ = true;
        if (!fast()) begin();
      }
    }
    if (start) drain();
  }
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
  // | [>] begin_receive                                            ( public ) |
  // +-------------------------------------------------------------------------+
  bool begin_receive() {
    auto active = active_.load(std::memory_order_relaxed);
    do {
      if (active & kStopping) return false;
    } while (!active_.compare_exchange_weak(
        active, active + 1, std::memory_order_acquire,
        std::memory_order_relaxed));
    if (!accepting_.load(std::memory_order_acquire)) {
      end();
      return false;
    }
    direct_ = !active;
    if (direct_) {
      direct_accepting_ = true;
      direct_closing_ = false;
    }
    receiving_ = this;
    return true;
  }
  // +=========================================================================+
  // | [>] end_receive                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void end_receive() {
    receiving_ = nullptr;
    end();
  }
  // +=========================================================================+
  // | [>] begin                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Counts admitted receives and pending execution.                         |
  // +-------------------------------------------------------------------------+
  void begin() noexcept { active_.fetch_add(1, std::memory_order_release); }
  // +=========================================================================+
  // | [>] end                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  // | Completes a stop after the last admitted operation has retired.         |
  // +-------------------------------------------------------------------------+
  void end() {
    if (active_.fetch_sub(1, std::memory_order_acq_rel) != kStopping + 1) {
      return;
    }
    std::function<void()> completed;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      completed = std::move(on_completed_);
    }
    if (completed) completed();
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Closes admission and returns true if no active operations remain.       |
  // | Otherwise retains the callback until the last operation retires.        |
  // +-------------------------------------------------------------------------+
  bool stop(std::function<void()> completed) {
    std::lock_guard<std::mutex> lock(mutex_);
    on_completed_ = std::move(completed);
    const auto active = active_.fetch_or(kStopping, std::memory_order_acq_rel);
    if (active & ~kStopping) return false;
    on_completed_ = {};
    return true;
  }
  // +=========================================================================+
  // | [>] closing                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns whether the engine is in the process of closing the connection. |
  // +-------------------------------------------------------------------------+
  bool closing() const noexcept {
    if (fast()) return direct_closing_;
    return closing_.load(std::memory_order_acquire);
  }
  // +=========================================================================+
  // | [>] accepting                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns whether the engine is in the process of accepting new           |
  // | connections.                                                            |
  // +-------------------------------------------------------------------------+
  bool accepting() const noexcept {
    if (fast()) return direct_accepting_;
    return accepting_.load(std::memory_order_acquire);
  }
  // +=========================================================================+
  // | [>] close_now                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Immediately closes the connection. It sets the accepting flag to false, |
  // | sets the closing flag to true, and calls the 'on_close_' callback.      |
  // +-------------------------------------------------------------------------+
  void close_now() {
    if (fast()) {
      direct_accepting_ = false;
      direct_closing_ = true;
    }
    accepting_.store(false, std::memory_order_release);
    if (closing_.exchange(true)) return;
    on_close_();
  }

 private:
  // +=========================================================================+
  // | [>] fast                                                    ( private ) |
  // +-------------------------------------------------------------------------+
  bool fast() const noexcept { return receiving_ == this && direct_; }
  // +=========================================================================+
  // | [>] clear                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  void clear(slot& entry) {
    entry.output.reset();
    entry.interim_output.reset();
    entry.response.reset();
    entry.interim.reset();
    entry.request.reset();
    entry.parameters.reset();
    entry.keep_alive.reset();
    entry.error = {};
    entry.execution = {};
    if (entry.storage) {
      detail::buffer_pool::release(std::move(entry.storage));
    }
    if (entry.interim_storage) {
      detail::buffer_pool::release(std::move(entry.interim_storage));
    }
    entry.next = nullptr;
    entry.completed_next = nullptr;
    entry.close = false;
    entry.starting = false;
    entry.completed = false;
    entry.inline_completed = false;
    entry.tracked = false;
    entry.ready = false;
  }
  // +=========================================================================+
  // | [>] submit                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  void submit(slot& entry, serialization_result output) {
    if (entry.close) {
      if (fast()) direct_accepting_ = false;
      accepting_.store(false, std::memory_order_release);
    }
    bool start;
    {
      std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
      if (!fast()) lock.lock();
      entry.output.emplace(std::move(output));
      entry.ready = true;
      start = !draining_;
      if (start) {
        draining_ = true;
        if (!fast()) begin();
      }
    }
    if (start) drain();
  }
  // +=========================================================================+
  // | [>] enqueue                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  bool enqueue(slot& entry) {
    if (completions_tail_) {
      completions_tail_->completed_next = &entry;
    } else {
      completions_ = &entry;
    }
    completions_tail_ = &entry;
    if (draining_) return false;
    draining_ = true;
    begin();
    return true;
  }
  // +=========================================================================+
  // | [>] complete                                                ( private ) |
  // +-------------------------------------------------------------------------+
  void complete(slot& entry, std::exception_ptr error) {
    if (fast() && entry.starting) {
      entry.error = error;
      entry.completed = true;
      entry.inline_completed = true;
      return;
    }
    if (error) accepting_.store(false, std::memory_order_release);
    std::shared_ptr<engine_async> state;
    bool start;
    {
      std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
      if (!fast()) lock.lock();
      entry.error = error;
      entry.completed = true;
      if (entry.starting) return;
      state = entry.keep_alive;
      start = enqueue(entry);
    }
    if (start) state->drain();
  }
  // +=========================================================================+
  // | [>] dispatch                                                ( private ) |
  // +-------------------------------------------------------------------------+
  void dispatch(slot& entry, const router_match_result<RQty, RSty>& match,
                  std::string_view path,
                  const std::shared_ptr<engine_async>& state) {
    bool started = false;
    try {
      if (!authenticate(match, *entry.request, *entry.response)) {
        submit(entry, serialize(*entry.request, *entry.response, entry.close));
        return;
      }
      common::task<void> work =
          match.handler
              ? (*match.handler)(*entry.request, *entry.response)
              : match.parametrized_handler->invoke(
                    *entry.request, *entry.response, path, entry.parameters);
      if (on_async_started_) on_async_started_();
      if (!fast()) {
        begin();
        entry.tracked = true;
      }
      started = true;
      entry.starting = true;
      common::task_scheduler scheduler;
      if (entry.execution.submit) {
        scheduler = {
            &entry,
            [](void* owner, std::coroutine_handle<> handle) {
              const auto execution = static_cast<slot*>(owner)->execution;
              execution.submit(
                  execution.owner,
                  std::chrono::steady_clock::time_point::min(),
                  {handle.address(), [](void* context) noexcept {
                     std::coroutine_handle<>::from_address(context).resume();
                   }});
            },
            [](void* owner, std::chrono::steady_clock::time_point due,
               std::coroutine_handle<> handle) {
              const auto execution = static_cast<slot*>(owner)->execution;
              execution.submit(
                  execution.owner, due,
                  {handle.address(), [](void* context) noexcept {
                     std::coroutine_handle<>::from_address(context).resume();
                   }});
            }};
      }
      work.start(&entry, [](void* context, std::exception_ptr error) {
        auto& entry = *static_cast<slot*>(context);
        entry.owner->complete(entry, error);
      }, scheduler);
      if (fast() && entry.inline_completed) {
        entry.starting = false;
        draining_ = true;
        finish(entry);
        drain();
        return;
      }
      direct_ = false;
      bool start = false;
      {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!entry.tracked) {
          begin();
          entry.tracked = true;
        }
        entry.starting = false;
        if (entry.completed) {
          start = enqueue(entry);
        } else {
          entry.keep_alive = state;
        }
      }
      if (start) drain();
    } catch (...) {
      if (started) {
        try {
          if (on_async_finished_) on_async_finished_();
        } catch (...) {
        }
        if (entry.tracked) end();
      }
      entry.close = true;
      build_error_response(*entry.response);
      submit(entry, serialize(*entry.request, *entry.response, entry.close));
    }
  }
  // +=========================================================================+
  // | [>] finish                                                  ( private ) |
  // +-------------------------------------------------------------------------+
  void finish(slot& entry) {
    auto state = std::move(entry.keep_alive);
    const bool tracked = entry.tracked;
    try {
      if (entry.error) {
        entry.close = true;
        build_error_response(*entry.response);
      }
      submit(entry, serialize(*entry.request, *entry.response, entry.close));
    } catch (...) {
      try {
        close_now();
      } catch (...) {
      }
      std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
      if (!fast()) lock.lock();
      entry.ready = true;
    }
    try {
      if (on_async_finished_) on_async_finished_();
    } catch (...) {
    }
    if (tracked) end();
  }
  // +=========================================================================+
  // | [>] drain                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  void drain() {
    const bool tracked = !fast();
    for (;;) {
      slot* completed = nullptr;
      slot* sending = nullptr;
      slot* retired = nullptr;
      std::optional<serialization_result> output;
      bool interim = false;
      {
        std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
        if (!fast()) lock.lock();
        if (completions_) {
          completed = completions_;
          completions_ = completed->completed_next;
          completed->completed_next = nullptr;
          if (!completions_) completions_tail_ = nullptr;
        } else if (head_) {
          if (closing()) {
            if (head_->ready) retired = head_;
          } else if (head_->interim_output) {
            sending = head_;
            interim = true;
            output.emplace(std::move(*head_->interim_output));
            head_->interim_output.reset();
          } else if (head_->ready) {
            sending = head_;
            output.emplace(std::move(*head_->output));
            head_->output.reset();
          }
          if (retired) {
            head_ = retired->next;
            if (!head_) tail_ = nullptr;
          }
        }
        if (!completed && !sending && !retired) {
          draining_ = false;
          if (tracked && receiving_ == this && !head_) {
            direct_accepting_ = accepting_.load(std::memory_order_acquire);
            direct_closing_ = closing_.load(std::memory_order_acquire);
            direct_ = true;
          }
        }
      }
      if (completed) {
        finish(*completed);
      } else if (sending) {
        try {
          on_send_(output->head, output->body, std::move(output->source));
          if (!interim && sending->close) close_now();
        } catch (...) {
          try {
            close_now();
          } catch (...) {
          }
        }
        if (interim) {
          sending->interim.reset();
          detail::buffer_pool::release(std::move(sending->interim_storage));
        } else {
          {
            std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
            if (!fast()) lock.lock();
            head_ = sending->next;
            if (!head_) tail_ = nullptr;
          }
          retired = sending;
        }
      } else if (!retired) {
        if (tracked) end();
        return;
      }
      if (retired) {
        clear(*retired);
        std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
        if (!fast()) lock.lock();
        retired->free_next = free_;
        free_ = retired;
      }
    }
  }
  // +=========================================================================+
  // | [>] execute                                                 ( private ) |
  // +-------------------------------------------------------------------------+
  // | Executes the request against the router and builds the response. It     |
  // | handles different request targets and sets the appropriate response.    |
  // | If an exception occurs during execution, it builds an error response.   |
  // +-------------------------------------------------------------------------+
  static void execute(
      const ROty& router, const RQty& request, RSty& response, bool& close,
      const router_match_result<RQty, RSty>* selected = nullptr) {
    try {
      switch (request.get_target()) {
        case target::kOriginForm:
        case target::kAbsoluteForm: {
          const std::string_view path = request.get_absolute_path();
          const router_match_result<RQty, RSty> match =
              selected ? *selected : router.match(request.get_method(), path);
          if (match && !authenticate(match, request, response)) return;
          const std::string allowed = router.allowed_methods(path);
          if (allowed.empty()) {
            response.not_found_404();
            return;
          }
          // RFC 9110 S15.5.6: a 405 response must advertise allowed methods.
          response.method_not_allowed_405();
          response.set_header(header_names::kAllow, allowed);
          return;
        }
        case target::kAuthorityForm:
          response.not_implemented_501();
          return;
        case target::kAsteriskForm:
          response.ok_200();
          return;
        default:
          response.bad_request_400();
          return;
      }
    } catch (...) {
      close = true;
      build_error_response(response);
    }
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
  // | Builds a generic error response with a 500 Internal Server Error        |
  // | status. It sets the response body to "Internal Server Error".           |
  // | This is used when an exception occurs during request processing.        |
  // +-------------------------------------------------------------------------+
  static void build_error_response(RSty& response) {
    response.internal_server_error_500();
    response.set_body("Internal Server Error");
  }
  // +=========================================================================+
  // | [>] serialize                                               ( private ) |
  // +-------------------------------------------------------------------------+
  // | Serializes the response. If the connection should be closed,            |
  // | it sets the "Connection: close" header. It returns the                  |
  // | serialized result.                                                      |
  // +-------------------------------------------------------------------------+
  static serialization_result serialize(RSty& response, bool close) {
    if (close) response.set_header(header_names::kConnection, "close");
    return response.serialize();
  }
  // +=========================================================================+
  // | [>] serialize                                               ( private ) |
  // +-------------------------------------------------------------------------+
  // | Serializes the response based on the request. If the connection should  |
  // | be closed, it sets the "Connection: close" header. It returns the       |
  // | serialized result.                                                      |
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
  static constexpr std::size_t kStopping =
      std::size_t{1} << (std::numeric_limits<std::size_t>::digits - 1);
  inline static thread_local engine_async* receiving_{nullptr};
  protocol::send_delegate on_send_;
  const std::size_t max_pending_requests_;
  std::atomic<std::size_t> active_{0};
  std::function<void()> on_close_;
  std::function<void()> on_async_started_;
  std::function<void()> on_async_finished_;
  std::function<void()> on_completed_;
  std::atomic<bool> accepting_{true};
  std::atomic<bool> closing_{false};
  std::mutex mutex_;
  std::deque<slot> slots_;
  slot* free_{nullptr};
  slot* head_{nullptr};
  slot* tail_{nullptr};
  slot* completions_{nullptr};
  slot* completions_tail_{nullptr};
  bool draining_{false};
  bool direct_{false};
  bool direct_accepting_{true};
  bool direct_closing_{false};
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
