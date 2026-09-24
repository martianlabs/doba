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
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <utility>

#include "common/output.h"
#include "protocol/serialization.h"
#include "protocol/deserialization.h"
#include "protocol/http/common/header_names.h"
#include "protocol/http/common/method_names.h"
#include "protocol/http/common/router.h"
#include "protocol/http/v11/decoder.h"
#include "protocol/http/v11/policies.h"

namespace martianlabs::doba::protocol::http::v11 {
namespace detail {
// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] sequencer                                                   ( class ) |
// +---------------------------------------------------------------------------+
// | Delivers serialized responses in request order.                           |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
class sequencer {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  using ticket = std::size_t;
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
  // | [>] reserve                                                  ( public ) |
  // +=========================================================================+
  ticket reserve() {
    std::lock_guard<std::mutex> lock(mutex_);
    const ticket id = next_id_++;
    if (!closed_) pending_.emplace_back();
    return id;
  }
  // +=========================================================================+
  // | [>] interim                                                  ( public ) |
  // +=========================================================================+
  void interim(ticket id,
               std::unique_ptr<protocol::serialization_result> response) {
    submit(id, std::move(response), false, true);
  }
  // +=========================================================================+
  // | [>] complete                                                 ( public ) |
  // +=========================================================================+
  void complete(ticket id,
                std::unique_ptr<protocol::serialization_result> response,
                bool close) {
    submit(id, std::move(response), close, false);
  }
  // +=========================================================================+
  // | [>] closed                                                   ( public ) |
  // +=========================================================================+
  bool closed() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return closed_;
  }
  // +=========================================================================+
  // | [>] query_for_close                                          ( public ) |
  // +=========================================================================+
  void query_for_close() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (closed_) return;
      closed_ = true;
      pending_.clear();
    }
    on_close_();
  }

 private:
  // +=========================================================================+
  // | [>] TYPEs                                                   ( private ) |
  // +=========================================================================+
  struct pending_response {
    std::unique_ptr<protocol::serialization_result> interim;
    std::unique_ptr<protocol::serialization_result> final;
    bool close{false};
  };
  // +=========================================================================+
  // | [>] submit                                                  ( private ) |
  // +=========================================================================+
  void submit(ticket id,
              std::unique_ptr<protocol::serialization_result> response,
              bool close, bool interim) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (closed_ || id < first_id_ || id - first_id_ >= pending_.size()) {
      return;
    }
    pending_response& current = pending_[id - first_id_];
    if (interim) {
      current.interim = std::move(response);
    } else {
      current.final = std::move(response);
      current.close = close;
    }
    if (draining_) return;
    draining_ = true;
    while (!closed_) {
      if (pending_.empty()) break;
      pending_response& first = pending_.front();
      std::unique_ptr<protocol::serialization_result> next;
      bool close_after_send = false;
      if (first.interim) {
        next = std::move(first.interim);
      } else if (first.final) {
        next = std::move(first.final);
        close_after_send = first.close;
        pending_.pop_front();
        first_id_++;
      } else {
        break;
      }
      lock.unlock();
      try {
        on_send_(std::move(next->prefix), next->prefix_size,
                 std::move(next->source));
      } catch (...) {
        query_for_close();
        return;
      }
      if (close_after_send) {
        query_for_close();
        return;
      }
      lock.lock();
    }
    draining_ = false;
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  mutable std::mutex mutex_;              // Mutex to protect access.
  std::deque<pending_response> pending_;  // Queue of pending responses.
  ticket first_id_{0};                    // ID of the first pending response.
  ticket next_id_{0};                     // Next ticket ID to be assigned.
  bool draining_{false};  // Flag indicating if draining is in progress.
  bool closed_{false};    // Flag indicating if the sequencer is closed.
  common::send_delegate on_send_;   //  Delegate for sending responses.
  std::function<void()> on_close_;  // Delegate for handling close events.
};
}  // namespace detail

