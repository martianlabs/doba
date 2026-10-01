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

#include "common/output.h"
#include "protocol/serialization.h"
#include "protocol/deserialization.h"
#include "protocol/http/common/header_names.h"
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
 public:
  // +=========================================================================+
  // | [>] TYPEs                                                    ( public ) |
  // +-------------------------------------------------------------------------+
  using policies_type = v11::policies;
  // +=========================================================================+
  // | [>] CONSTRUCTORs/DESTRUCTORs                                 ( public ) |
  // +-------------------------------------------------------------------------+
  engine(policies_type configuration, const ROty& router)
      : router_{router}, decoder_{configuration} {}
  // +=========================================================================+
  // | [>] set_on_send                                              ( public ) |
  // +-------------------------------------------------------------------------+
  void set_on_send(common::send_delegate output) {
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
  std::size_t on_bytes_received(const char* buffer, const std::size_t size,
                                const std::size_t capacity) {
    if (closed_) return size;
    if (!size) return 0;
    std::size_t total = 0;
    while (total < size && !closed_) {
      std::size_t consumed = 0;
      try {
        deserialization_result result = decoder_.deserialize(
            buffer + total, size - total, capacity, consumed,
            [this](const RQty& request) {
              bool close = request.wants_connection_close();
              RSty response(decoder_.response_storage());
              execute_request(request, response, close);
              enqueue_response(request, response, close);
            });
        total += consumed;
        switch (result.code) {
          case deserialization_status::kSucceeded:
            break;
          case deserialization_status::kInvalidSource:
            if (result.response) enqueue_response(*result.response, true);
            return size;
          case deserialization_status::kMoreBytesNeeded:
            if (result.response) enqueue_response(*result.response, false);
            return total;
        }
      } catch (...) {
        query_for_close();
        return total + consumed;
      }
    }
    return total;
  }

 private:
  // +=========================================================================+
  // | [>] execute_request                                         ( private ) |
  // +-------------------------------------------------------------------------+
  void execute_request(const RQty& request, RSty& response, bool& close) {
    try {
      switch (request.get_target()) {
        case target::kOriginForm:
        case target::kAbsoluteForm: {
          const std::string_view path = request.get_absolute_path();
          const typename ROty::route_match match =
              router_.match(request.get_method(), path);
          if (match.handler) {
            (*match.handler)(request, response);
            return;
          }
          if (match.parametrized_handler) {
            match.parametrized_handler->invoke(request, response, path);
            return;
          }
          const std::string allowed = router_.allowed_methods(path);
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
  // | [>] query_for_close                                         ( private ) |
  // +-------------------------------------------------------------------------+
  void query_for_close() {
    if (closed_) return;
    closed_ = true;
    on_close_();
  }
  // +=========================================================================+
  // | [>] build_error_response                                    ( private ) |
  // +-------------------------------------------------------------------------+
  static void build_error_response(RSty& response) {
    response.internal_server_error_500();
    response.set_body("Internal Server Error");
  }
  // +=========================================================================+
  // | [>] enqueue_response                                        ( private ) |
  // +-------------------------------------------------------------------------+
  void enqueue_response(RSty& response, bool close) {
    if (close) response.set_header(header_names::kConnection, "close");
    auto serialized = response.serialize();
    on_send_(serialized.head, serialized.body, std::move(serialized.source));
    if (close) query_for_close();
  }
  // +=========================================================================+
  // | [>] enqueue_response                                        ( private ) |
  // +-------------------------------------------------------------------------+
  void enqueue_response(const RQty& request, RSty& response, bool close) {
    protocol::serialization_result serialized;
    try {
      if (response.is_continue_100()) {
        // A handler should never set a 100 Continue response. If it does, we
        // treat it as an error.
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
    on_send_(serialized.head, serialized.body, std::move(serialized.source));
    if (close) query_for_close();
  }
  // +=========================================================================+
  // | [>] ATTRIBUTEs                                              ( private ) |
  // +-------------------------------------------------------------------------+
  const ROty& router_;  // Reference to the router for handling requests.
  decoder<RQty, RSty> decoder_;  // Decoder for processing incoming requests.
  common::send_delegate on_send_;
  std::function<void()> on_close_;
  bool closed_{false};
};
}  // namespace martianlabs::doba::protocol::http::v11

#endif
