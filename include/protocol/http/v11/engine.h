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

#include <atomic>
#include <coroutine>
#include <cstddef>
#include <exception>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <stop_token>
#include <string_view>
#include <utility>

#include "common/output.h"
#include "common/task.h"
#include "protocol/deserialization.h"
#include "protocol/http/common/header_names.h"
#include "protocol/http/common/helpers.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/router.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/v11/policies.h"

namespace martianlabs::doba::protocol::http::v11 {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | HTTP/1.1 protocol engine.                                                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, typename ROty = router<RQty, RSty>>
class engine {
 private:
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  enum class response_state { kQueued, kStarting, kWaiting, kReady };
  struct pending_response {
    explicit pending_response(std::shared_ptr<RQty> input)
        : request{std::move(input)} {}
    std::shared_ptr<RQty> request;
    std::optional<RSty> response;
    std::exception_ptr error;
    std::function<void()> wake;
    std::atomic<response_state> state{response_state::kStarting};
  };
  struct completion {
    struct promise_type {
      completion get_return_object() noexcept { return {}; }
      std::suspend_never initial_suspend() const noexcept { return {}; }
      std::suspend_never final_suspend() const noexcept { return {}; }
      void return_void() noexcept {}
      void unhandled_exception() noexcept { std::terminate(); }
    };
  };

 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  using policies_type = v11::policies;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  engine(policies_type configuration, const ROty& routes)
      : decoder_{configuration}, router_{routes} {}
  ~engine() { on_stop(); }
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +=========================================================================+
  void set_on_send(common::send_delegate output) {
    on_send_ = std::move(output);
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +=========================================================================+
  void set_on_close(std::function<void()> close) {
    on_close_ = std::move(close);
  }
  // +=========================================================================+
  // | [>] set_on_wake                                              ( public ) |
  // +=========================================================================+
  void set_on_wake(std::function<void()> wake) {
    on_wake_ = std::move(wake);
  }
  // +=========================================================================+
  // | [>] on_wake                                                  ( public ) |
  // +=========================================================================+
  void on_wake() {
    if (stopped_) return;
    try {
      while (!pending_.empty()) {
        auto entry = pending_.front();
        auto state = entry->state.load(std::memory_order_acquire);
        if (state == response_state::kQueued) {
          process_request(entry->request, true);
          state = entry->state.load(std::memory_order_acquire);
        }
        if (state != response_state::kReady) return;
        if (entry->error) {
          entry->response.emplace(RSty::internal_server_error_500());
        }
        send_response(*entry->request, *entry->response);
        pending_.pop_front();
        if (stopped_) return;
      }
      serial_ = false;
    } catch (...) {
      stopped_ = true;
      on_close_();
    }
  }
  // +=========================================================================+
  // | [>] on_stop                                                  ( public ) |
  // +=========================================================================+
  void on_stop() noexcept {
    stopped_ = true;
    pending_.clear();
    if (cancellation_) cancellation_->request_stop();
  }
  // +=========================================================================+
  // | [>] on_send_completed                                        ( public ) |
  // +=========================================================================+
  void on_send_completed(bool) {}
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +=========================================================================+
  std::size_t on_bytes_received(const char* buffer, const std::size_t size,
                                const std::size_t capacity) {
    if (stopped_ || input_stopped_) return size;
    std::size_t processed = 0;
    while (processed < size) {
      std::size_t consumed = 0;
      deserialization_result<RQty> result = decoder_.deserialize(
          buffer + processed, size - processed, capacity, consumed);
      processed += consumed;
      switch (result.code) {
        case deserialization_status::kSucceeded: {
          try {
            const auto& request = result.request;
            const auto method = request->get_method();
            // RFC 9112 S9.3.2: only safe requests execute concurrently.
            if (serial_ ||
                (!pending_.empty() && method != method_names::kGet &&
                 method != method_names::kHead &&
                 method != method_names::kOptions &&
                 method != method_names::kTrace)) {
              auto entry = std::make_shared<pending_response>(request);
              entry->state.store(response_state::kQueued,
                                  std::memory_order_relaxed);
              pending_.push_back(std::move(entry));
              serial_ = true;
            } else {
              process_request(request);
              if (!pending_.empty() && method != method_names::kGet &&
                  method != method_names::kHead &&
                  method != method_names::kOptions &&
                  method != method_names::kTrace) serial_ = true;
            }
            if (!pending_.empty() && request->wants_connection_close()) {
              input_stopped_ = true;
            }
          } catch (...) {
            stopped_ = true;
            on_close_();
          }
          if (stopped_ || input_stopped_) return processed;
          break;
        }
        case deserialization_status::kInvalidSource:
          stopped_ = true;
          on_close_();
          return processed;
        case deserialization_status::kMoreBytesNeeded:
          return processed;
      }
    }
    return processed;
  }

 private:
  // +=========================================================================+
  // | [>] process_request                                         ( private ) |
  // +=========================================================================+
  void process_request(const std::shared_ptr<RQty>& input,
                       bool queued = false) {
    const RQty& request = *input;
    bool delivering = false;
    auto deliver = [&](RSty response) {
      delivering = true;
      if (!queued && pending_.empty()) {
        send_response(request, response);
      } else {
        auto entry = queued ? pending_.front()
                            : std::make_shared<pending_response>(input);
        entry->response.emplace(std::move(response));
        entry->state.store(response_state::kReady, std::memory_order_release);
        if (!queued) pending_.push_back(std::move(entry));
      }
    };
    try {
      switch (request.get_target()) {
        case target::kOriginForm:
        case target::kAbsoluteForm: {
          const std::string_view path = request.get_absolute_path();
          const auto match = router_.match(request.get_method(), path);
          if (match.handler) {
            if (match.handler->is_async()) {
              if (!cancellation_) cancellation_.emplace();
              auto response = start_response(
                  input, match.handler->async_callback(
                      input, cancellation_->get_token()), queued);
              if (response) deliver(std::move(*response));
            } else {
              deliver(match.handler->callback(request));
            }
            return;
          }
          if (match.parametrized_handler) {
            if (match.parametrized_handler->is_async()) {
              if (!cancellation_) cancellation_.emplace();
              auto response = start_response(input,
                  match.parametrized_handler->invoke_async(
                      input, cancellation_->get_token(), path), queued);
              if (response) deliver(std::move(*response));
            } else {
              deliver(match.parametrized_handler->invoke(request, path));
            }
            return;
          }
          const auto allowed = router_.allowed_methods(path);
          if (allowed.empty()) {
            deliver(RSty::not_found_404());
            return;
          }
          // RFC 9110 S15.5.6: a 405 response must advertise allowed methods.
          RSty response = RSty::method_not_allowed_405();
          response.set_header(header_names::kAllow, allowed);
          deliver(std::move(response));
          return;
        }
        case target::kAuthorityForm:
          deliver(RSty::not_implemented_501());
          return;
        case target::kAsteriskForm:
          deliver(RSty::ok_200());
          return;
        default:
          deliver(RSty::bad_request_400());
          return;
      }
    } catch (...) {
      if (delivering) throw;
      deliver(RSty::internal_server_error_500());
    }
  }
  // +=========================================================================+
  // | [>] collect_response                                        ( private ) |
  // +=========================================================================+
  static completion collect_response(
      common::task<RSty> task, std::shared_ptr<pending_response> entry) {
    try {
      entry->response.emplace(co_await std::move(task));
    } catch (...) {
      entry->error = std::current_exception();
    }
    if (entry->state.exchange(response_state::kReady,
                              std::memory_order_acq_rel) ==
        response_state::kWaiting) {
      entry->wake();
    }
  }
  // +=========================================================================+
  // | [>] start_response                                          ( private ) |
  // +=========================================================================+
  std::optional<RSty> start_response(
      const std::shared_ptr<RQty>& request, common::task<RSty> task,
      bool queued) {
    auto entry = queued ? pending_.front()
                        : std::make_shared<pending_response>(request);
    entry->wake = on_wake_;
    entry->state.store(response_state::kStarting, std::memory_order_relaxed);
    collect_response(std::move(task), entry);
    if (entry->state.load(std::memory_order_acquire) ==
        response_state::kReady) {
      if (entry->error) std::rethrow_exception(entry->error);
      return std::move(entry->response);
    }
    if (!queued) pending_.push_back(entry);
    // Publish the FIFO position before arming an external wake.
    auto expected = response_state::kStarting;
    if (!entry->state.compare_exchange_strong(
            expected, response_state::kWaiting, std::memory_order_acq_rel)) {
      if (!queued) pending_.pop_back();
      if (entry->error) std::rethrow_exception(entry->error);
      return std::move(entry->response);
    }
    return std::nullopt;
  }
  // +=========================================================================+
  // | [>] send_response                                           ( private ) |
  // +=========================================================================+
  void send_response(const RQty& request, RSty& response) {
    bool close = request.wants_connection_close();
    if (close) {
      response.set_header(header_names::kConnection, "close");
    } else if (response.has_header(header_names::kConnection)) {
      const std::size_t count = response.get_headers_length();
      for (std::size_t i = 0; i < count && !close; i++) {
        const auto header = response.get_header(i);
        if (!helpers::iequals(header.first, header_names::kConnection)) {
          continue;
        }
        helpers::for_each_list_element(
            header.second, [&close](std::string_view value) {
              helpers::ows_ltrim(value);
              helpers::ows_rtrim(value);
              if (helpers::iequals(value, "close")) close = true;
              return true;
            });
      }
    }
    // RFC 9110 S9.3.2: HEAD preserves framing but never sends body bytes.
    if (request.get_method() == method_names::kHead) response.suppress_body();
    auto serialized = response.serialize();
    on_send_(std::move(serialized->prefix), serialized->prefix_size,
             std::move(serialized->source));
    if (close && !stopped_) {
      stopped_ = true;
      on_close_();
    }
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  decoder<RQty, RSty> decoder_;
  const ROty& router_;
  common::send_delegate on_send_;
  std::function<void()> on_close_;
  std::function<void()> on_wake_;
  std::list<std::shared_ptr<pending_response>> pending_;
  std::optional<std::stop_source> cancellation_;
  bool serial_{false};
  bool input_stopped_{false};
  bool stopped_{false};
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