// /////////////////////////////////////////////////////////////////////////////
// +---------------------------------------------------------------------------+
// | [>] engine                                                      ( class ) |
// +---------------------------------------------------------------------------+
// | HTTP/1.1 protocol engine.                                                 |
// +---------------------------------------------------------------------------+
// /////////////////////////////////////////////////////////////////////////////
template <typename RQty, typename RSty, typename ROty = router<RQty, RSty>>
class engine {
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +=========================================================================+
  using policies_type = v11::policies;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +=========================================================================+
  engine(policies_type configuration, const ROty& router)
      : router_{router}, decoder_{configuration} {}
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +=========================================================================+
  void set_on_send(common::send_delegate output) {
    sequencer_.set_on_send(std::move(output));
  }
  // +=========================================================================+
  // | [>] set_on_close                                             ( public ) |
  // +=========================================================================+
  void set_on_close(std::function<void()> close) {
    sequencer_.set_on_close(std::move(close));
  }
  // +=========================================================================+
  // | [>] on_bytes_received                                        ( public ) |
  // +=========================================================================+
  std::size_t on_bytes_received(const char* buffer, const std::size_t size,
                                const std::size_t capacity) {
    if (sequencer_.closed()) return size;
    std::size_t processed = 0;
    while (processed < size) {
      std::size_t consumed = 0;
      deserialization_result result = decoder_.deserialize(
          buffer + processed, size - processed, capacity, consumed);
      processed += consumed;
      try {
        switch (result.code) {
          case deserialization_status::kSucceeded: {
            if (!current_ticket_) current_ticket_ = sequencer_.reserve();
            const std::size_t id = *current_ticket_;
            current_ticket_.reset();
            const RQty& request = *result.request;
            bool close = request.wants_connection_close();
            RSty response = execute_request(request, close);
            enqueue_response(id, request, response, close);
            if (sequencer_.closed()) return processed;
            break;
          }
          case deserialization_status::kInvalidSource:
            if (result.response) {
              if (!current_ticket_) current_ticket_ = sequencer_.reserve();
              const std::size_t id = *current_ticket_;
              current_ticket_.reset();
              enqueue_response(id, *result.response, true, false);
            }
            return size;
          case deserialization_status::kMoreBytesNeeded:
            if (result.response) {
              if (!current_ticket_) current_ticket_ = sequencer_.reserve();
              enqueue_response(*current_ticket_, *result.response, false, true);
            }
            return processed;
        }
      } catch (...) {
        query_for_close();
        return result.code == deserialization_status::kInvalidSource
                   ? size
                   : processed;
      }
    }
    return processed;
  }

 private:
  // +=========================================================================+
  // | [>] execute_request                                         ( private ) |
  // +=========================================================================+
  RSty execute_request(const RQty& request, bool& close) {
    try {
      switch (request.get_target()) {
        case target::kOriginForm:
        case target::kAbsoluteForm: {
          const std::string_view path = request.get_absolute_path();
          const ROty::route_match match =
              router_.match(request.get_method(), path);
          if (match.handler) return (*match.handler)(request);
          if (match.parametrized_handler) {
            return match.parametrized_handler->invoke(request, path);
          }
          const std::string allowed = router_.allowed_methods(path);
          if (allowed.empty()) return RSty::not_found_404();
          // RFC 9110 S15.5.6: a 405 response must advertise allowed methods.
          RSty response = RSty::method_not_allowed_405();
          response.set_header(header_names::kAllow, allowed);
          return response;
        }
        case target::kAuthorityForm:
          return RSty::not_implemented_501();
        case target::kAsteriskForm:
          return RSty::ok_200();
        default:
          return RSty::bad_request_400();
      }
    } catch (...) {
      close = true;
      return build_error_response();
    }
  }
  // +=========================================================================+
  // | [>] query_for_close                                         ( private ) |
  // +=========================================================================+
  void query_for_close() { sequencer_.query_for_close(); }
  // +=========================================================================+
  // | [>] build_error_response                                    ( private ) |
  // +=========================================================================+
  static RSty build_error_response() {
    RSty response = RSty::internal_server_error_500();
    response.set_body("Internal Server Error");
    return response;
  }
  // +=========================================================================+
  // | [>] enqueue_response                                        ( private ) |
  // +=========================================================================+
  void enqueue_response(detail::sequencer::ticket id, RSty& response,
                        bool close, bool interim) {
    if (close) response.set_header(header_names::kConnection, "close");
    std::unique_ptr<protocol::serialization_result> serialized =
        response.serialize();
    if (interim) {
      sequencer_.interim(id, std::move(serialized));
    } else {
      sequencer_.complete(id, std::move(serialized), close);
    }
  }
  // +=========================================================================+
  // | [>] enqueue_response                                        ( private ) |
  // +=========================================================================+
  void enqueue_response(detail::sequencer::ticket id, const RQty& request,
                        RSty& response, bool close) {
    std::unique_ptr<protocol::serialization_result> serialized;
    try {
      if (response.is_continue_100()) {
        // A handler should never return a 100 Continue response. If it does, we
        // treat it as an error.
        close = true;
        response = build_error_response();
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
      RSty error = build_error_response();
      error.set_header(header_names::kConnection, "close");
      if (request.get_method() == method_names::kHead) error.suppress_body();
      serialized = error.serialize();
    }
    sequencer_.complete(id, std::move(serialized), close);
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +=========================================================================+
  const ROty& router_;  // Reference to the router for handling requests.
  decoder<RQty, RSty> decoder_;  // Decoder for processing incoming requests.
  detail::sequencer sequencer_;  // Manages the order of responses.
  std::optional<detail::sequencer::ticket> current_ticket_;  // ticketing.
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
