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
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "common/task.h"
#include "protocol/contracts.h"
#include "protocol/http/common/router_match_result.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/v11/engine_sync.h"
#include "protocol/http/v11/results.h"
#include "transport/server/contracts.h"
#include "transport/server/operating_state.h"

namespace martianlabs::doba::protocol::http::v11::detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine_async                                                ( class ) |
// +---------------------------------------------------------------------------+
// | The asynchronous engine manages the state of multiple concurrent requests |
// | and responses. It ensures that responses are sent in the order of their   |
// | corresponding requests, even if they complete out of order. It also       |
// | handles connection closing and resource management for pending requests.  |
// /////////////////////////////////////////////////////////////////////////////
class engine_async {
 public:
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  explicit engine_async(std::size_t max_pending_requests,
                        std::function<void()> on_async_started,
                        std::function<void()> on_async_finished,
                        std::size_t max_pending_bytes = 1024 * 1024)
      : max_pending_requests_{max_pending_requests},
        max_pending_bytes_{max_pending_bytes},
        on_async_started_{std::move(on_async_started)},
        on_async_finished_{std::move(on_async_finished)} {}
  engine_async(const engine_async&) = delete;
  engine_async(engine_async&&) noexcept = delete;
  ~engine_async() = default;
  // +=========================================================================+
  // | [>] OPERATORs                                                ( public ) |
  // +-------------------------------------------------------------------------+
  engine_async& operator=(const engine_async&) = delete;
  engine_async& operator=(engine_async&&) noexcept = delete;
  // +=========================================================================+
  // | [>] handles                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Determines if the given router match result indicates that the request  |
  // | should be handled asynchronously.                                       |
  // +-------------------------------------------------------------------------+
  template <typename Match>
  static bool handles(const Match& match) {
    return match.async_handler || match.async_parametrized_handler;
  }
  // +=========================================================================+
  // | [>] handle_request                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Handles an incoming request. If the request is to be handled            |
  // | asynchronously, it dispatches it to the appropriate handler and manages |
  // | the response.                                                           |
  // +-------------------------------------------------------------------------+
  template <typename RQty, typename RSty, typename ROty>
  bool handle_request(decoder<RQty, RSty>& owner, const ROty& router,
                      RQty& request,
                      const router_match_result<RQty, RSty>& match,
                      std::string_view path,
                      transport::server::execution_capacity execution,
                      const std::shared_ptr<engine_async>& state,
                      bool& closed, std::string& head, std::string& body) {
    const auto position = request_position();
    if (!position) return false;
    bool close = request.wants_connection_close();
    if (handles(match)) {
      dispatch<RQty, RSty, ROty>(owner, std::move(request), match, path,
                                 *position, close, execution, state);
      if (close) closed = true;
      return true;
    }
    RSty response = owner.make_response(head, body);
    engine_sync<RQty, RSty, ROty>::execute(router, request, response, close,
                                           &match);
    auto serialized =
        engine_sync<RQty, RSty, ROty>::serialize(request, response, close);
    submit(*position, std::move(serialized), true, close);
    if (close) closed = true;
    return true;
  }
  // +=========================================================================+
  // | [>] handle_error                                             ( public ) |
  // +-------------------------------------------------------------------------+
  // | Handles an error response. It serializes the response and submits it to |
  // | the transport.                                                          |
  // +-------------------------------------------------------------------------+
  template <typename RQty, typename RSty, typename ROty>
  bool handle_error(RSty& response, bool& closed) {
    const auto position = request_position();
    if (!position) return false;
    auto serialized = engine_sync<RQty, RSty, ROty>::serialize(response, true);
    submit(*position, std::move(serialized), true, true);
    closed = true;
    return true;
  }
  // +=========================================================================+
  // | [>] handle_interim                                           ( public ) |
  // +-------------------------------------------------------------------------+
  // | Handles an interim response. It serializes the response and submits it  |
  // | to the transport.                                                       |
  // +-------------------------------------------------------------------------+
  template <typename RQty, typename RSty, typename ROty>
  bool handle_interim(RSty& response) {
    if (!interim_position_) {
      interim_position_ = reserve();
      if (!interim_position_) return false;
    }
    auto serialized = engine_sync<RQty, RSty, ROty>::serialize(response, false);
    submit(*interim_position_, std::move(serialized), false, false);
    return true;
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
  // | [>] reserve                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Reserves a position for a new request. If the maximum number of pending |
  // | requests has been reached, it closes the connection and returns nullopt.|
  // +-------------------------------------------------------------------------+
  std::optional<std::size_t> reserve() {
    if (!accepting()) return std::nullopt;
    if (next_request_.load() == static_cast<std::size_t>(-1)) {
      close_now();
      return std::nullopt;
    }
    if (max_pending_requests_ &&
        next_request_ - next_send_.load(std::memory_order_acquire) >=
            max_pending_requests_) {
      close_now();
      return std::nullopt;
    }
    return next_request_++;
  }
  // +=========================================================================+
  // | [>] begin                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  // | Increments the count of active asynchronous operations. This should be  |
  // | called when an asynchronous operation starts.                           |
  // +-------------------------------------------------------------------------+
  void begin() noexcept { active_.fetch_add(1, std::memory_order_release); }
  // +=========================================================================+
  // | [>] end                                                      ( public ) |
  // +-------------------------------------------------------------------------+
  // | Decrements the count of active asynchronous operations. If this was the |
  // | last active operation and the engine is stopping,                       |
  // | it calls the 'on_completed_' callback. This should be called when an    |
  // | asynchronous operation finishes.                                        |
  // +-------------------------------------------------------------------------+
  void end() {
    if (active_.fetch_sub(1, std::memory_order_acq_rel) != 1 ||
        !stopping_.load(std::memory_order_acquire)) {
      return;
    }
    std::function<void()> completed;
    {
      std::lock_guard<std::mutex> lock(completion_mutex_);
      completed = std::move(on_completed_);
    }
    if (completed) completed();
  }
  // +=========================================================================+
  // | [>] stop                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Initiates the stopping process for the engine. It sets the stopping     |
  // | flag and, if there are no active asynchronous operations,               |
  // | it calls the 'on_completed_' callback.                                  |
  // +-------------------------------------------------------------------------+
  bool stop(std::function<void()> completed) {
    std::lock_guard<std::mutex> lock(completion_mutex_);
    on_completed_ = std::move(completed);
    stopping_.store(true, std::memory_order_release);
    if (active_.load(std::memory_order_acquire) != 0) return false;
    on_completed_ = {};
    return true;
  }
  // +=========================================================================+
  // | [>] closing                                                  ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns whether the engine is in the process of closing the connection. |
  // +-------------------------------------------------------------------------+
  bool closing() const noexcept {
    return closing_.load(std::memory_order_acquire);
  }
  // +=========================================================================+
  // | [>] accepting                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns whether the engine is in the process of accepting new           |
  // | connections.                                                            |
  // +-------------------------------------------------------------------------+
  bool accepting() const noexcept {
    return accepting_.load(std::memory_order_acquire);
  }
  // +=========================================================================+
  // | [>] idle                                                     ( public ) |
  // +-------------------------------------------------------------------------+
  // | Returns whether the engine is idle, meaning it is accepting new         |
  // | connections, has no active asynchronous operations, and all pending     |
  // | requests have been sent.                                                |
  // +-------------------------------------------------------------------------+
  bool idle() const noexcept {
    return accepting() && active_.load(std::memory_order_acquire) == 0 &&
           next_request_ == next_send_.load(std::memory_order_acquire);
  }
  // +=========================================================================+
  // | [>] submit                                                   ( public ) |
  // +-------------------------------------------------------------------------+
  // | Submits a serialized response to the transport. It ensures that         |
  // | responses are sent in the order of their corresponding requests.        |
  // +-------------------------------------------------------------------------+
  void set_on_input_ready(std::function<void()> ready) { on_input_ready_ = std::move(ready); }
  bool pressured() noexcept {
    const auto sent = next_send_.load();
    const bool requests = transport::server::detail::update_pressure(
        request_pressure_, next_request_.load() - sent, max_pending_requests_);
    request_pressure_ = requests;
    const bool bytes = transport::server::detail::update_pressure(
        byte_pressure_, pending_bytes_.load(), max_pending_bytes_);
    byte_pressure_ = bytes;
    return requests || bytes;
  }
  void submit(std::size_t position, serialization_result result, bool final,
              bool close) {
    struct notify_on_exit {
      const std::function<void()>& callback;
      ~notify_on_exit() { if (callback) { try { callback(); } catch (...) {} } }
    } notify{on_input_ready_};
    if (close) accepting_.store(false, std::memory_order_release);
    if (active_.load(std::memory_order_acquire) == 0 &&
        position == next_send_.load(std::memory_order_acquire) &&
        pending_.empty()) {
      if (closing()) return;
      on_send_(result.head, result.body, std::move(result.source));
      if (final) {
        next_send_++;
        if (close) close_now();
      } else {
        pending_.try_emplace(position);
      }
      return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (closing()) return;
    if (final && position == next_send_.load(std::memory_order_relaxed) &&
        (pending_.empty() || pending_.find(position) == pending_.end())) {
      on_send_(result.head, result.body, std::move(result.source));
      next_send_++;
      if (close) {
        pending_.clear();
        pending_bytes_ = 0;
        close_now();
        return;
      }
      if (pending_.empty()) return;
    } else {
      const auto used = pending_bytes_.load();
      const auto capacity = max_pending_bytes_ ? max_pending_bytes_
                                               : static_cast<std::size_t>(-1);
      if (result.head.size() > capacity - used ||
          result.body.size() > capacity - used - result.head.size()) {
        pending_.clear();
        pending_bytes_ = 0;
        close_now();
        return;
      }
      auto& slot = pending_[position];
      slot.outputs.push_back(
          {std::string(result.head), std::string(result.body),
           std::move(result.source)});
      pending_bytes_.fetch_add(result.head.size() + result.body.size());
      slot.complete = final;
      slot.close = close;
    }
    for (;;) {
      auto current = pending_.find(next_send_.load());
      if (current == pending_.end()) break;
      while (!current->second.outputs.empty()) {
        auto output = std::move(current->second.outputs.front());
        current->second.outputs.pop_front();
        pending_bytes_.fetch_sub(output.head.size() + output.body.size());
        on_send_(output.head, output.body, std::move(output.source));
      }
      if (!current->second.complete) break;
      const bool should_close = current->second.close;
      pending_.erase(current);
      next_send_++;
      if (should_close) {
        pending_.clear();
        pending_bytes_ = 0;
        close_now();
        break;
      }
    }
  }
  // +=========================================================================+
  // | [>] close_now                                                ( public ) |
  // +-------------------------------------------------------------------------+
  // | Immediately closes the connection. It sets the accepting flag to false, |
  // | sets the closing flag to true, and calls the 'on_close_' callback.      |
  // +-------------------------------------------------------------------------+
  void close_now() {
    accepting_.store(false, std::memory_order_release);
    if (closing_.exchange(true)) return;
    on_close_();
  }

 private:
  // +=========================================================================+
  // | [>] finish                                                  ( private ) |
  // +=========================================================================+
  // | Notifies observers before releasing the active operation, even if the  |
  // | notification throws.                                                    |
  // +-------------------------------------------------------------------------+
  void finish() {
    try {
      if (on_async_finished_) on_async_finished_();
    } catch (...) {
      end();
      throw;
    }
    end();
  }
  // +=========================================================================+
  // | [>] request_position                                        ( private ) |
  // +-------------------------------------------------------------------------+
  std::optional<std::size_t> request_position() {
    if (interim_position_) {
      const std::size_t position = *interim_position_;
      interim_position_.reset();
      return position;
    }
    return reserve();
  }
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  template <typename RQty, typename RSty>
  struct async_request {
    RQty request;
    std::shared_ptr<void> parameters;
    RSty response;
    transport::server::execution_capacity execution;
    async_request(decoder<RQty, RSty>& owner, RQty value)
        : request(std::move(value)), response(owner.make_response()) {}
  };
  // +=========================================================================+
  // | [>] dispatch                                                ( private ) |
  // +-------------------------------------------------------------------------+
  // | Dispatches an asynchronous request to the appropriate handler and       |
  // | manages the response. It ensures that the response is submitted in the  |
  // | correct order.                                                          |
  // +-------------------------------------------------------------------------+
  template <typename RQty, typename RSty, typename ROty>
  void dispatch(decoder<RQty, RSty>& owner, RQty request,
                const router_match_result<RQty, RSty>& match,
                std::string_view path, std::size_t position, bool close,
                transport::server::execution_capacity execution,
                const std::shared_ptr<engine_async>& state) {
    auto entry = std::make_shared<async_request<RQty, RSty>>(
        owner, std::move(request));
    entry->execution = execution;
    try {
      entry->request.make_owned();
      path = entry->request.get_absolute_path();
      if (!engine_sync<RQty, RSty, ROty>::authenticate(
              match, entry->request, entry->response)) {
        auto serialized = engine_sync<RQty, RSty, ROty>::serialize(
            entry->request, entry->response, close);
        state->submit(position, std::move(serialized), true, close);
        return;
      }
      common::task<void> work =
          match.async_handler
              ? (*match.async_handler)(entry->request, entry->response)
              : match.async_parametrized_handler->invoke(
                    entry->request, entry->response, path, entry->parameters);
      if (on_async_started_) on_async_started_();
      state->begin();
      try {
        common::task_scheduler scheduler;
        if (entry->execution.submit) {
          scheduler = {
              entry.get(),
              [](void* owner, std::coroutine_handle<> handle) {
                const auto execution =
                    static_cast<async_request<RQty, RSty>*>(owner)->execution;
                execution.submit(
                    execution.owner,
                    std::chrono::steady_clock::time_point::min(),
                    {handle.address(), [](void* context) noexcept {
                       std::coroutine_handle<>::from_address(context).resume();
                     }});
              },
              [](void* owner, std::chrono::steady_clock::time_point due,
                 std::coroutine_handle<> handle) {
                const auto execution =
                    static_cast<async_request<RQty, RSty>*>(owner)->execution;
                execution.submit(
                    execution.owner, due,
                    {handle.address(), [](void* context) noexcept {
                       std::coroutine_handle<>::from_address(context).resume();
                     }});
              }};
        }
        work.start(
            [entry, state, position, close](std::exception_ptr error) mutable {
              try {
                if (error) {
                  close = true;
                  engine_sync<RQty, RSty, ROty>::build_error_response(
                      entry->response);
                }
                auto serialized = engine_sync<RQty, RSty, ROty>::serialize(
                    entry->request, entry->response, close);
                state->submit(position, std::move(serialized), true, close);
              } catch (...) {
                try {
                  state->close_now();
                } catch (...) {
                }
              }
              state->finish();
            },
            scheduler);
      } catch (...) {
        state->finish();
        throw;
      }
    } catch (...) {
      close = true;
      engine_sync<RQty, RSty, ROty>::build_error_response(entry->response);
      auto serialized = engine_sync<RQty, RSty, ROty>::serialize(
          entry->request, entry->response, close);
      state->submit(position, std::move(serialized), true, close);
    }
  }
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +-------------------------------------------------------------------------+
  struct output {
    std::string head;
    std::string body;
    std::unique_ptr<common::reader> source;
  };
  struct slot {
    std::deque<output> outputs;
    bool complete{false};
    bool close{false};
  };
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  protocol::send_delegate on_send_;
  const std::size_t max_pending_requests_;
  const std::size_t max_pending_bytes_;
  std::atomic<std::size_t> pending_bytes_{0};
  bool request_pressure_{false};
  bool byte_pressure_{false};
  std::function<void()> on_input_ready_;
  std::atomic<std::size_t> active_{0};
  std::function<void()> on_close_;
  std::function<void()> on_async_started_;
  std::function<void()> on_async_finished_;
  std::function<void()> on_completed_;
  std::atomic<bool> stopping_{false};
  std::atomic<bool> accepting_{true};
  std::atomic<bool> closing_{false};
  std::atomic<std::size_t> next_request_{0};
  std::atomic<std::size_t> next_send_{0};
  std::optional<std::size_t> interim_position_;
  std::mutex mutex_;
  std::mutex completion_mutex_;
  std::map<std::size_t, slot> pending_;
};
}  // namespace martianlabs::doba::protocol::http::v11::detail

#endif
